# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/RobotDynamicsEstimatorDevice

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/isorrentino/dev/robotology-superbuild/build/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xconfigurationx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/plugins" TYPE FILE RENAME "robot_dynamics_estimator_device.ini" FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/CMakeFiles/robot_dynamics_estimator_device_for_install.ini")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:/home/isorrentino/dev/bayes-filters-lib/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/yarp" TYPE MODULE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/yarp/RobotDynamicsEstimatorDevice.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:/home/isorrentino/dev/bayes-filters-lib/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:/home/isorrentino/dev/bayes-filters-lib/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/yarp/RobotDynamicsEstimatorDevice.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/plugins" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/RobotDynamicsEstimatorDevice/CMakeFiles/RobotDynamicsEstimatorDevice.ini")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/RobotDynamicsEstimatorDevice/app/cmake_install.cmake")
endif()
