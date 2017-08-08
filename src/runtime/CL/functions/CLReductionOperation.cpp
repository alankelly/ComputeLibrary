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
#include "arm_compute/runtime/CL/functions/CLReductionOperation.h"

#include "arm_compute/core/CL/ICLTensor.h"
#include "arm_compute/core/CL/kernels/CLReductionOperationKernel.h"
#include "arm_compute/core/Error.h"
#include "arm_compute/core/PixelValue.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/CL/CLScheduler.h"
#include "arm_compute/runtime/Tensor.h"
#include "support/ToolchainSupport.h"

using namespace arm_compute;

CLReductionOperation::CLReductionOperation()
    : _sums_vector(), _reduction_kernels_vector(), _border_handlers_vector(), _num_of_stages()
{
}

void CLReductionOperation::configure(ICLTensor *input, ICLTensor *output, unsigned int axis, ReductionOperation op)
{
    // Calculate number of WGs. 16 elements per thread, 8 threads per WG
    unsigned int num_of_wg = ceil(input->info()->dimension(0) / 128.f);

    // Calculate number of stages. First stage performs op and the rest reduction sum
    // depending on the size of the input. Last stage should have only 1 WG.
    _num_of_stages = num_of_wg / 128 + 2;

    // Configure reduction operation kernels
    _reduction_kernels_vector = arm_compute::support::cpp14::make_unique<CLReductionOperationKernel[]>(_num_of_stages);
    _border_handlers_vector   = arm_compute::support::cpp14::make_unique<CLFillBorderKernel[]>(_num_of_stages);

    TensorShape shape{ input->info()->tensor_shape() };
    for(unsigned int i = 0; i < _num_of_stages - 1; i++)
    {
        shape.set(0, ceil(shape.x() / 128.f));
        auto *tensor = new CLTensor;
        tensor->allocator()->init(TensorInfo(shape, input->info()->num_channels(), input->info()->data_type(), input->info()->fixed_point_position()));
        _sums_vector.push_back(tensor);
    }

    // Apply ReductionOperation only on first kernel
    _reduction_kernels_vector[0].configure(input, _sums_vector.at(0), axis, op);
    _border_handlers_vector[0].configure(input, _reduction_kernels_vector[0].border_size(), BorderMode::CONSTANT, PixelValue(0));
    for(unsigned int i = 1; i < _num_of_stages; i++)
    {
        // Last sum vector is the output vector
        _reduction_kernels_vector[i].configure(_sums_vector.at(i - 1), i == _num_of_stages - 1 ? output : _sums_vector.at(i), axis, ReductionOperation::SUM);
        _border_handlers_vector[i].configure(_sums_vector.at(i - 1), _reduction_kernels_vector[i].border_size(), BorderMode::CONSTANT, PixelValue(0));
        _sums_vector.at(i - 1)->allocator()->allocate();
    }
}

void CLReductionOperation::run()
{
    for(unsigned int i = 0; i < _num_of_stages; ++i)
    {
        CLScheduler::get().enqueue(_border_handlers_vector[i], false);
        CLScheduler::get().enqueue(_reduction_kernels_vector[i], false);
    }
}