#!/usr/bin/env python3

# This software may be modified and distributed under the terms of the BSD-3-Clause license.

import signal
import numpy as np
import datetime
import h5py
import scipy

import bipedal_locomotion_framework.bindings as blf
import yarp

from datetime import timedelta
from itertools import product

## extend the python path
from pathlib import Path
import sys

sys.path.extend(
    [
        str(
            Path(__file__).parent.resolve()
            / ".."
            / "share"
            / "BipedalLocomotionFramework"
            / "python"
        )
    ]
)


def build_remote_control_board_driver(
    param_handler: blf.parameters_handler.IParametersHandler, local_prefix: str
):
    param_handler.set_parameter_string("local_prefix", local_prefix)
    return blf.robot_interface.construct_remote_control_board_remapper(param_handler)

def generate_trajectory_for_joint(
    initial_current, delta_t, min_delta_current, max_delta_current, delta_current_increment, min_frequency, max_frequency, frequency_increment
):
    trajectory = []
    A = min_delta_current
    f_in = min_frequency
    f_end = max_frequency
    delta_f = frequency_increment

    while A <= max_delta_current:
        print(type(delta_t))
        print(delta_t)

        for f in np.arange(f_in, f_end, delta_f):
            t_max = 2.0 / f
            t = np.arange(0, t_max, delta_t)
            trajectory.append(initial_current + A * np.sin(2 * np.pi * f * t))
        
        A += delta_current_increment
        if f_in == min_frequency:
            f_in = max_frequency
            f_end = min_frequency
            delta_f = -frequency_increment
        else:
            f_in = min_frequency
            f_end = max_frequency
            delta_f = frequency_increment

    # Concatenate trajectories and plot the final trajectory
    trajectory = np.concatenate(trajectory)
    import matplotlib.pyplot as plt
    plt.plot(trajectory)
    plt.show()

    return trajectory


def create_ctrl_c_handler(sensor_bridge, robot_control):
    def ctrl_c_handler(sig, frame):
        print("Ctrl+C pressed. Exiting gracefully.")
        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")

        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("Unable to get the joint positions")

        if not robot_control.set_references(
            joint_positions, blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("Unable to set the references")

        blf.log().info("Sleep for two seconds. Just to be sure the interfaces are on.")
        blf.clock().sleep_for(datetime.timedelta(seconds=2))
        sys.exit(0)

    return ctrl_c_handler


def main():
    param_handler = blf.parameters_handler.YarpParametersHandler()
    if not param_handler.set_from_filename("blf-motor-current-sinusoid-options.ini"):
        raise RuntimeError("Unable to load the parameters")

    sinusoid_group = param_handler.get_group("SINUSOID")
    min_delta_current = sinusoid_group.get_parameter_vector_float("min_delta_current")
    max_delta_current = sinusoid_group.get_parameter_vector_float("max_delta_current")
    delta_current_increment = sinusoid_group.get_parameter_vector_float(
        "delta_current_increment"
    )
    min_frequency = sinusoid_group.get_parameter_vector_float("min_frequency")
    max_frequency = sinusoid_group.get_parameter_vector_float("max_frequency")
    frequency_increment = sinusoid_group.get_parameter_vector_float(
        "frequency_increment"
    )

    # Load joints to control
    robot_control_handler = param_handler.get_group("ROBOT_CONTROL")
    joints_to_control = robot_control_handler.get_parameter_vector_string("joints_list")

    dt = param_handler.get_parameter_datetime("dt")

    poly_drivers = dict()

    poly_drivers["REMOTE_CONTROL_BOARD"] = build_remote_control_board_driver(
        param_handler=robot_control_handler,
        local_prefix="motor_current_sinusoid",
    )
    if not poly_drivers["REMOTE_CONTROL_BOARD"].is_valid():
        raise RuntimeError("Unable to create the remote control board driver")

    blf.log().info("Sleep for two seconds. Just to be sure the interfaces are on.")
    blf.clock().sleep_for(datetime.timedelta(seconds=2))

    robot_control = blf.robot_interface.YarpRobotControl()
    if not robot_control.initialize(robot_control_handler):
        raise RuntimeError("Unable to initialize the robot control")
    if not robot_control.set_driver(poly_drivers["REMOTE_CONTROL_BOARD"].poly):
        raise RuntimeError("Unable to set the driver for the robot control")

    # Create the sensor bridge
    sensor_bridge = blf.robot_interface.YarpSensorBridge()
    sensor_bridge_handler = param_handler.get_group("SENSOR_BRIDGE")
    if not sensor_bridge.initialize(sensor_bridge_handler):
        raise RuntimeError("Unable to initialize the sensor bridge")
    if not sensor_bridge.set_drivers_list(list(poly_drivers.values())):
        raise RuntimeError("Unable to set the drivers for the sensor bridge")

    # set the kindyn computations with the robot state
    if not sensor_bridge.advance():
        raise RuntimeError("Unable to advance the sensor bridge")

    are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()
    if not are_joints_ok:
        raise RuntimeError("Unable to get the joint positions")

    blf.log().info("Waiting for your input")
    blf.log().info("Press enter to start the trajectory")
    input()
    blf.log().info("Starting the trajectory")

    are_joints_ok, joint_torques, _ = sensor_bridge.get_joint_torques()
    if not are_joints_ok:
        raise RuntimeError("Unable to get the joint torques")

    trajectory = []
    for index in range(len(joints_to_control)):
        trajectory.append(generate_trajectory_for_joint(
            joint_torques[index],
            dt.total_seconds(),
            min_delta_current[index],
            max_delta_current[index],
            delta_current_increment[index],
            min_frequency[index],
            max_frequency[index],
            frequency_increment[index],
        ))
        print(joints_to_control[index])

    print(np.array(trajectory).shape)
    print(np.array(trajectory).T.shape)

    trajectory = np.array(trajectory).T

    gear_ratio = [100.0, 160.0]
    ktau = [0.0581, 0.0288]

    # gear_ratio = [100.0, 160.0, 100.0, 100.0, 100.0, 160.0]
    # ktau = [0.1079, 0.0433, 0.0425, 0.0960, 0.0581, 0.0288]

    delta_index = 1

    ctrl_c_handler = create_ctrl_c_handler(sensor_bridge=sensor_bridge, robot_control=robot_control)

    signal.signal(signal.SIGINT, ctrl_c_handler)

    while True:
        tic = blf.clock().now()

        # if sample < 5:
        #     continue

        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")

        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("Unable to get the joint positions")

        torque_ref = (
            trajectory[index] * np.array(ktau) * np.array(gear_ratio)
        )
        print(torque_ref)

        # send the motor current
        if not robot_control.set_references(
            torque_ref, blf.robot_interface.YarpRobotControl.Torque
        ):
            raise RuntimeError("Unable to set the references")

        toc = blf.clock().now()
        delta_time = toc - tic
        if delta_time < dt:
            blf.clock().sleep_for(dt - delta_time)

        index = index + delta_index

        if index == 0:
            delta_index = 1
        elif index == trajectory.shape[1]-1:
            delta_index = -1


if __name__ == "__main__":
    network = yarp.Network()
    main()
