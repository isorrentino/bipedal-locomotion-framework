/**
 * @file BaseAccelerationMeasurementDynamics.h
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef BIPEDAL_LOCOMOTION_ESTIMATORS_BASE_ACCELERATION_MEASUREMENT_DYNAMICS_H
#define BIPEDAL_LOCOMOTION_ESTIMATORS_BASE_ACCELERATION_MEASUREMENT_DYNAMICS_H

#include <BipedalLocomotion/RobotDynamicsEstimator/Dynamics.h>
#include <memory>

namespace BipedalLocomotion
{
namespace Estimators
{
namespace RobotDynamicsEstimator
{

/**
 * The BaseAccelerationMeasurementDynamics class is a concrete implementation of the Dynamics.
 * The model compares the acceleration of a sub-model base computed in two different ways.
 * The measurement is represented by the forward dynamics computed from the full-model,
 * and the model is given by the forward dynamics computed using the sub-model and considering the
 * action of the contact wrenches. This model is useful to help the estimation of contact wrenches.
 */
class BaseAccelerationMeasurementDynamics : public Dynamics
{
    int m_subModelIndex; /**< Index of the submodel having the frame as base frame. */
    std::vector<SubModel> m_subModelList; /**< List of SubModel objects. */
    std::vector<std::shared_ptr<KinDynWrapper>> m_kinDynWrapperList; /**< List of pointers
                                                                                to
                                                                                KinDynWrapper
                                                                                objects. */
    bool m_isSubModelListSet{false}; /**< Boolean flag saying if the sub-model list has been set. */
    std::vector<Eigen::VectorXd> m_subModelJointAcc; /**< Updated joint acceleration of each
                                                        sub-model. */
    UKFInput m_ukfInput; /**< Input of the UKF used to update the dynamics. */
    std::string m_name; /**< Name of dynamics. */
    std::string m_frame; /**< Frame of the sub-model base.. */
    System::VariablesHandler m_stateVariableHandler; /**< Variable handler describing the variables
                                                        and the sizes in the ukf state vector. */
    Eigen::VectorXd m_subModelBaseAcceleration; /**< Base acceleration of the sub-model. */
    manif::SE3d::Tangent m_frameVelocity; /** Velocity of the accelerometer given by the forward dynamics. */
    manif::SE3d::Tangent m_frameAcceleration; /** Acceleration of the accelerometer given by the forward dynamics. */
    Eigen::Vector3d m_vCrossW; /**< Accelerometer linear velocity cross accelerometer angular
                                  velocity. */

public:
    /*
     * Constructor
     */
    BaseAccelerationMeasurementDynamics();

    /*
     * Destructor
     */
    virtual ~BaseAccelerationMeasurementDynamics();

    /**
     * Initialize the state dynamics.
     * @param paramHandler pointer to the parameters handler.
     * @note the following parameters are required by the class
     * |           Parameter Name           |   Type   |                                       Description                                                       | Mandatory |
     * |:----------------------------------:|:--------:|:-------------------------------------------------------------------------------------------------------:|:---------:|
     * |               `name`               | `string` |   Name of the state contained in the `VariablesHandler` describing the state associated to this dynamics|    Yes    |
     * |            `covariance`            | `vector` |                                Process covariances                                                      |    Yes    |
     * |           `dynamic_model`          | `string` |               Type of dynamic model describing the state dynamics.                                      |    Yes    |
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

}; // class BaseAccelerationMeasurementDynamics

BLF_REGISTER_UKF_DYNAMICS(BaseAccelerationMeasurementDynamics,
                          ::BipedalLocomotion::Estimators::RobotDynamicsEstimator::Dynamics);

} // namespace RobotDynamicsEstimator
} // namespace Estimators
} // namespace BipedalLocomotion

#endif // BIPEDAL_LOCOMOTION_ESTIMATORS_BASE_ACCELERATION_MEASUREMENT_DYNAMICS_H
