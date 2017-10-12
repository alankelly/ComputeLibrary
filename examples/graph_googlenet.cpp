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
#ifndef ARM_COMPUTE_CL /* Needed by Utils.cpp to handle OpenCL exceptions properly */
#error "This example needs to be built with -DARM_COMPUTE_CL"
#endif /* ARM_COMPUTE_CL */

#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/Nodes.h"
#include "arm_compute/graph/SubGraph.h"
#include "arm_compute/runtime/CL/CLScheduler.h"
#include "arm_compute/runtime/Scheduler.h"
#include "support/ToolchainSupport.h"
#include "utils/GraphUtils.h"
#include "utils/Utils.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <tuple>

using namespace arm_compute::graph;
using namespace arm_compute::graph_utils;

/** Generates appropriate accessor according to the specified path
 *
 * @note If path is empty will generate a DummyAccessor else will generate a NumPyBinLoader
 *
 * @param path       Path to the data files
 * @param data_file  Relative path to the data files from path
 *
 * @return An appropriate tensor accessor
 */
std::unique_ptr<ITensorAccessor> get_accessor(const std::string &path, const std::string &data_file)
{
    if(path.empty())
    {
        return arm_compute::support::cpp14::make_unique<DummyAccessor>();
    }
    else
    {
        return arm_compute::support::cpp14::make_unique<NumPyBinLoader>(path + data_file);
    }
}

BranchLayer get_inception_node(const std::string &data_path, std::string &&param_path,
                               unsigned int a_filt,
                               std::tuple<unsigned int, unsigned int> b_filters,
                               std::tuple<unsigned int, unsigned int> c_filters,
                               unsigned int d_filt)
{
    std::string total_path = "/cnn_data/googlenet_model/" + param_path + "/" + param_path + "_";
    SubGraph    i_a;
    i_a << ConvolutionLayer(
            1U, 1U, a_filt,
            get_accessor(data_path, total_path + "1x1_w.npy"),
            get_accessor(data_path, total_path + "1x1_b.npy"),
            PadStrideInfo(1, 1, 0, 0))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU));

    SubGraph i_b;
    i_b << ConvolutionLayer(
            1U, 1U, std::get<0>(b_filters),
            get_accessor(data_path, total_path + "3x3_reduce_w.npy"),
            get_accessor(data_path, total_path + "3x3_reduce_b.npy"),
            PadStrideInfo(1, 1, 0, 0))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU))
        << ConvolutionLayer(
            3U, 3U, std::get<1>(b_filters),
            get_accessor(data_path, total_path + "3x3_w.npy"),
            get_accessor(data_path, total_path + "3x3_b.npy"),
            PadStrideInfo(1, 1, 1, 1))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU));

    SubGraph i_c;
    i_c << ConvolutionLayer(
            1U, 1U, std::get<0>(c_filters),
            get_accessor(data_path, total_path + "5x5_reduce_w.npy"),
            get_accessor(data_path, total_path + "5x5_reduce_b.npy"),
            PadStrideInfo(1, 1, 0, 0))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU))
        << ConvolutionLayer(
            5U, 5U, std::get<1>(c_filters),
            get_accessor(data_path, total_path + "5x5_w.npy"),
            get_accessor(data_path, total_path + "5x5_b.npy"),
            PadStrideInfo(1, 1, 2, 2))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU));

    SubGraph i_d;
    i_d << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, PadStrideInfo(1, 1, 1, 1, DimensionRoundingType::CEIL)))
        << ConvolutionLayer(
            1U, 1U, d_filt,
            get_accessor(data_path, total_path + "pool_proj_w.npy"),
            get_accessor(data_path, total_path + "pool_proj_b.npy"),
            PadStrideInfo(1, 1, 0, 0))
        << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU));

    return BranchLayer(BranchMergeMethod::DEPTH_CONCATENATE, std::move(i_a), std::move(i_b), std::move(i_c), std::move(i_d));
}

/** Example demonstrating how to implement Googlenet's network using the Compute Library's graph API
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments ( [optional] Path to the weights folder, [optional] batches )
 */
