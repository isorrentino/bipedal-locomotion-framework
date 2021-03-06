/**
 * @file FloatingBaseSystemDynamics.h
 * @authors Giulio Romualdi
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef BIPEDAL_LOCOMOTION_SYSTEM_FLOATING_BASE_SYSTEM_DYNAMICS_H
#define BIPEDAL_LOCOMOTION_SYSTEM_FLOATING_BASE_SYSTEM_DYNAMICS_H

#include <memory>
#include <vector>

#include <BipedalLocomotion/System/DynamicalSystem.h>
#include <BipedalLocomotion/System/ContactWrench.h>

#include <Eigen/Dense>

#include <iDynTree/KinDynComputations.h>

namespace BipedalLocomotion
{
namespace System
{

/**
 * FloatingBaseDynamicalSystem describes a floating base dynamical system.
 * The FloatingBaseDynamicalSystem inherits from a generic DynamicalSystem where:
 * - DynamicalSystem::StateType is described by an std::tuple containing:
 *   - Eigen::Vector6d: the base velocity expressed in mixed representation;
 *   - Eigen::VectorXd: the joint velocities [in rad/s];
 *   - Eigen::Vector3d: position of the base w.r.t. the inertial frame
 *   - Eigen::Matrix3d: rotation matrix \f${} ^ I R _ {b}\f$. Matrix that transform a vector
 * whose coordinates are expressed in the base frame in the inertial frame;
 *   - Eigen::VectorXd: the joint positions [in rad].
 * - DynamicalSystem::StateDerivativeType is described by an std::tuple containing:
 *   - Eigen::Vector6d: the base acceleration expressed in mixed representation;
 *   - Eigen::VectorXd: the joint accelerations [in rad/s^2];
 *   - Eigen::Vector3d: base velocity w.r.t. the inertial frame;
 *   - Eigen::Matrix3d: rate of change of the rotation matrix \f${} ^ I \dot{R} _ {b}\f$.
 * whose coordinates are expressed in the base frame in the inertial frame;
 *   - Eigen::VectorXd: the joint velocities [in rad/s].
 * - DynamicalSystem::InputType is described by an std::tuple containing:
 *   - Eigen::VectorXd: the joint torques [in Nm];
 *   - std::vector<ContactWrench>: List of contact wrenches.
 */
class FloatingBaseDynamicalSystem
    : public DynamicalSystem<std::tuple<Eigen::Matrix<double, 6, 1>,
                                        Eigen::VectorXd,
                                        Eigen::Vector3d,
                                        Eigen::Matrix3d,
                                        Eigen::VectorXd>,
                             std::tuple<Eigen::Matrix<double, 6, 1>,
                                        Eigen::VectorXd,
                                        Eigen::Vector3d,
                                        Eigen::Matrix3d,
                                        Eigen::VectorXd>,
                             std::tuple<Eigen::VectorXd, std::vector<ContactWrench>>>
{
    static constexpr size_t m_baseDoFs = 6; /**< Number of degree of freedom associated to the
                                               floating base */

    std::shared_ptr<iDynTree::KinDynComputations> m_kinDyn; /**< Pointer to an existing instance of
                                                               kinDynComputations object */
    std::size_t m_actuatedDoFs{0}; /**< Number of actuated degree of freedom */

    Eigen::Vector3d m_gravity; /**< Gravity vector */

    Eigen::MatrixXd m_massMatrix; /**< Floating-base mass matrix  */

    Eigen::MatrixXd m_jacobianMatrix; /**< Jacobian Matrix  */

    // quantities useful to avoid dynamic allocation in the dynamic allocation in the
    // FloatingBaseDynamicalSystem::dynamics method
    Eigen::VectorXd m_generalizedRobotAcceleration;
    Eigen::VectorXd m_knownCoefficent;

    bool m_useMassMatrixRegularizationTerm{false};
    Eigen::MatrixXd m_massMatrixReglarizationTerm;

    double m_rho{0.01}; /**< Regularization term used for the Baumgarte stabilization over the SO(3)
                           group */


public:
    /**
     * Constructor.
     * @note The constructor set the gravity acceleration vector to
     * \f$\begin{bmatrix} 0 & 0 & -9.81\end{bmatrix}^\top\f$. Please call setGravityVector() if you
     * want define your custom gravity vector.
     */
    FloatingBaseDynamicalSystem();

    /**
     * Initialize the Dynamical system.
     * @note Please call this function only if you want to set an arbitrary value for the parameter
     * used in the Baumgarte stabilization \f$\rho\f$ (The default value is 0.01 ). In this case the
     * handler should contain a key called rho.
     * @param handler pointer to the parameter handler.
     * @return true in case of success/false otherwise.
     */
    bool initalize(std::weak_ptr<ParametersHandler::IParametersHandler> handler) override;

    /**
     * Set the vector of gravity.
     * @param gravity a 3D vector describing the gravity acceleration.
     */
    void setGravityVector(const Eigen::Ref<const Eigen::Vector3d>& gravity);

    /**
     * Set a kinDynComputations object.
     * @param kinDyn a pointer to the kinDynComputations object.
     * @return true in case of success, false otherwise.
     */
    bool setKinDyn(std::shared_ptr<iDynTree::KinDynComputations> kinDyn);

    /**
     * Set the mass matrix regularization term. i.e. $\f\bar{M} = M + M _ {reg}\f$. Where  $\fM\f$
     * is the mass matrix and  $\fM_{reg}\f$ is the matrix regularization term.
     * @param matrix the regularization term for the mass matrix.
     * @notice Calling this function is not mandatory. Call it only if you want to add a
     * regularization term.
     * @return true in case of success, false otherwise.
     */
    bool setMassMatrixRegularization(const Eigen::Ref<const Eigen::MatrixXd>& matrix);

    /**
     * Computes the floating based system dynamics. It return \f$f(x, u, t)\f$.
     * @note The control input and the state have to be set separately with the methods
     * setControlInput and setState.
     * @param time the time at witch the dynamics is computed.
     * @param stateDynamics tuple containing a reference to the element of the state derivative
     * @return true in case of success, false otherwise.
     */
    bool dynamics(const double& time, StateDerivativeType& stateDerivative) final;

    /**
     * Destructor.
     */
    ~FloatingBaseDynamicalSystem() = default;
};

} // namespace System
} // namespace BipedalLocomotion

#endif // BIPEDAL_LOCOMOTION_SYSTEM_FLOATING_BASE_SYSTEM_DYNAMICS_H
