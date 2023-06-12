# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xshlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so.0.13.100")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xshlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkSystem.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkSystem.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/BipedalLocomotion/System" TYPE FILE FILES
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/InputPort.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/OutputPort.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Advanceable.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Source.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Sink.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Factory.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/VariablesHandler.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/LinearTask.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/ILinearTaskSolver.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/ILinearTaskFactory.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/ITaskControllerManager.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/IClock.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/StdClock.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Clock.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/SharedResource.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/AdvanceableRunner.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/QuitHandler.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/Barrier.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/WeightProvider.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/include/BipedalLocomotion/System/ConstantWeightProvider.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/tests/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/YarpImplementation/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/System/RosImplementation/cmake_install.cmake")
endif()

