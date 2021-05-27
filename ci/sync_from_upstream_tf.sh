#!/usr/bin/env bash
# Copyright 2021 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

#
# Sync's the shared TfLite / TFLM code from the upstream Tensorflow repo.
#
# While the standalone TFLM repo is under development, we are also sync'ing all
# of the TFLM code via this script.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR=${SCRIPT_DIR}/..
cd "${ROOT_DIR}"

rm -rf /tmp/tensorflow

git clone https://github.com/tensorflow/tensorflow.git --depth=1 /tmp/tensorflow

SHARED_TFL_CODE="\
tensorflow/lite/c/common.c \
tensorflow/lite/core/api/error_reporter.cc \
tensorflow/lite/core/api/flatbuffer_conversions.cc \
tensorflow/lite/core/api/op_resolver.cc \
tensorflow/lite/core/api/tensor_utils.cc \
tensorflow/lite/kernels/internal/quantization_util.cc \
tensorflow/lite/kernels/kernel_util.cc \
tensorflow/lite/schema/schema_utils.cc
tensorflow/lite/c/builtin_op_data.h \
tensorflow/lite/c/c_api_types.h \
tensorflow/lite/c/common.h \
tensorflow/lite/core/api/error_reporter.h \
tensorflow/lite/core/api/flatbuffer_conversions.h \
tensorflow/lite/core/api/op_resolver.h \
tensorflow/lite/core/api/tensor_utils.h \
tensorflow/lite/kernels/internal/common.h \
tensorflow/lite/kernels/internal/compatibility.h \
tensorflow/lite/kernels/internal/quantization_util.h \
tensorflow/lite/kernels/internal/reference/add.h \
tensorflow/lite/kernels/internal/reference/add_n.h \
tensorflow/lite/kernels/internal/reference/arg_min_max.h \
tensorflow/lite/kernels/internal/reference/batch_to_space_nd.h \
tensorflow/lite/kernels/internal/reference/binary_function.h \
tensorflow/lite/kernels/internal/reference/ceil.h \
tensorflow/lite/kernels/internal/reference/comparisons.h \
tensorflow/lite/kernels/internal/reference/concatenation.h \
tensorflow/lite/kernels/internal/reference/conv.h \
tensorflow/lite/kernels/internal/reference/cumsum.h \
tensorflow/lite/kernels/internal/reference/depth_to_space.h \
tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h \
tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h \
tensorflow/lite/kernels/internal/reference/dequantize.h \
tensorflow/lite/kernels/internal/reference/elu.h \
tensorflow/lite/kernels/internal/reference/exp.h \
tensorflow/lite/kernels/internal/reference/fill.h \
tensorflow/lite/kernels/internal/reference/floor.h \
tensorflow/lite/kernels/internal/reference/floor_div.h \
tensorflow/lite/kernels/internal/reference/floor_mod.h \
tensorflow/lite/kernels/internal/reference/fully_connected.h \
tensorflow/lite/kernels/internal/reference/hard_swish.h \
tensorflow/lite/kernels/internal/reference/integer_ops/add.h \
tensorflow/lite/kernels/internal/reference/integer_ops/conv.h \
tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h \
tensorflow/lite/kernels/internal/reference/integer_ops/fully_connected.h \
tensorflow/lite/kernels/internal/reference/integer_ops/logistic.h \
tensorflow/lite/kernels/internal/reference/integer_ops/l2normalization.h \
tensorflow/lite/kernels/internal/reference/integer_ops/mean.h \
tensorflow/lite/kernels/internal/reference/integer_ops/mul.h \
tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h \
tensorflow/lite/kernels/internal/reference/integer_ops/tanh.h \
tensorflow/lite/kernels/internal/reference/integer_ops/transpose_conv.h \
tensorflow/lite/kernels/internal/reference/l2normalization.h \
tensorflow/lite/kernels/internal/reference/leaky_relu.h \
tensorflow/lite/kernels/internal/reference/log_softmax.h \
tensorflow/lite/kernels/internal/reference/maximum_minimum.h \
tensorflow/lite/kernels/internal/reference/mul.h \
tensorflow/lite/kernels/internal/reference/neg.h \
tensorflow/lite/kernels/internal/reference/pad.h \
tensorflow/lite/kernels/internal/reference/pooling.h \
tensorflow/lite/kernels/internal/reference/prelu.h \
tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h \
tensorflow/lite/kernels/internal/reference/quantize.h \
tensorflow/lite/kernels/internal/reference/reduce.h \
tensorflow/lite/kernels/internal/reference/requantize.h \
tensorflow/lite/kernels/internal/reference/resize_bilinear.h \
tensorflow/lite/kernels/internal/reference/resize_nearest_neighbor.h \
tensorflow/lite/kernels/internal/reference/round.h \
tensorflow/lite/kernels/internal/reference/softmax.h \
tensorflow/lite/kernels/internal/reference/space_to_batch_nd.h \
tensorflow/lite/kernels/internal/reference/sub.h \
tensorflow/lite/kernels/internal/reference/logistic.h \
tensorflow/lite/kernels/internal/reference/strided_slice.h \
tensorflow/lite/kernels/internal/reference/tanh.h \
tensorflow/lite/kernels/internal/reference/transpose.h \
tensorflow/lite/kernels/internal/reference/transpose_conv.h \
tensorflow/lite/kernels/internal/cppmath.h \
tensorflow/lite/kernels/internal/max.h \
tensorflow/lite/kernels/internal/min.h \
tensorflow/lite/kernels/internal/portable_tensor.h \
tensorflow/lite/kernels/internal/strided_slice_logic.h \
tensorflow/lite/kernels/internal/tensor_ctypes.h \
tensorflow/lite/kernels/internal/types.h \
tensorflow/lite/kernels/kernel_util.h \
tensorflow/lite/kernels/op_macros.h \
tensorflow/lite/kernels/padding.h \
tensorflow/lite/portable_type_to_tflitetype.h \
tensorflow/lite/schema/schema_generated.h \
tensorflow/lite/schema/schema_utils.h \
"

for filepath in ${SHARED_TFL_CODE}
do
  mkdir -p $(dirname ${filepath})
  /bin/cp /tmp/tensorflow/${filepath} ${filepath}
done

# The microfrontend is sync'd from upstream but not as part of the explicitly
# specified SHARED_TFL_CODE since this is only needed for the examples.
rm -rf tensorflow/lite/experimental/microfrontend/lib
cp -r /tmp/tensorflow/tensorflow/lite/experimental/microfrontend/lib tensorflow/lite/experimental/microfrontend/lib
