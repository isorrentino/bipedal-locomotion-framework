/**
 * @file FixedBaseTSID.h
 * @authors Ines Sorrentino
 * @copyright 2021 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef BIPEDAL_LOCOMOTION_FIXED_BASE_TSID_H
#define BIPEDAL_LOCOMOTION_FIXED_BASE_TSID_H

#include <memory>
#include <vector>
#include <string>

#include <Eigen/Dense>

#include <BipedalLocomotion/TSID/TaskSpaceInverseDynamics.h>
#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/System/Advanceable.h>

namespace BipedalLocomotion
{

namespace TSID
{

class FixedBaseTSID : public TaskSpaceInverseDynamics, public Task
{

   /**
     * Private implementation
     */
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:

    /**
     * Constructor.
     */
    FixedBaseTSID();

    /**
     * Destructor.
     */
    ~FixedBaseTSID();

    /**
     * Add a linear task in the task space inverse dynamics
     * @param task pointer to a given linear task
     * @param taskName is the name of the task
     * @return true if the task has been added to the fixed base TSID.
     */
    bool addTask(std::shared_ptr<Task> task,
                 const std::string& taskName) override;

    /**
     * Initialize the TSID algorithm.
     * @param handler pointer to the IParametersHandler interface.g
     * @note the following parameters are required by the class
     * |         Parameter Name         |   Type   |                                           Description                                          | Mandatory |
     * |:------------------------------:|:--------:|:----------------------------------------------------------------------------------------------:|:---------:|
     * | `robot_velocity_variable_name` | `string` | Name of the variable contained in `VariablesHandler` describing the generalized robot velocity |    Yes    |
     * Where the generalized robot velocity is a vector containing the base accelerations and the joint accelerations.
     * @return True in case of success, false otherwise.
     */
    bool initialize(std::weak_ptr<ParametersHandler::IParametersHandler> handler,
                                  const System::VariablesHandler& variablesHandler) override;

    /**
     * Return true if the content of get is valid.
     */
    bool isValid() const override;

    /**
     * Solve the fixed base TSID.
     * @return true in case of success and false otherwise.
     */
    bool advance() override;

    /**
     * Get the outcome of the optimization problem
     * @return the state of the TSID.
     */
    const TSIDState& get() const override;

    /**
     * Get a specific task
     * @param name name associated to the task.
     * @return a weak ptr associated to an existing task in the IK. If the task does not exist a
     * nullptr is returned.
     */
    std::weak_ptr<Task> getTask(const std::string& name) const override;

    /**
     * Get a vector containing the name of the tasks.
     * @return an std::vector containing all the names associated to the tasks
     */
    std::vector<std::string> getTaskNames() const override;
};

} // BipedalLocomotion
} // TSID

#endif // BIPEDAL_LOCOMOTION_FIXED_BASE_TSID_H
