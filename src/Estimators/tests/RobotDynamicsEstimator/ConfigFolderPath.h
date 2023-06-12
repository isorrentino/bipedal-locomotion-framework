/**
 * @file FolderPath.h(.in)
 * @authors Stefano Dafarra
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef CONFIG_FOLDERPATH_H_IN
#define CONFIG_FOLDERPATH_H_IN

#define SOURCE_CONFIG_DIR "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Estimators/tests/RobotDynamicsEstimator"

inline std::string getConfigPath()
{
    return std::string(SOURCE_CONFIG_DIR) + "/config/config.ini";
}

#endif // CONFIG_FOLDERPATH_H_IN
