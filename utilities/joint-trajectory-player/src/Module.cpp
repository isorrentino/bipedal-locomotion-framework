/**
 * @file Module.cpp
 * @authors Ines Sorrentino
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <iomanip>

#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>
#include <BipedalLocomotion/RobotInterface/YarpHelper.h>
#include <BipedalLocomotion/RobotInterface/YarpRobotControl.h>
#include <BipedalLocomotion/RobotInterface/YarpSensorBridge.h>

#include <Eigen/Dense>

#include <BipedalLocomotion/JointTrajectoryPlayer/Module.h>

#include <yarp/dev/IEncoders.h>

#include <BipedalLocomotion/Conversions/matioCppConversions.h>

using namespace BipedalLocomotion;
using namespace BipedalLocomotion::JointTrajectoryPlayer;

double Module::getPeriod()
{
    return m_dT;
}

bool Module::createPolydriver(std::shared_ptr<ParametersHandler::IParametersHandler> handler)
{
    auto ptr = handler->getGroup("ROBOT_INTERFACE").lock();
    if (ptr == nullptr)
    {
        std::cerr << "[Module::createPolydriver] Robot interface options is empty." << std::endl;
        return false;
    }
    ptr->setParameter("local_prefix", this->getName());
    m_controlBoard = RobotInterface::constructRemoteControlBoardRemapper(ptr);
    if (!m_controlBoard.isValid())
    {
        std::cerr << "[Module::createPolydriver] the robot polydriver has not been constructed."
                  << std::endl;
        return false;
    }

    return true;
}

bool Module::initializeRobotControl(std::shared_ptr<ParametersHandler::IParametersHandler> handler)
{
    if (!m_robotControl.initialize(handler->getGroup("ROBOT_CONTROL")))
    {
        std::cerr << "[Module::initializeRobotControl] Unable to initialize the "
                     "control board"
                  << std::endl;
        return false;
    }
    if (!m_robotControl.setDriver(m_controlBoard.poly))
    {
        std::cerr << "[Module::initializeRobotControl] Unable to initialize the "
                     "control board"
                  << std::endl;
        return false;
    }

    return true;
}

bool Module::instantiateSensorBridge(std::shared_ptr<ParametersHandler::IParametersHandler> handler)
{
    if (!m_sensorBridge.initialize(handler->getGroup("SENSOR_BRIDGE")))
    {
        std::cerr << "[Module::initializeSensorBridge] Unable to initialize the sensor bridge"
                  << std::endl;
        return false;
    }

    yarp::dev::PolyDriverList list;
    list.push(m_controlBoard.poly.get(), m_controlBoard.key.c_str());
    if (!m_sensorBridge.setDriversList(list))
    {
        std::cerr << "[Module::initializeSensorBridge] Unable to set the driver list" << std::endl;
        return false;
    }

    return true;
}

bool Module::readStateFromFile(const std::string& filename, const std::size_t numFields)
{
    std::deque<Eigen::VectorXd> data;

    matioCpp::File input(filename);

    if (!input.isOpen())
    {
        std::cout << "[Module::readStateFromFile] Failed to open " << filename << "." << std::endl;
        return false;
    } else
    {
        m_traj = input.read("joint_positions").asMultiDimensionalArray<double>(); // Read a multi dimensional
                                                                       // array named "traj"
        if (!m_traj.isValid())
        {
            std::cerr << "[Module::readStateFromFile] Error reading input file: " << filename << "."
                      << std::endl;
            return false;
        }

        return true;
    }
}

bool Module::configure(yarp::os::ResourceFinder& rf)
{
    constexpr auto logPrefix = "[Module::configure]";

    auto parametersHandler = std::make_shared<ParametersHandler::YarpImplementation>(rf);

    std::string name;
    if (!parametersHandler->getParameter("name", name))
        return false;
    this->setName(name.c_str());

    if (!parametersHandler->getParameter("sampling_time", m_dT))
        return false;

    std::string trajectoryFile;
    if (!parametersHandler->getParameter("trajectory_file", trajectoryFile))
    {
        std::cerr << "[Module::configure] Trajectory_file parameter not specified." << std::endl;
        return false;
    }

    if (!this->createPolydriver(parametersHandler))
    {
        std::cerr << "[Module::configure] Unable to create the polydriver." << std::endl;
        return false;
    }
    if (!this->initializeRobotControl(parametersHandler))
    {
        std::cerr << "[Module::configure] Unable to initialize the robotControl interface."
                  << std::endl;
        return false;
    }
    if (!this->instantiateSensorBridge(parametersHandler))
    {
        std::cerr << "[Module::configure] Unable toinitialize the sensorBridge interface."
                  << std::endl;
        return false;
    }

    m_numOfJoints = m_robotControl.getJointList().size();
    if (m_numOfJoints == 0)
    {
        std::cerr << "[Module::configure] No joints to control." << std::endl;
        return false;
    }

    m_axisList = m_robotControl.getJointList();

    m_currentJointPos.resize(m_numOfJoints);
    m_currentJointVel.resize(m_numOfJoints);
    m_currentMotorCurr.resize(m_numOfJoints);

    if (!readStateFromFile(trajectoryFile, m_numOfJoints))
    {
        return false;
    }

    std::cout << "[Module::configure] Starting the experiment." << std::endl;

    // Reach the first position of the desired trajectory in position control
    m_robotControl.setReferences(Conversions::toEigen(m_traj).row(m_idxTraj),
                                 RobotInterface::IRobotControl::ControlMode::Position);

    m_state = State::positioning;

    if (!m_portLogging.open("/JointTrajectoryPlayer/data:o"))
    {
        log()->error("{} Could not open the port to publish application data", logPrefix);
    }

    return true;
}

bool Module::updateModule()
{
    bool isMotionDone;
    bool isTimeExpired;
    std::vector<std::pair<std::string, double>> jointlist;

    switch (m_state)
    {
    case State::positioning:
        if (!m_robotControl.checkMotionDone(isMotionDone, isTimeExpired, jointlist))
        {
            std::cerr << "[Module::updateModule] Impossible to check if the motion is done."
                      << std::endl;
            return false;
        }
        if (isTimeExpired)
        {
            std::cerr << "[Module::updateModule] List of joints not finishing in time: "
                      << std::endl;
            for (int i = 0; i < jointlist.size(); i++)
            {
                log()->error("Joint {} --> Error {} rad", jointlist[i].first, jointlist[i].second);
            }
            return false;
        }
        if (isMotionDone)
        {
            m_state = State::running;
        }
        break;

    case State::running:
        if (!m_sensorBridge.advance())
        {
            std::cerr << "[Module::updateModule] Unable to read the sensor." << std::endl;
            return false;
        }

        if (!m_sensorBridge.getJointPositions(m_currentJointPos))
        {
            std::cerr << "[Module::updateModule] Error in reading current position." << std::endl;
            return false;
        }

        if (!m_sensorBridge.getJointVelocities(m_currentJointVel))
        {
            std::cerr << "[Module::updateModule] Error in reading current velocity." << std::endl;
            return false;
        }

        if (!m_sensorBridge.getMotorCurrents(m_currentMotorCurr))
        {
            std::cerr << "[Module::updateModule] Error in reading motor currents." << std::endl;
            return false;
        }


        m_idxTraj++;
        if (m_idxTraj == Conversions::toEigen(m_traj).rows())
        {
            std::cout << "[Module::updateModule] Experiment finished." << std::endl;

            m_portLogging.close();

            return false;
        }

        // set the reference
        if (!m_robotControl
                 .setReferences(Conversions::toEigen(m_traj).row(m_idxTraj),
                                RobotInterface::IRobotControl::ControlMode::PositionDirect))
        {
            std::cerr << "[Module::updateModule] Error while setting the reference position to "
                         "the robot."
                      << std::endl;
            return false;
        }

        break;

    default:
        std::cerr << "[Module::updateModule] The program is in an unknown state. Cannot proceed.";
        return false;
    }

    return true;
}

void Module::publishData()
{
    auto& data = m_portLogging.prepare();
    data.vectors.clear();

    data.vectors["joint_state::positions::measured"].assign(m_currentJointPos.data(),
                                                            m_currentJointPos.data()
                                                                + m_currentJointPos.size());
    data.vectors["joint_state::positions::desired"]
        .assign(Conversions::toEigen(m_traj).row(m_idxTraj).data(),
                Conversions::toEigen(m_traj).row(m_idxTraj).data()
                    + Conversions::toEigen(m_traj).row(m_idxTraj).size());

    m_portLogging.write();
}

bool Module::close()
{
    // switch back in position control
    m_robotControl.setReferences(m_currentJointPos,
                                 RobotInterface::IRobotControl::ControlMode::Position);

    return true;
}
