/**
 * @file ForwardEuler.tpp
 * @authors Giulio Romualdi
 * @copyright 2020 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef BIPEDAL_LOCOMOTION_SYSTEM_FORWARD_EULER_TPP
#define BIPEDAL_LOCOMOTION_SYSTEM_FORWARD_EULER_TPP

#include <BipedalLocomotion/System/ForwardEuler.h>

namespace BipedalLocomotion
{
namespace System
{

template <typename DynamicalSystemDerived>
bool ForwardEuler<DynamicalSystemDerived>::oneStepIntegration(double t0, double dT)
{

    if (this->m_dynamicalSystem == nullptr)
    {
        std::cerr << "[ForwardEuler::oneStepIntegration] Please specify the dynamical system."
                  << std::endl;
        return false;
    }

    if (!this->m_dynamicalSystem->dynamics(t0, this->m_computationalBufferStateDerivative))
    {
        std::cerr << "[ForwardEuler::oneStepIntegration] Unable to compute the system dynamics."
                  << std::endl;
        return false;
    }

    // x = x0 + dT * dx
    this->m_computationalBufferState = this->m_dynamicalSystem->getState();
    this->addArea(this->m_computationalBufferStateDerivative, dT, this->m_computationalBufferState);

    if (!this->m_dynamicalSystem->setState(this->m_computationalBufferState))
    {
        std::cerr << "[ForwardEuler::oneStepIntegration] Unable to set the new state in the "
                     "dynamical system."
                  << std::endl;
        return false;
    }

    return true;
}

} // namespace System
} // namespace BipedalLocomotion

#endif // BIPEDAL_LOCOMOTION_SYSTEM_FORWARD_EULER_TPP
