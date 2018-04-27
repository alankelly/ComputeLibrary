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
#ifndef __ARM_COMPUTE_GRAPH_TENSOR_DESCRIPTOR_H__
#define __ARM_COMPUTE_GRAPH_TENSOR_DESCRIPTOR_H__

#include "arm_compute/graph/Types.h"

namespace arm_compute
{
namespace graph
{
/** Tensor metadata class */
struct TensorDescriptor final
{
    /** Default Constructor **/
    TensorDescriptor() = default;
    /** Constructor
     *
     * @param[in] tensor_shape      Tensor shape
     * @param[in] tensor_data_type  Tensor data type
     * @param[in] tensor_quant_info Tensor quantization info
     * @param[in] tensor_target     Target to allocate the tensor for
     */
    TensorDescriptor(TensorShape tensor_shape, DataType tensor_data_type, QuantizationInfo tensor_quant_info = QuantizationInfo(), Target tensor_target = Target::UNSPECIFIED)
        : shape(tensor_shape), data_type(tensor_data_type), quant_info(tensor_quant_info), target(tensor_target)
    {
    }

    TensorShape      shape{};                        /**< Tensor shape */
    DataType         data_type{ DataType::UNKNOWN }; /**< Data type */
    QuantizationInfo quant_info{};                   /**< Quantization info */
    Target           target{ Target::UNSPECIFIED };  /**< Target */
};
} // namespace graph
} // namespace arm_compute
#endif /* __ARM_COMPUTE_GRAPH_TENSOR_DESCRIPTOR_H__ */