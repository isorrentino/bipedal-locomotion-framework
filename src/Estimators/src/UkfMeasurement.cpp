/**
 * @file UkfMeasurement.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <map>

#include <BipedalLocomotion/Math/Constants.h>
#include <BipedalLocomotion/Math/Wrench.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

#include <BipedalLocomotion/RobotDynamicsEstimator/AccelerometerMeasurementDynamics.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/ConstantMeasurementModel.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/GyroscopeMeasurementDynamics.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/KinDynWrapper.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/MotorCurrentMeasurementDynamics.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/SubModel.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/UkfMeasurement.h>

using namespace BipedalLocomotion;
using namespace BipedalLocomotion::Estimators::RobotDynamicsEstimator;

using namespace std::chrono;

struct UkfMeasurement::Impl : public UkfModel {
    bfl::VectorDescription measurementDescription;
    bfl::VectorDescription inputDescription;
    Eigen::MatrixXd covarianceR; /**< Covariance matrix. */
    int measurementSize{0}; /**< Length of the measurement vector. */
    System::VariablesHandler measurementVariableHandler; /**< Variable handler describing the
                                                            measurement vector. */
    Eigen::VectorXd tempPredictedMeas;
    Eigen::MatrixXd predictedMeasurement; /**< Vector containing the updated measurement. */
};

UkfMeasurement::UkfMeasurement()
{
    m_pimpl = std::make_unique<UkfMeasurement::Impl>();
}

UkfMeasurement::~UkfMeasurement() = default;

bool UkfMeasurement::initialize(std::weak_ptr<const ParametersHandler::IParametersHandler> handler)
{
    constexpr auto logPrefix = "[UkfMeasurement::initialize]";

    auto ptr = handler.lock();
    if (ptr == nullptr)
    {
        BipedalLocomotion::log()->error("{} The parameter handler is not valid.", logPrefix);
        return false;
    }

    if (!ptr->getParameter("sampling_time", m_pimpl->m_dT))
    {
        BipedalLocomotion::log()->error("{} Unable to find the `sampling_time` variable",
                                        logPrefix);
        return false;
    }

    m_pimpl->m_isInitialized = true;

    return true;
}

