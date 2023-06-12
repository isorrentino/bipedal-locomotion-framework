/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-FileCopyrightText: 2006-2010 RobotCub Consortium
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <yarp/conf/api.h>
#include <yarp/os/SharedLibraryClass.h>
#include <yarp/dev/Drivers.h>
#include </home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/RobotDynamicsEstimatorDevice/include/BipedalLocomotion/RobotDynamicsEstimatorDevice.h>

#ifdef YARP_STATIC_PLUGIN
#  define YARP_PLUGIN_IMPORT
#  define YARP_PLUGIN_EXPORT
#else
#  define YARP_PLUGIN_IMPORT YARP_HELPER_DLL_IMPORT
#  define YARP_PLUGIN_EXPORT YARP_HELPER_DLL_EXPORT
#endif

#ifdef YARP_STATIC_PLUGIN
YARP_PLUGIN_EXPORT void add_owned_RobotDynamicsEstimatorDevice(const char *owner) {
    yarp::dev::DriverCreator *factory =
        new yarp::dev::DriverCreatorOf<BipedalLocomotion::RobotDynamicsEstimatorDevice>("RobotDynamicsEstimatorDevice",
                "",
                "BipedalLocomotion::RobotDynamicsEstimatorDevice");
    yarp::dev::Drivers::factory().add(factory); // hand factory over to YARP
}
#endif

YARP_DEFINE_SHARED_SUBCLASS(RobotDynamicsEstimatorDevice, BipedalLocomotion::RobotDynamicsEstimatorDevice, yarp::dev::DeviceDriver)
