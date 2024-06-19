/**
 * @file JointTorqueControlDevice.h
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef BIPEDAL_LOCOMOTION_FRAMEWORK_JOINT_TORQUE_CONTROL_DEVICE_H
#define BIPEDAL_LOCOMOTION_FRAMEWORK_JOINT_TORQUE_CONTROL_DEVICE_H

#include <BipedalLocomotion/PassThroughControlBoard.h>
#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>
#include <BipedalLocomotion/TextLogging/Logger.h>
#include <BipedalLocomotion/JointFrictionTorqueEstimator.h>
#include <BipedalLocomotion/YarpUtilities/VectorsCollection.h>
#include <BipedalLocomotion/YarpUtilities/VectorsCollectionServer.h>

#include <iostream>

#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Property.h>
#include <yarp/sig/Vector.h>

#include <Eigen/Core>

namespace BipedalLocomotion
{
class JointTorqueControlDevice;
}

/**
 * Coupling matrices
 *
 */
struct CouplingMatrices
{
    Eigen::MatrixXd fromJointTorquesToMotorTorques;
    Eigen::MatrixXd fromMotorTorquesToJointTorques;
    Eigen::MatrixXd fromJointVelocitiesToMotorVelocities;
    void reset(int NDOF)
    {
        fromJointTorquesToMotorTorques = Eigen::MatrixXd::Identity(NDOF, NDOF);
        fromMotorTorquesToJointTorques = Eigen::MatrixXd::Identity(NDOF, NDOF);
        fromJointVelocitiesToMotorVelocities = Eigen::MatrixXd::Identity(NDOF, NDOF);
    }
};

/**
 * Parameters for the control of torque through current and for mechanical friction compensation
 *
 */
struct MotorTorqueCurrentParameters
{
    double kt; ///< motor torque to current gain
    double kfc; ///< friction compensation weight parameter
    double maxCurr; ///< maximum current
    std::string frictionModel; ///< friction model

    void reset()
    {
        kt = kfc = maxCurr = 0.0;
        frictionModel = "";
    }
};

/**
 * Parameters for friction model defined as PINN
 *
 */
struct PINNParameters
{
    std::string modelPath; ///< PINN model path
    int threadNumber; ///< number of threads
    int historyLength; ///< history length

    void reset()
    {
        modelPath = "";
        threadNumber = 0;
        historyLength = 0;
    }
};

/**
 * Parameters for friction model defined as coulomb + viscous
 *
 */
struct CoulombViscousParameters
{
    double kc; ///< coulomb friction
    double kv; ///< viscous friction

    void reset()
    {
        kc = kv = 0.0;
    }
};

/**
 * Parameters for friction model defined as coulomb + viscous
 *
 */
struct CoulombViscousStribeckParameters
{
    double kc; ///< coulomb friction
    double kv; ///< viscous friction
    double vs; ///< stiction velocity
    double ka; ///< tanh gain
    double ks; ///< stribeck friction

    void reset()
    {
        kc = kv = ka = vs = ks = 0.0;
    }
};

class BipedalLocomotion::JointTorqueControlDevice
    : public BipedalLocomotion::PassThroughControlBoard,
      public yarp::os::PeriodicThread
{
private:
    yarp::os::Property PropertyConfigOptions;
    int axes;
    std::vector<MotorTorqueCurrentParameters> motorTorqueCurrentParameters;
    std::vector<PINNParameters> pinnParameters;
    std::vector<CoulombViscousParameters> coulombViscousParameters;
    std::vector<CoulombViscousStribeckParameters> coulombViscousStribeckParameters;
    std::vector<std::unique_ptr<JointFrictionTorqueEstimator>> frictionEstimators;
    yarp::sig::Vector desiredJointTorques;
    yarp::sig::Vector desiredMotorCurrents;
    yarp::sig::Vector measuredJointVelocities;
    yarp::sig::Vector measuredMotorVelocities;
    yarp::sig::Vector measuredJointTorques;
    yarp::sig::Vector measuredJointPositions;
    yarp::sig::Vector measuredMotorPositions;
    yarp::sig::Vector estimatedFrictionTorques;
    std::string m_portPrefix{"/hijackingTrqCrl"}; /**< Default port prefix. */
    BipedalLocomotion::YarpUtilities::VectorsCollectionServer m_vectorsCollectionServer; /**< Logger server. */

    CouplingMatrices couplingMatrices;

    bool openCalledCorrectly{false};

    // HIJACKING CONTROL
    /**
     *  vector of getAxes() size.
     *  For each axis contains true if we are hijacking the torque control
     *  for this joint, or false otherwise.
     */
    std::vector<bool> hijackingTorqueControl;
    std::vector<int> hijackedMotors;
    void startHijackingTorqueControlIfNecessary(int j);
    void stopHijackingTorqueControlIfNecessary(int j);
    bool isHijackingTorqueControl(int j);
    double computeFrictionTorque(int joint);

    void computeDesiredCurrents();
    void readStatus();
    // bool loadGains(yarp::os::Searchable& config);
    bool loadFrictionParams(std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler);

    /**
     * Load the coupling matrices from the group whose name
     *      is specified in group_name
     *
     *
     */
    bool loadCouplingMatrix(yarp::os::Searchable& config,
                            CouplingMatrices& coupling_matrices,
                            std::string group_name);

    std::mutex globalMutex; ///< mutex protecting control variables & proxy interface methods

    // Method that actually executes one control loop
    double timeOfLastControlLoop{-1.0};
    void controlLoop();

    /**
     * Open the communication ports.
     * @return true/false on success/failure.
     */
    bool openCommunications();

public:
    // CONSTRUCTOR/DESTRUCTOR
    JointTorqueControlDevice();
    ~JointTorqueControlDevice();

    // DEVICE DRIVER
    virtual bool open(yarp::os::Searchable& config);
    virtual bool close();

    // IMULTIPLEWRAPPER
    virtual bool attachAll(const yarp::dev::PolyDriverList& p);
    virtual bool detachAll();

    // CONTROL MODE
    virtual bool getControlMode(int j, int* mode);
    virtual bool getControlModes(int* modes);

    // CONTROL MODE
    virtual bool getControlModes(const int n_joint, const int* joints, int* modes);
    virtual bool setControlMode(const int j, const int mode);
    virtual bool setControlModes(int* modes);
    virtual bool setControlModes(const int n_joint, const int* joints, int* modes);

    // TORQUE CONTROL
    virtual bool getRefTorques(double* t);
    virtual bool getRefTorque(int j, double* t);
    virtual bool setRefTorques(const int n_joints, const int* joints, const double* trqs);
    virtual bool setRefTorques(const double* t);
    virtual bool setRefTorque(int j, double t);

    // CONTROL THREAD
    virtual bool threadInit();
    virtual void run();
    virtual void threadRelease();
};

#endif // BIPEDAL_LOCOMOTION_FRAMEWORK_JOINT_TORQUE_CONTROL_DEVICE_H
