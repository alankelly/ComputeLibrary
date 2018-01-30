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
#ifndef __ARM_COMPUTE_WRAPPER_STORE_H__
#define __ARM_COMPUTE_WRAPPER_STORE_H__

#include "arm_compute/core/NEON/wrapper/traits.h"

#include <arm_neon.h>

namespace arm_compute
{
namespace wrapper
{
#define VSTORE_IMPL(stype, vtype, prefix, postfix) \
    inline void vstore(stype *ptr, vtype val)      \
    {                                              \
        prefix##_##postfix(ptr, val);              \
    }

VSTORE_IMPL(uint8_t, uint8x8_t, vst1, u8)
VSTORE_IMPL(int8_t, int8x8_t, vst1, s8)
VSTORE_IMPL(uint16_t, uint16x4_t, vst1, u16)
VSTORE_IMPL(int16_t, int16x4_t, vst1, s16)
VSTORE_IMPL(uint32_t, uint32x2_t, vst1, u32)
VSTORE_IMPL(int32_t, int32x2_t, vst1, s32)
//VSTORE_IMPL(uint64_t, 1, vst1, u64)
//VSTORE_IMPL(int64_t, 1, vst1, s64)
VSTORE_IMPL(float, float32x2_t, vst1, f32)

VSTORE_IMPL(uint8_t, uint8x16_t, vst1q, u8)
VSTORE_IMPL(int8_t, int8x16_t, vst1q, s8)
VSTORE_IMPL(uint16_t, uint16x8_t, vst1q, u16)
VSTORE_IMPL(int16_t, int16x8_t, vst1q, s16)
VSTORE_IMPL(uint32_t, uint32x4_t, vst1q, u32)
VSTORE_IMPL(int32_t, int32x4_t, vst1q, s32)
//VSTORE_IMPL(uint64_t, 2, vst1q, u64)
//VSTORE_IMPL(int64_t, 2, vst1q, s64)
VSTORE_IMPL(float, float32x4_t, vst1q, f32)
}
}
#endif /* __ARM_COMPUTE_WRAPPER_STORE_H__ */