bool UkfMeasurement::finalize(const System::VariablesHandler& handler)
{
    constexpr auto logPrefix = "[UkfMeasurement::finalize]";

    if (!m_pimpl->m_isInitialized)
    {
        BipedalLocomotion::log()->error("{} Please call initialize() before finalize().",
                                        logPrefix);
        return false;
    }

    m_pimpl->m_isFinalized = false;

    m_pimpl->measurementSize = 0;

    // finalize all the dynamics
    for (int indexDyn1 = 0; indexDyn1 < m_pimpl->m_dynamicsList.size(); indexDyn1++)
    {
        if (!m_pimpl->m_dynamicsList[indexDyn1].second->finalize(handler))
        {
            BipedalLocomotion::log()->error("{} Error while finalizing the dynamics named {}",
                                            logPrefix,
                                            m_pimpl->m_dynamicsList[indexDyn1].first);
            return false;
        }

        m_pimpl->measurementSize += m_pimpl->m_dynamicsList[indexDyn1].second->size();
    }

    // Set value of measurement covariance matrix
    m_pimpl->covarianceR.resize(m_pimpl->measurementSize, m_pimpl->measurementSize);
    m_pimpl->covarianceR.setZero();

    for (int indexDyn2 = 0; indexDyn2 < m_pimpl->m_dynamicsList.size(); indexDyn2++)
    {
        m_pimpl->measurementVariableHandler
            .addVariable(m_pimpl->m_dynamicsList[indexDyn2].first,
                         m_pimpl->m_dynamicsList[indexDyn2].second->getCovariance().size());

        m_pimpl->covarianceR.block(m_pimpl->measurementVariableHandler
                                       .getVariable(m_pimpl->m_dynamicsList[indexDyn2].first)
                                       .offset,
                                   m_pimpl->measurementVariableHandler
                                       .getVariable(m_pimpl->m_dynamicsList[indexDyn2].first)
                                       .offset,
                                   m_pimpl->measurementVariableHandler
                                       .getVariable(m_pimpl->m_dynamicsList[indexDyn2].first)
                                       .size,
                                   m_pimpl->measurementVariableHandler
                                       .getVariable(m_pimpl->m_dynamicsList[indexDyn2].first)
                                       .size)
            = m_pimpl->m_dynamicsList[indexDyn2].second->getCovariance().asDiagonal();
    }

    m_pimpl->m_jointVelocityState.resize(m_pimpl->m_kinDynFullModel->model().getNrOfDOFs());
    m_pimpl->m_jointAccelerationState.resize(m_pimpl->m_kinDynFullModel->model().getNrOfDOFs());

    for (int idx = 0; idx < m_pimpl->m_subModelList.size(); idx++)
    {
        m_pimpl->m_subModelJointVel.emplace_back(
            Eigen::VectorXd(m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_subModelJointAcc.emplace_back(
            Eigen::VectorXd(m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_subModelJointPos.emplace_back(
            Eigen::VectorXd(m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_subModelJointMotorTorque.emplace_back(
            Eigen::VectorXd(m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_subModelFrictionTorque.emplace_back(
            Eigen::VectorXd(m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_totalTorqueFromContacts.emplace_back(
            Eigen::VectorXd(6 + m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_tempJacobianList.emplace_back(
            Eigen::MatrixXd(6, 6 + m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
        m_pimpl->m_subModelNuDot.emplace_back(
            Eigen::VectorXd(6 + m_pimpl->m_subModelList[idx].getModel().getNrOfDOFs()));
    }

    m_pimpl->m_currentState.resize(handler.getNumberOfVariables());
    m_pimpl->m_currentState.setZero();

    m_pimpl->m_measurement.resize(m_pimpl->measurementSize);
    m_pimpl->m_measurement.setZero();

    m_pimpl->tempPredictedMeas.resize(m_pimpl->measurementSize);
    m_pimpl->tempPredictedMeas.setZero();

    m_pimpl->measurementDescription = bfl::VectorDescription(m_pimpl->measurementSize);

    m_pimpl->predictedMeasurement.resize(m_pimpl->measurementSize, 2 * handler.getNumberOfVariables() + 1);
    m_pimpl->predictedMeasurement.setZero();

    m_pimpl->m_isFinalized = true;

    return true;
}

std::unique_ptr<UkfMeasurement>
UkfMeasurement::build(std::weak_ptr<const ParametersHandler::IParametersHandler> handler,
                      System::VariablesHandler& stateVariableHandler,
                      std::shared_ptr<iDynTree::KinDynComputations> kinDynFullModel,
                      const std::vector<SubModel>& subModelList,
                      const std::vector<std::shared_ptr<KinDynWrapper>>& kinDynWrapperList)
{
    constexpr auto logPrefix = "[UkfMeasurement::build]";

    auto ptr = handler.lock();
    if (ptr == nullptr)
    {
        BipedalLocomotion::log()->error("{} Invalid parameter handler.", logPrefix);
        return nullptr;
    }

    std::unique_ptr<UkfMeasurement> measurement = std::make_unique<UkfMeasurement>();

    if (!measurement->initialize(ptr))
    {
        BipedalLocomotion::log()->error("{} Unable to initialize the measurement object of the "
                                        "ukf.",
                                        logPrefix);
        return nullptr;
    }

    std::vector<std::string> dynamicsList;
    if (!ptr->getParameter("dynamics_list", dynamicsList))
    {
        BipedalLocomotion::log()->error("{} Unable to find the parameter 'dynamics_list'.",
                                        logPrefix);
        return nullptr;
    }

    if (kinDynFullModel == nullptr)
    {
        BipedalLocomotion::log()->error("{} The pointer to the `KinDynComputation` object is not "
                                        "valid.",
                                        logPrefix);
        return nullptr;
    }

    measurement->m_pimpl->m_stateVariableHandler = stateVariableHandler;

    // Set KinDynComputation for the full model
    measurement->m_pimpl->m_kinDynFullModel = kinDynFullModel;

    // Set the list of SubModel
    measurement->m_pimpl->m_subModelList.reserve(subModelList.size());
    measurement->m_pimpl->m_subModelList = subModelList;

    // set the list of KinDynWrapper
    measurement->m_pimpl->m_kinDynWrapperList.reserve(kinDynWrapperList.size());
    measurement->m_pimpl->m_kinDynWrapperList = kinDynWrapperList;

    // per each dynamics add variable to the variableHandler
    // and add the dynamics to the list
    for (const auto& dynamicsGroupName : dynamicsList)
    {
        auto dynamicsGroupTmp = ptr->getGroup(dynamicsGroupName).lock();
        if (dynamicsGroupTmp == nullptr)
        {
            BipedalLocomotion::log()->error("{} Unable to find the group '{}'.",
                                            logPrefix,
                                            dynamicsGroupName);
            return nullptr;
        }

        // add dT parameter since it is required by all the dynamics
        auto dynamicsGroup = dynamicsGroupTmp->clone();
        dynamicsGroup->setParameter("sampling_time", measurement->m_pimpl->m_dT);

        // create variable handler
        std::string dynamicsName;
        std::vector<double> covariances;
        if (!dynamicsGroup->getParameter("name", dynamicsName))
        {
            BipedalLocomotion::log()->error("{} Unable to find the parameter 'name'.", logPrefix);
            return nullptr;
        }
        if (!dynamicsGroup->getParameter("covariance", covariances))
        {
            BipedalLocomotion::log()->error("{} Unable to find the parameter 'covariance'.",
                                            logPrefix);
            return nullptr;
        }

        std::string dynamicModel;
        if (!dynamicsGroup->getParameter("dynamic_model", dynamicModel))
        {
            BipedalLocomotion::log()->error("{} Unable to find the parameter 'dynamic_model'.",
                                            logPrefix);
            return nullptr;
        }

        std::shared_ptr<Dynamics> dynamicsInstance = DynamicsFactory::createInstance(dynamicModel);
        if (dynamicsInstance == nullptr)
        {
            BipedalLocomotion::log()->error("{} The dynamic model '{}' has not been registered for "
                                            "the measurement variable `{}`.",
                                            logPrefix,
                                            dynamicModel,
                                            dynamicsName);
            return nullptr;
        }

        dynamicsInstance->setSubModels(subModelList, kinDynWrapperList);

        dynamicsInstance->initialize(dynamicsGroup);

        // add dynamics to the list
        //        measurement->m_pimpl->dynamicsList.insert({dynamicsName, dynamicsInstance});
        measurement->m_pimpl->m_dynamicsList.emplace_back(dynamicsName, dynamicsInstance);
    }

    measurement->m_pimpl->inputDescription
        = bfl::VectorDescription(stateVariableHandler.getNumberOfVariables());

    // finalize estimator
    if (!measurement->finalize(measurement->m_pimpl->m_stateVariableHandler))
    {
        BipedalLocomotion::log()->error("{} Unable to finalize the RobotDynamicsEstimator.",
                                        logPrefix);
        return nullptr;
    }

    return measurement;
}

void UkfMeasurement::setUkfInputProvider(std::shared_ptr<const UkfInputProvider> ukfInputProvider)
{
    m_pimpl->m_ukfInputProvider = ukfInputProvider;
}

std::pair<bool, Eigen::MatrixXd> UkfMeasurement::getNoiseCovarianceMatrix() const
{
    return std::make_pair(true, m_pimpl->covarianceR);
}

bfl::VectorDescription UkfMeasurement::getMeasurementDescription() const
{
    return m_pimpl->measurementDescription;
}

bfl::VectorDescription UkfMeasurement::getInputDescription() const
{
    return m_pimpl->inputDescription;
}

// TODO
// Here the cur_state has size state_size x n_sigma_points
// this means that the computation can be parallelized
std::pair<bool, bfl::Data>
UkfMeasurement::predictedMeasure(const Eigen::Ref<const Eigen::MatrixXd>& cur_states) const
{
    constexpr auto logPrefix = "[UkfMeasurement::propagate]";

    // Check that everything is initialized and set
    if (!m_pimpl->m_isFinalized)
    {
        BipedalLocomotion::log()->error("{} The ukf state is not well initialized.", logPrefix);
        throw std::runtime_error("Error");
    }

    if (m_pimpl->m_ukfInputProvider == nullptr)
    {
        BipedalLocomotion::log()->error("{} The ukf input provider is not set.", logPrefix);
        throw std::runtime_error("Error");
    }

    // Get input of ukf from provider
    m_pimpl->m_ukfInput =m_pimpl-> m_ukfInputProvider->getOutput();

    for (int sample = 0; sample < cur_states.cols(); sample++)
    {
        m_pimpl->m_currentState = cur_states.col(sample);

        m_pimpl->unpackState();

        if (!m_pimpl->updateState())
        {
            BipedalLocomotion::log()->error("{} The joint accelerations are not updated.",
                                            logPrefix);
            throw std::runtime_error("Error");
        }

        m_pimpl->m_ukfInput.robotJointAccelerations = m_pimpl->m_jointAccelerationState;

        //        BipedalLocomotion::log()->info("Joint acceleration\n{}",
        //        m_pimpl->ukfInput.robotJointAccelerations);

        // TODO
        // This could be parallelized
        for (int indexDyn = 0; indexDyn < m_pimpl->m_dynamicsList.size(); indexDyn++)
        {
            m_pimpl->m_dynamicsList[indexDyn].second->setState(m_pimpl->m_currentState);

            m_pimpl->m_dynamicsList[indexDyn].second->setInput(m_pimpl->m_ukfInput);

            if (!m_pimpl->m_dynamicsList[indexDyn].second->update())
            {
                BipedalLocomotion::log()->error("{} Cannot update the dynamics with name `{}`.",
                                                logPrefix,
                                                m_pimpl->m_dynamicsList[indexDyn].first);
                throw std::runtime_error("Error");
            }

            m_pimpl->tempPredictedMeas
                .segment(m_pimpl->measurementVariableHandler
                             .getVariable(m_pimpl->m_dynamicsList[indexDyn].first)
                             .offset,
                         m_pimpl->measurementVariableHandler
                             .getVariable(m_pimpl->m_dynamicsList[indexDyn].first)
                             .size)
                = m_pimpl->m_dynamicsList[indexDyn].second->getUpdatedVariable();
        }
        m_pimpl->predictedMeasurement.col(sample) = m_pimpl->tempPredictedMeas;
    }

    //    BipedalLocomotion::log()->info("Predicted measurement sigma points");
    //    BipedalLocomotion::log()->info(m_pimpl->predictedMeasurement.transpose());

    return std::make_pair(true, m_pimpl->predictedMeasurement);
}

std::pair<bool, bfl::Data> UkfMeasurement::innovation(const bfl::Data& predicted_measurements,
                                                      const bfl::Data& measurements) const
{
    Eigen::MatrixXd innovation
        = -(bfl::any::any_cast<Eigen::MatrixXd>(predicted_measurements).colwise()
            - bfl::any::any_cast<Eigen::VectorXd>(measurements));

    return std::make_pair(true, std::move(innovation));
}

std::size_t UkfMeasurement::getMeasurementSize()
{
    return m_pimpl->measurementSize;
}

System::VariablesHandler& UkfMeasurement::getMeasurementVariableHandler()
{
    return m_pimpl->measurementVariableHandler;
}

void UkfMeasurement::setStateVariableHandler(System::VariablesHandler stateVariableHandler)
{
    m_pimpl->m_stateVariableHandler = stateVariableHandler;
}

std::pair<bool, bfl::Data> UkfMeasurement::measure(const bfl::Data& /**data**/) const
{
    return std::make_pair(true, m_pimpl->m_measurement);
}

bool UkfMeasurement::freeze(const bfl::Data& data)
{
    constexpr auto logPrefix = "[UkfMeasurement::freeze]";

    m_pimpl->m_measurementMap = bfl::any::any_cast<std::map<std::string, Eigen::VectorXd>>(data);

    for (int index = 0; index < m_pimpl->m_dynamicsList.size(); index++)
    {
        m_pimpl->m_offsetMeasurement
            = m_pimpl->measurementVariableHandler.getVariable(m_pimpl->m_dynamicsList[index].first)
                  .offset;

        if (m_pimpl->m_measurementMap.count(m_pimpl->m_dynamicsList[index].first) == 0)
        {
            BipedalLocomotion::log()->error("{} Measurement with name `{}` not found.",
                                            logPrefix,
                                            m_pimpl->m_dynamicsList[index].first);
            return false;
        }

        // If more sub-models share the same accelerometer or gyroscope sensor, the measurement
        // vector is concatenated a number of times equal to the number of sub-models using the
        // sensor.
        while (
            m_pimpl->m_offsetMeasurement
            < (m_pimpl->measurementVariableHandler.getVariable(m_pimpl->m_dynamicsList[index].first)
                   .offset
               + m_pimpl->measurementVariableHandler.getVariable(m_pimpl->m_dynamicsList[index].first)
                     .size))
        {
            m_pimpl->m_measurement
                .segment(m_pimpl->m_offsetMeasurement,
                         m_pimpl->m_measurementMap[m_pimpl->m_dynamicsList[index].first].size())
                = m_pimpl->m_measurementMap[m_pimpl->m_dynamicsList[index].first];

            m_pimpl->m_offsetMeasurement
                += m_pimpl->m_measurementMap[m_pimpl->m_dynamicsList[index].first].size();
        }
    }

    return true;
}
