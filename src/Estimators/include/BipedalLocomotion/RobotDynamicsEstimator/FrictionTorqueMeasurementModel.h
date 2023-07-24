/**
 * @file FrictionTorqueMeasurementModel.h
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef BIPEDAL_LOCOMOTION_ESTIMATORS_FRICTION_TORQUE_MEASUREMENT_MODEL_H
#define BIPEDAL_LOCOMOTION_ESTIMATORS_FRICTION_TORQUE_MEASUREMENT_MODEL_H

#include <memory>
#include <BipedalLocomotion/RobotDynamicsEstimator/Dynamics.h>

namespace BipedalLocomotion
{
namespace Estimators
{
namespace RobotDynamicsEstimator
{

/**
 * The FrictionTorqueMeasurementModel class is a concrete implementation of the Dynamics.
 * Please use this element if you want to use a friction model as friction torque measurement.
 * The FrictionTorqueMeasurementModel represents the following equation in the continuous time:
 * \f[
 * \tau_{F} = k0 tanh( k1 \dot{s}) + k2 \dot{s}
 * \f]
 * where the joint velocity is estimated by the ukf.
 */
class FrictionTorqueMeasurementModel : public Dynamics
{
    Eigen::VectorXd m_jointVelocityFullModel; /**< Vector of joint velocities. */
    Eigen::VectorXd m_k0, m_k1, m_k2; /**< Friction parameters (see class description). */
    double m_dT; /**< Sampling time. */
    UKFInput m_ukfInput; /**< Input of the UKF used to update the dynamics. */
    std::string m_name; /**< Name of dynamics. */
    std::vector<std::string> m_elements = {}; /**< Elements composing the variable vector. */
    System::VariablesHandler m_stateVariableHandler; /**< Variable handler describing the variables
                                                        and the sizes in the ukf state vector. */
    Eigen::VectorXd m_coshArgument;
    Eigen::VectorXd m_tempVar;

public:
    /*
     * Constructor
     */
    FrictionTorqueMeasurementModel();

    /*
     * Destructor
     */
    virtual ~FrictionTorqueMeasurementModel();

    /**
     * Initialize the state dynamics.
     * @param paramHandler pointer to the parameters handler.
     * @note the following parameters are required by the class
     * |           Parameter Name           |   Type   |                                       Description                                                       | Mandatory |
     * |:----------------------------------:|:--------:|:-------------------------------------------------------------------------------------------------------:|:---------:|
     * |               `name`               | `string` |   Name of the state contained in the `VariablesHandler` describing the state associated to this dynamics|    Yes    |
     * |            `covariance`            | `vector` |                                Process covariances                                                      |    Yes    |
     * |           `dynamic_model`          | `string` |               Type of dynamic model describing the state dynamics.                                      |    Yes    |
     * |            `friction_k0`           | `vector` |  Vector of coefficients k0. For more info check the class description.                                  |    Yes    |
     * |            `friction_k1`           | `vector` |  Vector of coefficients k1. For more info check the class description.                                  |    Yes    |
     * |            `friction_k2`           | `vector` |  Vector of coefficients k2. For more info check the class description.                                  |    Yes    |
     * |             `use_bias`             |`boolean` |     Boolean saying if the dynamics depends on a bias. False if not specified.                           |    No     |
     * @return True in case of success, false otherwise.
     */
    bool
    initialize(std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler) override;

    /**
     * Finalize the Dynamics.
     * @param stateVariableHandler object describing the variables in the state vector.
     * @note You should call this method after you add ALL the state dynamics to the state variable
     * handler.
     * @return true in case of success, false otherwise.
     */
    bool finalize(const System::VariablesHandler& stateVariableHandler) override;

    /**
     * Set the KinDynWrapper object.
     * @param subModelList list of SubModel objects
     * @param kinDynWrapperList list of pointers to KinDynWrapper objects.
     * @return True in case of success, false otherwise.
     */
    bool setSubModels(
        const std::vector<SubModel>& subModelList,
        const std::vector<std::shared_ptr<KinDynWrapper>>& kinDynWrapperList) override;

    /**
     * Controls whether the variable handler contains the variables on which the dynamics depend.
     * @return True in case all the dependencies are contained in the variable handler, false
     * otherwise.
     */
    bool checkStateVariableHandler() override;

    /**
     * Update the content of the element.
     * @return True in case of success, false otherwise.
     */
    bool update() override;

    /**
     * Set the state of the ukf needed to update the dynamics of the measurement variable associated
     * to ths object.
     * @param ukfState reference to the ukf state.
     */
    void setState(const Eigen::Ref<const Eigen::VectorXd> ukfState) override;

    /**
     * Set a `UKFInput` object.
     * @param ukfInput reference to the UKFInput struct.
     */
    void setInput(const UKFInput& ukfInput) override;

}; // class FrictionTorqueMeasurementModel

BLF_REGISTER_UKF_DYNAMICS(FrictionTorqueMeasurementModel,
                          ::BipedalLocomotion::Estimators::RobotDynamicsEstimator::Dynamics);

} // namespace RobotDynamicsEstimator
} // namespace Estimators
} // namespace BipedalLocomotion

#endif // BIPEDAL_LOCOMOTION_ESTIMATORS_FRICTION_TORQUE_MEASUREMENT_MODEL_H
