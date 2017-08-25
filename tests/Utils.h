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
#ifndef __ARM_COMPUTE_TEST_UTILS_H__
#define __ARM_COMPUTE_TEST_UTILS_H__

#include "arm_compute/core/Coordinates.h"
#include "arm_compute/core/Error.h"
#include "arm_compute/core/FixedPoint.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "support/ToolchainSupport.h"
#include "tests/validation_new/half.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

namespace arm_compute
{
namespace test
{
/** Round floating-point value with half value rounding to positive infinity.
 *
 * @param[in] value floating-point value to be rounded.
 *
 * @return Floating-point value of rounded @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T round_half_up(T value)
{
    return std::floor(value + 0.5f);
}

/** Round floating-point value with half value rounding to nearest even.
 *
 * @param[in] value   floating-point value to be rounded.
 * @param[in] epsilon precision.
 *
 * @return Floating-point value of rounded @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T round_half_even(T value, T epsilon = std::numeric_limits<T>::epsilon())
{
    T positive_value = std::abs(value);
    T ipart          = 0;
    std::modf(positive_value, &ipart);
    // If 'value' is exactly halfway between two integers
    if(std::abs(positive_value - (ipart + 0.5f)) < epsilon)
    {
        // If 'ipart' is even then return 'ipart'
        if(std::fmod(ipart, 2.f) < epsilon)
        {
            return support::cpp11::copysign(ipart, value);
        }
        // Else return the nearest even integer
        return support::cpp11::copysign(std::ceil(ipart + 0.5f), value);
    }
    // Otherwise use the usual round to closest
    return support::cpp11::copysign(support::cpp11::round(positive_value), value);
}

namespace traits
{
// *INDENT-OFF*
// clang-format off
template <typename T> struct promote { };
template <> struct promote<uint8_t> { using type = uint16_t; };
template <> struct promote<int8_t> { using type = int16_t; };
template <> struct promote<uint16_t> { using type = uint32_t; };
template <> struct promote<int16_t> { using type = int32_t; };
template <> struct promote<uint32_t> { using type = uint64_t; };
template <> struct promote<int32_t> { using type = int64_t; };
template <> struct promote<float> { using type = float; };
template <> struct promote<half_float::half> { using type = half_float::half; };


template <typename T>
using promote_t = typename promote<T>::type;

template <typename T>
using make_signed_conditional_t = typename std::conditional<std::is_integral<T>::value, std::make_signed<T>, std::common_type<T>>::type;
// clang-format on
// *INDENT-ON*
}

/** Look up the format corresponding to a channel.
 *
 * @param[in] channel Channel type.
 *
 * @return Format that contains the given channel.
 */
inline Format get_format_for_channel(Channel channel)
{
    switch(channel)
    {
        case Channel::R:
        case Channel::G:
        case Channel::B:
            return Format::RGB888;
        default:
            throw std::runtime_error("Unsupported channel");
    }
}

/** Return the format of a channel.
 *
 * @param[in] channel Channel type.
 *
 * @return Format of the given channel.
 */
inline Format get_channel_format(Channel channel)
{
    switch(channel)
    {
        case Channel::R:
        case Channel::G:
        case Channel::B:
            return Format::U8;
        default:
            throw std::runtime_error("Unsupported channel");
    }
}

/** Base case of foldl.
 *
 * @return value.
 */
template <typename F, typename T>
inline T foldl(F &&, const T &value)
{
    return value;
}

/** Base case of foldl.
 *
 * @return func(value1, value2).
 */
template <typename F, typename T, typename U>
inline auto foldl(F &&func, T &&value1, U &&value2) -> decltype(func(value1, value2))
{
    return func(value1, value2);
}

/** Fold left.
 *
 * @param[in] func    Binary function to be called.
 * @param[in] initial Initial value.
 * @param[in] value   Argument passed to the function.
 * @param[in] values  Remaining arguments.
 */
template <typename F, typename I, typename T, typename... Vs>
inline I foldl(F &&func, I &&initial, T &&value, Vs &&... values)
{
    return foldl(std::forward<F>(func), func(std::forward<I>(initial), std::forward<T>(value)), std::forward<Vs>(values)...);
}

/** Create a valid region based on tensor shape, border mode and border size
 *
 * @param[in] shape            Shape used as size of the valid region.
 * @param[in] border_undefined (Optional) Boolean indicating if the border mode is undefined.
 * @param[in] border_size      (Optional) Border size used to specify the region to exclude.
 *
 * @return A valid region starting at (0, 0, ...) with size of @p shape if @p border_undefined is false; otherwise
 *  return A valid region starting at (@p border_size.left, @p border_size.top, ...) with reduced size of @p shape.
 */
inline ValidRegion shape_to_valid_region(TensorShape shape, bool border_undefined = false, BorderSize border_size = BorderSize(0))
{
    Coordinates anchor;
    anchor.set_num_dimensions(shape.num_dimensions());

    if(border_undefined)
    {
        ARM_COMPUTE_ERROR_ON(shape.num_dimensions() < 2);

        anchor.set(0, border_size.left);
        anchor.set(1, border_size.top);

        const int valid_shape_x = std::max(0, static_cast<int>(shape.x()) - static_cast<int>(border_size.left) - static_cast<int>(border_size.right));
        const int valid_shape_y = std::max(0, static_cast<int>(shape.y()) - static_cast<int>(border_size.top) - static_cast<int>(border_size.bottom));

        shape.set(0, valid_shape_x);
        shape.set(1, valid_shape_y);
    }

    return ValidRegion(std::move(anchor), std::move(shape));
}

/** Write the value after casting the pointer according to @p data_type.
 *
 * @warning The type of the value must match the specified data type.
 *
 * @param[out] ptr       Pointer to memory where the @p value will be written.
 * @param[in]  value     Value that will be written.
 * @param[in]  data_type Data type that will be written.
 */
template <typename T>
void store_value_with_data_type(void *ptr, T value, DataType data_type)
{
    switch(data_type)
    {
        case DataType::U8:
            *reinterpret_cast<uint8_t *>(ptr) = value;
            break;
        case DataType::S8:
        case DataType::QS8:
            *reinterpret_cast<int8_t *>(ptr) = value;
            break;
        case DataType::U16:
            *reinterpret_cast<uint16_t *>(ptr) = value;
            break;
        case DataType::S16:
        case DataType::QS16:
            *reinterpret_cast<int16_t *>(ptr) = value;
            break;
        case DataType::U32:
            *reinterpret_cast<uint32_t *>(ptr) = value;
            break;
        case DataType::S32:
            *reinterpret_cast<int32_t *>(ptr) = value;
            break;
        case DataType::U64:
            *reinterpret_cast<uint64_t *>(ptr) = value;
            break;
        case DataType::S64:
            *reinterpret_cast<int64_t *>(ptr) = value;
            break;
        case DataType::F16:
            *reinterpret_cast<half_float::half *>(ptr) = value;
            break;
        case DataType::F32:
            *reinterpret_cast<float *>(ptr) = value;
            break;
        case DataType::F64:
            *reinterpret_cast<double *>(ptr) = value;
            break;
        case DataType::SIZET:
            *reinterpret_cast<size_t *>(ptr) = value;
            break;
        default:
            ARM_COMPUTE_ERROR("NOT SUPPORTED!");
    }
}

/** Saturate a value of type T against the numeric limits of type U.
 *
 * @param[in] val Value to be saturated.
 *
 * @return saturated value.
 */
template <typename U, typename T>
T saturate_cast(T val)
{
    if(val > static_cast<T>(std::numeric_limits<U>::max()))
    {
        val = static_cast<T>(std::numeric_limits<U>::max());
    }
    if(val < static_cast<T>(std::numeric_limits<U>::lowest()))
    {
        val = static_cast<T>(std::numeric_limits<U>::lowest());
    }
    return val;
}

/** Find the signed promoted common type.
 */
template <typename... T>
struct common_promoted_signed_type
{
    using common_type       = typename std::common_type<T...>::type;
    using promoted_type     = traits::promote_t<common_type>;
    using intermediate_type = typename traits::make_signed_conditional_t<promoted_type>::type;
};

/** Convert a linear index into n-dimensional coordinates.
 *
 * @param[in] shape Shape of the n-dimensional tensor.
 * @param[in] index Linear index specifying the i-th element.
 *
 * @return n-dimensional coordinates.
 */
inline Coordinates index2coord(const TensorShape &shape, int index)
{
    int num_elements = shape.total_size();

    ARM_COMPUTE_ERROR_ON_MSG(index < 0 || index >= num_elements, "Index has to be in [0, num_elements]");
    ARM_COMPUTE_ERROR_ON_MSG(num_elements == 0, "Cannot create coordinate from empty shape");

    Coordinates coord{ 0 };

    for(int d = shape.num_dimensions() - 1; d >= 0; --d)
    {
        num_elements /= shape[d];
        coord.set(d, index / num_elements);
        index %= num_elements;
    }

    return coord;
}

/** Linearise the given coordinate.
 *
 * Transforms the given coordinate into a linear offset in terms of
 * elements.
 *
 * @param[in] shape Shape of the n-dimensional tensor.
 * @param[in] coord The to be converted coordinate.
 *
 * @return Linear offset to the element.
 */
inline int coord2index(const TensorShape &shape, const Coordinates &coord)
{
    ARM_COMPUTE_ERROR_ON_MSG(shape.total_size() == 0, "Cannot get index from empty shape");
    ARM_COMPUTE_ERROR_ON_MSG(coord.num_dimensions() == 0, "Cannot get index of empty coordinate");

    int index    = 0;
    int dim_size = 1;

    for(unsigned int i = 0; i < coord.num_dimensions(); ++i)
    {
        index += coord[i] * dim_size;
        dim_size *= shape[i];
    }

    return index;
}

/** Check if Coordinates dimensionality can match the respective shape one.
 *
 * @param coords Coordinates
 * @param shape  Shape to match dimensionality
 *
 * @return True if Coordinates can match the dimensionality of the shape else false.
 */
inline bool match_shape(Coordinates &coords, const TensorShape &shape)
{
    auto check_nz = [](int i)
    {
        return i != 0;
    };

    const int coords_dims = coords.num_dimensions();
    const int shape_dims  = shape.num_dimensions();

    // Increase coordinates scenario
    if(coords_dims < shape_dims)
    {
        coords.set_num_dimensions(shape_dims);
        return true;
    }
    // Decrease coordinates scenario
    if(coords_dims > shape_dims && !std::any_of(coords.begin() + shape_dims, coords.end(), check_nz))
    {
        coords.set_num_dimensions(shape_dims);
        return true;
    }

    return (coords_dims == shape_dims);
}

/** Check if a coordinate is within a valid region */
inline bool is_in_valid_region(const ValidRegion &valid_region, Coordinates coord)
{
    const bool match = match_shape(coord, valid_region.shape);

    if(!match)
    {
        return false;
    }

    for(int d = 0; static_cast<size_t>(d) < coord.num_dimensions(); ++d)
    {
        if(coord[d] < valid_region.start(d) || coord[d] >= valid_region.end(d))
        {
            return false;
        }
    }

    return true;
}

/** Create and initialize a tensor of the given type.
 *
 * @param[in] shape                Tensor shape.
 * @param[in] data_type            Data type.
 * @param[in] num_channels         (Optional) Number of channels.
 * @param[in] fixed_point_position (Optional) Number of fractional bits.
 *
 * @return Initialized tensor of given type.
 */
template <typename T>
inline T create_tensor(const TensorShape &shape, DataType data_type, int num_channels = 1, int fixed_point_position = 0)
{
    T tensor;
    tensor.allocator()->init(TensorInfo(shape, num_channels, data_type, fixed_point_position));

    return tensor;
}
} // namespace test
} // namespace arm_compute
#endif /* __ARM_COMPUTE_TEST_UTILS_H__ */
