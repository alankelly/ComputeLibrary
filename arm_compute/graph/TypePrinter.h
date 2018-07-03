/*
 * Copyright (c) 2018 ARM Limited.
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
#ifndef __ARM_COMPUTE_GRAPH_TYPE_PRINTER_H__
#define __ARM_COMPUTE_GRAPH_TYPE_PRINTER_H__

#include "arm_compute/core/Error.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/graph/Types.h"

#include "utils/TypePrinter.h"

namespace arm_compute
{
namespace graph
{
/** Formatted output of the Target. */
inline ::std::ostream &operator<<(::std::ostream &os, const Target &target)
{
    switch(target)
    {
        case Target::UNSPECIFIED:
            os << "UNSPECIFIED";
            break;
        case Target::NEON:
            os << "NEON";
            break;
        case Target::CL:
            os << "CL";
            break;
        case Target::GC:
            os << "GC";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}

inline ::std::ostream &operator<<(::std::ostream &os, const NodeType &node_type)
{
    switch(node_type)
    {
        case NodeType::ActivationLayer:
            os << "ActivationLayer";
            break;
        case NodeType::BatchNormalizationLayer:
            os << "BatchNormalizationLayer";
            break;
        case NodeType::ChannelShuffleLayer:
            os << "ChannelShuffleLayer";
            break;
        case NodeType::ConvolutionLayer:
            os << "ConvolutionLayer";
            break;
        case NodeType::DeconvolutionLayer:
            os << "DeconvolutionLayer";
            break;
        case NodeType::DepthConcatenateLayer:
            os << "DepthConcatenateLayer";
            break;
        case NodeType::DepthwiseConvolutionLayer:
            os << "DepthwiseConvolutionLayer";
            break;
        case NodeType::EltwiseLayer:
            os << "EltwiseLayer";
            break;
        case NodeType::FlattenLayer:
            os << "FlattenLayer";
            break;
        case NodeType::FullyConnectedLayer:
            os << "FullyConnectedLayer";
            break;
        case NodeType::NormalizationLayer:
            os << "NormalizationLayer";
            break;
        case NodeType::PoolingLayer:
            os << "PoolingLayer";
            break;
        case NodeType::ReshapeLayer:
            os << "ReshapeLayer";
            break;
        case NodeType::ResizeLayer:
            os << "ResizeLayer";
            break;
        case NodeType::SoftmaxLayer:
            os << "SoftmaxLayer";
            break;
        case NodeType::SplitLayer:
            os << "SplitLayer";
            break;
        case NodeType::Input:
            os << "Input";
            break;
        case NodeType::Output:
            os << "Output";
            break;
        case NodeType::Const:
            os << "Const";
            break;
        case NodeType::Dummy:
            os << "Dummy";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}

/** Formatted output of the EltwiseOperation type. */
inline ::std::ostream &operator<<(::std::ostream &os, const EltwiseOperation &eltwise_op)
{
    switch(eltwise_op)
    {
        case EltwiseOperation::ADD:
            os << "ADD";
            break;
        case EltwiseOperation::MUL:
            os << "MUL";
            break;
        case EltwiseOperation::SUB:
            os << "SUB";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}

/** Formatted output of the ConvolutionMethod type. */
inline ::std::ostream &operator<<(::std::ostream &os, const ConvolutionMethod &method)
{
    switch(method)
    {
        case ConvolutionMethod::DEFAULT:
            os << "DEFAULT";
            break;
        case ConvolutionMethod::DIRECT:
            os << "DIRECT";
            break;
        case ConvolutionMethod::GEMM:
            os << "GEMM";
            break;
        case ConvolutionMethod::WINOGRAD:
            os << "WINOGRAD";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}

/** Formatted output of the FastMathHint type. */
inline ::std::ostream &operator<<(::std::ostream &os, const FastMathHint &hint)
{
    switch(hint)
    {
        case FastMathHint::ENABLED:
            os << "ENABLED";
            break;
        case FastMathHint::DISABLED:
            os << "DISABLED";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}

/** Formatted output of the DepthwiseConvolutionMethod type. */
inline ::std::ostream &operator<<(::std::ostream &os, const DepthwiseConvolutionMethod &method)
{
    switch(method)
    {
        case DepthwiseConvolutionMethod::DEFAULT:
            os << "DEFAULT";
            break;
        case DepthwiseConvolutionMethod::GEMV:
            os << "GEMV";
            break;
        case DepthwiseConvolutionMethod::OPTIMIZED_3x3:
            os << "OPTIMIZED_3x3";
            break;
        default:
            ARM_COMPUTE_ERROR("NOT_SUPPORTED!");
    }

    return os;
}
} // namespace graph
} // namespace arm_compute
#endif /* __ARM_COMPUTE_GRAPH_TYPE_PRINTER_H__ */
