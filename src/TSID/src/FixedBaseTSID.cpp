/**
 * @file FixedBaseTSID.cpp
 * @authors Ines Sorrentino
 * @copyright 2021 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the GNU Lesser General Public License v2.1 or any later version.
 */

#include <memory>

#include <BipedalLocomotion/TSID/FixedBaseTSID.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

#include <iDynTree/Model/Model.h>

using namespace BipedalLocomotion::TSID;

struct FixedBaseTSID::Impl
{
    struct ContactWrench
    {
        iDynTree::FrameIndex frameIndex; /**< Frame used to express the contact wrench */
        System::VariablesHandler::VariableDescription variable; /**< Variable describing the contact
                                                                   wrench */
    };

    std::vector<ContactWrench> contactWrenches; /**< List of the information related to the
                                                     contact wrenches */

    std::unordered_map<std::string, std::shared_ptr<Task>> tasks;

    TSIDState solution;

    System::VariablesHandler::VariableDescription robotVelocityVariable;
    System::VariablesHandler::VariableDescription robotAccelerationVariable;

    System::VariablesHandler::VariableDescription jointsTorqueVariable; 
    
    Eigen::MatrixXd massMatrix;
    Eigen::MatrixXd jacobian;
    Eigen::VectorXd generalizedBiasForces;

    static constexpr std::size_t spatialVelocitySize{6};
};

FixedBaseTSID::FixedBaseTSID()
{
    m_pimpl = std::make_unique<FixedBaseTSID::Impl>();
}

FixedBaseTSID::~FixedBaseTSID() = default;

bool FixedBaseTSID::addTask(std::shared_ptr<Task> task, const std::string& taskName)
{
    constexpr auto logPrefix = "[FixedBaseTSID::addTask]";

    // check if the task already exist
    const bool taskExist = (m_pimpl->tasks.find(taskName) != m_pimpl->tasks.end());
    if (taskExist)
    {
        log()->error("{} The task named {} already exist.", logPrefix, taskName);
        return false;
    }

    m_pimpl->tasks[taskName] = task;

    return true;
}

std::vector<std::string> FixedBaseTSID::getTaskNames() const
{
    std::vector<std::string> tasksName;

    for (const auto& [name, task] : m_pimpl->tasks)
    {
        tasksName.push_back(name);
    }

    return tasksName;
}

std::weak_ptr<Task> FixedBaseTSID::getTask(const std::string& name) const
{
    constexpr auto logPrefix = "[FixedBaseTSID::getTask]";
    auto task = m_pimpl->tasks.find(name);

    if (task == m_pimpl->tasks.end())
    {
        log()->warn("{} The task named {} does not exist. A nullptr will be returned.",
                    logPrefix,
                    name);
        return std::shared_ptr<Task>(nullptr);
    }

    return task->second;
}

const TSIDState& FixedBaseTSID::get() const
{
    return m_pimpl->solution;
}

bool FixedBaseTSID::initialize(std::weak_ptr<ParametersHandler::IParametersHandler> handler,
                                  const System::VariablesHandler& variablesHandler)
{
    constexpr auto errorPrefix = "[FixedBaseTSID::initialize]";

    if (m_kinDyn == nullptr || !m_kinDyn->isValid())
    {
        std::cerr << errorPrefix << "KinDynComputations object is not valid." << std::endl;
        return false;
    }

    if (m_kinDyn->getFrameVelocityRepresentation()
        != iDynTree::FrameVelocityRepresentation::MIXED_REPRESENTATION)
    {
        std::cerr << errorPrefix
                  << "The task supports only quantities expressed in MIXED representation. "
                     "Please provide a KinDynComputations with Frame velocity representation set "
                     "to MIXED_REPRESENTATION."
                  << std::endl;
        return false;
    }

    auto ptr = handler.lock();
    if (ptr == nullptr)
    {
        std::cerr << errorPrefix << "The parameter handler is not valid." << std::endl;
        return false;
    }

    std::string robotAccelerationVariableName;
    if (!ptr->getParameter("robot_acceleration_variable_name", robotAccelerationVariableName)
        || !variablesHandler.getVariable(robotAccelerationVariableName,
                                         m_pimpl->robotAccelerationVariable))
    {
        std::cerr << errorPrefix << "Error while retrieving the robot acceleration variable."
                  << std::endl;
        return false;
    }

    if (m_pimpl->robotAccelerationVariable.size
        != m_kinDyn->getNrOfDegreesOfFreedom() + m_pimpl->spatialVelocitySize)
    {
        std::cerr << errorPrefix << "Error while retrieving the robot acceleration variable."
                  << std::endl;
        return false;
    }


    std::string jointTorquesVariableName;
    if (!ptr->getParameter("joint_torques_variable_name", jointTorquesVariableName)
        || !variablesHandler.getVariable(jointTorquesVariableName, m_pimpl->jointsTorqueVariable))
    {
        std::cerr << errorPrefix << "Error while retrieving the joint torques variable."
                  << std::endl;
        return false;
    }

    if (m_pimpl->jointsTorqueVariable.size != m_kinDyn->getNrOfDegreesOfFreedom())
    {
        std::cerr << errorPrefix
                  << "The size of the joint torques in the variable name is different from the "
                     "expected one. Expected size: "
                  << m_kinDyn->getNrOfDegreesOfFreedom()
                  << ", Joint Torque: " << m_pimpl->jointsTorqueVariable.size << "." << std::endl;
        return false;
    }

    int numberOfContacts = 0;
    ptr->getParameter("max_number_of_contacts", numberOfContacts);

    for (int i = 0; i < numberOfContacts; i++)
    {
        auto groupWeak = ptr->getGroup("CONTACT_" + std::to_string(i));
        auto group = groupWeak.lock();
        if (group == nullptr)
        {
            std::cerr << errorPrefix << "The group named: CONTACT_" << i << " do not exist."
                      << std::endl;
            return false;
        }

        std::string frameName;
        FixedBaseTSID::Impl::ContactWrench tmp;
        if (!group->getParameter("frame_name", frameName)
            || (tmp.frameIndex = m_kinDyn->model().getFrameIndex(frameName))
                   == iDynTree::FRAME_INVALID_INDEX)
        {
            std::cerr << errorPrefix
                      << "Error while retrieving the frame associated to the CONTACT_" << i << "."
                      << std::endl;
            return false;
        }

        std::string variableName;
        if (!group->getParameter("variable_name", variableName)
            || !variablesHandler.getVariable(variableName, tmp.variable))
        {
            std::cerr << errorPrefix
                      << "Error while retrieving the variable name associated to the CONTACT_" << i
                      << "." << std::endl;
            return false;
        }

        if (tmp.variable.size != m_pimpl->spatialVelocitySize)
        {
            std::cerr << errorPrefix << "The variable size associated to the CONTACT_" << i
                      << " is different from " << m_pimpl->spatialVelocitySize << "." << std::endl;
            return false;
        }

        m_pimpl->contactWrenches.push_back(tmp);
    }

    // set the description
    m_description = "Fixed base TSID.";

    // resize the matrices
    m_A.resize(m_kinDyn->getNrOfDegreesOfFreedom(), variablesHandler.getNumberOfVariables());
    m_A.setZero();
    m_b.resize(m_pimpl->spatialVelocitySize*2 + m_kinDyn->getNrOfDegreesOfFreedom() + 
                            m_pimpl->spatialVelocitySize*numberOfContacts);

    return true;
}
