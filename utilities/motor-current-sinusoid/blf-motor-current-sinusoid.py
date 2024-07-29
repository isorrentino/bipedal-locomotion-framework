#!/usr/bin/env python3

# This software may be modified and distributed under the terms of the BSD-3-Clause license.

import datetime
import os
import signal
import sys
from abc import ABC, abstractmethod

import bipedal_locomotion_framework as blf
import numpy as np
import yarp

logPrefix = "[MotorCurrentSinusoidApplication]"


class MotorParameters(ABC):
    # k_tau[A/Nm] includes the gear ratio
    k_tau = {
        "l_hip_roll": 94 * 1e-3,
        "l_hip_pitch": 64 * 1e-3,
        "l_hip_yaw": 150 * 1e-3,
        "l_knee": 64 * 1e-3,
        "l_ankle_pitch": 64 * 1e-3,
        "l_ankle_roll": 177 * 1e-3,
        "r_hip_roll": 94 * 1e-3,
        "r_hip_pitch": 64 * 1e-3,
        "r_hip_yaw": 150 * 1e-3,
        "r_knee": 64 * 1e-3,
        "r_ankle_pitch": 64 * 1e-3,
        "r_ankle_roll": 177 * 1e-3,
    }


class Trajectory(ABC):
    @abstractmethod
    def generate(self, *args, **kwargs):
        pass


class SinusoidTrajectoryGenerator(Trajectory):
    def __init__(
        self,
        min_delta_current,
        max_delta_current,
        delta_current_increment,
        min_frequency,
        max_frequency,
        frequency_increment,
    ):
        self.min_delta_current = min_delta_current
        self.max_delta_current = max_delta_current
        self.delta_current_increment = delta_current_increment
        self.min_frequency = min_frequency
        self.max_frequency = max_frequency
        self.frequency_increment = frequency_increment
        self.joint_list = []
        self.trajectory = np.array([])

    def from_parameter_handler(param_handler):

        min_delta_current = param_handler.get_parameter_vector_float(
            "min_delta_current"
        )
        max_delta_current = param_handler.get_parameter_vector_float(
            "max_delta_current"
        )
        delta_current_increment = param_handler.get_parameter_vector_float(
            "delta_current_increment"
        )
        min_frequency = param_handler.get_parameter_vector_float("min_frequency")
        max_frequency = param_handler.get_parameter_vector_float("max_frequency")
        frequency_increment = param_handler.get_parameter_vector_float(
            "frequency_increment"
        )

        return SinusoidTrajectoryGenerator(
            min_delta_current=min_delta_current,
            max_delta_current=max_delta_current,
            delta_current_increment=delta_current_increment,
            min_frequency=min_frequency,
            max_frequency=max_frequency,
            frequency_increment=frequency_increment,
        )

    def set_joint_list(self, joint_list):
        self.joint_list = joint_list

    def generate(self, dt, initial_current, joint_index=None):

        # Check if joint list is set
        if not self.joint_list:
            raise ValueError("Joint list must be set before generating the trajectory")

        # Check if joint index has to be specified
        if len(self.joint_list) > 1 and (joint_index is None):
            raise ValueError(
                "Joint index must be specified when more than one joint is controlled"
            )

        # Set default joint index, if not specified
        if joint_index is None:
            joint_index = 0

        A = self.min_delta_current[joint_index]
        f_in = self.max_frequency[joint_index]
        f_end = self.min_frequency[joint_index]
        delta_f = -self.frequency_increment[joint_index]
        delta_current_increment = self.delta_current_increment[joint_index]

        # Generate the trajectory
        trajectory = []

        while np.abs(A) <= np.abs(self.max_delta_current[joint_index]):

            for f in np.arange(f_in, f_end, delta_f):
                t_max = 2.0 / f
                t = np.arange(0, t_max, dt)
                trajectory.extend(initial_current + A * np.sin(2 * np.pi * f * t))

            A += delta_current_increment

        return trajectory


def build_remote_control_board_driver(
    param_handler: blf.parameters_handler.IParametersHandler, local_prefix: str
):
    param_handler.set_parameter_string("local_prefix", local_prefix)
    return blf.robot_interface.construct_remote_control_board_remapper(param_handler)


