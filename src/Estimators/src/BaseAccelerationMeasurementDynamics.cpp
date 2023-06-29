/**
 * @file BaseAccelerationMeasurementDynamics.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <Eigen/Dense>

#include <BipedalLocomotion/Math/Constants.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/BaseAccelerationMeasurementDynamics.h>
#include <BipedalLocomotion/TextLogging/Logger.h>
#include <BipedalLocomotion/Conversions/ManifConversions.h>

namespace RDE = BipedalLocomotion::Estimators::RobotDynamicsEstimator;

RDE::BaseAccelerationMeasurementDynamics::BaseAccelerationMeasurementDynamics() = default;

RDE::BaseAccelerationMeasurementDynamics::~BaseAccelerationMeasurementDynamics() = default;

bool RDE::BaseAccelerationMeasurementDynamics::initialize(
    std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler)
{
    constexpr auto errorPrefix = "[BaseAccelerationMeasurementDynamics::initialize]";

    auto ptr = paramHandler.lock();
    if (ptr == nullptr)
    {
        log()->error("{} The parameter handler is not valid.", errorPrefix);
        return false;
    }

    // Set the state dynamics name
    if (!ptr->getParameter("name", m_name))
    {
        log()->error("{} Error while retrieving the `name` variable.", errorPrefix);
        return false;
    }

    // Set the frame associated to this dynamics
    if (!ptr->getParameter("frame", m_frame))
    {
        log()->error("{} Error while retrieving the `frame` variable.", errorPrefix);
        return false;
    }

    // Set the state process covariance
    if (!ptr->getParameter("covariance", m_covariances))
    {
        log()->error("{} Error while retrieving the `covariance` variable.", errorPrefix);
        return false;
    }

    m_description = "Base acceleration measurement dynamics";

    m_isInitialized = true;

    return true;
}

bool RDE::BaseAccelerationMeasurementDynamics::finalize(
    const System::VariablesHandler& stateVariableHandler)
{
    constexpr auto errorPrefix = "[BaseAccelerationMeasurementDynamics::finalize]";

    if (!m_isInitialized)
    {
        log()->error("{} Please initialize the dynamics before calling finalize.", errorPrefix);
        return false;
    }

    if (!m_isSubModelListSet)
    {
        log()->error("{} Please call `setSubModels` before finalizing.", errorPrefix);
        return false;
    }

    if (stateVariableHandler.getNumberOfVariables() == 0)
    {
        log()->error("{} The state variable handler is empty.", errorPrefix);
        return false;
    }

    m_stateVariableHandler = stateVariableHandler;

    if (!checkStateVariableHandler())
    {
        log()->error("{} The state variable handler is not valid.", errorPrefix);
        return false;
    }

    // Search and save all the submodels containing the sensor
    m_subModelIndex = 0;
    while(m_kinDynWrapperList[m_subModelIndex]->getFloatingBase() != m_name)
    {
        m_subModelIndex++;
    }

    m_size = m_covariances.size();

    m_subModelJointAcc.resize(m_subModelList.size());

    for (int idx = 0; idx < m_subModelList.size(); idx++)
    {
        m_subModelJointAcc[idx].resize(m_subModelList[idx].getJointMapping().size());
        m_subModelJointAcc[idx].setZero();
    }

    m_updatedVariable.resize(m_size);
    m_updatedVariable.setZero();

    return true;
}

bool RDE::BaseAccelerationMeasurementDynamics::setSubModels(
    const std::vector<SubModel>& subModelList,
    const std::vector<std::shared_ptr<KinDynWrapper>>& kinDynWrapperList)
{
    m_subModelList = subModelList;
    m_kinDynWrapperList = kinDynWrapperList;
    m_isSubModelListSet = true;

    return true;
}

bool RDE::BaseAccelerationMeasurementDynamics::checkStateVariableHandler()
{
    return true;
}

bool RDE::BaseAccelerationMeasurementDynamics::update()
{
    constexpr auto errorPrefix = "[BaseAccelerationMeasurementDynamics::update]";

    // compute joint acceleration per each sub-model containing the accelerometer
    for (int subDynamicsIdx = 0; subDynamicsIdx < m_subModelList.size(); subDynamicsIdx++)
    {
        if (m_subModelList[subDynamicsIdx].getModel().getNrOfDOFs() > 0)
        {
            for (int jointIdx = 0; jointIdx < m_subModelList[subDynamicsIdx].getJointMapping().size(); jointIdx++)
            {
                m_subModelJointAcc[subDynamicsIdx][jointIdx] = m_ukfInput.robotJointAccelerations[m_subModelList[subDynamicsIdx].getJointMapping()[jointIdx]];
            }
        }
    }

    m_subModelBaseAcceleration = m_kinDynWrapperList[m_subModelIndex]->getNuDot().head(6);

    if (!m_kinDynWrapperList[m_subModelIndex]->getFrameAcc(m_name,
                                                 m_subModelBaseAcceleration,
                                                 m_subModelJointAcc[m_subModelIndex],
                                                 iDynTree::make_span(
                                                     m_frameAcceleration)))
    {
        log()->error("{} Failed while getting the accelerometer frame acceleration.",
                     errorPrefix);
        return false;
    }

    m_frameVelocity = Conversions::toManifTwist(m_kinDynWrapperList[m_subModelIndex]->getFrameVel(m_name));

    m_vCrossW = m_frameVelocity.lin().cross(m_frameVelocity.ang());

    m_updatedVariable = m_frameAcceleration.coeffs();
    m_updatedVariable.head(3) -= m_vCrossW;

    return true;
}

void RDE::BaseAccelerationMeasurementDynamics::setState(
    const Eigen::Ref<const Eigen::VectorXd> /**ukfState**/)
{
    return ;
}

void RDE::BaseAccelerationMeasurementDynamics::setInput(const UKFInput& ukfInput)
{
    m_ukfInput = ukfInput;
}
