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
#include "NEON/Accessor.h"
#include "PaddingCalculator.h"
#include "TypePrinter.h"
#include "Utils.h"
#include "tests/AssetsLibrary.h"
#include "tests/Globals.h"
#include "tests/validation_old/Datasets.h"
#include "tests/validation_old/Reference.h"
#include "tests/validation_old/Validation.h"

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEPixelWiseMultiplication.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"

#include "tests/validation_old/boost_wrapper.h"

#include <random>
#include <string>

using namespace arm_compute;
using namespace arm_compute::test;
using namespace arm_compute::test::validation;

namespace
{
/** Compute Neon pixel-wise multiplication function.
 *
 * @param[in] shape                Shape of the input and output tensors.
 * @param[in] dt_in0               Data type of first input tensor.
 * @param[in] dt_in1               Data type of second input tensor.
 * @param[in] dt_out               Data type of the output tensor.
 * @param[in] scale                Non-negative scale.
 * @param[in] convert_policy       Overflow policy of the operation.
 * @param[in] rounding_policy      Rounding policy of the operation.
 * @param[in] fixed_point_position (Optional) Fixed point position that expresses the number of bits for the fractional part of the number.
 *
 * @return Computed output tensor.
 */
Tensor compute_pixel_wise_multiplication(const TensorShape &shape, DataType dt_in0, DataType dt_in1, DataType dt_out, float scale, ConvertPolicy convert_policy, RoundingPolicy rounding_policy,
                                         int fixed_point_position = 0)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, dt_in0, 1, fixed_point_position);
    Tensor src2 = create_tensor<Tensor>(shape, dt_in1, 1, fixed_point_position);
    Tensor dst  = create_tensor<Tensor>(shape, dt_out, 1, fixed_point_position);

    // Create and configure function
    NEPixelWiseMultiplication multiply;
    multiply.configure(&src1, &src2, &dst, scale, convert_policy, rounding_policy);

    // Allocate tensors
    src1.allocator()->allocate();
    src2.allocator()->allocate();
    dst.allocator()->allocate();

    BOOST_TEST(!src1.info()->is_resizable());
    BOOST_TEST(!src2.info()->is_resizable());
    BOOST_TEST(!dst.info()->is_resizable());

    // Fill tensors
    library->fill_tensor_uniform(Accessor(src1), 0);
    library->fill_tensor_uniform(Accessor(src2), 1);

    // Compute function
    multiply.run();

    return dst;
}

void validate_configuration(const Tensor &src1, const Tensor &src2, Tensor &dst, TensorShape shape, float scale, ConvertPolicy convert_policy, RoundingPolicy rounding_policy)
{
    BOOST_TEST(src1.info()->is_resizable());
    BOOST_TEST(src2.info()->is_resizable());
    BOOST_TEST(dst.info()->is_resizable());

    // Create and configure function
    NEPixelWiseMultiplication multiply;
    multiply.configure(&src1, &src2, &dst, scale, convert_policy, rounding_policy);

    // Validate valid region
    const ValidRegion valid_region = shape_to_valid_region(shape);
    validate(src1.info()->valid_region(), valid_region);
    validate(src2.info()->valid_region(), valid_region);
    validate(dst.info()->valid_region(), valid_region);

    // Validate padding
    const PaddingSize padding = PaddingCalculator(shape.x(), 16).required_padding();
    validate(src1.info()->padding(), padding);
    validate(src2.info()->padding(), padding);
    validate(dst.info()->padding(), padding);
}
} // namespace

#ifndef DOXYGEN_SKIP_THIS
BOOST_AUTO_TEST_SUITE(NEON)
BOOST_AUTO_TEST_SUITE(PixelWiseMultiplication)

BOOST_AUTO_TEST_SUITE(U8)
BOOST_AUTO_TEST_SUITE(Scale255)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, DataType::U8);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::U8);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::U8);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::U8, DataType::U8, DataType::U8, scale, convert_policy,
                                                   rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::U8, DataType::U8,
                                                                               DataType::U8, scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 1.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 1.f, 0.f, std::numeric_limits<uint8_t>::max());
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::U8, DataType::U8, DataType::U8, scale, convert_policy,
                                                   rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::U8, DataType::U8,
                                                                               DataType::U8, scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 1.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 1.f, 0.f, std::numeric_limits<uint8_t>::max());
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ScaleOther)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * boost::unit_test::data::make({ 1.f, 1.f / 32768.f })
                     * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, DataType::U8);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::U8);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::U8);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::U8, DataType::U8, DataType::U8, scale, convert_policy,
                                                   rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::U8, DataType::U8,
                                                                               DataType::U8, scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::U8, DataType::U8, DataType::U8, scale, convert_policy,
                                                   rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::U8, DataType::U8,
                                                                               DataType::U8, scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(S16)
