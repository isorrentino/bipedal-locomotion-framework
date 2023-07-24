/**
 * @file FrictionTorqueMeasurementModel.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <Eigen/Dense>

#include <BipedalLocomotion/Math/Constants.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/FrictionTorqueMeasurementModel.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

namespace RDE = BipedalLocomotion::Estimators::RobotDynamicsEstimator;

RDE::FrictionTorqueMeasurementModel::FrictionTorqueMeasurementModel() = default;

RDE::FrictionTorqueMeasurementModel::~FrictionTorqueMeasurementModel() = default;

bool RDE::FrictionTorqueMeasurementModel::initialize(
    std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler)
{
    constexpr auto errorPrefix = "[FrictionTorqueMeasurementModel::initialize]";

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

    // Set the friction parameters
    if (!ptr->getParameter("friction_k0", m_k0))
    {
        log()->error("{} Error while retrieving the friction_k0 variable.", errorPrefix);
        return false;
    }

    if (!ptr->getParameter("friction_k1", m_k1))
    {
        log()->error("{} Error while retrieving the friction_k1 variable.", errorPrefix);
        return false;
    }

    if (!ptr->getParameter("friction_k2", m_k2))
    {
        log()->error("{} Error while retrieving the friction_k2 variable.", errorPrefix);
        return false;
    }

    m_description = "Friction torque measurement model";

    m_isInitialized = true;

    return true;
}

bool RDE::FrictionTorqueMeasurementModel::finalize(
    const System::VariablesHandler& stateVariableHandler)
{
    constexpr auto errorPrefix = "[FrictionTorqueMeasurementModel::finalize]";

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

    m_jointVelocityFullModel.resize(m_stateVariableHandler.getVariable("ds").size);
    m_jointVelocityFullModel.setZero();

    m_updatedVariable.resize(m_size);
    m_updatedVariable.setZero();

    m_tempVar.resize(m_size);
    m_tempVar.setZero();

    return true;
}

bool RDE::FrictionTorqueMeasurementModel::setSubModels(
    const std::vector<SubModel>& subModelList,
    const std::vector<std::shared_ptr<KinDynWrapper>>& kinDynWrapperList)
{
    return true;
}

bool RDE::FrictionTorqueMeasurementModel::checkStateVariableHandler()
{
    constexpr auto errorPrefix = "[FrictionTorqueMeasurementModel::checkStateVariableHandler]";

    if (!m_stateVariableHandler.getVariable("ds").isValid())
    {
        log()->error("{} The variable handler does not contain the expected state with name `ds`.",
                     errorPrefix);
        return false;
    }

    return true;
}

bool RDE::FrictionTorqueMeasurementModel::update()
{
    m_tempVar.noalias() = m_k1.asDiagonal() * m_jointVelocityFullModel;
    m_updatedVariable = m_k0.array() * m_tempVar.array().tanh();

    m_updatedVariable.noalias() += m_k2.asDiagonal() * m_jointVelocityFullModel;

    return true;
}

void RDE::FrictionTorqueMeasurementModel::setState(const Eigen::Ref<const Eigen::VectorXd> ukfState)
{
    m_jointVelocityFullModel = ukfState.segment(m_stateVariableHandler.getVariable("ds").offset,
                                                m_stateVariableHandler.getVariable("ds").size);
}

void RDE::FrictionTorqueMeasurementModel::setInput(const UKFInput& ukfInput)
{
    m_ukfInput = ukfInput;
}
