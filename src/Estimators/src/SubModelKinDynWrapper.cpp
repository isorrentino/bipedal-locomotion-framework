/**
 * @file SubModelKinDynWrapper.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <Eigen/QR>

#include <iDynTree/Core/EigenHelpers.h>
#include <iDynTree/Core/MatrixView.h>
#include <iDynTree/Core/Span.h>

// BLF
#include <BipedalLocomotion/RobotDynamicsEstimator/SubModelKinDynWrapper.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

namespace blf = BipedalLocomotion;
namespace RDE = BipedalLocomotion::Estimators::RobotDynamicsEstimator;

bool RDE::SubModelKinDynWrapper::setKinDyn(std::shared_ptr<iDynTree::KinDynComputations> kinDyn)
{
    if ((kinDyn == nullptr) || (!kinDyn->isValid()))
    {
        log()->error("[SubModelKinDynWrapper::setKinDyn] Invalid kinDyn object.");
        return false;
    }

    this->m_kinDynFullModel = kinDyn;

    return true;
}

bool RDE::SubModelKinDynWrapper::initialize(const RDE::SubModel& subModel)
{
    constexpr auto logPrefix = "[BipedalLocomotion::Estimators::SubModelKinDynWrapper::initialize]";

    this->m_subModel = subModel;

    // Load robot model to the kindyncomputation object
    if (!m_kinDyn.loadRobotModel(m_subModel.getModel()))
    {
        blf::log()->error("{} Unable to load the robot model.", logPrefix);
        return false;
    }

    if (!m_kinDyn.setFrameVelocityRepresentation(iDynTree::BODY_FIXED_REPRESENTATION))
    {
        blf::log()->error("{} Unable to set the frame velocity representation.", logPrefix);
        return false;
    }

    m_numOfJoints = m_subModel.getModel().getNrOfDOFs();

    m_baseFrame = m_kinDyn.getFloatingBase();

    m_baseVelocity.setZero();

    m_jointPositionModel.resize(m_kinDynFullModel->getNrOfDegreesOfFreedom());
    m_jointVelocityModel.resize(m_kinDynFullModel->getNrOfDegreesOfFreedom());

    m_qj.resize(m_numOfJoints);
    m_dqj.resize(m_numOfJoints);

    m_FTranspose.resize(m_numOfJoints, 6);
    m_H.resize(m_numOfJoints, m_numOfJoints);
    m_genBiasJointTorques.resize(m_numOfJoints);
    m_pseudoInverseH.resize(m_numOfJoints, m_numOfJoints);
    m_FTBaseAcc.resize(m_numOfJoints);
    m_totalTorques.resize(6 + m_numOfJoints);
    m_subModelNuDot.resize(6 + m_numOfJoints);
    m_subModelNuDot.setZero();

    m_subModelBaseAcceleration.setZero();

    // Initialize attributes with correct sizes
    m_massMatrix.resize(6 + m_subModel.getModel().getNrOfDOFs(),
                        6 + m_subModel.getModel().getNrOfDOFs());
    m_massMatrix.setZero();

    m_genForces.resize(6 + m_subModel.getModel().getNrOfDOFs());
    m_genForces.setZero();

    for (auto& [key, value] : m_subModel.getFTList())
    {
        Eigen::MatrixXd tempJacobian(6, 6 + m_numOfJoints);
        tempJacobian.setZero();
        m_jacFTList.emplace(key, std::move(tempJacobian));
    }

    for (auto& [key, value] : m_subModel.getAccelerometerList())
    {
        Eigen::MatrixXd tempJacobian(6, 6 + m_numOfJoints);
        tempJacobian.setZero();
        m_jacAccList.emplace(key, std::move(tempJacobian));

        Eigen::VectorXd tempAcceleration(6);
        tempAcceleration.setZero();
        m_dJnuList.emplace(key, std::move(tempAcceleration));

        manif::SO3d rotMatrix;
        rotMatrix.coeffs().setZero();
        m_accRworldList.emplace(key, std::move(rotMatrix));

        manif::SE3d::Tangent vel;
        vel.coeffs().setZero();
        m_accVelList.emplace(key, std::move(vel));
    }

    for (auto& [key, value] : m_subModel.getGyroscopeList())
    {
        Eigen::MatrixXd tempJacobian(6, 6 + m_numOfJoints);
        tempJacobian.setZero();
        m_jacGyroList.emplace(key, std::move(tempJacobian));
    }

    for (int idx = 0; idx < m_subModel.getExternalContactList().size(); idx++)
    {
        Eigen::MatrixXd tempJacobian(6, 6 + m_numOfJoints);
        tempJacobian.setZero();
        m_jacContactList.emplace(m_subModel.getExternalContactList()[idx], std::move(tempJacobian));
    }

    return true;
}

bool RDE::SubModelKinDynWrapper::updateState(UpdateMode updateMode)
{
    constexpr auto logPrefix = "[BipedalLocomotion::Estimators::SubModelKinDynWrapper::"
                               "updateKinDynState]";

    // Get world transform for the base frame
    m_worldTBase = blf::Conversions::toManifPose(m_kinDynFullModel->getWorldTransform(m_baseFrame));

    // Get base velocity
    m_baseVelocity = blf::Conversions::toManifTwist(m_kinDynFullModel->getFrameVel(m_baseFrame));

    // Get position and velocity of the joints in the full model
    m_kinDynFullModel->getRobotState(m_jointPositionModel, m_jointVelocityModel, m_worldGravity);

    // Get positions and velocities of joints of the sub model
    for (int idx = 0; idx < m_subModel.getJointMapping().size(); idx++)
    {
        m_qj(idx) = m_jointPositionModel[m_subModel.getJointMapping()[idx]];
        m_dqj(idx) = m_jointVelocityModel[m_subModel.getJointMapping()[idx]];
    }

    if (!m_kinDyn.setRobotState(m_worldTBase.transform(),
                                m_qj,
                                iDynTree::make_span(m_baseVelocity.data(),
                                                    manif::SE3d::Tangent::DoF),
                                m_dqj,
                                m_worldGravity))
    {
        blf::log()->error("{} Unable to set the robot state.", logPrefix);
        return false;
    }

    return updateDynamicsVariableState(updateMode);
}

bool RDE::SubModelKinDynWrapper::updateDynamicsVariableState(UpdateMode updateMode)
{
    constexpr auto logPrefix = "[BipedalLocomotion::Estimators::SubModelKinDynWrapper::"
                               "updateDynamicsVariableState]";

    if (!m_kinDyn.getFreeFloatingMassMatrix(m_massMatrix))
    {
        blf::log()->error("{} Unable to get the mass matrix of the sub-model.", logPrefix);
        return false;
    }

    if (!m_kinDyn.generalizedBiasForces(
            iDynTree::make_span(m_genForces.data(), m_genForces.size())))
    {
        blf::log()->error("{} Unable to get the generalized bias forces of the sub-model.",
                          logPrefix);
        return false;
    }

    // The jacobians are computed wrt the base of the submodel
    // (floating base with known pos and vel thanks to the kinematics)
    for (auto& [key, value] : m_subModel.getFTList())
    {
        if (!m_kinDyn.getFrameFreeFloatingJacobian(value.frame, m_jacFTList[key]))
        {
            blf::log()->error("{} Unable to get the compute the free floating jacobian of the "
                              "frame {}.",
                              logPrefix,
                              value.frame);
            return false;
        }
    }

    // Update external contact jacobians
    for (int idx = 0; idx < m_subModel.getExternalContactList().size(); idx++)
    {
        if (!m_kinDyn.getFrameFreeFloatingJacobian(m_subModel.getExternalContactList()[idx],
                                                   m_jacContactList
                                                       [m_subModel.getExternalContactList()[idx]]))
        {
            blf::log()->error("{} Unable to get the compute the free floating jacobian of the "
                              "frame {}.",
                              logPrefix,
                              m_subModel.getExternalContactList()[idx]);
            return false;
        }
    }

    if (updateMode == UpdateMode::Full)
    {
        // Update accelerometer jacobians, dJnu, rotMatrix
        for (auto& [key, value] : m_subModel.getAccelerometerList())
        {
            // Update rotMatrix
            m_accRworldList[key] = blf::Conversions::toManifRot(
                m_kinDynFullModel->getWorldTransform(value.frame).getRotation().inverse());

            // Update accelerometer velocity
            m_accVelList[key] = iDynTree::toEigen(m_kinDyn.getFrameVel(value.frame));

            log()->info("accelerometer velocity\n{}", m_accVelList[key]);
        }

        // Update gyroscope jacobians
        for (auto& [key, value] : m_subModel.getGyroscopeList())
        {
            if (!m_kinDyn.getFrameFreeFloatingJacobian(value.frame, m_jacGyroList[key]))
            {
                blf::log()->error("{} Unable to get the compute the free floating jacobian of the "
                                  "frame {}.",
                                  logPrefix,
                                  value.frame);
                return false;
            }
        }
    }

    return true;
}

bool RDE::SubModelKinDynWrapper::forwardDynamics(
    Eigen::Ref<const Eigen::VectorXd> motorTorqueAfterGearbox,
    Eigen::Ref<const Eigen::VectorXd> frictionTorques,
    Eigen::Ref<const Eigen::VectorXd> tauExt,
    const manif::SE3d::Tangent baseAcceleration)
{
    log()->info("--------------------------------------- Sigma point {}", counter);
    counter++;

//    log()->info("tau m\n{}", motorTorqueAfterGearbox);
//    log()->info("tau F\n{}", frictionTorques);
//    log()->info("tau ext\n{}", tauExt.tail(motorTorqueAfterGearbox.size()));

    m_totalTorques.setZero();
    m_totalTorques.tail(motorTorqueAfterGearbox.size()) = motorTorqueAfterGearbox - frictionTorques;

    if (m_baseFrame == m_kinDynFullModel->getFloatingBase())
    {
        m_FTBaseAcc = m_massMatrix.block(6, 0, motorTorqueAfterGearbox.size(), 6)
                      * baseAcceleration.coeffs();

//        log()->info("F transpose");
//        log()->info(m_massMatrix.block(6, 0, motorTorqueAfterGearbox.size(), 6));
//        log()->info("base acceleration\n{}", baseAcceleration.coeffs());

        m_totalTorques.tail(motorTorqueAfterGearbox.size())
            = m_totalTorques.tail(motorTorqueAfterGearbox.size())
              + tauExt.tail(motorTorqueAfterGearbox.size())
              - m_genForces.tail(motorTorqueAfterGearbox.size()) - m_FTBaseAcc;

        m_totalTorques.head(6) = 0*tauExt.head(6);

//        log()->info("m_totalTorques");
//        log()->info(m_totalTorques);

        m_subModelNuDot.tail(motorTorqueAfterGearbox.size())
            = m_massMatrix
                  .block(6, 6, motorTorqueAfterGearbox.size(), motorTorqueAfterGearbox.size())
                  .llt()
                  .solve(m_totalTorques.tail(motorTorqueAfterGearbox.size()));

        //        m_subModelNuDot = m_massMatrix.llt().solve(m_totalTorques);

//        log()->info("H\n{}", m_massMatrix
//              .block(6, 6, motorTorqueAfterGearbox.size(), motorTorqueAfterGearbox.size()));

//        log()->info("H inverse\n{}", m_massMatrix
//                    .block(6, 6, motorTorqueAfterGearbox.size(), motorTorqueAfterGearbox.size())
//                    .llt()
//                    .solve(Eigen::MatrixXd::Identity(motorTorqueAfterGearbox.size(), motorTorqueAfterGearbox.size())));

//        log()->info("m_subModelNuDot");
//        log()->info(m_subModelNuDot);

        m_subModelNuDot.head(6) = baseAcceleration.coeffs();

//        log()->info("m_subModelNuDot");
//        log()->info(m_subModelNuDot);

        m_subModelBaseAcceleration = baseAcceleration.coeffs();
    } else
    {
        m_totalTorques = m_totalTorques + tauExt - m_genForces;
        m_subModelNuDot = m_massMatrix.llt().solve(m_totalTorques);
        m_subModelBaseAcceleration = m_subModelNuDot.head(6);
    }

//    m_totalTorques = m_totalTorques + tauExt - m_genForces;
//    m_subModelNuDot = m_massMatrix.llt().solve(m_totalTorques);
//    m_subModelBaseAcceleration = m_subModelNuDot.head(6);

    log()->info("joint acceleration");
    log()->info(m_subModelNuDot);

    return true;
}

manif::SE3d::Tangent RDE::SubModelKinDynWrapper::getFrameAcceleration(const std::string & frameName,
                                                                      Eigen::Ref<const Eigen::VectorXd> nuDot)
{
    manif::SE3d::Tangent frameAcceleration;
    frameAcceleration.setZero();

    m_kinDyn.getFrameAcc(frameName,
                         nuDot.head(6),
                         nuDot.tail(m_kinDyn.model().getNrOfDOFs()),
                         iDynTree::make_span(frameAcceleration.data(), manif::SE3d::Tangent::DoF));

//    log()->info("Frame {} acceleration\n{}", frameName, frameAcceleration.coeffs());

    return frameAcceleration;
}

const manif::SE3d::Tangent& RDE::SubModelKinDynWrapper::getBaseAcceleration()
{
    return m_subModelBaseAcceleration;
}

const Eigen::Ref<const Eigen::VectorXd> RDE::SubModelKinDynWrapper::getnudot()
{
    return m_subModelNuDot;
}

const manif::SE3d::Tangent& RDE::SubModelKinDynWrapper::getBaseVelocity()
{
    return m_baseVelocity;
}

const std::string& RDE::SubModelKinDynWrapper::getBaseFrameName() const
{
    return m_baseFrame;
}

const Eigen::Ref<const Eigen::MatrixXd> RDE::SubModelKinDynWrapper::getMassMatrix() const
{
    return m_massMatrix;
}

const Eigen::Ref<const Eigen::VectorXd> RDE::SubModelKinDynWrapper::getGeneralizedForces() const
{
    return m_genForces;
}

const Eigen::Ref<const Eigen::MatrixXd>
RDE::SubModelKinDynWrapper::getFTJacobian(const std::string& ftName) const
{
    return m_jacFTList.at(ftName);
}

const Eigen::Ref<const Eigen::MatrixXd>
RDE::SubModelKinDynWrapper::getAccelerometerJacobian(const std::string& accName) const
{
    return m_jacAccList.at(accName);
}

const Eigen::Ref<const Eigen::MatrixXd>
RDE::SubModelKinDynWrapper::getGyroscopeJacobian(const std::string& gyroName) const
{
    return m_jacGyroList.at(gyroName);
}

const Eigen::Ref<const Eigen::MatrixXd>
RDE::SubModelKinDynWrapper::getExtContactJacobian(const std::string& extContactName) const
{
    return m_jacContactList.at(extContactName);
}

const Eigen::Ref<const Eigen::VectorXd>
RDE::SubModelKinDynWrapper::getAccelerometerBiasAcceleration(const std::string& accName) const
{
    return m_dJnuList.at(accName);
}

const manif::SO3d&
RDE::SubModelKinDynWrapper::getAccelerometerRotation(const std::string& accName) const
{
    return m_accRworldList.at(accName);
}

const manif::SE3d::Tangent&
RDE::SubModelKinDynWrapper::getAccelerometerVelocity(const std::string& accName)
{
    return m_accVelList[accName];
}
