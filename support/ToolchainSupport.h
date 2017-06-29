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
#ifndef ARM_COMPUTE_TEST_TOOLCHAINSUPPORT
#define ARM_COMPUTE_TEST_TOOLCHAINSUPPORT

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>

namespace arm_compute
{
namespace support
{
namespace cpp11
{
#ifdef __ANDROID__
/** Convert integer and float values to string.
 *
 * @note This function implements the same behaviour as std::to_string. The
 *       latter is missing in some Android toolchains.
 *
 * @param[in] value Value to be converted to string.
 *
 * @return String representation of @p value.
 */
template <typename T, typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value, int>::type = 0>
std::string to_string(T && value)
{
    std::stringstream stream;
    stream << std::forward<T>(value);
    return stream.str();
}

/** Convert string values to integer.
 *
 * @note This function implements the same behaviour as std::stoi. The latter
 *       is missing in some Android toolchains.
 *
 * @param[in] str String to be converted to int.
 *
 * @return Integer representation of @p str.
 */
inline int stoi(const std::string &str)
{
    std::stringstream stream(str);
    int               value = 0;
    stream >> value;
    return value;
}

/** Convert string values to unsigned long.
 *
 * @note This function implements the same behaviour as std::stoul. The latter
 *       is missing in some Android toolchains.
 *
 * @param[in] str String to be converted to unsigned long.
 *
 * @return Unsigned long representation of @p str.
 */
inline unsigned long stoul(const std::string &str)
{
    std::stringstream stream(str);
    unsigned long     value = 0;
    stream >> value;
    return value;
}

/** Convert string values to float.
 *
 * @note This function implements the same behaviour as std::stof. The latter
 *       is missing in some Android toolchains.
 *
 * @param[in] str String to be converted to float.
 *
 * @return Float representation of @p str.
 */
inline float stof(const std::string &str)
{
    std::stringstream stream(str);
    float             value = 0.f;
    stream >> value;
    return value;
}

/** Round floating-point value with half value rounding away from zero.
 *
 * @note This function implements the same behaviour as std::round except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] value floating-point value to be rounded.
 *
 * @return Floating-point value of rounded @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T round(T value)
{
    return ::round(value);
}

/** Truncate floating-point value.
 *
 * @note This function implements the same behaviour as std::truncate except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] value floating-point value to be truncated.
 *
 * @return Floating-point value of truncated @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T trunc(T value)
{
    return ::trunc(value);
}

/** Composes a floating point value with the magnitude of @p x and the sign of @p y.
 *
 * @note This function implements the same behaviour as std::copysign except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] x value that contains the magnitued to be used in constructing the result.
 * @param[in] y value that contains the sign to be used in constructin the result.
 *
 * @return Floating-point value with magnitude of @p x and sign of @p y.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T copysign(T x, T y)
{
    return ::copysign(x, y);
}
#else
/** Convert integer and float values to string.
 *
 * @note This function acts as a convenience wrapper around std::to_string. The
 *       latter is missing in some Android toolchains.
 *
 * @param[in] value Value to be converted to string.
 *
 * @return String representation of @p value.
 */
template <typename T>
std::string to_string(T &&value)
{
    return ::std::to_string(std::forward<T>(value));
}

/** Convert string values to integer.
 *
 * @note This function acts as a convenience wrapper around std::stoi. The
 *       latter is missing in some Android toolchains.
 *
 * @param[in] args Arguments forwarded to std::stoi.
 *
 * @return Integer representation of input string.
 */
template <typename... Ts>
int stoi(Ts &&... args)
{
    return ::std::stoi(std::forward<Ts>(args)...);
}

/** Convert string values to unsigned long.
 *
 * @note This function acts as a convenience wrapper around std::stoul. The
 *       latter is missing in some Android toolchains.
 *
 * @param[in] args Arguments forwarded to std::stoul.
 *
 * @return Unsigned long representation of input string.
 */
template <typename... Ts>
int stoul(Ts &&... args)
{
    return ::std::stoul(std::forward<Ts>(args)...);
}

/** Convert string values to float.
 *
 * @note This function acts as a convenience wrapper around std::stof. The
 *       latter is missing in some Android toolchains.
 *
 * @param[in] args Arguments forwarded to std::stof.
 *
 * @return Float representation of input string.
 */
template <typename... Ts>
int stof(Ts &&... args)
{
    return ::std::stof(std::forward<Ts>(args)...);
}

/** Round floating-point value with half value rounding away from zero.
 *
 * @note This function implements the same behaviour as std::round except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] value floating-point value to be rounded.
 *
 * @return Floating-point value of rounded @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T round(T value)
{
    return std::round(value);
}

/** Truncate floating-point value.
 *
 * @note This function implements the same behaviour as std::truncate except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] value floating-point value to be truncated.
 *
 * @return Floating-point value of truncated @p value.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T trunc(T value)
{
    return std::trunc(value);
}

/** Composes a floating point value with the magnitude of @p x and the sign of @p y.
 *
 * @note This function implements the same behaviour as std::copysign except that it doesn't
 *       support Integral type. The latter is not in the namespace std in some Android toolchains.
 *
 * @param[in] x value that contains the magnitued to be used in constructing the result.
 * @param[in] y value that contains the sign to be used in constructin the result.
 *
 * @return Floating-point value with magnitude of @p x and sign of @p y.
 */
template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
inline T copysign(T x, T y)
{
    return std::copysign(x, y);
}
#endif
} // namespace cpp11

namespace cpp14
{
/** make_unqiue is missing in CPP11. Reimplement it according to the standard
 * proposal.
 */
template <class T>
struct _Unique_if
{
    typedef std::unique_ptr<T> _Single_object;
};

template <class T>
struct _Unique_if<T[]>
{
    typedef std::unique_ptr<T[]> _Unknown_bound;
};

template <class T, size_t N>
struct _Unique_if<T[N]>
{
    typedef void _Known_bound;
};

template <class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n)
{
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]());
}

template <class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args &&...) = delete;
} // namespace cpp14
} // namespace support
} // namespace arm_compute
#endif
