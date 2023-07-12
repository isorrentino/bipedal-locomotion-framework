/**
 * @file RobotDynamicsEstimatorDevice.h
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef BIPEDAL_LOCOMOTION_FRAMEWORK_ROBOT_DYNAMICS_ESTIMATOR_DEVICE_H
#define BIPEDAL_LOCOMOTION_FRAMEWORK_ROBOT_DYNAMICS_ESTIMATOR_DEVICE_H

#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/RobotDynamicsEstimator.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/SubModel.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/KinDynWrapper.h>
#include <BipedalLocomotion/RobotInterface/YarpSensorBridge.h>
#include <BipedalLocomotion/YarpUtilities/VectorsCollection.h>

#include <iDynTree/Estimation/ContactStateMachine.h>
#include <iDynTree/Estimation/ExtWrenchesAndJointTorquesEstimator.h>
#include <iDynTree/ModelIO/ModelLoader.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IFrameTransform.h>
#include <yarp/dev/IMultipleWrapper.h>
#include <yarp/dev/IVirtualAnalogSensor.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/ResourceFinder.h>

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace BipedalLocomotion
{
class RobotDynamicsEstimatorDevice;
}

class BipedalLocomotion::RobotDynamicsEstimatorDevice : public yarp::dev::DeviceDriver,
                                                        public yarp::dev::IMultipleWrapper,
                                                        public yarp::os::PeriodicThread
{
public:
    RobotDynamicsEstimatorDevice(double period,
                                 yarp::os::ShouldUseSystemClock useSystemClock
                                 = yarp::os::ShouldUseSystemClock::No);
    RobotDynamicsEstimatorDevice();
    ~RobotDynamicsEstimatorDevice();

    virtual bool open(yarp::os::Searchable& config) final;
    virtual bool close() final;
    virtual bool attachAll(const yarp::dev::PolyDriverList& poly) final;
    virtual bool detachAll() final;
    virtual void run() final;

private:
    // class members
    std::string m_portPrefix{"/rde"};
    std::string m_robot{"ergocubSim"};
    std::string m_baseLink{"root_link"};
    std::vector<std::string> m_jointNameList{};
    Eigen::VectorXd m_gearboxRatio;
    Eigen::VectorXd m_torqueConstant;
    std::shared_ptr<iDynTree::KinDynComputations> m_kinDyn;
    std::unique_ptr<BipedalLocomotion::Estimators::RobotDynamicsEstimator::RobotDynamicsEstimator>
        m_estimator;
    std::unique_ptr<BipedalLocomotion::RobotInterface::YarpSensorBridge> m_robotSensorBridge;
    yarp::os::BufferedPort<BipedalLocomotion::YarpUtilities::VectorsCollection>
        m_loggerPort; /**<
                         Logger
                         port.
                       */
    std::unordered_map<std::string, Eigen::VectorXd> m_ftOffset;
    bool m_isFirstRun{true};
    iDynTree::ExtWrenchesAndJointTorquesEstimator m_iDynEstimator;
    std::thread m_publishEstimationThread;
    struct EstimatorInput
    {
        std::mutex mutex;
        BipedalLocomotion::Estimators::RobotDynamicsEstimator::RobotDynamicsEstimatorInput input;
    } m_estimatorInput;
    struct EstimatorOutput
    {
        std::mutex mutex;
        BipedalLocomotion::Estimators::RobotDynamicsEstimator::RobotDynamicsEstimatorOutput output;
    } m_estimatorOutput;
    bool m_estimatorIsRunning;
    Eigen::VectorXd m_estimatedTauj;
    Eigen::VectorXd m_measuredTauj;
    yarp::dev::PolyDriver m_remappedVirtualAnalogSensors; /**< Remapped virtual analog sensor
                                                           containg the axes for which the joint
                                                           torques estimates are published */
    struct
    {
        yarp::dev::IVirtualAnalogSensor* ivirtsens;
        yarp::dev::IMultipleWrapper* multwrap;
    } m_remappedVirtualAnalogSensorsInterfaces;
    yarp::sig::Vector m_estimatedJointTorquesYARP;
    Eigen::Vector3d m_temp3DMeasurement;

    // class methods
    bool setupRobotModel(std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler,
                         iDynTree::ModelLoader& mdlLdr);
    bool createSubModels(
        std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler,
        iDynTree::ModelLoader& modelLoader,
        std::vector<Estimators::RobotDynamicsEstimator::SubModel>& subModelList,
        std::vector<std::shared_ptr<Estimators::RobotDynamicsEstimator::KinDynWrapper>>&
            kinDynWrapperList);
    bool setupRobotDynamicsEstimator(
        std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler);
    bool
    setupRobotSensorBridge(std::weak_ptr<const ParametersHandler::IParametersHandler> paramHandler);
    bool openCommunications();
    bool updateMeasurements();
    bool resizeEstimatorInitialState(
        BipedalLocomotion::ParametersHandler::IParametersHandler::weak_ptr modelHandler);
    bool setEstimatorInitialState();
    bool resizeEstimatorMeasurement(
        BipedalLocomotion::ParametersHandler::IParametersHandler::weak_ptr modelHandler);
    void publishEstimatorOutput();
    bool openRemapperVirtualSensors();
};

#endif // BIPEDAL_LOCOMOTION_FRAMEWORK_ROBOT_DYNAMICS_ESTIMATOR_DEVICE_H
