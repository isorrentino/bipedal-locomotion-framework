/**
 * @file RobotDynamicsEstimatorTest.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <catch2/catch_test_macros.hpp>
#include <iCubModels/iCubModels.h>
#include <yarp/os/ResourceFinder.h>
#include <ConfigFolderPath.h>

//BLF
#include <BipedalLocomotion/System/VariablesHandler.h>
#include <BipedalLocomotion/System/Clock.h>

#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>
#include <BipedalLocomotion/Math/Constants.h>

// iDynTree
#include <iDynTree/ModelIO/ModelLoader.h>
#include <iDynTree/KinDynComputations.h>

// RDE
#include <BipedalLocomotion/RobotDynamicsEstimator/SubModel.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/SubModelKinDynWrapper.h>
#include <BipedalLocomotion/RobotDynamicsEstimator/RobotDynamicsEstimator.h>

using namespace BipedalLocomotion::ParametersHandler;
using namespace BipedalLocomotion::System;
using namespace BipedalLocomotion::Estimators::RobotDynamicsEstimator;

IParametersHandler::shared_ptr loadParameterHandler()
{
    std::shared_ptr<YarpImplementation> originalHandler = std::make_shared<YarpImplementation>();
    IParametersHandler::shared_ptr parameterHandler = originalHandler;

    yarp::os::ResourceFinder& rf = yarp::os::ResourceFinder::getResourceFinderSingleton();
    rf.setDefaultConfigFile("/config/config.ini");

    std::vector<std::string> arguments = {" ", "--from ", getConfigPath()};

    std::vector<char*> argv;
    for (const auto& arg : arguments)
        argv.push_back((char*)arg.data());
    argv.push_back(nullptr);

    rf.configure(argv.size() - 1, argv.data());

    REQUIRE_FALSE(rf.isNull());
    parameterHandler->clear();
    REQUIRE(parameterHandler->isEmpty());

    originalHandler->set(rf);

    return parameterHandler;
}

void createModelLoader(IParametersHandler::weak_ptr group,
                       iDynTree::ModelLoader& mdlLdr)
{
    // List of joints and fts to load the model
    std::vector<SubModel> subModelList;

    const std::string modelPath = iCubModels::getModelFile("iCubGenova09");

    std::vector<std::string> jointList;
    REQUIRE(group.lock()->getParameter("joint_list", jointList));

    std::vector<std::string> ftFramesList;
    auto ftGroup = group.lock()->getGroup("FT").lock();
    REQUIRE(ftGroup->getParameter("associated_joints", ftFramesList));

    std::vector<std::string> jointsAndFTs;
    jointsAndFTs.insert(jointsAndFTs.begin(), jointList.begin(), jointList.end());
    jointsAndFTs.insert(jointsAndFTs.end(), ftFramesList.begin(), ftFramesList.end());

    REQUIRE(mdlLdr.loadReducedModelFromFile(modelPath, jointsAndFTs));
}

void createInitialState(std::shared_ptr<iDynTree::KinDynComputations> kinDynFullModel,
                        IParametersHandler::weak_ptr modelHandler,
                        RobotDynamicsEstimatorOutput& initialState)
{
    // All the values set here come from an experiment on the real robot

    // Resize variables
    initialState.ds.resize(kinDynFullModel->model().getNrOfDOFs()); // ds
    initialState.tau_m.resize(kinDynFullModel->model().getNrOfDOFs()); // tau_m
    initialState.tau_F.resize(kinDynFullModel->model().getNrOfDOFs()); // tau_F
    std::vector<std::string> ftList;
    auto ftGroup = modelHandler.lock()->getGroup("FT").lock();
    REQUIRE(ftGroup->getParameter("names", ftList));
    for (auto ft : ftList)
    {
        initialState.ftWrenches[ft] = Eigen::VectorXd(6).setZero(); // FT

        std::string ftBias = ft + "_bias";
        initialState.ftWrenchesBiases[ftBias] = Eigen::VectorXd(6).setZero(); // FT bias
    }
    std::vector<std::string> accList;
    auto accGroup = modelHandler.lock()->getGroup("ACCELEROMETER").lock();
    REQUIRE(accGroup->getParameter("names", accList));
    for (auto acc : accList)
    {
        std::string accBias = acc + "_bias";
        initialState.accelerometerBiases[accBias] = Eigen::VectorXd(3).setZero(); // ACC BIAS
    }
    std::vector<std::string> gyroList;
    auto gyroGroup = modelHandler.lock()->getGroup("GYROSCOPE").lock();
    REQUIRE(gyroGroup->getParameter("names", gyroList));
    for (auto gyro : gyroList)
    {
        std::string gyroBias = gyro + "_bias";
        initialState.gyroscopeBiases[gyroBias] = Eigen::VectorXd(3).setZero(); // GYRO BIAS
    }


    // Set values
    initialState.ds.setZero();
    initialState.tau_m << 22.644 ,  13.5454,  -7.6093,  19.3029, -15.0072,  -0.805;
    initialState.tau_F.setZero();

    initialState.ftWrenches["r_leg_ft"] << 49.45480758,  73.91349929,  46.85057916, -15.38562865, 9.04317083, 1.97395434;
    initialState.ftWrenches["r_foot_front_ft"] << 1.38697938e-01,  8.71035288e-01, 1.52496873e+00,  1.36409975e-02, -1.99708849e-03, -9.99651158e-05;
    initialState.ftWrenches["r_foot_rear_ft"] << 1.38003059e-01,  8.66671384e-01,  1.51732861e+00,  1.70397864e-02, -2.03056058e-03, -3.89970831e-04;
}

void createInput(std::shared_ptr<iDynTree::KinDynComputations> kinDynFullModel,
                 IParametersHandler::weak_ptr modelHandler,
                 RobotDynamicsEstimatorInput& input)
{
    // All the values set here come from an experiment on the real robot

    manif::SE3d basePose;
    basePose.setIdentity();
    input.basePose = basePose;

    manif::SE3d::Tangent baseVel;
    baseVel.setZero();
    input.baseVelocity = baseVel;

    manif::SE3d::Tangent baseAcc;
    baseAcc.setZero();
    input.baseAcceleration = baseAcc;

    input.jointPositions.resize(kinDynFullModel->model().getNrOfDOFs());
    input.jointPositions << 0.91693925,  0.69777121,  0.00249272, -0.68166438, -0.07698685, -0.08628663;

    input.jointVelocities.resize(kinDynFullModel->model().getNrOfDOFs());
    input.jointVelocities.setZero();

    // Use gearbox ratio and torque constants to convert joint torques into motor currents
    Eigen::VectorXd gearRatio(kinDynFullModel->model().getNrOfDOFs());
    gearRatio << 100.0, -100.0, 100.0, 100.0, 100.0, 100.0;

    Eigen::VectorXd torqueConstant(kinDynFullModel->model().getNrOfDOFs());
    torqueConstant << 0.111, 0.047, 0.047, 0.111, 0.111, 0.025;

//    input.motorCurrents = jointTorques.array() / (gearRatio.array() * torqueConstant.array());
    input.motorCurrents.resize(kinDynFullModel->model().getNrOfDOFs());
    input.motorCurrents << 2.04 , -2.882, -1.619,  1.739, -1.352, -0.322;

    input.ftWrenches["r_leg_ft"] = Eigen::VectorXd(6).setZero();
    input.ftWrenches["r_leg_ft"] << 49.45480758,  73.91349929,  46.85057916, -15.38562865, 9.04317083,   1.97395434;

    input.ftWrenches["r_foot_front_ft"] = Eigen::VectorXd(6).setZero();
    input.ftWrenches["r_foot_front_ft"] << 1.38697938e-01,  8.71035288e-01,  1.52496873e+00,  1.36409975e-02, -1.99708849e-03, -9.99651158e-05;

    input.ftWrenches["r_foot_rear_ft"] = Eigen::VectorXd(6).setZero();
    input.ftWrenches["r_foot_rear_ft"] << 1.38003059e-01,  8.66671384e-01,  1.51732861e+00,  1.70397864e-02, -2.03056058e-03, -3.89970831e-04;

    std::vector<std::string> accList;
    auto accGroup = modelHandler.lock()->getGroup("ACCELEROMETER").lock();
    REQUIRE(accGroup->getParameter("names", accList));
    for (auto acc : accList)
    {
        input.linearAccelerations[acc] = Eigen::VectorXd(3).setZero();
        input.linearAccelerations[acc](2) = - BipedalLocomotion::Math::StandardAccelerationOfGravitation;
    }
    input.linearAccelerations["r_leg_ft_acc"] << -3.76, -7.65, -4.32;
    input.linearAccelerations["r_foot_front_ft_acc"] << 1.44, 5.08, 8.11;
    input.linearAccelerations["r_foot_rear_ft_acc"] << 2.12, 5.23, 8.13;

    std::vector<std::string> gyroList;
    auto gyroGroup = modelHandler.lock()->getGroup("GYROSCOPE").lock();
    REQUIRE(gyroGroup->getParameter("names", gyroList));
    for (auto gyro : gyroList)
    {
        input.angularVelocities[gyro] = Eigen::VectorXd(3).setZero();
    }
    input.angularVelocities["r_leg_ft_gyro"] << -0.00654498,  0.0,  0.00109083;
    input.angularVelocities["r_foot_front_ft_gyro"] << -0.00327249,  0.0,  0.00218166;
    input.angularVelocities["r_foot_rear_ft_gyro"] << -0.00545415,  0.00109083,  0.00218166;
}

TEST_CASE("RobotDynamicsEstimator")
{
    auto parameterHandler = loadParameterHandler();

    iDynTree::ModelLoader modelLoader;
    auto modelHandler = parameterHandler->getGroup("MODEL").lock();

    REQUIRE_FALSE(modelHandler == nullptr);

    createModelLoader(modelHandler, modelLoader);

    std::cout << modelLoader.model().toString() << std::endl;

    auto kinDyn = std::make_shared<iDynTree::KinDynComputations>();
    REQUIRE(kinDyn->loadRobotModel(modelLoader.model()));
    REQUIRE(kinDyn->setFrameVelocityRepresentation(iDynTree::BODY_FIXED_REPRESENTATION));

    SubModelCreator subModelCreator;
    subModelCreator.setModelAndSensors(kinDyn->model(), modelLoader.sensors());
    REQUIRE(subModelCreator.setKinDyn(kinDyn));

    REQUIRE(subModelCreator.createSubModels(modelHandler));

    std::vector<std::shared_ptr<SubModelKinDynWrapper>> kinDynWrapperList;

    const auto & subModelList = subModelCreator.getSubModelList();

    for (int idx = 0; idx < subModelList.size(); idx++)
    {
        kinDynWrapperList.emplace_back(std::make_shared<SubModelKinDynWrapper>());
        REQUIRE(kinDynWrapperList.at(idx)->setKinDyn(kinDyn));
        REQUIRE(kinDynWrapperList.at(idx)->initialize(subModelList[idx]));
    }

    // automatic build the Estimator from parameter handler
    auto estimator = RobotDynamicsEstimator::build(parameterHandler, kinDyn, subModelList, kinDynWrapperList);
    REQUIRE_FALSE(estimator == nullptr);

    manif::SE3d::Tangent robotBaseAcceleration;
    robotBaseAcceleration.setZero();

    Eigen::VectorXd robotJointAcceleration(kinDyn->model().getNrOfDOFs());
    robotJointAcceleration.setZero();

    for (int idx = 0; idx < subModelCreator.getSubModelList().size(); idx++)
    {
        REQUIRE(kinDynWrapperList.at(idx)->updateState(robotBaseAcceleration,
                                                       robotJointAcceleration,
                                                       UpdateMode::Full));
    }

    iDynTree::LinkNetExternalWrenches extWrench(kinDyn->model());
    extWrench.zero();

    // Compute joint torques in static configuration from inverse dynamics on the full model
    iDynTree::FreeFloatingGeneralizedTorques jointTorques(kinDyn->model());

    kinDyn->inverseDynamics(iDynTree::make_span(robotBaseAcceleration.data(),
                                                manif::SE3d::Tangent::DoF),
                            robotJointAcceleration,
                            extWrench,
                            jointTorques);

    RobotDynamicsEstimatorOutput initialState;
    createInitialState(kinDyn,
                       modelHandler,
                       initialState);

    REQUIRE(estimator->setInitialState(initialState));

    RobotDynamicsEstimatorInput measurement;
    createInput(kinDyn,
                modelHandler,
                measurement);

        auto tic = BipedalLocomotion::clock().now();
        REQUIRE(estimator->setInput(measurement));
        REQUIRE(estimator->advance());
        auto toc = BipedalLocomotion::clock().now();

        BipedalLocomotion::log()->error("{}", toc - tic);

        RobotDynamicsEstimatorOutput result = estimator->getOutput();
}
