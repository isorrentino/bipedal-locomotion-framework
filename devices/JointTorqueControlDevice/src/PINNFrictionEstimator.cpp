/**
 * @file PINNFrictionEstimator.cpp
 * @authors Ines Sorrentino
 * @copyright 2024 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */

#include <memory>
#include <deque>
#include <string>
#include <cmath>

#include <Eigen/Dense>

// onnxruntime
#include <onnxruntime_cxx_api.h>

#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/System/VariablesHandler.h>
#include <BipedalLocomotion/TextLogging/Logger.h>

#include <BipedalLocomotion/PINNFrictionEstimator.h>


struct PINNFrictionEstimator::Impl
{
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memoryInfo;

    std::deque<float> jointPositionBuffer;
    std::deque<float> jointVelocityBuffer;
    std::deque<float> errorPositionBuffer;
    std::deque<float> motorPositionBuffer;
    std::deque<float> motorVelocityBuffer;

    size_t historyLength;
    size_t modelType;

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

PINNFrictionEstimator::PINNFrictionEstimator()
{
    m_pimpl = std::make_unique<PINNFrictionEstimator::Impl>();
}

PINNFrictionEstimator::~PINNFrictionEstimator() = default;

bool PINNFrictionEstimator::initialize(const std::string& networkModelPath,
                                       const std::size_t intraOpNumThreads,
                                       const std::size_t interOpNumThreads,
                                       const std::size_t modelType)
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
        BipedalLocomotion::log()->error("Unable to load the model from the file: {}", networkModelPath);
        return false;
    }

    // Get model input size
    std::vector<int64_t> inputShape = m_pimpl->session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();

    const std::size_t inputCount = inputShape[1];

    // Check the model type
    m_pimpl->modelType = modelType;

    int numberOfInputs = 0;
    if (modelType == 1 || modelType == 2)
    {
        numberOfInputs = 2;
    }
    else if (modelType == 3 || modelType == 4)
    {
        numberOfInputs = 3;
    }
    else if (modelType == 5)
    {
        numberOfInputs = 4;
    }
    else
    {
        BipedalLocomotion::log()->error("Wrong model type");
        return false;
    }

    m_pimpl->historyLength = inputCount / numberOfInputs;

    // format the input
    m_pimpl->structuredInput.rawData.resize(inputCount);
    m_pimpl->structuredInput.shape[0] = 1; // batch
    m_pimpl->structuredInput.shape[1] = inputCount;

    m_pimpl->jointPositionBuffer.resize(m_pimpl->historyLength);
    m_pimpl->jointVelocityBuffer.resize(m_pimpl->historyLength);
    m_pimpl->errorPositionBuffer.resize(m_pimpl->historyLength);
    m_pimpl->motorPositionBuffer.resize(m_pimpl->historyLength);
    m_pimpl->motorVelocityBuffer.resize(m_pimpl->historyLength);

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

void PINNFrictionEstimator::resetEstimator()
{
    m_pimpl->jointPositionBuffer.clear();
    m_pimpl->motorPositionBuffer.clear();
    m_pimpl->motorVelocityBuffer.clear();
    m_pimpl->jointVelocityBuffer.clear();
    m_pimpl->errorPositionBuffer.clear();
}

bool PINNFrictionEstimator::estimate(double inputJointPositon,
                                     double inputMotorPosition,
                                     double inputMotorVelocity,
                                     double inputDeltaPosition,
                                     double inputJointVelocity,
                                     double& output)
{
    if (m_pimpl->errorPositionBuffer.size() == m_pimpl->historyLength)
    {
        // The buffer is full, remove the oldest element
        m_pimpl->jointPositionBuffer.pop_front();
        m_pimpl->motorPositionBuffer.pop_front();
        m_pimpl->motorVelocityBuffer.pop_front();
        m_pimpl->errorPositionBuffer.pop_front();
        m_pimpl->jointVelocityBuffer.pop_front();
    }

    // Push element into the queue
    m_pimpl->jointPositionBuffer.push_back(inputJointPositon);
    m_pimpl->motorPositionBuffer.push_back(inputMotorPosition);
    m_pimpl->motorVelocityBuffer.push_back(inputMotorVelocity);
    m_pimpl->errorPositionBuffer.push_back(inputDeltaPosition);
    m_pimpl->jointVelocityBuffer.push_back(inputJointVelocity);

    // Check if the buffer is full
    if (m_pimpl->errorPositionBuffer.size() < m_pimpl->historyLength)
    {
        // The buffer is not full yet
        return false;
    }

    // Fill the input
    // Copy the joint positions and then the motor positions in the
    // structured input without emptying the buffer
    // Use iterators to copy the data to the vector
    if (m_pimpl->modelType == 1)
    {
        std::copy(m_pimpl->errorPositionBuffer.cbegin(),
              m_pimpl->errorPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
        std::copy(m_pimpl->jointVelocityBuffer.cbegin(),
              m_pimpl->jointVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);
    }
    else if (m_pimpl->modelType == 2)
    {
        std::copy(m_pimpl->motorVelocityBuffer.cbegin(),
              m_pimpl->motorVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
        std::copy(m_pimpl->jointVelocityBuffer.cbegin(),
              m_pimpl->jointVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);
    }
    else if (m_pimpl->modelType == 3)
    {
        std::copy(m_pimpl->errorPositionBuffer.cbegin(),
              m_pimpl->errorPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
        std::copy(m_pimpl->motorVelocityBuffer.cbegin(),
              m_pimpl->motorVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);
        std::copy(m_pimpl->jointVelocityBuffer.cbegin(),
              m_pimpl->jointVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + 2 * m_pimpl->historyLength);
    }
    else if (m_pimpl->modelType == 4)
    {
        std::copy(m_pimpl->errorPositionBuffer.cbegin(),
              m_pimpl->errorPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
        std::copy(m_pimpl->jointPositionBuffer.cbegin(),
              m_pimpl->jointPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);
        std::copy(m_pimpl->jointVelocityBuffer.cbegin(),
              m_pimpl->jointVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + 2 * m_pimpl->historyLength);
    }
    else if (m_pimpl->modelType == 5)
    {
        std::copy(m_pimpl->motorPositionBuffer.cbegin(),
              m_pimpl->motorPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin());
        std::copy(m_pimpl->motorVelocityBuffer.cbegin(),
              m_pimpl->motorVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + m_pimpl->historyLength);
        std::copy(m_pimpl->jointPositionBuffer.cbegin(),
              m_pimpl->jointPositionBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + 2 * m_pimpl->historyLength);
        std::copy(m_pimpl->jointVelocityBuffer.cbegin(),
              m_pimpl->jointVelocityBuffer.cend(),
              m_pimpl->structuredInput.rawData.begin() + 3 * m_pimpl->historyLength);
    }

    // perform the inference
    const char* inputNames[] = {"input"};
    const char* outputNames[] = {"output"};

    try
    {
        m_pimpl->session->Run(Ort::RunOptions(),
                            inputNames,
                            &(m_pimpl->structuredInput.tensor),
                            1,
                            outputNames,
                            &(m_pimpl->structuredOutput.tensor),
                            1);

    } catch (const Ort::Exception& e) {
        BipedalLocomotion::log()->error("Error during the inference: {}", e.what());
        return false;
    }

    // copy the output
    output = static_cast<double>(m_pimpl->structuredOutput.rawData[0]);

    return true;
}

