# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.16.3)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(tiny_process_library-populate NONE)

include(ExternalProject)
ExternalProject_Add(tiny_process_library-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://gitlab.com/eidheim/tiny-process-library.git" "GIT_TAG" "v2.0.4"
                    SOURCE_DIR          "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/_deps/tiny_process_library-src"
                    BINARY_DIR          "/home/isorrentino/dev/robotology-superbuild/src/bipedal-locomotion-framework/_deps/tiny_process_library-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
