# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/script/blf-balancing-position-control.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/isorrentino/dev/robotology-superbuild/build/install/share/BipedalLocomotionFramework/python/balancing_position_control")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/isorrentino/dev/robotology-superbuild/build/install/share/BipedalLocomotionFramework/python" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/balancing_position_control")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/ergoCubGazeboV1" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/config/robots/ergoCubGazeboV1/blf-balancing-position-control-options.ini")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/ergoCubGazeboV1" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/config/robots/ergoCubGazeboV1/balancing_control")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/ergoCubSN000" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/config/robots/ergoCubSN000/blf-balancing-position-control-options.ini")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/ergoCubSN000" TYPE DIRECTORY FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/balancing-position-control/config/robots/ergoCubSN000/balancing_control")
endif()

