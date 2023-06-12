set(BipedalLocomotionFramework_VERSION 0.13.100)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was BipedalLocomotionFrameworkConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

####################################################################################

#### Expanded from @PACKAGE_DEPENDENCIES@ by install_basic_package_files() ####

include(CMakeFindDependencyMacro)
set(CMAKE_MODULE_PATH_BK_BipedalLocomotionFramework ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${PACKAGE_PREFIX_DIR}/share/BipedalLocomotionFramework/cmake)
find_package(iDynTree 3.0.0)
find_package(Eigen3 3.2.92)
find_package(spdlog 1.5.0)
find_package(YARP 3.7.0 COMPONENTS companion profiler dev os idl_tools)
find_package(Qhull 8.0.0)
find_package(cppad)
find_package(manif 0.0.4)
find_package(matioCpp)
find_package(LieGroupControllers 0.2.0)
find_package(OpenCV)
find_package(PCL)
find_package(tomlplusplus 3.0.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_BK_BipedalLocomotionFramework})

###############################################################################


if(NOT TARGET BipedalLocomotion::ContinuousDynamicalSystem)
  include("${CMAKE_CURRENT_LIST_DIR}/BipedalLocomotionFrameworkTargets.cmake")
endif()

# Compatibility
get_property(BipedalLocomotionFramework_ContinuousDynamicalSystem_INCLUDE_DIR TARGET BipedalLocomotion::ContinuousDynamicalSystem PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_TextLoggingYarpImplementation_INCLUDE_DIR TARGET BipedalLocomotion::TextLoggingYarpImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_TextLogging_INCLUDE_DIR TARGET BipedalLocomotion::TextLogging PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_GenericContainer_INCLUDE_DIR TARGET BipedalLocomotion::GenericContainer PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_YarpUtilities_INCLUDE_DIR TARGET BipedalLocomotion::YarpUtilities PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_VectorsCollection_INCLUDE_DIR TARGET BipedalLocomotion::VectorsCollection PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ParametersHandlerYarpImplementation_INCLUDE_DIR TARGET BipedalLocomotion::ParametersHandlerYarpImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ParametersHandlerTomlImplementation_INCLUDE_DIR TARGET BipedalLocomotion::ParametersHandlerTomlImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ParametersHandler_INCLUDE_DIR TARGET BipedalLocomotion::ParametersHandler PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_CommonConversions_INCLUDE_DIR TARGET BipedalLocomotion::CommonConversions PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ManifConversions_INCLUDE_DIR TARGET BipedalLocomotion::ManifConversions PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_matioCppConversions_INCLUDE_DIR TARGET BipedalLocomotion::matioCppConversions PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Contacts_INCLUDE_DIR TARGET BipedalLocomotion::Contacts PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ContactDetectors_INCLUDE_DIR TARGET BipedalLocomotion::ContactDetectors PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Estimators_INCLUDE_DIR TARGET BipedalLocomotion::Estimators PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_FloatingBaseEstimators_INCLUDE_DIR TARGET BipedalLocomotion::FloatingBaseEstimators PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_SystemYarpImplementation_INCLUDE_DIR TARGET BipedalLocomotion::SystemYarpImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_System_INCLUDE_DIR TARGET BipedalLocomotion::System PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Planners_INCLUDE_DIR TARGET BipedalLocomotion::Planners PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Unicycle_INCLUDE_DIR TARGET BipedalLocomotion::Unicycle PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_ContactModels_INCLUDE_DIR TARGET BipedalLocomotion::ContactModels PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_AutoDiffCppAD_INCLUDE_DIR TARGET BipedalLocomotion::AutoDiffCppAD PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_RobotInterfaceYarpImplementation_INCLUDE_DIR TARGET BipedalLocomotion::RobotInterfaceYarpImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_PerceptionInterfaceYarpImplementation_INCLUDE_DIR TARGET BipedalLocomotion::PerceptionInterfaceYarpImplementation PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_RobotInterface_INCLUDE_DIR TARGET BipedalLocomotion::RobotInterface PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_PerceptionInterface_INCLUDE_DIR TARGET BipedalLocomotion::PerceptionInterface PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Math_INCLUDE_DIR TARGET BipedalLocomotion::Math PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_TSID_INCLUDE_DIR TARGET BipedalLocomotion::TSID PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_PerceptionFeatures_INCLUDE_DIR TARGET BipedalLocomotion::PerceptionFeatures PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_IK_INCLUDE_DIR TARGET BipedalLocomotion::IK PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_SimplifiedModelControllers_INCLUDE_DIR TARGET BipedalLocomotion::SimplifiedModelControllers PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(BipedalLocomotionFramework_Framework_INCLUDE_DIR TARGET BipedalLocomotion::Framework PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

set(BipedalLocomotionFramework_LIBRARIES BipedalLocomotion::ContinuousDynamicalSystem BipedalLocomotion::TextLoggingYarpImplementation BipedalLocomotion::TextLogging BipedalLocomotion::GenericContainer BipedalLocomotion::YarpUtilities BipedalLocomotion::VectorsCollection BipedalLocomotion::ParametersHandlerYarpImplementation BipedalLocomotion::ParametersHandlerTomlImplementation BipedalLocomotion::ParametersHandler BipedalLocomotion::CommonConversions BipedalLocomotion::ManifConversions BipedalLocomotion::matioCppConversions BipedalLocomotion::Contacts BipedalLocomotion::ContactDetectors BipedalLocomotion::Estimators BipedalLocomotion::FloatingBaseEstimators BipedalLocomotion::SystemYarpImplementation BipedalLocomotion::System BipedalLocomotion::Planners BipedalLocomotion::Unicycle BipedalLocomotion::ContactModels BipedalLocomotion::AutoDiffCppAD BipedalLocomotion::RobotInterfaceYarpImplementation BipedalLocomotion::PerceptionInterfaceYarpImplementation BipedalLocomotion::RobotInterface BipedalLocomotion::PerceptionInterface BipedalLocomotion::Math BipedalLocomotion::TSID BipedalLocomotion::PerceptionFeatures BipedalLocomotion::IK BipedalLocomotion::SimplifiedModelControllers BipedalLocomotion::Framework)
set(BipedalLocomotionFramework_INCLUDE_DIRS "${BipedalLocomotionFramework_ContinuousDynamicalSystem_INCLUDE_DIR}" "${BipedalLocomotionFramework_TextLoggingYarpImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_TextLogging_INCLUDE_DIR}" "${BipedalLocomotionFramework_GenericContainer_INCLUDE_DIR}" "${BipedalLocomotionFramework_YarpUtilities_INCLUDE_DIR}" "${BipedalLocomotionFramework_VectorsCollection_INCLUDE_DIR}" "${BipedalLocomotionFramework_ParametersHandlerYarpImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_ParametersHandlerTomlImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_ParametersHandler_INCLUDE_DIR}" "${BipedalLocomotionFramework_CommonConversions_INCLUDE_DIR}" "${BipedalLocomotionFramework_ManifConversions_INCLUDE_DIR}" "${BipedalLocomotionFramework_matioCppConversions_INCLUDE_DIR}" "${BipedalLocomotionFramework_Contacts_INCLUDE_DIR}" "${BipedalLocomotionFramework_ContactDetectors_INCLUDE_DIR}" "${BipedalLocomotionFramework_Estimators_INCLUDE_DIR}" "${BipedalLocomotionFramework_FloatingBaseEstimators_INCLUDE_DIR}" "${BipedalLocomotionFramework_SystemYarpImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_System_INCLUDE_DIR}" "${BipedalLocomotionFramework_Planners_INCLUDE_DIR}" "${BipedalLocomotionFramework_Unicycle_INCLUDE_DIR}" "${BipedalLocomotionFramework_ContactModels_INCLUDE_DIR}" "${BipedalLocomotionFramework_AutoDiffCppAD_INCLUDE_DIR}" "${BipedalLocomotionFramework_RobotInterfaceYarpImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_PerceptionInterfaceYarpImplementation_INCLUDE_DIR}" "${BipedalLocomotionFramework_RobotInterface_INCLUDE_DIR}" "${BipedalLocomotionFramework_PerceptionInterface_INCLUDE_DIR}" "${BipedalLocomotionFramework_Math_INCLUDE_DIR}" "${BipedalLocomotionFramework_TSID_INCLUDE_DIR}" "${BipedalLocomotionFramework_PerceptionFeatures_INCLUDE_DIR}" "${BipedalLocomotionFramework_IK_INCLUDE_DIR}" "${BipedalLocomotionFramework_SimplifiedModelControllers_INCLUDE_DIR}" "${BipedalLocomotionFramework_Framework_INCLUDE_DIR}")
list(REMOVE_DUPLICATES BipedalLocomotionFramework_INCLUDE_DIRS)


