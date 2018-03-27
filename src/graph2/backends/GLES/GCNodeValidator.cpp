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
#include "arm_compute/graph2/backends/GLES/GCNodeValidator.h"

#include "arm_compute/graph2/backends/ValidateHelpers.h"
#include "arm_compute/graph2/nodes/Nodes.h"

#include "arm_compute/core/utils/misc/Cast.h"
#include "arm_compute/runtime/GLES_COMPUTE/GCFunctions.h"

using namespace arm_compute::utils::cast;

namespace arm_compute
{
namespace graph2
{
namespace backends
{
namespace
{
/** Validates a Depthwise Convolution layer node
 *
 * @param[in] node Node to validate
 *
 * @return Status
 */
Status validate_depthwise_convolution_layer(DepthwiseConvolutionLayerNode &node)
{
    ARM_COMPUTE_LOG_GRAPH_VERBOSE("Validating GCDepthwiseConvolutionLayer node with ID : " << node.id() << " and Name: " << node.name() << std::endl);
    ARM_COMPUTE_RETURN_ERROR_ON(node.num_inputs() != 3);
    ARM_COMPUTE_RETURN_ERROR_ON(node.num_outputs() != 1);

    // Extract IO and info
    arm_compute::ITensorInfo *weights = detail::get_backing_tensor_info(node.input(1));
    ARM_COMPUTE_ERROR_ON(weights == nullptr);

    // TODO (geopin01) : Switch when validation is implemented
    // Validate function
    ARM_COMPUTE_RETURN_ERROR_ON_MSG(weights->tensor_shape().x() != 3 && weights->tensor_shape().y() != 3, "Unsupported depthwise convolution");
    node.set_depthwise_convolution_method(DepthwiseConvolutionMethod::OPTIMIZED_3x3);

    return Status{};
}
/** Validates a Convolution layer node
 *
 * @param[in] node Node to validate
 *
 * @return Status
 */
Status validate_convolution_layer(ConvolutionLayerNode &node)
{
    ARM_COMPUTE_LOG_GRAPH_VERBOSE("Validating ConvolutionLayer node with ID : " << node.id() << " and Name: " << node.name() << std::endl);
    ARM_COMPUTE_RETURN_ERROR_ON(node.num_inputs() != 3);
    ARM_COMPUTE_RETURN_ERROR_ON(node.num_outputs() != 1);

    // Extract IO and info
    arm_compute::ITensorInfo *weights        = detail::get_backing_tensor_info(node.input(1));
    const PadStrideInfo       conv_info      = node.convolution_info();
    const ConvolutionMethod   conv_algorithm = node.convolution_method();

    // Validate function
    if(conv_algorithm == ConvolutionMethod::DIRECT)
    {
        bool is_square         = weights->tensor_shape().x() == weights->tensor_shape().y();
        bool is_direct         = (weights->tensor_shape().x() == 1) || (weights->tensor_shape().x() == 3) || (weights->tensor_shape().x() == 5);
        bool is_correct_stride = (conv_info.stride().first) <= 2 && (conv_info.stride().second <= 2);
        if(!(is_square && is_direct && is_correct_stride))
        {
            node.set_convolution_method(ConvolutionMethod::DEFAULT);
        }
    }

    return Status{};
}
} // namespace

Status GCNodeValidator::validate(INode *node)
{
    if(node == nullptr)
    {
        return Status{};
    }

    NodeType type = node->type();
    switch(type)
    {
        case NodeType::ConvolutionLayer:
            return validate_convolution_layer(*polymorphic_downcast<ConvolutionLayerNode *>(node));
        case NodeType::DepthwiseConvolutionLayer:
            return validate_depthwise_convolution_layer(*polymorphic_downcast<DepthwiseConvolutionLayerNode *>(node));
        case NodeType::FlattenLayer:
            return ARM_COMPUTE_CREATE_ERROR(arm_compute::ErrorCode::RUNTIME_ERROR, "Unsupported operation");
        case NodeType::ReshapeLayer:
            return ARM_COMPUTE_CREATE_ERROR(arm_compute::ErrorCode::RUNTIME_ERROR, "Unsupported operation");
        default:
            return Status{};
    }
}
} // namespace backends
} // namespace graph2
} // namespace arm_compute