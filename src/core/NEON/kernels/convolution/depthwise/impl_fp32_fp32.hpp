/*
 * Copyright (c) 2018-2019 ARM Limited.
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

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *          NOTE: Header to be included by implementation files only.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include "arm_compute/core/NEON/kernels/convolution/common/arm.hpp"
#include "arm_compute/core/NEON/kernels/convolution/depthwise/impl_base.hpp"

#pragma once

using namespace neon_convolution_kernels;

namespace depthwise
{

template <
  unsigned int OutputTileRows, unsigned int OutputTileCols,
  unsigned int KernelRows, unsigned int KernelCols,
  unsigned int StrideRows, unsigned int StrideCols
>
DepthwiseConvolution<
  OutputTileRows, OutputTileCols,
  KernelRows, KernelCols, StrideRows, StrideCols,
  float, float, float
>::DepthwiseConvolution(
  int n_batches, int n_input_rows, int n_input_cols, int n_channels,
  ActivationFunction activation,
  unsigned int padding_top,
  unsigned int padding_left,
  unsigned int padding_bottom,
  unsigned int padding_right
) : Base(
      n_batches, n_input_rows, n_input_cols, n_channels, activation,
      padding_top, padding_left, padding_bottom, padding_right
    )
{
}


template <
  unsigned int OutputTileRows, unsigned int OutputTileCols,
  unsigned int KernelRows, unsigned int KernelCols,
  unsigned int StrideRows, unsigned int StrideCols
>
template <ActivationFunction Activation>
void DepthwiseConvolution<
  OutputTileRows, OutputTileCols,
  KernelRows, KernelCols, StrideRows, StrideCols,
  float, float, float
>::execute_tile(
  int n_channels,
  const void *weights_biases_ptr,
  const float *input,
  const unsigned int in_row_stride,
  const unsigned int in_col_stride,
  float *output,
  const unsigned int out_row_stride,
  const unsigned int out_col_stride
)
{
  // Instantiate pointers
  const float* __restrict__ inptr_base = input;
  float* __restrict__ outptr_base = output;
  const float* __restrict__ params = static_cast<const float*>(weights_biases_ptr);

  // Perform the depthwise convolution
  int channels_remaining = n_channels;
  for (; channels_remaining >= 4; channels_remaining -= 4)
  {
    // Load input tile
    float32x4_t u[Base::inner_tile_rows][Base::inner_tile_cols];
    for (int i = 0; i < Base::inner_tile_rows; i++)
    {
      const float* const inptr_row = inptr_base + i*in_row_stride;
      for (int j = 0; j < Base::inner_tile_cols; j++)
      {
        u[i][j] = vld1q_f32(inptr_row + j*in_col_stride);
      }
    }
    inptr_base += 4;

    // Load weights tile
    float32x4_t vbias = vld1q_f32(params);
    params += 4;

    float32x4_t w[KernelRows][KernelCols];
    for (unsigned int i = 0; i < KernelRows; i++)
    {
      for (unsigned int j = 0; j < KernelCols; j++)
      {
        w[i][j] = vld1q_f32(params);
        params += 4;
      }
    }

    // Perform the convolution
    float32x4_t v[OutputTileRows][OutputTileCols];
    for (unsigned int out_i = 0; out_i < OutputTileRows; out_i++)
    {
      for (unsigned int out_j = 0; out_j < OutputTileCols; out_j++)
      {
        v[out_i][out_j] = vbias;

        // Base co-ordinate
        const int base_i = out_i * StrideRows;
        const int base_j = out_j * StrideCols;

        // Fill the accumulator
        for (unsigned int in_i = 0; in_i < KernelRows; in_i++)
        {
          const unsigned int i = base_i + in_i;
          for (unsigned int in_j = 0; in_j < KernelCols; in_j++)
          {
            const unsigned int j = base_j + in_j;

            // v[out_i][out_j] += w[in_i][in_j] * u[i][j];
            v[out_i][out_j] = vmlaq_f32(v[out_i][out_j], w[in_i][in_j], u[i][j]);
          }
        }

        // Apply the activation function
        if (Activation == ActivationFunction::ReLU ||
            Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = vmaxq_f32(v[out_i][out_j], vdupq_n_f32(0.0f));
        }
        if (Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = vminq_f32(v[out_i][out_j], vdupq_n_f32(6.0f));
        }
      }
    }

    // Store the output tile
    for (unsigned int i = 0; i < OutputTileRows; i++)
    {
      float* const outptr_row = outptr_base + i*out_row_stride;
      for (unsigned int j = 0; j < OutputTileCols; j++)
      {
        vst1q_f32(outptr_row + j*out_col_stride, v[i][j]);
      }
    }
    outptr_base += 4;
  }
  for (; channels_remaining; channels_remaining--)
  {
    // Load input tile
    float u[Base::inner_tile_rows][Base::inner_tile_cols];
    for (int i = 0; i < Base::inner_tile_rows; i++)
    {
      const float* const inptr_row = inptr_base + i*in_row_stride;
      for (int j = 0; j < Base::inner_tile_cols; j++)
      {
        u[i][j] = *(inptr_row + j*in_col_stride);
      }
    }
    inptr_base++;

    // Load weights tile
    float bias = *(params++);
    float w[KernelRows][KernelCols];
    for (unsigned int i = 0; i < KernelRows; i++)
    {
      for (unsigned int j = 0; j < KernelCols; j++)
      {
        w[i][j] = *(params++);
      }
    }

    // Perform the convolution
    float v[OutputTileRows][OutputTileCols];
    for (unsigned int out_i = 0; out_i < OutputTileRows; out_i++)
    {
      for (unsigned int out_j = 0; out_j < OutputTileCols; out_j++)
      {
        // Clear the accumulator
        v[out_i][out_j] = bias;

        // Base co-ordinate
        const int base_i = out_i * StrideRows;
        const int base_j = out_j * StrideCols;

        // Fill the accumulator
        for (unsigned int in_i = 0; in_i < KernelRows; in_i++)
        {
          const unsigned int i = base_i + in_i;
          for (unsigned int in_j = 0; in_j < KernelCols; in_j++)
          {
            const int j = base_j + in_j;
            v[out_i][out_j] += w[in_i][in_j] * u[i][j];
          }
        }

        // Apply the activation function
        if (Activation == ActivationFunction::ReLU ||
            Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = std::max(0.0f, v[out_i][out_j]);
        }
        if (Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = std::min(6.0f, v[out_i][out_j]);
        }
      }
    }

    // Store the output tile
    for (unsigned int i = 0; i < OutputTileRows; i++)
    {
      float* const outptr_row = outptr_base + i*out_row_stride;
      for (unsigned int j = 0; j < OutputTileCols; j++)
      {
        *(outptr_row + j*out_col_stride) = v[i][j];
      }
    }
    outptr_base++;
  }
}


template <
  unsigned int OutputTileRows, unsigned int OutputTileCols,
  unsigned int KernelRows, unsigned int KernelCols,
  unsigned int StrideRows, unsigned int StrideCols
>
template <ActivationFunction Activation>
void DepthwiseConvolution<
  OutputTileRows, OutputTileCols,
  KernelRows, KernelCols, StrideRows, StrideCols,
  float, float, float
>::execute_tile(
  int n_channels,
  const void *weights_biases_ptr,
  const float *inptrs[Base::inner_tile_rows][Base::inner_tile_cols],
  float *outptrs[Base::output_tile_rows][Base::output_tile_cols]
)
{
  const float* __restrict__ params = static_cast<const float*>(weights_biases_ptr);

  // Perform the depthwise convolution
  int channels_remaining = n_channels;
  int n = 0;
  for (; channels_remaining >= 4; channels_remaining -= 4, n += 4)
  {
    // Load input tile
    float32x4_t u[Base::inner_tile_rows][Base::inner_tile_cols];
    for (int i = 0; i < Base::inner_tile_rows; i++)
    {
      for (int j = 0; j < Base::inner_tile_cols; j++)
      {
        u[i][j] = vld1q_f32(inptrs[i][j] + n);
      }
    }

    // Load weights tile
    float32x4_t vbias = vld1q_f32(params);
    params += 4;

    float32x4_t w[KernelRows][KernelCols];
    for (unsigned int i = 0; i < KernelRows; i++)
    {
      for (unsigned int j = 0; j < KernelCols; j++)
      {
        w[i][j] = vld1q_f32(params);
        params += 4;
      }
    }

    // Perform the convolution
    float32x4_t v[OutputTileRows][OutputTileCols];
    for (unsigned int out_i = 0; out_i < OutputTileRows; out_i++)
    {
      for (unsigned int out_j = 0; out_j < OutputTileCols; out_j++)
      {
        v[out_i][out_j] = vbias;

        // Base co-ordinate
        const int base_i = out_i * StrideRows;
        const int base_j = out_j * StrideCols;

        // Fill the accumulator
        for (unsigned int in_i = 0; in_i < KernelRows; in_i++)
        {
          const unsigned int i = base_i + in_i;
          for (unsigned int in_j = 0; in_j < KernelCols; in_j++)
          {
            const unsigned int j = base_j + in_j;

            // v[out_i][out_j] += w[in_i][in_j] * u[i][j];
            v[out_i][out_j] = vmlaq_f32(v[out_i][out_j], w[in_i][in_j], u[i][j]);
          }
        }

        // Apply the activation function
        if (Activation == ActivationFunction::ReLU ||
            Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = vmaxq_f32(v[out_i][out_j], vdupq_n_f32(0.0f));
        }
        if (Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = vminq_f32(v[out_i][out_j], vdupq_n_f32(6.0f));
        }
      }
    }

    // Store the output tile
    for (unsigned int i = 0; i < OutputTileRows; i++)
    {
      for (unsigned int j = 0; j < OutputTileCols; j++)
      {
        vst1q_f32(outptrs[i][j] + n, v[i][j]);
      }
    }
  }
  for (; channels_remaining; channels_remaining--, n++)
  {
    // Load input tile
    float u[Base::inner_tile_rows][Base::inner_tile_cols];
    for (int i = 0; i < Base::inner_tile_rows; i++)
    {
      for (int j = 0; j < Base::inner_tile_cols; j++)
      {
        u[i][j] = *(inptrs[i][j] + n);
      }
    }

    // Load weights tile
    float bias = *(params++);
    float w[KernelRows][KernelCols];
    for (unsigned int i = 0; i < KernelRows; i++)
    {
      for (unsigned int j = 0; j < KernelCols; j++)
      {
        w[i][j] = *(params++);
      }
    }

    // Perform the convolution
    float v[OutputTileRows][OutputTileCols];
    for (unsigned int out_i = 0; out_i < OutputTileRows; out_i++)
    {
      for (unsigned int out_j = 0; out_j < OutputTileCols; out_j++)
      {
        // Clear the accumulator
        v[out_i][out_j] = bias;

        // Base co-ordinate
        const int base_i = out_i * StrideRows;
        const int base_j = out_j * StrideCols;

        // Fill the accumulator
        for (unsigned int in_i = 0; in_i < KernelRows; in_i++)
        {
          const unsigned int i = base_i + in_i;
          for (unsigned int in_j = 0; in_j < KernelCols; in_j++)
          {
            const int j = base_j + in_j;
            v[out_i][out_j] += w[in_i][in_j] * u[i][j];
          }
        }

        // Apply the activation function
        if (Activation == ActivationFunction::ReLU ||
            Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = std::max(0.0f, v[out_i][out_j]);
        }
        if (Activation == ActivationFunction::ReLU6)
        {
          v[out_i][out_j] = std::min(6.0f, v[out_i][out_j]);
        }
      }
    }

    // Store the output tile
    for (unsigned int i = 0; i < OutputTileRows; i++)
    {
      for (unsigned int j = 0; j < OutputTileCols; j++)
      {
        *(outptrs[i][j] + n) = v[i][j];
      }
    }
  }
}

}  // namespace depthwise