BOOST_AUTO_TEST_SUITE(Scale255)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, dt);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::S16);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::S16);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 2.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 2.f, 0.f, std::numeric_limits<int16_t>::max());
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16,
                                                                               scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 2.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 2.f, 0.f, std::numeric_limits<int16_t>::max());
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ScaleOther)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * boost::unit_test::data::make({ 1.f, 1.f / 32768.f })
                     * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, dt);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::S16);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::S16);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * boost::unit_test::data::make({ DataType::U8, DataType::S16 }) * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, dt, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, DataType::S16, DataType::S16,
                                                                               scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#ifdef ARM_COMPUTE_ENABLE_FP16
BOOST_AUTO_TEST_SUITE(F16)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))

BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::F16, DataType::F16, DataType::F16, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::F16, DataType::F16, DataType::F16, scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 1.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 1.f, 0.f, std::numeric_limits<int16_t>::max());
}

BOOST_AUTO_TEST_SUITE_END()
#endif /* ARM_COMPUTE_ENABLE_FP16 */

BOOST_AUTO_TEST_SUITE(F32)
BOOST_AUTO_TEST_SUITE(Scale255)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, DataType::F32);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::F32);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::F32);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 1.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 1.f, 0.f, std::numeric_limits<int16_t>::max());
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * (1.f / 255.f) * ConvertPolicies()
                     * RoundingPolicy::TO_NEAREST_UP,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32,
                                                                               scale, convert_policy, rounding_policy);

    // Validate output
    // Allow tolerance value of 1.f to counteract imprecision due to 32-bit float conversion
    validate(Accessor(dst), ref_dst, 1.f, 0.f, std::numeric_limits<int16_t>::max());
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ScaleOther)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit") * boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(Configuration, (SmallShapes() + LargeShapes()) * boost::unit_test::data::make({ 1.f, 1.f / 32768.f })
                     * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(shape, DataType::F32);
    Tensor src2 = create_tensor<Tensor>(shape, DataType::F32);
    Tensor dst  = create_tensor<Tensor>(shape, DataType::F32);

    validate_configuration(src1, src2, dst, shape, scale, convert_policy, rounding_policy);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * boost::unit_test::data::make({ 1.f, 1.f / 32768.f }) * ConvertPolicies()
                     * RoundingPolicy::TO_ZERO,
                     shape, scale, convert_policy, rounding_policy)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32, scale, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, DataType::F32, DataType::F32, DataType::F32,
                                                                               scale, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Quantized)
BOOST_AUTO_TEST_SUITE(QS8)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * DataType::QS8 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange<int>(1, 7),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmallScale255, SmallShapes() * DataType::QS8 * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_NEAREST_UP * boost::unit_test::data::xrange(1, 7),
                     shape, dt, scale, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmallScaleOther, SmallShapes() * DataType::QS8 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange(1, 7),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    const float scale = 1.f / static_cast<float>(1 << fixed_point_position);

    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst, 1.f);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * DataType::QS8 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange<int>(1, 7),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLargeScale255, LargeShapes() * DataType::QS8 * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange(1, 7),
                     shape, dt, scale, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLargeScaleOther, LargeShapes() * DataType::QS8 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange(1, 7),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    const float scale = 1.f / static_cast<float>(1 << fixed_point_position);

    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst, 1.f);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(QS16)
BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmall, SmallShapes() * DataType::QS16 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange<int>(1, 15),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmallScale255, SmallShapes() * DataType::QS16 * (1.f / 255.f) * ConvertPolicies() * RoundingPolicy::TO_NEAREST_UP * boost::unit_test::data::xrange(1, 15),
                     shape, dt, scale, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("precommit"))
BOOST_DATA_TEST_CASE(RunSmallScaleOther, SmallShapes() * DataType::QS16 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange(1, 15),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    const float scale = 1.f / static_cast<float>(1 << fixed_point_position);

    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, scale, convert_policy, rounding_policy, fixed_point_position);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_fixed_point_pixel_wise_multiplication(shape, dt, dt, dt, scale, fixed_point_position, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst, 1.f);
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("nightly"))
BOOST_DATA_TEST_CASE(RunLarge, LargeShapes() * DataType::QS16 *ConvertPolicies() * RoundingPolicy::TO_ZERO * boost::unit_test::data::xrange<int>(1, 15),
                     shape, dt, convert_policy, rounding_policy, fixed_point_position)
{
    // Compute function
    Tensor dst = compute_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy);

    // Compute reference
    RawTensor ref_dst = Reference::compute_reference_pixel_wise_multiplication(shape, dt, dt, dt, 1.f, convert_policy, rounding_policy);

    // Validate output
    validate(Accessor(dst), ref_dst);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
#endif /* DOXYGEN_SKIP_THIS */