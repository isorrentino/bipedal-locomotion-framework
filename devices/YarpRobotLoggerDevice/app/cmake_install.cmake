# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/ergoCubGazeboV1")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/ergoCubSN000")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/iCubGazeboV2_5")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/iCubGazeboV3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/iCubGenova02")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/iCubGenova04")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/yarp/robots" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/devices/YarpRobotLoggerDevice/app/robots/iCubGenova09")
endif()
