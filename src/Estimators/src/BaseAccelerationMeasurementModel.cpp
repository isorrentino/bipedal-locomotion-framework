/**
 * @file BaseAccelerationMeasurementModel.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <BipedalLocomotion/TextLogging/Logger.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/BaseAccelerationMeasurementModel.h>

namespace RDE = BipedalLocomotion::Estimators::RobotDynamicsEstimator;

bool RDE::BaseAccelerationMeasurementModel::initialize(std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler)
{
    constexpr auto errorPrefix = "[BaseAccelerationMeasurementModel::initialize]";

    auto ptr = paramHandler.lock();
    if (ptr == nullptr)
    {
        log()->error("{} The parameter handler is not valid.", errorPrefix);
        return false;
    }

    // Set the state dynamics name
    if (!ptr->getParameter("name", m_name))
    {
        log()->error("{} Error while retrieving the name variable.", errorPrefix);
        return false;
    }

    // Set the state process covariance
    if (!ptr->getParameter("covariance", m_covariances))
    {
        log()->error("{} Error while retrieving the covariance variable.", errorPrefix);
        return false;
    }

    // Set the list of elements if it exists
    if (!ptr->getParameter("elements", m_elements))
    {
        log()->debug("{} Variable elements not found.", errorPrefix);
    }

    m_description = "Base acceleration dynamics";

    m_isInitialized = true;

    return true;
}

bool RDE::BaseAccelerationMeasurementModel::finalize(const System::VariablesHandler &stateVariableHandler)
{
    constexpr auto errorPrefix = "[BaseAccelerationMeasurementModel::finalize]";

    if (!m_isInitialized)
    {
        log()->error("{} Please initialize the dynamics before calling finalize.", errorPrefix);
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

    m_size = m_covariances.size();

    m_currentState.resize(m_size);
    m_currentState.setZero();

    m_updatedVariable.resize(m_size);
    m_updatedVariable.setZero();

    return true;
}

bool RDE::BaseAccelerationMeasurementModel::setSubModels(const std::vector<RDE::SubModel>& subModelList,
                                                         const std::vector<std::shared_ptr<RDE::SubModelKinDynWrapper>>& kinDynWrapperList)
{
    m_subModelList = subModelList;
    m_kinDynWrapperList = kinDynWrapperList;
    m_isSubModelListSet = true;

    return true;
}

bool RDE::BaseAccelerationMeasurementModel::checkStateVariableHandler()
{
    return true;
}

bool RDE::BaseAccelerationMeasurementModel::update()
{
    if (m_isSubModelListSet)
    {
//        log()->info("Base acceleration");
//        log()->info(m_kinDynWrapperList[0]->getBaseAcceleration().coeffs());

        m_updatedVariable = m_kinDynWrapperList[0]->getBaseAcceleration().coeffs();
    }

    return true;
}

void RDE::BaseAccelerationMeasurementModel::setState(const Eigen::Ref<const Eigen::VectorXd> /*ukfState*/)
{
    return;
}

void RDE::BaseAccelerationMeasurementModel::setInput(const UKFInput& /*ukfInput*/)
{
    return;
}
