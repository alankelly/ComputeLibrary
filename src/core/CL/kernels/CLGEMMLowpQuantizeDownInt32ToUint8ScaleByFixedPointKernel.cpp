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
#include "arm_compute/core/CL/kernels/CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel.h"

#include "arm_compute/core/AccessWindowStatic.h"
#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/Error.h"
#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/core/Window.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"

#include "support/ToolchainSupport.h"

using namespace arm_compute;

namespace arm_compute
{
namespace
{
Status validate_arguments(const ITensorInfo *input, const ITensorInfo *bias, const ITensorInfo *output,
                          int min, int max, unsigned int output_3d_depth)
{
    ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(input, 1, DataType::S32);
    ARM_COMPUTE_RETURN_ERROR_ON(max > 255);
    ARM_COMPUTE_RETURN_ERROR_ON(min < 0 || min > max);

    // Check biases if exist
    if(bias != nullptr)
    {
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_DATA_TYPES(input, bias);
        ARM_COMPUTE_RETURN_ERROR_ON(bias->num_dimensions() > 1);
        ARM_COMPUTE_RETURN_ERROR_ON(input->dimension(0) != bias->dimension(0));
    }

    if(output->total_size() != 0)
    {
        const TensorShape output_shape       = arm_compute::misc::shape_calculator::compute_output_stage_shape(*input, output_3d_depth, true);
        const TensorInfo  tensor_info_output = output->clone()->set_tensor_shape(output_shape);
        ARM_COMPUTE_RETURN_ERROR_ON_DATA_TYPE_CHANNEL_NOT_IN(output, 1, DataType::QASYMM8);
        ARM_COMPUTE_RETURN_ERROR_ON_MISMATCHING_SHAPES(output, &tensor_info_output);
    }

    return Status{};
}

std::pair<Status, Window> validate_and_configure_window(ITensorInfo *input, ITensorInfo *bias, ITensorInfo *output)
{
    constexpr unsigned int num_elems_processed_per_iteration = 16;

    // Configure kernel window
    Window win = calculate_max_window(*input, Steps(num_elems_processed_per_iteration));

    AccessWindowHorizontal input_access(input, 0, num_elems_processed_per_iteration);

    bool window_changed = update_window_and_padding(win,
                                                    input_access);

    if(output->total_size() != 0)
    {
        Window                 win_out = calculate_max_window(*output, Steps(num_elems_processed_per_iteration));
        AccessWindowHorizontal output_result_access(output, 0, num_elems_processed_per_iteration);
        window_changed = window_changed || update_window_and_padding(win_out, output_result_access);

        output_result_access.set_valid_region(win, ValidRegion(Coordinates(), output->tensor_shape()));
    }

    if(bias != nullptr)
    {
        AccessWindowStatic bias_access(bias, 0, 0, ceil_to_multiple(bias->dimension(0), num_elems_processed_per_iteration), bias->tensor_shape()[1]);
        window_changed = window_changed || update_window_and_padding(win, bias_access);
    }

    Status err = (window_changed) ? ARM_COMPUTE_CREATE_ERROR(ErrorCode::RUNTIME_ERROR, "Insufficient Padding!") : Status{};
    return std::make_pair(err, win);
}
} // namespace

class Coordinates;
} // namespace arm_compute

CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel::CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel()
    : _input(nullptr), _bias(nullptr), _output(nullptr), _reinterpret_as_3d(false)
{
}

Status CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel::validate(const ITensorInfo *input, const ITensorInfo *bias, const ITensorInfo *output,
                                                                           int min, int max, unsigned int output_3d_depth)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, output);
    ARM_COMPUTE_RETURN_ON_ERROR(validate_arguments(input, bias, output, min, max, output_3d_depth));
    ARM_COMPUTE_RETURN_ON_ERROR(validate_and_configure_window(input->clone().get(),
                                                              (bias != nullptr) ? bias->clone().get() : nullptr,
                                                              output->clone().get())
                                .first);

    return Status{};
}

void CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel::configure(const ICLTensor *input, const ICLTensor *bias, ICLTensor *output,
                                                                          int result_fixedpoint_multiplier, int result_shift, int result_offset_after_shift,
                                                                          int min, int max, unsigned int output_3d_depth)
{
    // Perform validate step
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, output);

    // Output auto inizialitation if not yet initialized
    const TensorShape output_shape = arm_compute::misc::shape_calculator::compute_output_stage_shape(*input->info(), output_3d_depth, true);
    auto_init_if_empty(*output->info(), input->info()->clone()->set_data_type(DataType::QASYMM8).set_tensor_shape(output_shape));

    ARM_COMPUTE_ERROR_THROW_ON(validate_arguments(input->info(), (bias != nullptr) ? bias->info() : nullptr, output->info(),
                                                  min, max, output_3d_depth));

    _input             = input;
    _bias              = bias;
    _output            = output;
    _reinterpret_as_3d = output_3d_depth > 1;

    // Set the arguments to pass at compile time
    CLBuildOptions build_opts;
    build_opts.add_option("-DRESULT_OFFSET_AFTER_SHIFT=" + support::cpp11::to_string(result_offset_after_shift));
    build_opts.add_option("-DRESULT_FIXEDPOINT_MULTIPLIER=" + support::cpp11::to_string(result_fixedpoint_multiplier));
    build_opts.add_option("-DRESULT_SHIFT=" + support::cpp11::to_string(result_shift));
    build_opts.add_option_if((min != 0) && (min != max), "-DMIN_BOUND=" + support::cpp11::to_string(min));
    build_opts.add_option_if((max != 255) && (min != max), "-DMAX_BOUND=" + support::cpp11::to_string(max));
    build_opts.add_option_if(bias != nullptr, "-DADD_BIAS");
    build_opts.add_option_if(_reinterpret_as_3d, "-DDST_HEIGHT=" + support::cpp11::to_string(input->info()->tensor_shape().y() / output_3d_depth));

    // Create kernel
    _kernel = static_cast<cl::Kernel>(CLKernelLibrary::get().create_kernel("gemmlowp_output_stage_quantize_down_fixedpoint", build_opts.options()));

    // Configure kernel window
    auto win_config = validate_and_configure_window(input->info(), (bias != nullptr) ? bias->info() : nullptr, output->info());
    ARM_COMPUTE_ERROR_THROW_ON(win_config.first);
    ICLKernel::configure_internal(win_config.second);
}

void CLGEMMLowpQuantizeDownInt32ToUint8ScaleByFixedPointKernel::run(const Window &window, cl::CommandQueue &queue)
{
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(ICLKernel::window(), window);

    // Create input window
    Window collapsed = window.collapse_if_possible(ICLKernel::window(), Window::DimZ);
    Window slice     = collapsed.first_slice_window_3D();

    // Setup bias slice
    unsigned int idx1 = num_arguments_per_3D_tensor();
    if(_bias != nullptr)
    {
        Window biases_slice(slice);
        biases_slice.set(Window::DimY, Window::Dimension(0, 1, 1));
        biases_slice.set(Window::DimZ, Window::Dimension(0, 1, 1));
        add_1D_tensor_argument(idx1, _bias, biases_slice);
    }

    if(_reinterpret_as_3d)
    {
        // Create output window
        Window window_out;
        window_out.use_tensor_dimensions(_output->info()->tensor_shape());
        Window collapsed_out = window_out.collapse_if_possible(window_out, 3);
        Window slice_out     = collapsed.first_slice_window_4D();

        do
        {
            unsigned int idx = 0;
            add_3D_tensor_argument(idx, _input, slice);
            add_4D_tensor_argument(idx1, _output, slice_out);
            enqueue(queue, *this, slice);
        }
        while(collapsed.slide_window_slice_3D(slice) && collapsed_out.slide_window_slice_4D(slice_out));
    }
    else
    {
        do
        {
            unsigned int idx = 0;
            add_3D_tensor_argument(idx, _input, slice);
            add_3D_tensor_argument(idx1, _output, slice);
            enqueue(queue, *this, slice);
        }
        while(collapsed.slide_window_slice_3D(slice));
    }
}
