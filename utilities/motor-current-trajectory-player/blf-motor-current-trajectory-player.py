#!/usr/bin/env python3

# This software may be modified and distributed under the terms of the BSD-3-Clause license.

import signal
import numpy as np
import datetime
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
    if not param_handler.set_from_filename(
        "blf-motor-current-trajectory-player-options.ini"
    ):
        raise RuntimeError("Unable to load the parameters")

    # Load joints to control
    robot_control_handler = param_handler.get_group("ROBOT_CONTROL")
    joints_to_control = robot_control_handler.get_parameter_vector_string("joints_list")

    dt = param_handler.get_parameter_datetime("dt")
    dataset = param_handler.get_parameter_string("dataset_trajectory")

    data = scipy.io.loadmat(dataset, squeeze_me=True)

    print(data)
    print(data.keys())

    poly_drivers = dict()

    poly_drivers["REMOTE_CONTROL_BOARD"] = build_remote_control_board_driver(
        param_handler=robot_control_handler,
        local_prefix="motor_current_trajectory_player",
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

    initial_positions = data["joint_position"][0]
    if len(joints_to_control) != len(initial_positions):
        raise RuntimeError(
            "The number of joints to control is different from the number of knots"
        )

    blf.log().info("Waiting for your input")
    blf.log().info("Press enter to move to the initial position")
    input()
    blf.log().info("Positioning")

    if not robot_control.set_references(initial_positions,
                                       blf.robot_interface.YarpRobotControl.Position):
            raise RuntimeError("Unable to set the references")
    
    # Read joint positions and check if the position is reached
    while True:
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")
        
        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("Unable to get the joint positions")
        
        if np.allclose(joint_positions, initial_positions, atol=1e-2):
            break

    # switch to position direct
    blf.log().info("Waiting for your input")
    blf.log().info("Press enter to start the motor current trajectory")
    input()
    blf.log().info("Starting the trajectory")

    # gear_ratio = [
    #     -100.0,
    #     160.0,
    #     -100.0,
    #     100.0,
    #     100.0,
    #     160.0
    # ]
    # ktau = [
    #     0.1079,
    #     0.0433,
    #     0.0425,
    #     0.0960,
    #     0.0581,
    #     0.0288
    # ]

    ctrl_c_handler = create_ctrl_c_handler(sensor_bridge=sensor_bridge, robot_control=robot_control)

    signal.signal(signal.SIGINT, ctrl_c_handler)

    for sample in range(len(data["motor_current_resampled"])):
        tic = blf.clock().now()

        # if sample < 5:
        #     continue

        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")
        
        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()

        if not are_joints_ok:
            raise RuntimeError("Unable to get the joint positions")
        
        current_ref = np.array(data["motor_current_resampled"][sample])

        print(current_ref)

        # send the motor current
        if not robot_control.set_references(
            current_ref, blf.robot_interface.YarpRobotControl.Current
        ):
            raise RuntimeError("Unable to set the references")
        
        toc = blf.clock().now()
        delta_time = toc - tic
        if delta_time < dt:
            blf.clock().sleep_for(dt - delta_time)

        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")
        
        are_joints_ok, joint_positions, _ = sensor_bridge.get_joint_positions()
        
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


if __name__ == "__main__":
    network = yarp.Network()
    main()
