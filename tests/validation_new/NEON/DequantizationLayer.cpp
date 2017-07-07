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
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEDequantizationLayer.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"
#include "framework/Asserts.h"
#include "framework/Macros.h"
#include "framework/datasets/Datasets.h"
#include "tests/NEON/Accessor.h"
#include "tests/PaddingCalculator.h"
#include "tests/datasets_new/ShapeDatasets.h"
#include "tests/validation_new/Validation.h"
#include "tests/validation_new/fixtures/DequantizationLayerFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
namespace
{
/** Tolerance for float operations */
constexpr AbsoluteTolerance<float> tolerance_f32(0.001f);
} // namespace

TEST_SUITE(NEON)
TEST_SUITE(DequantizationLayer)

DATA_TEST_CASE(Configuration, framework::DatasetMode::ALL, combine(concat(datasets::Small2DShapes(), datasets::Large2DShapes()), framework::dataset::make("DataType", DataType::U8)), shape, data_type)
{
    // Create tensors
    Tensor src = create_tensor<Tensor>(shape, data_type);
    Tensor dst = create_tensor<Tensor>(shape, DataType::F32);

    ARM_COMPUTE_EXPECT(src.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(dst.info()->is_resizable(), framework::LogLevel::ERRORS);

    // Create and configure function
    float                 min = 0.f;
    float                 max = 0.f;
    NEDequantizationLayer dequant_layer;
    dequant_layer.configure(&src, &dst, &min, &max);

    // Validate valid region
    const ValidRegion valid_region = shape_to_valid_region(shape);
    validate(src.info()->valid_region(), valid_region);
    validate(dst.info()->valid_region(), valid_region);

    // Validate padding
    const PaddingSize padding = PaddingCalculator(shape.x(), 8).required_padding();
    validate(src.info()->padding(), padding);
    validate(dst.info()->padding(), padding);
}

template <typename T>
using NEDequantizationLayerFixture = DequantizationValidationFixture<Tensor, Accessor, NEDequantizationLayer, T>;

TEST_SUITE(Integer)
TEST_SUITE(U8)
FIXTURE_DATA_TEST_CASE(RunSmall, NEDequantizationLayerFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(datasets::Small2DShapes(), framework::dataset::make("DataType", DataType::U8)))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEDequantizationLayerFixture<uint8_t>, framework::DatasetMode::NIGHTLY, combine(datasets::Large2DShapes(), framework::dataset::make("DataType", DataType::U8)))
{
    // Validate output
    validate(Accessor(_target), _reference, tolerance_f32);
}
TEST_SUITE_END()
TEST_SUITE_END()

TEST_SUITE_END()
TEST_SUITE_END()
} // namespace validation
} // namespace test
} // namespace arm_compute
