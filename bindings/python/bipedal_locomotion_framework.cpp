/**
 * @file bipedal_locomotion.cpp.in
 * @authors Giulio Romualdi, Diego Ferigo
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

// clang-format off
#include <pybind11/pybind11.h>

#include <BipedalLocomotion/bindings/Math/Module.h>
#include <BipedalLocomotion/bindings/ParametersHandler/Module.h>
#include <BipedalLocomotion/bindings/TextLogging/Module.h>
#include <BipedalLocomotion/bindings/TextLogging/TextLogging.h>




#include <BipedalLocomotion/bindings/TextLogging/YarpModule.h>



#include <BipedalLocomotion/bindings/ParametersHandler/YarpModule.h>



#include <BipedalLocomotion/bindings/ParametersHandler/TomlModule.h>



#include <BipedalLocomotion/bindings/System/Module.h>
#include <BipedalLocomotion/bindings/System/Clock.h>



#include <BipedalLocomotion/bindings/System/YarpModule.h>





#include <BipedalLocomotion/bindings/Contacts/Module.h>



#include <BipedalLocomotion/bindings/Planners/Module.h>



#include <BipedalLocomotion/bindings/Planners/UnicycleModule.h>



#include <BipedalLocomotion/bindings/RobotInterface/Module.h>



#include <BipedalLocomotion/bindings/FloatingBaseEstimators/Module.h>



#include <BipedalLocomotion/bindings/IK/Module.h>



#include <BipedalLocomotion/bindings/TSID/Module.h>



#include <BipedalLocomotion/bindings/Conversions/Module.h>



#include <BipedalLocomotion/bindings/YarpUtilities/Module.h>



#include <BipedalLocomotion/bindings/ContinuousDynamicalSystem/Module.h>



#include <BipedalLocomotion/bindings/RobotDynamicsEstimator/Module.h>


// Create the Python module
PYBIND11_MODULE(bindings, m)
{
    namespace py = ::pybind11;
    using namespace BipedalLocomotion;

    
    py::module::import("manifpy");
    

    m.doc() = "BipedalLocomotionFramework bindings";

    py::module textLoggingModule = m.def_submodule("text_logging");
    bindings::TextLogging::CreateModule(textLoggingModule);
    bindings::CreateLogger(m);

    

    
    bindings::TextLogging::CreateYarpModule(textLoggingModule);
    

    py::module parametersHandlerModule = m.def_submodule("parameters_handler");
    bindings::ParametersHandler::CreateModule(parametersHandlerModule);

    
    bindings::ParametersHandler::CreateYarpModule(parametersHandlerModule);
    

    
    bindings::ParametersHandler::CreateTomlModule(parametersHandlerModule);
    

    py::module mathModule = m.def_submodule("math");
    bindings::Math::CreateModule(mathModule);

    
    py::module systemModule = m.def_submodule("system");
    bindings::System::CreateModule(systemModule);
    bindings::CreateClock(m);
    

    
    bindings::System::CreateYarpModule(systemModule);
    

    

    
    py::module contactsModule = m.def_submodule("contacts");
    bindings::Contacts::CreateModule(contactsModule);
    

    
    py::module plannersModule = m.def_submodule("planners");
    bindings::Planners::CreateModule(plannersModule);
    

    
    bindings::Planners::CreateUnicycleModule(plannersModule);
    

    
    py::module robotInterfaceModule = m.def_submodule("robot_interface");
    bindings::RobotInterface::CreateModule(robotInterfaceModule);
    

    
    py::module floatingBaseEstimatorModule = m.def_submodule("floating_base_estimators");
    bindings::FloatingBaseEstimators::CreateModule(floatingBaseEstimatorModule);
    

    
    py::module ikModule = m.def_submodule("ik");
    bindings::IK::CreateModule(ikModule);
    

    
    py::module tsidModule = m.def_submodule("tsid");
    bindings::TSID::CreateModule(tsidModule);
    

    
    py::module conversionsModule = m.def_submodule("conversions");
    bindings::Conversions::CreateModule(conversionsModule);
    

    
    py::module yarpUtilitiesModule = m.def_submodule("yarp_utilities");
    bindings::YarpUtilities::CreateModule(yarpUtilitiesModule);
    

    
    py::module continuousDynamicalSystemModule = m.def_submodule("continuous_dynamical_system");
    bindings::ContinuousDynamicalSystem::CreateModule(continuousDynamicalSystemModule);
    

    
    py::module robotDynamicsEstimatorModule = m.def_submodule("robot_dynamics_estimator");
    bindings::RobotDynamicsEstimator::CreateModule(robotDynamicsEstimatorModule);
    
}