def create_ctrl_c_handler(sensor_bridge, robot_control):
    def ctrl_c_handler(sig, frame):
        blf.log().info("{} Ctrl+C pressed. Exiting gracefully.".format(logPrefix))
        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError(
                "{} Unable to advance the sensor bridge".format(logPrefix)
            )

        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

        # set the control mode to position
        if not robot_control.set_control_mode(
            blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
        if not robot_control.set_references(
            joint_positions, blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("{} Unable to set the references".format(logPrefix))

        blf.log().info(
            "{} Sleep for two seconds. Just to be sure the interfaces are on.".format(
                logPrefix
            )
        )
        blf.clock().sleep_for(datetime.timedelta(seconds=2))
        sys.exit(0)

    return ctrl_c_handler


def main():

    isGazebo = False
    if "gazebo" in (os.environ.get("YARP_ROBOT_NAME")).lower():
        isGazebo = True

    param_handler = blf.parameters_handler.YarpParametersHandler()

    param_file = "blf-motor-current-sinusoid-options.ini"

    if not param_handler.set_from_filename(param_file):
        raise RuntimeError("{} Unable to load the parameters".format(logPrefix))

    # Load the trajectory parameters
    sinusoid_group = param_handler.get_group("SINUSOID")

    # Create the trajectory generator
    trajectory_generator = SinusoidTrajectoryGenerator.from_parameter_handler(
        sinusoid_group
    )

    # Load joints to control and build the control board driver
    robot_control_handler = param_handler.get_group("ROBOT_CONTROL")
    joints_to_control = robot_control_handler.get_parameter_vector_string("joints_list")
    blf.log().info("{} Joints to control: {}".format(logPrefix, joints_to_control))
    bypass_motor_current_measure = [
        True
        if joint in ["l_ankle_roll", "r_ankle_roll", "l_ankle_pitch", "r_ankle_pitch", "l_hip_yaw", "r_hip_yaw"]
        else False
        for joint in joints_to_control
    ]
    trajectory_generator.set_joint_list(joints_to_control)

    poly_drivers = dict()

    poly_drivers["REMOTE_CONTROL_BOARD"] = build_remote_control_board_driver(
        param_handler=robot_control_handler,
        local_prefix="motor_current_sinusoid",
    )
    if not poly_drivers["REMOTE_CONTROL_BOARD"].is_valid():
        raise RuntimeError(
            "{} Unable to create the remote control board driver".format(logPrefix)
        )

    blf.log().info(
        "{} Sleep for two seconds. Just to be sure the interfaces are on.".format(
            logPrefix
        )
    )
    blf.clock().sleep_for(datetime.timedelta(seconds=2))

    robot_control = blf.robot_interface.YarpRobotControl()
    if not robot_control.initialize(robot_control_handler):
        raise RuntimeError(
            "{} Unable to initialize the robot control".format(logPrefix)
        )
    if not robot_control.set_driver(poly_drivers["REMOTE_CONTROL_BOARD"].poly):
        raise RuntimeError(
            "{} Unable to set the driver for the robot control".format(logPrefix)
        )

    # Get joint limits
    safety_threshold = param_handler.get_parameter_float("safety_threshold")
    safety_threshold = np.deg2rad(safety_threshold)
    _, lower_limits, upper_limits = robot_control.get_joint_limits()
    lower_limits = np.deg2rad(lower_limits)
    upper_limits = np.deg2rad(upper_limits)

    # Create the sensor bridge
    sensor_bridge = blf.robot_interface.YarpSensorBridge()
    sensor_bridge_handler = param_handler.get_group("SENSOR_BRIDGE")
    if not sensor_bridge.initialize(sensor_bridge_handler):
        raise RuntimeError(
            "{} Unable to initialize the sensor bridge".format(logPrefix)
        )
    if not sensor_bridge.set_drivers_list(list(poly_drivers.values())):
        raise RuntimeError(
            "{} Unable to set the drivers for the sensor bridge".format(logPrefix)
        )
    if not sensor_bridge.advance():
        raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))

    are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()
    if not are_joints_ok:
        raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

    # Create the vectors collection server for logging
    vectors_collection_server = blf.yarp_utilities.VectorsCollectionServer()
    if not vectors_collection_server.initialize(
        param_handler.get_group("DATA_LOGGING")
    ):
        raise RuntimeError(
            "{} Unable to initialize the vectors collection server".format(logPrefix)
        )

    vectors_collection_server.populate_metadata(
        "motors::desired::current",
        joints_to_control,
    )
    vectors_collection_server.populate_metadata(
        "motors::control_mode::current",
        joints_to_control,
    )
    vectors_collection_server.finalize_metadata()
    blf.clock().sleep_for(datetime.timedelta(milliseconds=200))

    # Create the ctrl+c handler
    ctrl_c_handler = create_ctrl_c_handler(
        sensor_bridge=sensor_bridge, robot_control=robot_control
    )
    signal.signal(signal.SIGINT, ctrl_c_handler)

    # Check if tqdm is installed
    try:
        from tqdm import tqdm

        is_tqdm_installed = True
    except ImportError:
        blf.log().warn(
            "{} tqdm is not installed, the progress bar will not be shown".format(
                logPrefix
            )
        )
        is_tqdm_installed = False
        last_printed_progress = -1
        # hijack tqdm with a dummy function
        tqdm = lambda x: x

    # Set the time step
    dt = param_handler.get_parameter_datetime("dt")

    # Get the starting position: the trajectory will be generated for each starting position
    number_of_starting_points = param_handler.get_parameter_int(
        "number_of_starting_points"
    )
    starting_positions = np.zeros((number_of_starting_points, len(joints_to_control)))
    for joint_index, joint in enumerate(joints_to_control):
        tmp = np.linspace(
            lower_limits[joint_index],  # + safety_threshold,
            upper_limits[joint_index],  # - safety_threshold,
            number_of_starting_points + 2,
        )
        starting_positions[:, joint_index] = tmp[1:-1]
    blf.log().info(
        "{} Starting positions: \n {}".format(logPrefix, np.rad2deg(starting_positions))
    )

    # Start the data collection
    blf.log().info(
        "{} Waiting for your input, press ENTER to start the data collection".format(
            logPrefix
        )
    )
    input()
    blf.log().info("{} Start".format(logPrefix))

    for counter, starting_position in enumerate(starting_positions):

        # drive the joints to the starting position
        if not robot_control.set_control_mode(
            blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
        if not robot_control.set_references(
            starting_position, blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("{} Unable to set the references".format(logPrefix))

        # wait for the joints to reach the starting position
        is_motion_done = False
        while not is_motion_done:
            is_ok, is_motion_done, _, _ = robot_control.check_motion_done()
            if not is_ok:
                raise RuntimeError(
                    "{} Unable to check if the motion is done".format(logPrefix)
                )
            blf.clock().sleep_for(datetime.timedelta(milliseconds=200))
        blf.clock().sleep_for(datetime.timedelta(seconds=1))
        blf.log().info(
            "{} Joints moved to the starting position #{}, starting the trajectory".format(
                logPrefix, counter + 1
            )
        )

        # Generate the trajectory
        if not sensor_bridge.advance():
            raise RuntimeError(
                "{} Unable to advance the sensor bridge".format(logPrefix)
            )

        if isGazebo:
            are_joints_ok, joint_torques, _ = sensor_bridge.get_joint_torques()
            motor_currents = joint_torques * [
                MotorParameters.k_tau[joint] for joint in joints_to_control
            ]
            if not are_joints_ok:
                raise RuntimeError(
                    "{} Unable to get the joint torques".format(logPrefix)
                )
        else:
            are_joints_ok, motor_currents, _ = sensor_bridge.get_motor_currents()
            if not are_joints_ok:
                raise RuntimeError(
                    "{} Unable to get the motor current".format(logPrefix)
                )

        trajectory = []
        for joint_index in range(len(joints_to_control)):
            trajectory.append(
                trajectory_generator.generate(
                    dt=dt.total_seconds(),
                    initial_current=motor_currents[joint_index]
                    if not (bypass_motor_current_measure[joint_index])
                    else 0,
                    joint_index=joint_index,
                )
            )
        trajectory = np.array(trajectory).T

        # reset control modes for current/torque
        control_modes = [
            blf.robot_interface.YarpRobotControl.Torque
            if isGazebo
            else blf.robot_interface.YarpRobotControl.Current
            for _ in joints_to_control
        ]

        # update motor control modes
        if not robot_control.set_control_mode(control_modes):
            raise RuntimeError("{} Unable to set the control mode".format(logPrefix))

        # reset variables
        is_out_of_safety_limits = [False for _ in joints_to_control]
        position_reference = starting_position
        traj_index = 0
        delta_traj_index = 1

        # loop over the trajectory
        for _ in tqdm(range(trajectory.shape[0])):
            tic = blf.clock().now()

            # get the feedback
            if not sensor_bridge.advance():
                raise RuntimeError(
                    "{} Unable to advance the sensor bridge".format(logPrefix)
                )

            are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()
            if not are_joints_ok:
                raise RuntimeError(
                    "{} Unable to get the joint positions".format(logPrefix)
                )

            # check if the joints are within the safety limits
            # if not, stop the trajectory and set the related control mode to position
            for joint_index, joint in enumerate(joints_to_control):
                if is_out_of_safety_limits[joint_index]:
                    # nothing to do, it was already notified before
                    continue
                if (
                    np.abs(joint_positions[joint_index] - lower_limits[joint_index])
                    < safety_threshold
                ) or (
                    np.abs(joint_positions[joint_index] - upper_limits[joint_index])
                    < safety_threshold
                ):
                    # set the control mode to position
                    control_modes[
                        joint_index
                    ] = blf.robot_interface.YarpRobotControl.Position

                    # set the reference to the current position
                    position_reference[joint_index] = joint_positions[joint_index]

                    # update flag
                    is_out_of_safety_limits[joint_index] = True

                    # set control modes
                    if not robot_control.set_control_mode(control_modes):
                        raise RuntimeError(
                            "{} Unable to set the control mode".format(logPrefix)
                        )

                    blf.log().warn(
                        "{} Joint {} is out of the safety limits, stopping its trajectory and switching to Position control with reference position {}".format(
                            logPrefix, joint, position_reference[joint_index]
                        )
                    )

            # get the current/torque references
            if isGazebo:
                current_reference = trajectory[traj_index] / [
                    MotorParameters.k_tau[joint] for joint in joints_to_control
                ]
            else:
                current_reference = trajectory[traj_index]

            # merge the position and current/torque references depending on the control mode
            reference = np.where(
                is_out_of_safety_limits, position_reference, current_reference
            )

            # send the references motor current (or joint torque for Gazebo)
            if not robot_control.set_references(
                reference, control_modes, joint_positions
            ):
                raise RuntimeError("{} Unable to set the references".format(logPrefix))

            # check if it is time to move to next starting position
            if all(is_out_of_safety_limits):
                blf.log().info(
                    "{} The trajectory is stopped due to all joints exceeding safety limits. Moving to next starting position, if available.".format(
                        logPrefix
                    )
                )
                break

            # log the data
            vectors_collection_server.prepare_data()
            if not vectors_collection_server.clear_data():
                raise RuntimeError("{} Unable to clear the data".format(logPrefix))
            vectors_collection_server.populate_data(
                "motors::desired::current", current_reference
            )
            vectors_collection_server.populate_data(
                "motors::control_mode::current", np.where(is_out_of_safety_limits, 0, 1)
            )
            vectors_collection_server.send_data()

            # sleep
            toc = blf.clock().now()
            delta_time = toc - tic
            if delta_time < dt:
                blf.clock().sleep_for(dt - delta_time)
            else:
                blf.log().debug(
                    "{} The control loop is too slow, real time constraints not satisfied".format(
                        logPrefix
                    )
                )

            # Print progress percentage, only if tqdm is not installed
            if not is_tqdm_installed:

                # Calculate progress
                progress = (traj_index + 1) / trajectory.shape[0] * 100

                # Check if progress is a multiple of 10 and different from last printed progress
                if int(progress) % 10 == 0 and int(progress) != last_printed_progress:
                    print(f"Progress: {int(progress)}%", end="\r")
                    last_printed_progress = int(progress)

            # update the trajectory index
            traj_index = traj_index + delta_traj_index

    blf.log().info("{} Data colection completed".format(logPrefix))

    # get the feedback
    if not sensor_bridge.advance():
        raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))

    are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

    if not are_joints_ok:
        raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

    # set the control mode to position
    if not robot_control.set_control_mode(
        blf.robot_interface.YarpRobotControl.Position
    ):
        raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
    if not robot_control.set_references(
        joint_positions, blf.robot_interface.YarpRobotControl.Position
    ):
        raise RuntimeError("{} Unable to set the references".format(logPrefix))


if __name__ == "__main__":
    network = yarp.Network()
    main()
