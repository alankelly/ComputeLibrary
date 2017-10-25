/*
 * Copyright (c) 2017 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/core/Error.h"
#include "arm_compute/core/ITensor.h"
#include "arm_compute/graph/IOperation.h"
#include "arm_compute/graph/NodeContext.h"
#include "arm_compute/graph/OperationRegistrar.h"
#include "arm_compute/graph/Types.h"
#include "arm_compute/runtime/NEON/NEFunctions.h"
#include "support/ToolchainSupport.h"
#include "utils/GraphTypePrinter.h"
#include "utils/TypePrinter.h"

#include <memory>

using namespace arm_compute::graph;

/* Activation Layer */
REGISTER_SIMPLE_OPERATION(NEActivationLayerOperation, NEON, OperationType::ActivationLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto      *in       = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto      *out      = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));
    const auto act_info = ctx.parameter<ActivationLayerInfo>("ActivationLayerInfo");

    // Create and configure function
    auto activation = arm_compute::support::cpp14::make_unique<arm_compute::NEActivationLayer>();
    activation->configure(in, out, act_info);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEActivationLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << " Activation function: " << act_info.activation()
                               << " a: " << act_info.a()
                               << " b: " << act_info.b()
                               << std::endl);

    return std::move(activation);
}

/* Batch Normalization Layer */
REGISTER_SIMPLE_OPERATION(NEBatchNormalizationLayerOperation, NEON, OperationType::BatchNormalizationLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 5);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(1)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(2)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(3)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(4)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto      *in      = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto      *mean    = dynamic_cast<arm_compute::ITensor *>(ctx.input(1));
    auto      *var     = dynamic_cast<arm_compute::ITensor *>(ctx.input(2));
    auto      *beta    = dynamic_cast<arm_compute::ITensor *>(ctx.input(3));
    auto      *gamma   = dynamic_cast<arm_compute::ITensor *>(ctx.input(4));
    auto      *out     = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));
    const auto epsilon = ctx.parameter<float>("epsilon");

    // Create and configure function
    auto batch_norm = arm_compute::support::cpp14::make_unique<arm_compute::NEBatchNormalizationLayer>();
    batch_norm->configure(in, out, mean, var, beta, gamma, epsilon);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEBatchNormalizationLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << " Mean shape: " << mean->info()->tensor_shape()
                               << " Var shape: " << var->info()->tensor_shape()
                               << " Beta shape: " << beta->info()->tensor_shape()
                               << " Gamma shape: " << gamma->info()->tensor_shape()
                               << " Epsilon: " << epsilon
                               << std::endl);

    return std::move(batch_norm);
}

/* Floor Layer */
REGISTER_SIMPLE_OPERATION(NEFloorLayerOperation, NEON, OperationType::FloorLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto *in  = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto *out = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));

    // Create and configure function
    auto floor = arm_compute::support::cpp14::make_unique<arm_compute::NEFloor>();
    floor->configure(in, out);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEFloorLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << std::endl);

    return std::move(floor);
}

/* Fully Connected Layer */
REGISTER_SIMPLE_OPERATION(NEFullyConnectedLayer, NEON, OperationType::FullyConnectedLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 3);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(1)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(2)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto *in      = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto *weights = dynamic_cast<arm_compute::ITensor *>(ctx.input(1));
    auto *biases  = dynamic_cast<arm_compute::ITensor *>(ctx.input(2));
    auto *out     = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));

    // Create and configure function
    auto fc = arm_compute::support::cpp14::make_unique<arm_compute::NEFullyConnectedLayer>();
    fc->configure(in, weights, biases, out);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEFullyConnectedLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Weights shape: " << weights->info()->tensor_shape()
                               << " Biases Shape: " << biases->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << std::endl);

    return std::move(fc);
}

/* L2 Normalize Layer */
REGISTER_SIMPLE_OPERATION(NEL2NormalizeLayerOperation, NEON, OperationType::L2NormalizeLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto      *in      = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto      *out     = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));
    const auto axis    = ctx.parameter<unsigned int>("axis");
    const auto epsilon = ctx.parameter<float>("epsilon");

    // Create and configure function
    auto l2_norm = arm_compute::support::cpp14::make_unique<arm_compute::NEL2Normalize>();
    l2_norm->configure(in, out, axis, epsilon);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEL2NormalizeLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << " Axis: " << axis
                               << " Epsilon: " << epsilon
                               << std::endl);

    return std::move(l2_norm);
}

/* Normalization Layer */
REGISTER_SIMPLE_OPERATION(NENormalizationLayerOperation, NEON, OperationType::NormalizationLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto      *in        = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto      *out       = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));
    const auto norm_info = ctx.parameter<NormalizationLayerInfo>("NormalizationLayerInfo");

    // Create and configure function
    auto norm = arm_compute::support::cpp14::make_unique<arm_compute::NENormalizationLayer>();
    norm->configure(in, out, norm_info);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NENormalizationLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << " Normalization info: " << norm_info
                               << std::endl);

    return std::move(norm);
}

/* Pooling Layer */
REGISTER_SIMPLE_OPERATION(NEPoolingLayerOperation, NEON, OperationType::PoolingLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto      *in        = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto      *out       = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));
    const auto pool_info = ctx.parameter<PoolingLayerInfo>("PoolingLayerInfo");

    // Create and configure function
    auto pool = arm_compute::support::cpp14::make_unique<arm_compute::NEPoolingLayer>();
    pool->configure(in, out, pool_info);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NEPoolingLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << " Pooling info: " << pool_info
                               << std::endl);

    return std::move(pool);
}

/* Softmax Layer */
REGISTER_SIMPLE_OPERATION(NESoftmaxLayerOperation, NEON, OperationType::SoftmaxLayer)
{
    ARM_COMPUTE_ERROR_ON(ctx.num_inputs() != 1);
    ARM_COMPUTE_ERROR_ON(ctx.num_outputs() != 1);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.input(0)) == nullptr);
    ARM_COMPUTE_ERROR_ON(dynamic_cast<arm_compute::ITensor *>(ctx.output(0)) == nullptr);

    // Extract IO and info
    auto *in  = dynamic_cast<arm_compute::ITensor *>(ctx.input(0));
    auto *out = dynamic_cast<arm_compute::ITensor *>(ctx.output(0));

    // Create and configure function
    auto smx = arm_compute::support::cpp14::make_unique<arm_compute::NESoftmaxLayer>();
    smx->configure(in, out);

    // Log info
    ARM_COMPUTE_LOG_GRAPH_INFO("Instantiating NESoftmaxLayer"
                               << " Data Type: " << in->info()->data_type()
                               << " Input shape: " << in->info()->tensor_shape()
                               << " Output shape: " << out->info()->tensor_shape()
                               << std::endl);

    return std::move(smx);
}