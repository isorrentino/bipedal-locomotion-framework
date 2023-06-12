# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so.0.13.100")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xshlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkPlanners.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkPlanners.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/BipedalLocomotion/Planners" TYPE FILE FILES
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/ConvexHullHelper.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/DCMPlanner.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/TimeVaryingDCMPlanner.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/Spline.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/QuinticSpline.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/SO3Planner.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/SO3Planner.tpp"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/SwingFootPlanner.h"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/CubicSpline.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/tests/cmake_install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xshlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so.0.13.100")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xshlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so"
         RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib/libBipedalLocomotionFrameworkUnicycle.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so"
         OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
         NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libBipedalLocomotionFrameworkUnicycle.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/BipedalLocomotion/Planners" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/src/Planners/include/BipedalLocomotion/Planners/UnicyclePlanner.h")
endif()

