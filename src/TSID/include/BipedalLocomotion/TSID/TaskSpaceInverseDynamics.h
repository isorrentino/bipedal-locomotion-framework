/**
 * @file TaskSpaceInverseDynamics.h
 * @authors Ines Sorrentino
 * @copyright 2021 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef BIPEDAL_LOCOMOTION_TASK_SPACE_INVERSE_DYNAMICS_H
#define BIPEDAL_LOCOMOTION_TASK_SPACE_INVERSE_DYNAMICS_H

#include <BipedalLocomotion/System/Advanceable.h>

#include <BipedalLocomotion/TSID/Task.h>

namespace BipedalLocomotion
{

namespace TSID
{

/**
 * State of the TaskSpaceInverseDynamics
 */
 struct ContactWrench
{
    iDynTree::FrameIndex frameIndex; /**< Frame used to express the contact wrench */
    System::VariablesHandler::VariableDescription variable; /**< Variable describing the contact
                                                                wrench */
};
struct TSIDState
{
    System::VariablesHandler::VariableDescription robotAccelerationVariable; /**< Variable describing the robot acceleration
                                                                                  (base + joint) */
    Eigen::VectorXd torques; /**< Joint torques */
    std::vector<ContactWrench> contactWrenches; /**< List of the information related to the
                                                     contact wrenches */
};

class TaskSpaceInverseDynamics : public BipedalLocomotion::System::Advanceable<TSIDState>
{

public:
    /**
     * Initialize the task space inverse dynamics algorithm.
     * @param handler pointer to the IParametersHandler interface.
     * @return true in case of success/false otherwise.
     */
    virtual bool initialize(std::weak_ptr<ParametersHandler::IParametersHandler> handler,
                                  const System::VariablesHandler& variablesHandler);

    /**
     * Add a linear task in the task space inverse dynamics
     * @param task pointer to a given linear task
     * @param taskName is the name of the task
     * @return true if the task has been added to the fixed base TSID.
     */
    virtual bool addTask(std::shared_ptr<Task> task, const std::string& taskName) = 0;

    /**
     * Get a vector containing the name of the tasks.
     * @return an std::vector containing all the names associated to the tasks
     */
    virtual std::vector<std::string> getTaskNames() const = 0;

    /**
     * Finalize the TSID.
     * @param handler parameter handler.
     * @note You should call this method after you add ALL the tasks.
     * @return true in case of success, false otherwise.
     */
    virtual bool finalize(const System::VariablesHandler& handler) = 0;

    /**
     * Get a specific task
     * @param name name associated to the task.
     * @return a weak ptr associated to an existing task in the TSID. If the task does not exist a
     * nullptr is returned.
     */
    virtual std::weak_ptr<Task> getTask(const std::string& name) const = 0;

    /**
     * Destructor.
     */
    virtual ~TaskSpaceInverseDynamics() = default;
};

} // namespace TSID
} // namespace BipedalLocomotion

#endif // BIPEDAL_LOCOMOTION_TSID_TASK_SPACE_INVERSE_DYNAMICS_H
