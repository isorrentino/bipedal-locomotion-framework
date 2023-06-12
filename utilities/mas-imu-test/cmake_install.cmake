# Install script for directory: /home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/mas-imu-test

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xbinx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/blf-mas-imu-test-0.13.100"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/blf-mas-imu-test"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/bin/blf-mas-imu-test-0.13.100"
    "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/bin/blf-mas-imu-test"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/blf-mas-imu-test-0.13.100"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/blf-mas-imu-test"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib:"
           NEW_RPATH "\$ORIGIN/:\$ORIGIN/../lib:/home/isorrentino/dev/robotology-superbuild/build/install/lib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/iCubGazeboV2_5" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/mas-imu-test/app/robots/iCubGazeboV2_5/MasImuTestConfig.ini")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/BipedalLocomotionFramework/robots/iCubGenova04" TYPE FILE FILES "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/utilities/mas-imu-test/app/robots/iCubGenova04/MasImuTestConfig.ini")
endif()

