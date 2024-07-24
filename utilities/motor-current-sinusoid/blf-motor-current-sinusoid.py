#!/usr/bin/env python3

# This software may be modified and distributed under the terms of the BSD-3-Clause license.

import signal
import numpy as np
import datetime

import bipedal_locomotion_framework as blf
import yarp

## extend the python path
from pathlib import Path
import sys
import os

logPrefix = "[MotorCurrentSinusoidApplication]"

class MotorParameters:
    # k_tau[A/Nm] includes the gear ratio
    k_tau = {
        "l_hip_roll":      94 * 1e-3,
        "l_hip_pitch":     64 * 1e-3,
        "l_hip_yaw":      150 * 1e-3,
        "l_knee":          64 * 1e-3,
        "l_ankle_pitch":   64 * 1e-3,
        "l_ankle_roll":   177 * 1e-3,
        "r_hip_roll":      94 * 1e-3,  
        "r_hip_pitch":     64 * 1e-3,
        "r_hip_yaw":      150 * 1e-3,
        "r_knee":          64 * 1e-3,
        "r_ankle_pitch":   64 * 1e-3,
        "r_ankle_roll":   177 * 1e-3,
    }

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

    while np.abs(A) <= np.abs(max_delta_current):

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

    # Concatenate trajectories 
    trajectory = np.concatenate(trajectory)
    # # Plot the trajectory
    # import matplotlib.pyplot as plt
    # plt.plot(trajectory)
    # plt.show()

    return trajectory


def create_ctrl_c_handler(sensor_bridge, robot_control):
    def ctrl_c_handler(sig, frame):
        blf.log().info("{} Ctrl+C pressed. Exiting gracefully.".format(logPrefix))
        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))

        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

        # set the control mode to position
        if not robot_control.set_control_mode(blf.robot_interface.YarpRobotControl.Position):
            raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
        if not robot_control.set_references(
            joint_positions, blf.robot_interface.YarpRobotControl.Position
        ):
            raise RuntimeError("{} Unable to set the references".format(logPrefix))

        blf.log().info("{} Sleep for two seconds. Just to be sure the interfaces are on.".format(logPrefix))
        blf.clock().sleep_for(datetime.timedelta(seconds=2))
        sys.exit(0)

    return ctrl_c_handler


