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
 * IMPLIED, INNEUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY NEAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ARM_COMPUTE_NEELEMENTWISEOPERATIONKERNEL_H__
#define __ARM_COMPUTE_NEELEMENTWISEOPERATIONKERNEL_H__

#include "arm_compute/core/NEON/INEKernel.h"
#include "arm_compute/core/Types.h"

namespace arm_compute
{
class ITensor;

/** Interface for an element-wise operation kernel
 *
 * Element-wise operation is computed by:
 * @f[ output(x,y) = OP(input1(x,y), input2(x,y))@f]
 *
 */
class NEElementwiseOperationKernel : public INEKernel
{
public:
    const char *name() const override
    {
        return "NEElementwiseOperationKernel";
    }
    /** Default constructor */
    NEElementwiseOperationKernel();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEElementwiseOperationKernel(const NEElementwiseOperationKernel &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    NEElementwiseOperationKernel &operator=(const NEElementwiseOperationKernel &) = delete;
    /** Allow instances of this class to be moved */
    NEElementwiseOperationKernel(NEElementwiseOperationKernel &&) = default;
    /** Allow instances of this class to be moved */
    NEElementwiseOperationKernel &operator=(NEElementwiseOperationKernel &&) = default;
    /** Default destructor */
    ~NEElementwiseOperationKernel() = default;

    // Inherited methods overridden:
    void run(const Window &window, const ThreadInfo &info) override;

protected:
    /** Validate the argument passed to the kernel
     *
     * @param[in] input1 First tensor input. Data types supported: S16/F16/S32/F32.
     * @param[in] input2 Second tensor input. Data types supported: Same as @p input1.
     * @param[in] output Output tensor. Data types supported: Same as @p input1.
     */
    virtual Status validate_arguments(const ITensorInfo &input1, const ITensorInfo &input2, const ITensorInfo &output) = 0;

    /** Commmon configure function for element-wise operators with no additional options (e.g. Min, Max, SquaredDiff)
     *
     */
    template <ArithmeticOperation op>
    void configure_common(const ITensor *input1, const ITensor *input2, ITensor *output);

    ArithmeticOperation _op; // Code of the operation to execute

private:
    /** Common signature for all the specialised add functions
     *
     * @param[in]  input1 An input tensor. Data types supported: S16/F16/S32/F32
     * @param[in]  input2 An input tensor. Data types supported: S16/F16/S32/F32
     * @param[out] output The output tensor. Data types supported: S16/F16/S32/F32
     * @param[in]  window Region on which to execute the kernel.
     */
    using ElementwiseFunction = void(const ITensor *input1, const ITensor *input2, ITensor *output, const Window &window);
    /** Add function to use for the particular tensor types passed to configure() */
    ElementwiseFunction *_func;
    const ITensor       *_input1;
    const ITensor       *_input2;
    ITensor             *_output;
};

class NEArithmeticOperationKernel : public NEElementwiseOperationKernel
{
public:
    NEArithmeticOperationKernel()
        : NEElementwiseOperationKernel()
    {
    }

    /** Static function to check if given info will lead to a valid configuration of @ref NEArithmeticOperationKernel
     *
     * @param[in] op     Arithmetic operation to be executed.
     * @param[in] input1 First tensor input. Data types supported: QASYMM8/S16/F16/S32/F32.
     * @param[in] input1 First tensor input. Data types supported: Same as @p input1.
     * @param[in] input2 Second tensor input. Data types supported: Same as @p input1.
     * @param[in] output Output tensor. Data types supported: Same as @p input1.
     */
    void configure(ArithmeticOperation op, const ITensor *input1, const ITensor *input2, ITensor *output);

    /** Static function to check if given info will lead to a valid configuration of @ref NEArithmeticOperationKernel
     *
     * @param[in] op     Arithmetic operation to be executed.
     * @param[in] input1 First tensor input info. Data types supported: QASYMM8/S16/F16/S32/F32.
     * @param[in] input1 First tensor input info. Data types supported: Same as @p input1
     * @param[in] input2 Second tensor input info. Data types supported: Same as @p input1.
     * @param[in] output Output tensor info. Data types supported: Same as @p input1.
     *
     * @return a Status
     */
    static Status validate(ArithmeticOperation op, const ITensorInfo *input1, const ITensorInfo *input2, const ITensorInfo *output);

protected:
    // Inherited methods overridden:
    Status validate_arguments(const ITensorInfo &input1, const ITensorInfo &input2, const ITensorInfo &output) override;
};
} // namespace arm_compute
#endif /* __ARM_COMPUTE_NEELEMENTWISEOPERATIONKERNEL_H__ */
