/*
 * Copyright (c) 2017-2018 ARM Limited.
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
#include "arm_compute/runtime/NEON/functions/NEWinogradLayer.h"

#include "arm_compute/core/Utils.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/NEON/NEScheduler.h"
#include "support/ToolchainSupport.h"

#include "arm_compute/core/NEON/kernels/convolution/winograd/winograd_gemm.hpp"

namespace
{
inline Tensor4DShape internal_get_input_shape(const arm_compute::ITensor *input)
{
    const int in_width    = input->info()->dimension(0);
    const int in_height   = input->info()->dimension(1);
    const int in_batches  = input->info()->dimension(3);
    const int in_channels = input->info()->dimension(2);
    return Tensor4DShape({ in_batches, in_height, in_width, in_channels });
}
} /* namespace */

namespace arm_compute
{
NEWinogradLayer::NEWinogradLayer(std::shared_ptr<IMemoryManager> memory_manager)
    : _memory_group(std::move(memory_manager)), _winograd_kernel(), _transform_input_kernel(), _transform_output_kernel(), _transform_weights_kernel(), _permute_input(), _permute_weights(),
      _permute_output(), _input_workspace(), _output_workspace(), _kernel_storage(), _input_nhwc(), _output_nhwc(), _weights_hwio(), _input(), _weights(), _output(), _reshaped_kernel(false)
{
} /* arm_compute */

void NEWinogradLayer::configure(const ITensor *input, const ITensor *weights, const ITensor *biases, ITensor *output, const PadStrideInfo &conv_info)
{
    ARM_COMPUTE_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(input, 1, DataType::F32);
    ARM_COMPUTE_ERROR_ON_MISMATCHING_DATA_TYPES(input, weights, biases);
    ARM_COMPUTE_ERROR_ON_MSG(weights->info()->dimension(1) != 3 || weights->info()->dimension(0) != 3, "Only 3x3 kernels are supported");
    ARM_COMPUTE_ERROR_ON(weights->info()->num_dimensions() > 4);

    if(biases != nullptr)
    {
        ARM_COMPUTE_ERROR_ON_MISMATCHING_DATA_TYPES(input, biases);
        ARM_COMPUTE_ERROR_ON(biases->info()->num_dimensions() > 1);
    }

    _weights = weights;
    _input   = input;
    _output  = output;

    const PaddingType use_padding_type = (conv_info.pad_left() != 0u) ? PADDING_SAME : PADDING_VALID;
    const bool        use_same_padding = use_padding_type == PADDING_SAME;

    // Get parameters from conv_info
    unsigned int stride_x = 0;
    unsigned int stride_y = 0;
    std::tie(stride_x, stride_y) = conv_info.stride();
    ARM_COMPUTE_ERROR_ON_MSG(stride_y != 1 || stride_x != 1, "Winograd layer only supports unit strides.");

    // Get convolved dimensions
    const int in_channels  = input->info()->dimension(2);
    const int out_channels = output->info()->dimension(2);

    const Tensor4DShape in_shape(internal_get_input_shape(input));
    const size_t        data_type_size = input->info()->element_size();
    // Get the memory required to instantiate a new Winograd operator.
    constexpr size_t storage_alignment   = 64;
    const size_t     kernel_storage_size = NEWinogradLayerTransformWeightsKernel<2, 2, 3, 3>::get_weight_storage_size(out_channels, in_channels) * data_type_size;
    _kernel_storage.allocator()->init(TensorInfo(TensorShape{ (kernel_storage_size + storage_alignment - 1) }, 1, DataType::U8));
    _memory_group.manage(&_kernel_storage);
    _memory_group.manage(&_input_nhwc);
    _kernel_storage.allocator()->allocate();
    // Input storage

    using IT                        = NEWinogradLayerTransformInputKernel<2, 2, 3, 3>;
    const size_t input_storage_size = IT::get_input_storage_size(in_shape.n_batches, in_shape.n_channels, in_shape.n_rows, in_shape.n_cols, use_same_padding) * data_type_size;
    _input_workspace.allocator()->init(TensorInfo(TensorShape{ (input_storage_size + storage_alignment - 1) }, 1, DataType::U8));
    _memory_group.manage(&_input_workspace);
    _input_workspace.allocator()->allocate();

    // Output storage
    using OT                         = NEWinogradLayerTransformOutputKernel<2, 2, 3, 3>;
    const size_t output_storage_size = OT::get_output_storage_size(in_shape.n_batches, in_shape.n_rows, in_shape.n_cols, out_channels, use_same_padding) * data_type_size;
    _output_workspace.allocator()->init(TensorInfo(TensorShape{ (output_storage_size + storage_alignment - 1) }, 1, DataType::U8));
    _memory_group.manage(&_output_workspace);
    _output_workspace.allocator()->allocate();

    // configure and allocate dst tensor to be used to convert from winograd domain to spatial domain when calling to reshape_output()
    TensorInfo info(TensorShape(_output->info()->dimension(2), _output->info()->dimension(0),
                                _output->info()->dimension(1), _output->info()->dimension(3)),
                    1, _output->info()->data_type());
    _output_nhwc.allocator()->init(info);
    _output_nhwc.allocator()->allocate();

    // Re-order a weight tensor from [Output feature map x Input feature map x Height x Width] to [Height x Width x Input feature map x Output feature map]
    _permute_weights.configure(weights, &_weights_hwio, PermutationVector(3U, 2U, 0U, 1U));
    _weights_hwio.allocator()->allocate();

    // configure the kernel to transform the input tensor from NCHW -> NHWC
    _permute_input.configure(input, &_input_nhwc, PermutationVector(2U, 0U, 1U));
    _input_nhwc.allocator()->allocate();

    using T                          = winograd::WinogradGEMM<2, 2, 3, 3>::Convolution<float, float>;
    const int         weights_width  = weights->info()->dimension(0);
    const int         weights_height = weights->info()->dimension(1);
    const KernelShape kernel_shape({ out_channels, weights_height, weights_width, in_channels });

    // Configure the InputTransform
    const int input_matrix_stride = T::get_input_matrix_stride(kernel_shape, in_shape, use_padding_type);
    _transform_input_kernel.configure(reinterpret_cast<float *>(_input_nhwc.buffer()), in_shape.n_batches, in_shape.n_rows, in_shape.n_cols, in_shape.n_channels, use_padding_type,
                                      reinterpret_cast<float *>(_input_workspace.buffer()), input_matrix_stride);

    // Configure WeightsTransform
    const int kernel_matrix_stride = T::get_kernel_matrix_stride(kernel_shape);
    _transform_weights_kernel.configure(&_weights_hwio, reinterpret_cast<float *>(_kernel_storage.buffer()), kernel_matrix_stride, out_channels, in_channels);

    // Configure OutputTransform
    //The biases tensor has not been allocated at this point in time, the output transform will add the biases to the final result in the run() method
    const int  output_matrix_stride = T::get_output_matrix_stride(kernel_shape, in_shape, use_padding_type);
    const auto output_shape(T::get_output_shape(kernel_shape, in_shape, use_padding_type));

    _transform_output_kernel.configure(biases, reinterpret_cast<float *>(_output_workspace.buffer()),
                                       output_matrix_stride, reinterpret_cast<float *>(_output_nhwc.buffer()),
                                       in_shape.n_batches, output_shape.n_rows, output_shape.n_cols, out_channels);

    // Configure Batched GEMMs
    const int tile_rows                = iceildiv(output_shape.n_rows, NEWinogradLayerKernel<2, 2, 3, 3>::_output_tile_rows);
    const int tile_cols                = iceildiv(output_shape.n_cols, NEWinogradLayerKernel<2, 2, 3, 3>::_output_tile_cols);
    const int m                        = in_shape.n_batches * tile_rows * tile_cols;
    const int k                        = in_shape.n_channels;
    const int n                        = out_channels;
    const int input_matrix_row_stride  = in_shape.n_channels;
    const int kernel_matrix_row_stride = roundup(out_channels, NEWinogradLayerKernel<2, 2, 3, 3>::WinogradConv::N_BLOCK);
    const int output_matrix_row_stride = kernel_matrix_row_stride;

    _winograd_kernel.configure(NEWinogradLayerKernel<2, 2, 3, 3>::WinogradBase::N_GEMMS, m, k, n,
                               input_matrix_stride, input_matrix_row_stride,
                               kernel_matrix_stride, kernel_matrix_row_stride,
                               output_matrix_stride, output_matrix_row_stride,
                               reinterpret_cast<float *>(_input_workspace.buffer()), reinterpret_cast<float *>(_kernel_storage.buffer()), reinterpret_cast<float *>(_output_workspace.buffer()));

    // Reorder the convoluted output to ACL's ordering NCHW
    _permute_output.configure(&_output_nhwc, _output, PermutationVector(1U, 2U, 0U));
}

void NEWinogradLayer::run()
{
    _memory_group.acquire();
    if(!_reshaped_kernel)
    {
        _reshaped_kernel = true;
        _permute_weights.run();
        NEScheduler::get().schedule(&_transform_weights_kernel, Window::DimX);
    }
    //Bring channels to the front as Winograd code expects the tensor to be in the format NHWC
    _permute_input.run();

    // Transform input tensor to the winograd domain
    NEScheduler::get().schedule(&_transform_input_kernel, Window::DimX);

    //Run 16 GEMMs in multiple threads, each kernel runs one or more GEMMs
    NEScheduler::get().schedule(&_winograd_kernel, Window::DimX);

    // Transform output tensor to the spatial domain
    NEScheduler::get().schedule(&_transform_output_kernel, Window::DimX);

    // Reorder the convoluted output to ACL's ordering NCHW
    _permute_output.run();
    _memory_group.release();
}
} // namespace arm_compute
