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
#ifndef __ARM_COMPUTE_TEST_DATASET_GEMM_DATASET_H__
#define __ARM_COMPUTE_TEST_DATASET_GEMM_DATASET_H__

#include "TypePrinter.h"

#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "tests/validation_old/dataset/GenericDataset.h"

#include <ostream>
#include <sstream>

#include <tuple>
#include <type_traits>

#ifdef BOOST
#include "tests/validation_old/boost_wrapper.h"
#endif /* BOOST */

namespace arm_compute
{
namespace test
{
class GEMMDataObject
{
public:
    //Data object used for matrix multiple
    //D = alpha * A * B + beta * C;
    TensorShape shape_a;
    TensorShape shape_b;
    TensorShape shape_c;
    TensorShape shape_d;
    float       alpha;
    float       beta;

    operator std::string() const
    {
        std::stringstream ss;
        ss << "GEMM";
        ss << "_A" << shape_a;
        ss << "_B" << shape_b;
        ss << "_C" << shape_c;
        ss << "_D" << shape_d;
        ss << "_alpha" << alpha;
        ss << "_beta" << beta;
        return ss.str();
    }

    friend std::ostream &operator<<(std::ostream &os, const GEMMDataObject &obj)
    {
        os << static_cast<std::string>(obj);
        return os;
    }
};

class SmallGEMMDataset : public GenericDataset<GEMMDataObject, 4>
{
public:
    SmallGEMMDataset()
        : GenericDataset
    {
        GEMMDataObject{ TensorShape(21U, 13U), TensorShape(33U, 21U), TensorShape(33U, 13U), TensorShape(33U, 13U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(31U, 1U), TensorShape(23U, 31U), TensorShape(23U, 1U), TensorShape(23U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(38U, 12U), TensorShape(21U, 38U), TensorShape(21U, 12U), TensorShape(21U, 12U), 0.2f, 1.2f },
        GEMMDataObject{ TensorShape(32U, 1U), TensorShape(17U, 32U), TensorShape(17U, 1U), TensorShape(17U, 1U), 0.4f, 0.7f },
    }
    {
    }

    ~SmallGEMMDataset() = default;
};

class LargeGEMMDataset : public GenericDataset<GEMMDataObject, 4>
{
public:
    LargeGEMMDataset()
        : GenericDataset
    {
        GEMMDataObject{ TensorShape(923U, 429U), TensorShape(871U, 923U), TensorShape(871U, 429U), TensorShape(871U, 429U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1021U, 1U), TensorShape(783U, 1021U), TensorShape(783U, 1U), TensorShape(783U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(681U, 1023U), TensorShape(213U, 681U), TensorShape(213U, 1023U), TensorShape(213U, 1023U), 0.2f, 1.2f },
        GEMMDataObject{ TensorShape(941U, 1U), TensorShape(623U, 941U), TensorShape(623U, 1U), TensorShape(623U, 1U), 0.4f, 0.7f },
    }
    {
    }

    ~LargeGEMMDataset() = default;
};

class GoogLeNetGEMMDataset1 : public GenericDataset<GEMMDataObject, 32>
{
public:
    GoogLeNetGEMMDataset1()
        : GenericDataset
    {
        GEMMDataObject{ TensorShape(147U, 12544U), TensorShape(64U, 147U), TensorShape(64U, 12544U), TensorShape(64U, 12544U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(64U, 3136U), TensorShape(64U, 64U), TensorShape(64U, 3136U), TensorShape(64U, 3136U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(576U, 3136U), TensorShape(192U, 576U), TensorShape(192U, 3136U), TensorShape(192U, 3136U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(192U, 784U), TensorShape(64U, 192U), TensorShape(64U, 784U), TensorShape(64U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(192U, 784U), TensorShape(96U, 192U), TensorShape(96U, 784U), TensorShape(96U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(864U, 784U), TensorShape(128U, 864U), TensorShape(128U, 784U), TensorShape(128U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(192U, 784U), TensorShape(16U, 192U), TensorShape(16U, 784U), TensorShape(16U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(400U, 784U), TensorShape(32U, 400U), TensorShape(32U, 784U), TensorShape(32U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(192U, 784U), TensorShape(32U, 192U), TensorShape(32U, 784U), TensorShape(32U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(256U, 784U), TensorShape(128U, 256U), TensorShape(128U, 784U), TensorShape(128U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(256U, 784U), TensorShape(128U, 256U), TensorShape(128U, 784U), TensorShape(128U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1152U, 784U), TensorShape(192U, 1152U), TensorShape(192U, 784U), TensorShape(192U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(256U, 784U), TensorShape(32U, 256U), TensorShape(32U, 784U), TensorShape(32U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(800U, 784U), TensorShape(96U, 800U), TensorShape(96U, 784U), TensorShape(96U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(256U, 784U), TensorShape(64U, 256U), TensorShape(64U, 784U), TensorShape(64U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(480U, 196U), TensorShape(192U, 480U), TensorShape(192U, 196U), TensorShape(192U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(480U, 196U), TensorShape(96U, 480U), TensorShape(96U, 196U), TensorShape(96U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(864U, 196U), TensorShape(204U, 864U), TensorShape(204U, 196U), TensorShape(204U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(480U, 196U), TensorShape(16U, 480U), TensorShape(16U, 196U), TensorShape(16U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(400U, 196U), TensorShape(48U, 400U), TensorShape(48U, 196U), TensorShape(48U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(480U, 196U), TensorShape(64U, 480U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(508U, 196U), TensorShape(160U, 508U), TensorShape(160U, 196U), TensorShape(160U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(508U, 196U), TensorShape(112U, 508U), TensorShape(112U, 196U), TensorShape(112U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1008U, 196U), TensorShape(224U, 1008U), TensorShape(224U, 196U), TensorShape(224U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(508U, 196U), TensorShape(24U, 508U), TensorShape(24U, 196U), TensorShape(24U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(600U, 196U), TensorShape(64U, 600U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(508U, 196U), TensorShape(64U, 508U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(128U, 512U), TensorShape(128U, 196U), TensorShape(128U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(128U, 512U), TensorShape(128U, 196U), TensorShape(128U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1152U, 196U), TensorShape(256U, 1152U), TensorShape(256U, 196U), TensorShape(256U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(24U, 512U), TensorShape(24U, 196U), TensorShape(24U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(600U, 196U), TensorShape(64U, 600U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f }
    }
    {
    }

    ~GoogLeNetGEMMDataset1() = default;
};

class GoogLeNetGEMMDataset2 : public GenericDataset<GEMMDataObject, 32>
{
public:
    GoogLeNetGEMMDataset2()
        : GenericDataset
    {
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(64U, 512U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(112U, 512U), TensorShape(112U, 196U), TensorShape(112U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(144U, 512U), TensorShape(144U, 196U), TensorShape(144U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1296U, 196U), TensorShape(288U, 1296U), TensorShape(288U, 196U), TensorShape(288U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(32U, 512U), TensorShape(32U, 196U), TensorShape(32U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(800U, 196U), TensorShape(64U, 800U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(512U, 196U), TensorShape(64U, 512U), TensorShape(64U, 196U), TensorShape(64U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(528U, 196U), TensorShape(256U, 528U), TensorShape(256U, 196U), TensorShape(256U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(528U, 196U), TensorShape(160U, 528U), TensorShape(160U, 196U), TensorShape(160U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1440U, 196U), TensorShape(320U, 1440U), TensorShape(320U, 196U), TensorShape(320U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(528U, 196U), TensorShape(32U, 528U), TensorShape(32U, 196U), TensorShape(32U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(800U, 196U), TensorShape(128U, 800U), TensorShape(128U, 196U), TensorShape(128U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(528U, 196U), TensorShape(128U, 528U), TensorShape(128U, 196U), TensorShape(128U, 196U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(256U, 832U), TensorShape(256U, 49U), TensorShape(256U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(160U, 832U), TensorShape(160U, 49U), TensorShape(160U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1440U, 49U), TensorShape(320U, 1440U), TensorShape(320U, 49U), TensorShape(320U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(48U, 832U), TensorShape(48U, 49U), TensorShape(48U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1200U, 49U), TensorShape(128U, 1200U), TensorShape(128U, 49U), TensorShape(128U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(128U, 832U), TensorShape(128U, 49U), TensorShape(128U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(384U, 832U), TensorShape(384U, 49U), TensorShape(384U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(192U, 832U), TensorShape(192U, 49U), TensorShape(192U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1728U, 49U), TensorShape(384U, 1728U), TensorShape(384U, 49U), TensorShape(384U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(48U, 832U), TensorShape(48U, 49U), TensorShape(48U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1200U, 49U), TensorShape(128U, 1200U), TensorShape(128U, 49U), TensorShape(128U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(832U, 49U), TensorShape(128U, 832U), TensorShape(128U, 49U), TensorShape(128U, 49U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(508U, 16U), TensorShape(128U, 508U), TensorShape(128U, 16U), TensorShape(128U, 16U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(2048U, 1U), TensorShape(1024U, 2048U), TensorShape(1024U, 1U), TensorShape(1024U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1024U, 1U), TensorShape(1008U, 1024U), TensorShape(1008U, 1U), TensorShape(1008U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(528U, 16U), TensorShape(128U, 528U), TensorShape(128U, 16U), TensorShape(128U, 16U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(2048U, 1U), TensorShape(1024U, 2048U), TensorShape(1024U, 1U), TensorShape(1024U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1024U, 1U), TensorShape(1008U, 1024U), TensorShape(1008U, 1U), TensorShape(1008U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1024U, 1U), TensorShape(1008U, 1024U), TensorShape(1008U, 1U), TensorShape(1008U, 1U), 1.0f, 0.0f }
    }
    {
    }

    ~GoogLeNetGEMMDataset2() = default;
};

class MatrixMultiplyDataset : public GenericDataset<GEMMDataObject, 3>
{
public:
    MatrixMultiplyDataset()
        : GenericDataset
    {
        GEMMDataObject{ TensorShape(1024U, 1U), TensorShape(1000U, 1024U), TensorShape(1000U, 1U), TensorShape(1000U, 1U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(256U, 784U), TensorShape(64U, 256U), TensorShape(64U, 784U), TensorShape(64U, 784U), 1.0f, 0.0f },
        GEMMDataObject{ TensorShape(1152U, 2704U), TensorShape(256U, 1152U), TensorShape(256U, 2704U), TensorShape(256U, 2704U), 1.0f, 0.0f },
    }
    {
    }

    ~MatrixMultiplyDataset() = default;
};
} // namespace test
} // namespace arm_compute
#endif //__ARM_COMPUTE_TEST_DATASET_GEMM_DATASET_H__