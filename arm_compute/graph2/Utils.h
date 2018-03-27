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
#ifndef __ARM_COMPUTE_GRAPH2_UTILS_H__
#define __ARM_COMPUTE_GRAPH2_UTILS_H__

#include "arm_compute/graph2/Graph.h"
#include "arm_compute/graph2/PassManager.h"

namespace arm_compute
{
namespace graph2
{
// Forward Declaration
class GraphContext;

/** Returns the tensor descriptor of a given tensor
 *
 * @param[in] g   Graph that the tensor belongs to
 * @param[in] tid Tensor ID
 *
 * @return Tensor descriptor if tensor was found else empty descriptor
 */
inline TensorDescriptor get_tensor_descriptor(const Graph &g, TensorID tid)
{
    const Tensor *tensor = g.tensor(tid);
    return (tensor != nullptr) ? tensor->desc() : TensorDescriptor();
}
/** Sets an accessor on a given tensor
 *
 * @param[in] tensor   Tensor to set the accessor to
 * @param[in] accessor Accessor to set
 *
 * @return True if accessor was set else false
 */
inline Status set_tensor_accessor(Tensor *tensor, std::unique_ptr<ITensorAccessor> accessor)
{
    ARM_COMPUTE_RETURN_ERROR_ON(tensor == nullptr);
    tensor->set_accessor(std::move(accessor));

    return Status{};
}
/** Checks if a specific target is supported
 *
 * @param[in] target Target to check
 *
 * @return True if target is support else false
 */
bool is_target_supported(Target target);
/** Returns default target for execution
 *
 * @note If an OpenCL backend exists then OpenCL is returned,
 *       else if the NEON backend exists returns NEON as target.
 *       If no backends are registered an error is raised.
 *
 * @return Default target
 */
Target get_default_target();
/** Forces a single target to all graph constructs
 *
 * @param[in] g      Graph to force target on
 * @param[in] target Target to force
 */
void force_target_to_graph(Graph &g, Target target);
/** Creates a default @ref PassManager
 *
 * @param[in] target Target to create the pass manager for
 *
 * @return A PassManager with default mutating passes
 */
PassManager create_default_pass_manager(Target target);
/** Default setups the graph context if not done manually
 *
 * @param[in] ctx Graph Context
 */
void setup_default_graph_context(GraphContext &ctx);
} // namespace graph2
} // namespace arm_compute
#endif /* __ARM_COMPUTE_GRAPH2_UTILS_H__ */
