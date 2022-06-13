/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_DIV_H_
#define TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_DIV_H_

#include <algorithm>
#include <limits>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"

namespace tflite {

// Element-wise div that can often be used for inner loop of broadcast Div as
// well as the non-broadcast Div.
inline void DivElementwise(int size, const ArithmeticParams& params,
                           const int8_t* input1_data, const int8_t* input2_data,
                           int8_t* output_data) {
  TFLITE_DCHECK_GT(params.input1_offset, -256);
  TFLITE_DCHECK_LT(params.input1_offset, 256);
  TFLITE_DCHECK_GT(params.input2_offset, -256);
  TFLITE_DCHECK_LT(params.input2_offset, 256);
  TFLITE_DCHECK_GT(params.output_offset, -256);
  TFLITE_DCHECK_LT(params.output_offset, 256);

  for (int i = 0; i < size; ++i) {
    int32_t input1_val = params.input1_offset + input1_data[i];
    int32_t input2_val = params.input2_offset + input2_data[i];
    TFLITE_DCHECK_NE(input2_val, 0);
    if (input2_val < 0) {
      // Invert signs to avoid a negative input2_val as input2_inv needs to be
      // positive to be used as multiplier of MultiplyByQuantizedMultiplier.
      input1_val = -input1_val;
      input2_val = -input2_val;
    }
    int recip_shift;
    const int32_t input2_inv = GetReciprocal(input2_val, 31, &recip_shift);
    const int headroom = CountLeadingSignBits(input1_val);
    const int32_t unscaled_quotient =
        MultiplyByQuantizedMultiplierGreaterThanOne(input1_val, input2_inv,
                                                    headroom);
    const int total_shift = params.output_shift - recip_shift - headroom;
    const int32_t unclamped_result =
        params.output_offset +
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            unscaled_quotient, params.output_multiplier, total_shift);
    const int32_t clamped_output =
        std::min(params.quantized_activation_max,
                 std::max(params.quantized_activation_min, unclamped_result));
    output_data[i] = static_cast<int8_t>(clamped_output);
  }
}

inline void Div(const ArithmeticParams& params,
                const RuntimeShape& input1_shape, const int8_t* input1_data,
                const RuntimeShape& input2_shape, const int8_t* input2_data,
                const RuntimeShape& output_shape, int8_t* output_data) {
  TFLITE_DCHECK_LE(params.quantized_activation_min,
                   params.quantized_activation_max);
  const int flat_size =
      MatchingElementsSize(input1_shape, input2_shape, output_shape);

  DivElementwise(flat_size, params, input1_data, input2_data, output_data);
}

template <int N = 5>
inline void BroadcastDivSlow(const ArithmeticParams& params,
                             const RuntimeShape& unextended_input1_shape,
                             const int8_t* input1_data,
                             const RuntimeShape& unextended_input2_shape,
                             const int8_t* input2_data,
                             const RuntimeShape& unextended_output_shape,
                             int8_t* output_data) {
  TFLITE_DCHECK_LE(unextended_input1_shape.DimensionsCount(), N);
  TFLITE_DCHECK_LE(unextended_input2_shape.DimensionsCount(), N);
  TFLITE_DCHECK_LE(unextended_output_shape.DimensionsCount(), N);

  NdArrayDesc<N> desc1;
  NdArrayDesc<N> desc2;
  NdArrayDesc<N> output_desc;
  NdArrayDescsForElementwiseBroadcast(unextended_input1_shape,
                                      unextended_input2_shape, &desc1, &desc2);
  CopyDimsToDesc(RuntimeShape::ExtendedShape(N, unextended_output_shape),
                 &output_desc);

  TFLITE_DCHECK_GT(params.input1_offset, -256);
  TFLITE_DCHECK_LT(params.input1_offset, 256);
  TFLITE_DCHECK_GT(params.input2_offset, -256);
  TFLITE_DCHECK_LT(params.input2_offset, 256);
  TFLITE_DCHECK_GT(params.output_offset, -256);
  TFLITE_DCHECK_LT(params.output_offset, 256);

  auto div_func = [&](int indexes[N]) {
    int32_t input1_val =
        params.input1_offset + input1_data[SubscriptToIndex(desc1, indexes)];
    int32_t input2_val =
        params.input2_offset + input2_data[SubscriptToIndex(desc2, indexes)];
    TFLITE_DCHECK_NE(input2_val, 0);
    if (input2_val < 0) {
      // Invert signs to avoid a negative input2_val as input2_inv needs to be
      // positive to be used as multiplier of MultiplyByQuantizedMultiplier.
      input1_val = -input1_val;
      input2_val = -input2_val;
    }
    int recip_shift;
    const int32_t input2_inv = GetReciprocal(input2_val, 31, &recip_shift);
    const int headroom = CountLeadingSignBits(input1_val);
    const int32_t unscaled_quotient =
        MultiplyByQuantizedMultiplierGreaterThanOne(input1_val, input2_inv,
                                                    headroom);
    const int total_shift = params.output_shift - recip_shift - headroom;
    const int32_t unclamped_result =
        params.output_offset +
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            unscaled_quotient, params.output_multiplier, total_shift);
    const int32_t clamped_output =
        std::min(params.quantized_activation_max,
                 std::max(params.quantized_activation_min, unclamped_result));
    output_data[SubscriptToIndex(output_desc, indexes)] =
        static_cast<int8_t>(clamped_output);
  };
  NDOpsHelper<N>(output_desc, div_func);
}

