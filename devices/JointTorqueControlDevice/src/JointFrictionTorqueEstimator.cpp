/**
 * @file JointFrictionTorqueEstimator.cpp
 * @authors Ines Sorrentino
 * @copyright 2023 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <memory>
#include <deque>
#include <string>

#include <Eigen/Dense>

// onnxruntime
#include <onnxruntime_cxx_api.h>

#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/System/VariablesHandler.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

#include <BipedalLocomotion/JointFrictionTorqueEstimator.h>


struct JointFrictionTorqueEstimator::Impl
{
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memoryInfo;

    std::deque<float> jointPositionBuffer;
    std::deque<float> motorPositionBuffer;

    size_t historyLength;

    struct DataStructured
    {
        std::vector<float> rawData;

        Ort::Value tensor{nullptr};
        std::array<int64_t, 2> shape;
    };

    DataStructured structuredInput;
    DataStructured structuredOutput;

    Impl()
        : memoryInfo(::Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
    {
    }
};

JointFrictionTorqueEstimator::JointFrictionTorqueEstimator()
{
    m_pimpl = std::make_unique<JointFrictionTorqueEstimator::Impl>();
}

JointFrictionTorqueEstimator::~JointFrictionTorqueEstimator() = default;

bool JointFrictionTorqueEstimator::initialize(const std::string& networkModelPath,
                                              const std::size_t historyLength,
                                              const int intraOpNumThreads = 1,
                                              const int interOpNumThreads = 1)
{
    std::basic_string<ORTCHAR_T> networkModelPathAsOrtString(networkModelPath.begin(),
                                                             networkModelPath.end());
    Ort::SessionOptions sessionOptions;

	// Set the number of intra-op threads
	if (intraOpNumThreads > 0)
    {
        sessionOptions.SetIntraOpNumThreads(intraOpNumThreads);
    }
	// Set the number of inter-op threads
	if (interOpNumThreads > 0)
    {
        sessionOptions.SetInterOpNumThreads(interOpNumThreads);
    }
	m_pimpl->session = std::make_unique<Ort::Session>(m_pimpl->env,
                                                      networkModelPathAsOrtString.c_str(),
                                                      sessionOptions);


    if (m_pimpl->session == nullptr)
    {
        std::cerr << "Impossible to load the model from the file: " << networkModelPath
                  << std::endl;
        return false;
    }

    m_pimpl->historyLength = historyLength;

    // format the input
    m_pimpl->structuredInput.rawData.resize(historyLength * 2);
    m_pimpl->structuredInput.shape[0] = 1; // batch
    m_pimpl->structuredInput.shape[1] = historyLength * 2;

    // create tensor required by onnx
    m_pimpl->structuredInput.tensor
        = Ort::Value::CreateTensor<float>(m_pimpl->memoryInfo,
                                          m_pimpl->structuredInput.rawData.data(),
                                          m_pimpl->structuredInput.rawData.size(),
                                          m_pimpl->structuredInput.shape.data(),
                                          m_pimpl->structuredInput.shape.size());

    // format the output
    const std::size_t outputSize = 1;

    // resize the output
    m_pimpl->structuredOutput.rawData.resize(outputSize);
    m_pimpl->structuredOutput.shape[0] = 1; // batch
    m_pimpl->structuredOutput.shape[1] = outputSize;

    // create tensor required by onnx
    m_pimpl->structuredOutput.tensor
        = Ort::Value::CreateTensor<float>(m_pimpl->memoryInfo,
                                          m_pimpl->structuredOutput.rawData.data(),
                                          m_pimpl->structuredOutput.rawData.size(),
                                          m_pimpl->structuredOutput.shape.data(),
                                          m_pimpl->structuredOutput.shape.size());

    return true;
}

bool JointFrictionTorqueEstimator::estimate(const double inputJointPosition,
                                            const double inputMotorPosition,
                                            double& output)
{
    // Push element into the queue
    m_pimpl->jointPositionBuffer.push_back(inputJointPosition);
    m_pimpl->motorPositionBuffer.push_back(inputMotorPosition);

    // Check if the buffer is full
    if (m_pimpl->jointPositionBuffer.size() < m_pimpl->historyLength)
    {
        // The buffer is not full yet
        return false;
    }

    if (m_pimpl->jointPositionBuffer.size() > m_pimpl->historyLength)
    {
        // The buffer is full, remove the oldest element
        m_pimpl->jointPositionBuffer.pop_front();
        m_pimpl->motorPositionBuffer.pop_front();
    }

    // Fill the input
    // Copy the joint positions and then the motor positions in the
    // structured input without emptying the buffer
    // Use iterators to copy the data to the vector
    std::copy(m_pimpl->jointPositionBuffer.cbegin(),
              m_pimpl->jointPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
    std::copy(m_pimpl->motorPositionBuffer.cbegin(),
              m_pimpl->motorPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);

    // perform the inference
    const char* inputNames[] = {"input"};
    const char* outputNames[] = {"output"};

    m_pimpl->session->Run(Ort::RunOptions(),
                          inputNames,
                          &(m_pimpl->structuredInput.tensor),
                          1,
                          outputNames,
                          &(m_pimpl->structuredOutput.tensor),
                          1);

    // copy the output
    output = static_cast<double>(m_pimpl->structuredOutput.rawData[0]);

    return true;
}
