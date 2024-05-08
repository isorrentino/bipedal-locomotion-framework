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
    if not param_handler.set_from_filename("blf-motor-current-ramp-options.ini"):
        raise RuntimeError("Unable to load the parameters")

    ramp_group = param_handler.get_group("RAMP")
    max_delta_current = ramp_group.get_parameter_vector_float("max_delta_current_increment")
    delta_current_increment = ramp_group.get_parameter_vector_float(
        "delta_current_increment"
    )
    delta_time = ramp_group.get_parameter_float("delta_time")

    # Load joints to control
    robot_control_handler = param_handler.get_group("ROBOT_CONTROL")
    joints_to_control = robot_control_handler.get_parameter_vector_string("joints_list")

    dt = param_handler.get_parameter_datetime("dt")

    poly_drivers = dict()

    poly_drivers["REMOTE_CONTROL_BOARD"] = build_remote_control_board_driver(
        param_handler=robot_control_handler,
        local_prefix="motor_current_ramp",
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

    are_joints_ok, motor_currents, _ = sensor_bridge.get_motor_currents()
    if not are_joints_ok:
        raise RuntimeError("Unable to get the joint torques")
    
    # motor_currents = motor_currents * 0.0
    motor_currents_init = motor_currents

    vectors_collection_server = blf.yarp_utilities.VectorsCollectionServer()
    if not vectors_collection_server.initialize(
        param_handler.get_group("DATA_LOGGING")
    ):
        raise RuntimeError("Unable to initialize the vectors collection server")

    vectors_collection_server.populate_metadata(
        "motors::desired::current",
        param_handler.get_group("ROBOT_CONTROL").get_parameter_vector_string(
            "joints_list"
        ),
    )
    vectors_collection_server.finalize_metadata()

    gear_ratio = [100.0, 160.0]
    ktau = [0.0581, 0.0288]

    # gear_ratio = [100.0, 160.0, 100.0, 100.0, 100.0, 160.0]
    # ktau = [0.1079, 0.0433, 0.0425, 0.0960, 0.0581, 0.0288]

    ctrl_c_handler = create_ctrl_c_handler(sensor_bridge=sensor_bridge, robot_control=robot_control)

    signal.signal(signal.SIGINT, ctrl_c_handler)

    delta_time_ramp = 0.0

    end_loop = False
    
    motor_currents = motor_currents + delta_current_increment
    for i in range(len(motor_currents)):
        if np.abs(motor_currents[i] - motor_currents_init[i]) > max_delta_current[i]:
            end_loop = True

    while not end_loop:
        tic = blf.clock().now()

        # get the feedback
        if not sensor_bridge.advance():
            raise RuntimeError("Unable to advance the sensor bridge")

        # send the motor current
        if not robot_control.set_references(
            motor_currents, blf.robot_interface.YarpRobotControl.Current
        ):
            raise RuntimeError("Unable to set the references")
        
        vectors_collection_server.prepare_data()

        if not vectors_collection_server.clear_data():
            raise RuntimeError("Unable to clear the data")

        vectors_collection_server.populate_data("motors::desired::current", motor_currents)

        vectors_collection_server.send_data()

        toc = blf.clock().now()
        delta_time_loop = toc - tic
        if delta_time_loop < dt:
            blf.clock().sleep_for(dt - delta_time_loop)

        delta_time_ramp = delta_time_ramp + dt.total_seconds()

        print("delta time")
        print(delta_time_ramp)

        print("desired current")
        print(motor_currents)
        
        if delta_time_ramp > delta_time:
            delta_time_ramp = 0.0
            motor_currents = motor_currents + delta_current_increment
            for i in range(len(motor_currents)):
                if np.abs(motor_currents[i] - motor_currents_init[i]) > max_delta_current[i]:
                    print("motor current")
                    print(motor_currents)
                    print("motor current init")
                    print(motor_currents_init)
                    print("max delta current")
                    print(max_delta_current)
                    end_loop = True
    
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
