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
 * IMPLIED, INCLUDING BUT SELECT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ARM_COMPUTE_WRAPPER_BITSELECT_H__
#define __ARM_COMPUTE_WRAPPER_BITSELECT_H__

#include <arm_neon.h>

namespace arm_compute
{
namespace wrapper
{
#define VBITSELECT_IMPL(stype, vtype, ctype, prefix, postfix)               \
    inline vtype vbitselect(const ctype &a, const vtype &b, const vtype &c) \
    {                                                                       \
        return prefix##_##postfix(a, b, c);                                 \
    }

VBITSELECT_IMPL(uint8_t, uint8x8_t, uint8x8_t, vbsl, u8)
VBITSELECT_IMPL(int8_t, int8x8_t, uint8x8_t, vbsl, s8)
VBITSELECT_IMPL(uint16_t, uint16x4_t, uint16x4_t, vbsl, u16)
VBITSELECT_IMPL(int16_t, int16x4_t, uint16x4_t, vbsl, s16)
VBITSELECT_IMPL(uint32_t, uint32x2_t, uint32x2_t, vbsl, u32)
VBITSELECT_IMPL(int32_t, int32x2_t, uint32x2_t, vbsl, s32)
VBITSELECT_IMPL(float32x2_t, float32x2_t, uint32x2_t, vbsl, f32)
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
VBITSELECT_IMPL(float16x4_t, float16x4_t, uint16x4_t, vbsl, f16)
#endif // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC

VBITSELECT_IMPL(uint8_t, uint8x16_t, uint8x16_t, vbslq, u8)
VBITSELECT_IMPL(int8_t, int8x16_t, uint8x16_t, vbslq, s8)
VBITSELECT_IMPL(uint16_t, uint16x8_t, uint16x8_t, vbslq, u16)
VBITSELECT_IMPL(int16_t, int16x8_t, uint16x8_t, vbslq, s16)
VBITSELECT_IMPL(uint32_t, uint32x4_t, uint32x4_t, vbslq, u32)
VBITSELECT_IMPL(int32_t, int32x4_t, uint32x4_t, vbslq, s32)
VBITSELECT_IMPL(float32x4_t, float32x4_t, uint32x4_t, vbslq, f32)
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
VBITSELECT_IMPL(float16x8_t, float16x8_t, uint16x8_t, vbslq, f16)
#endif // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC

#undef VBITSELECT_IMPL
} // namespace wrapper
} // namespace arm_compute
#endif /* __ARM_COMPUTE_WRAPPER_BITSELECT_H__ */