// TODO(jiawen): We can implement BroadcastDiv on buffers of arbitrary
// dimensionality if the runtime code does a single loop over one dimension
// that handles broadcasting as the base case. The code generator would then
// generate max(D1, D2) nested for loops.
template <typename T, int N = 5>
void BroadcastDivSlow(const ArithmeticParams& params,
                      const RuntimeShape& unextended_input1_shape,
                      const T* input1_data,
                      const RuntimeShape& unextended_input2_shape,
                      const T* input2_data,
                      const RuntimeShape& unextended_output_shape,
                      T* output_data) {
  T output_activation_min;
  T output_activation_max;
  GetActivationParams(params, &output_activation_min, &output_activation_max);

  TFLITE_DCHECK_LE(unextended_input1_shape.DimensionsCount(), N);
  TFLITE_DCHECK_LE(unextended_input2_shape.DimensionsCount(), N);
  TFLITE_DCHECK_LE(unextended_output_shape.DimensionsCount(), N);

  NdArrayDesc<N> desc1;
  NdArrayDesc<N> desc2;
  NdArrayDesc<N> output_desc;
  NdArrayDescsForElementwiseBroadcast(unextended_input1_shape,
                                      unextended_input2_shape, &desc1, &desc2);
  CopyDimsToDesc(RuntimeShape::ExtendedShape(N, unextended_output_shape),
                 &output_desc);

  // In Tensorflow, the dimensions are canonically named (batch_number, row,
  // col, channel), with extents (batches, height, width, depth), with the
  // trailing dimension changing most rapidly (channels has the smallest
  // stride, typically 1 element).
  //
  // In generated C code, we store arrays with the dimensions reversed. The
  // first dimension has smallest stride.

  auto div_func = [&](int indexes[N]) {
    output_data[SubscriptToIndex(output_desc, indexes)] =
        ActivationFunctionWithMinMax(
            input1_data[SubscriptToIndex(desc1, indexes)] /
                input2_data[SubscriptToIndex(desc2, indexes)],
            output_activation_min, output_activation_max);
  };
  NDOpsHelper<N>(output_desc, div_func);
}

template <typename T>
inline void Div(const ArithmeticParams& params,
                const RuntimeShape& input1_shape, const T* input1_data,
                const RuntimeShape& input2_shape, const T* input2_data,
                const RuntimeShape& output_shape, T* output_data) {
  T output_activation_min;
  T output_activation_max;
  GetActivationParams(params, &output_activation_min, &output_activation_max);

  const int flat_size =
      MatchingElementsSize(input1_shape, input2_shape, output_shape);
  for (int i = 0; i < flat_size; ++i) {
    output_data[i] = ActivationFunctionWithMinMax(
        input1_data[i] / input2_data[i], output_activation_min,
        output_activation_max);
  }
}

}  // namespace tflite

#endif  // TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_DIV_H_
