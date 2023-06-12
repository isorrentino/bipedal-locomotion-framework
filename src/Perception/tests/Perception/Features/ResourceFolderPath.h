/**
 * @file ResourceFolderPath.h(.in)
 * @authors Stefano Dafarra
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#ifndef RESOURCE_FOLDERPATH_H_IN
#define RESOURCE_FOLDERPATH_H_IN

#define SOURCE_CONFIG_DIR "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Perception/tests/Perception/Features"

inline std::string getSampleImagePath()
{
    return std::string(SOURCE_CONFIG_DIR) + "/aruco-detector/aruco-sample.png";
}

#endif // RESOURCE_FOLDERPATH_H_IN