void main_graph_googlenet(int argc, const char **argv)
{
    std::string  data_path;   /** Path to the trainable data */
    unsigned int batches = 4; /** Number of batches */

    // Parse arguments
    if(argc < 2)
    {
        // Print help
        std::cout << "Usage: " << argv[0] << " [path_to_data] [batches]\n\n";
        std::cout << "No data folder provided: using random values\n\n";
    }
    else if(argc == 2)
    {
        //Do something with argv[1]
        data_path = argv[1];
        std::cout << "Usage: " << argv[0] << " [path_to_data] [batches]\n\n";
        std::cout << "No number of batches where specified, thus will use the default : " << batches << "\n\n";
    }
    else
    {
        //Do something with argv[1] and argv[2]
        data_path = argv[1];
        batches   = std::strtol(argv[2], nullptr, 0);
    }

    // Check if OpenCL is available and initialize the scheduler
    if(arm_compute::opencl_is_available())
    {
        arm_compute::CLScheduler::get().default_init();
    }

    Graph graph;

    graph << TargetHint::OPENCL
          << Tensor(TensorInfo(TensorShape(224U, 224U, 3U, batches), 1, DataType::F32), DummyAccessor())
          << ConvolutionLayer(
              7U, 7U, 64U,
              get_accessor(data_path, "/cnn_data/googlenet_model/conv1/conv1_7x7_s2_w.npy"),
              get_accessor(data_path, "/cnn_data/googlenet_model/conv1/conv1_7x7_s2_b.npy"),
              PadStrideInfo(2, 2, 3, 3))
          << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU))
          << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, PadStrideInfo(2, 2, 0, 0, DimensionRoundingType::CEIL)))
          << NormalizationLayer(NormalizationLayerInfo(NormType::CROSS_MAP, 5, 0.0001f, 0.75f))
          << ConvolutionLayer(
              1U, 1U, 64U,
              get_accessor(data_path, "/cnn_data/googlenet_model/conv2/conv2_3x3_reduce_w.npy"),
              get_accessor(data_path, "/cnn_data/googlenet_model/conv2/conv2_3x3_reduce_b.npy"),
              PadStrideInfo(1, 1, 0, 0))
          << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU))
          << ConvolutionLayer(
              3U, 3U, 192U,
              get_accessor(data_path, "/cnn_data/googlenet_model/conv2/conv2_3x3_w.npy"),
              get_accessor(data_path, "/cnn_data/googlenet_model/conv2/conv2_3x3_b.npy"),
              PadStrideInfo(1, 1, 1, 1))
          << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU))
          << NormalizationLayer(NormalizationLayerInfo(NormType::CROSS_MAP, 5, 0.0001f, 0.75f))
          << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, PadStrideInfo(2, 2, 0, 0, DimensionRoundingType::CEIL)))
          << get_inception_node(data_path, "inception_3a", 64, std::make_tuple(96U, 128U), std::make_tuple(16U, 32U), 32U)
          << get_inception_node(data_path, "inception_3b", 128, std::make_tuple(128U, 192U), std::make_tuple(32U, 96U), 64U)
          << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, PadStrideInfo(2, 2, 0, 0, DimensionRoundingType::CEIL)))
          << get_inception_node(data_path, "inception_4a", 192, std::make_tuple(96U, 208U), std::make_tuple(16U, 48U), 64U)
          << get_inception_node(data_path, "inception_4b", 160, std::make_tuple(112U, 224U), std::make_tuple(24U, 64U), 64U)
          << get_inception_node(data_path, "inception_4c", 128, std::make_tuple(128U, 256U), std::make_tuple(24U, 64U), 64U)
          << get_inception_node(data_path, "inception_4d", 112, std::make_tuple(144U, 288U), std::make_tuple(32U, 64U), 64U)
          << get_inception_node(data_path, "inception_4e", 256, std::make_tuple(160U, 320U), std::make_tuple(32U, 128U), 128U)
          << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, PadStrideInfo(2, 2, 0, 0, DimensionRoundingType::CEIL)))
          << get_inception_node(data_path, "inception_5a", 256, std::make_tuple(160U, 320U), std::make_tuple(32U, 128U), 128U)
          << get_inception_node(data_path, "inception_5b", 384, std::make_tuple(192U, 384U), std::make_tuple(48U, 128U), 128U)
          << PoolingLayer(PoolingLayerInfo(PoolingType::AVG, 7, PadStrideInfo(1, 1, 0, 0, DimensionRoundingType::CEIL)))
          << FullyConnectedLayer(
              1000U,
              get_accessor(data_path, "/cnn_data/googlenet_model/loss3/loss3_classifier_w.npy"),
              get_accessor(data_path, "/cnn_data/googlenet_model/loss3/loss3_classifier_b.npy"))
          << SoftmaxLayer()
          << Tensor(DummyAccessor());

    graph.run();
}

/** Main program for Googlenet
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments ( [optional] Path to the weights folder, [optional] batches )
 */
int main(int argc, const char **argv)
{
    return arm_compute::utils::run_example(argc, argv, main_graph_googlenet);
}