def main():

    isGazebo = False
    if "gazebo" in (os.environ.get("YARP_ROBOT_NAME")).lower():
        isGazebo = True

    param_handler = blf.parameters_handler.YarpParametersHandler()

    param_file = Path(__file__).parent / "config" / "robots"/ os.environ.get("YARP_ROBOT_NAME") / "blf-motor-current-sinusoid-options.ini"

    if not param_handler.set_from_filename(str(param_file)):
        raise RuntimeError("{} Unable to load the parameters".format(logPrefix))

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

    # Load joints to control and build the control board driver
    robot_control_handler = param_handler.get_group("ROBOT_CONTROL")
    joints_to_control = robot_control_handler.get_parameter_vector_string("joints_list")
    blf.log().info("{} Joints to control: {}".format(logPrefix, joints_to_control))

    poly_drivers = dict()

    poly_drivers["REMOTE_CONTROL_BOARD"] = build_remote_control_board_driver(
        param_handler=robot_control_handler,
        local_prefix="motor_current_sinusoid",
    )
    if not poly_drivers["REMOTE_CONTROL_BOARD"].is_valid():
        raise RuntimeError("{} Unable to create the remote control board driver".format(logPrefix))

    blf.log().info("{} Sleep for two seconds. Just to be sure the interfaces are on.".format(logPrefix))
    blf.clock().sleep_for(datetime.timedelta(seconds=2))

    robot_control = blf.robot_interface.YarpRobotControl()
    if not robot_control.initialize(robot_control_handler):
        raise RuntimeError("{} Unable to initialize the robot control".format(logPrefix))
    if not robot_control.set_driver(poly_drivers["REMOTE_CONTROL_BOARD"].poly):
        raise RuntimeError("{} Unable to set the driver for the robot control".format(logPrefix))
    
    # Get joint limits
    lower_limits, upper_limits = robot_control.get_joint_limits()

    # Create the sensor bridge
    sensor_bridge = blf.robot_interface.YarpSensorBridge()
    sensor_bridge_handler = param_handler.get_group("SENSOR_BRIDGE")
    if not sensor_bridge.initialize(sensor_bridge_handler):
        raise RuntimeError("{} Unable to initialize the sensor bridge".format(logPrefix))
    if not sensor_bridge.set_drivers_list(list(poly_drivers.values())):
        raise RuntimeError("{} Unable to set the drivers for the sensor bridge".format(logPrefix))
    if not sensor_bridge.advance():
        raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))
    
    are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()
    if not are_joints_ok:
        raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

    if isGazebo:
        are_joints_ok, joint_torques, _ = sensor_bridge.get_joint_torques()
        motor_currents = joint_torques * [MotorParameters.k_tau[joint] for joint in joints_to_control]
        if not are_joints_ok:
            raise RuntimeError("{} Unable to get the joint torques".format(logPrefix))
    else:
        are_joints_ok, motor_currents, _ = sensor_bridge.get_motor_currents()
        if not are_joints_ok:
            raise RuntimeError("{} Unable to get the motor current".format(logPrefix))        

    # Set the time step
    dt = param_handler.get_parameter_datetime("dt")

    # Generate the trajectory
    trajectory = []
    for joint_index in range(len(joints_to_control)):
        trajectory.append(generate_trajectory_for_joint(
            motor_currents[joint_index],
            dt.total_seconds(),
            min_delta_current[joint_index],
            max_delta_current[joint_index],
            delta_current_increment[joint_index],
            min_frequency[joint_index],
            max_frequency[joint_index],
            frequency_increment[joint_index],
        ))
    trajectory = np.array(trajectory).T
    blf.log().debug("{} Generated trajectory shape: {}".format(logPrefix, np.array(trajectory).T.shape))

    # Create the vectors collection server for logging
    vectors_collection_server = blf.yarp_utilities.VectorsCollectionServer()
    if not vectors_collection_server.initialize(
        param_handler.get_group("DATA_LOGGING")
    ):
        raise RuntimeError("{} Unable to initialize the vectors collection server".format(logPrefix))

    vectors_collection_server.populate_metadata(
        "motors::desired::current",
        joints_to_control,
    )
    vectors_collection_server.finalize_metadata()

    # Create the ctrl+c handler
    ctrl_c_handler = create_ctrl_c_handler(sensor_bridge=sensor_bridge, robot_control=robot_control)
    signal.signal(signal.SIGINT, ctrl_c_handler)

    # Start the trajectory
    blf.log().info("{} Waiting for your input, press ENTER to start the trajectory".format(logPrefix))
    input()
    blf.log().info("{} Starting the trajectory".format(logPrefix))

    traj_index = 0
    delta_traj_index = 1

    try:
        from tqdm import tqdm
        is_tqdm_installed = True
    except ImportError:
        blf.log().warn("{} tqdm is not installed, the progress bar will not be shown".format(logPrefix))
        is_tqdm_installed = False
        last_printed_progress = -1
        # hijack tqdm with a dummy function
        tqdm = lambda x: x

    for _ in tqdm(range(trajectory.shape[0])):
        tic = blf.clock().now()

        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))

        current_ref = trajectory[traj_index]

        # send the motor current (or joint torque for Gazebo)
        if isGazebo:
            if traj_index == 0:
                if not robot_control.set_control_mode(blf.robot_interface.YarpRobotControl.Torque):
                    raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
                
            if not robot_control.set_references(
                current_ref/[MotorParameters.k_tau[joint] for joint in joints_to_control], blf.robot_interface.YarpRobotControl.Torque
            ):
                raise RuntimeError("{} Unable to set the references".format(logPrefix))
        else:
            if traj_index == 0:
                if not robot_control.set_control_mode(blf.robot_interface.YarpRobotControl.Current):
                    raise RuntimeError("{} Unable to set the control mode".format(logPrefix))

            if not robot_control.set_references(
                current_ref, blf.robot_interface.YarpRobotControl.Current
            ):
                raise RuntimeError("{} Unable to set the references".format(logPrefix))

        # log the data
        vectors_collection_server.prepare_data()
        if not vectors_collection_server.clear_data():
            raise RuntimeError("{} Unable to clear the data".format(logPrefix))
        vectors_collection_server.populate_data("motors::desired::current", current_ref)
        vectors_collection_server.send_data()

        # sleep
        toc = blf.clock().now()
        delta_time = toc - tic
        if delta_time < dt:
            blf.clock().sleep_for(dt - delta_time)
        else:
            blf.log().warning("{} The control loop is too slow, real time constraints not satisfied".format(logPrefix))

        # Print progress percentage, only if tqdm is not installed
        if not is_tqdm_installed:
            
            # Calculate progress
            progress = (traj_index + 1) / trajectory.shape[0] * 100
            
            # Check if progress is a multiple of 10 and different from last printed progress
            if int(progress) % 10 == 0 and int(progress) != last_printed_progress:
                print(f'Progress: {int(progress)}%', end='\r')
                last_printed_progress = int(progress)

        # update the trajectory index
        traj_index = traj_index + delta_traj_index
    
    blf.log().info("{} The trajectory is finished".format(logPrefix))

    # get the feedback
    if not sensor_bridge.advance():
        raise RuntimeError("{} Unable to advance the sensor bridge".format(logPrefix))

    are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

    if not are_joints_ok:
        raise RuntimeError("{} Unable to get the joint positions".format(logPrefix))

    # set the control mode to position
    if not robot_control.set_control_mode(blf.robot_interface.YarpRobotControl.Position):
        raise RuntimeError("{} Unable to set the control mode".format(logPrefix))
    if not robot_control.set_references(
        joint_positions, blf.robot_interface.YarpRobotControl.Position
    ):
        raise RuntimeError("{} Unable to set the references".format(logPrefix))

if __name__ == "__main__":
    network = yarp.Network()
    main()
