/* Copyright 2024 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_
#define TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "flatbuffers/flatbuffers.h"  // from @flatbuffers
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/portable_type_to_tflitetype.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace tflite {
namespace testing {

constexpr int kOfflinePlannerHeaderSize = 3;
using TestingOpResolver = tflite::MicroMutableOpResolver<10>;

struct NodeConnection_ {
  std::initializer_list<int32_t> input;
  std::initializer_list<int32_t> output;
};
typedef struct NodeConnection_ NodeConnection;

// A simple operator that returns the median of the input with the number of
// times the kernel was invoked. The implementation below is deliberately
// complicated, just to demonstrate how kernel memory planning works.
class SimpleStatefulOp {
  static constexpr int kBufferNotAllocated = 0;
  // Inputs:
  static constexpr int kInputTensor = 0;
  // Outputs:
  static constexpr int kMedianTensor = 0;
  static constexpr int kInvokeCount = 1;
  struct OpData {
    int* invoke_count = nullptr;
    int sorting_buffer = kBufferNotAllocated;
  };

 public:
  static const TFLMRegistration* getRegistration();
  static TFLMRegistration* GetMutableRegistration();
  static void* Init(TfLiteContext* context, const char* buffer, size_t length);
  static TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);
  static TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);
};

class MockCustom {
 public:
  static const TFLMRegistration* getRegistration();
  static TFLMRegistration* GetMutableRegistration();
  static void* Init(TfLiteContext* context, const char* buffer, size_t length);
  static void Free(TfLiteContext* context, void* buffer);
  static TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);
  static TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);

  static bool freed_;
};

// A simple operator with the purpose of testing multiple inputs. It returns
// the sum of the inputs.
class MultipleInputs {
 public:
  static const TFLMRegistration* getRegistration();
  static TFLMRegistration* GetMutableRegistration();
  static void* Init(TfLiteContext* context, const char* buffer, size_t length);
  static void Free(TfLiteContext* context, void* buffer);
  static TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);
  static TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);

  static bool freed_;
};

// A simple no-op operator.
class NoOp {
 public:
  static const TFLMRegistration* getRegistration();
  static TFLMRegistration* GetMutableRegistration();
  static void* Init(TfLiteContext* context, const char* buffer, size_t length);
  static void Free(TfLiteContext* context, void* buffer);
  static TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node);
  static TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node);

  static bool freed_;
};

// Returns an Op Resolver that can be used in the testing code.
TfLiteStatus GetTestingOpResolver(TestingOpResolver& op_resolver);

// Returns a simple example flatbuffer TensorFlow Lite model. Contains 1 input,
// 1 layer of weights, 1 output Tensor, and 1 operator.
const Model* GetSimpleMockModel();

#ifdef USE_TFLM_COMPRESSION

// Returns a simple example flatbuffer TensorFlow Lite model. Contains 1 input,
// 1 layer of weights, 1 output Tensor, and 1 operator (BroadcastAddOp).  The
// weights tensor is compressed.
const Model* GetSimpleMockModelCompressed();

#endif  // USE_TFLM_COMPRESSION

// Returns a flatbuffer TensorFlow Lite model with more inputs, variable
// tensors, and operators.
const Model* GetComplexMockModel();

// Returns a simple example flatbuffer TensorFlow Lite model. Contains 1 input,
// 1 layer of weights, 1 output Tensor, and 1 operator.
// The size of all three tensors is 256 x 256, which is larger than what other
// models provide from this test helper.
const Model* GetModelWith256x256Tensor();

// Returns a simple flatbuffer model with two branches.
const Model* GetSimpleModelWithBranch();

// Returns a simple example flatbuffer TensorFlow Lite model. Contains 3 inputs,
// 1 output Tensor, and 1 operator.
const Model* GetSimpleMultipleInputsModel();

// Returns a simple flatbuffer model with offline planned tensors
// @param[in]       num_tensors           Number of tensors in the model.
// @param[in]       metadata_buffer       Metadata for offline planner.
// @param[in]       node_con              List of connections, i.e. operators
//                                        in the model.
// @param[in]       num_conns             Number of connections.
// @param[in]       num_subgraph_inputs   How many of the input tensors are in
//                                        the subgraph inputs. The default value
//                                        of 0 means all of the input tensors
//                                        are in the subgraph input list. There
//                                        must be at least 1 input tensor in the
//                                        subgraph input list.
const Model* GetModelWithOfflinePlanning(int num_tensors,
                                         const int32_t* metadata_buffer,
                                         NodeConnection* node_conn,
                                         int num_conns,
                                         int num_subgraph_inputs = 0);

// Returns a flatbuffer with a single operator, two inputs (one unused) and one
// output.
const Model* GetModelWithUnusedInputs();

// Returns a flatbuffer with a single operator, zero inputs and two outputs
// (one unused).
const Model* GetModelWithUnusedOperatorOutputs();

// Returns a flatbuffer model with `simple_stateful_op`
const Model* GetSimpleStatefulModel();

// Returns a flatbuffer model with "if" and two subgraphs.
const Model* GetSimpleModelWithSubgraphsAndIf();

// Returns a flatbuffer model with "if" and two subgraphs one of which is empty.
const Model* GetSimpleModelWithIfAndEmptySubgraph();

// Returns a flatbuffer model with "while" and three subgraphs.
const Model* GetSimpleModelWithSubgraphsAndWhile();

// Returns a flatbuffer model with "if" and two subgraphs and the input tensor 1
// of "if" subgraph overlaps with the input tensor 2 of subgraph 1.
const Model* GetModelWithIfAndSubgraphInputTensorOverlap();

// Returns a flatbuffer model with null subgraph/operator inputs and outputs.
const Model* GetSimpleModelWithNullInputsAndOutputs();

// Builds a one-dimensional flatbuffer tensor of the given size.
const Tensor* Create1dFlatbufferTensor(int size, bool is_variable = false);

// Builds a one-dimensional flatbuffer tensor of the given size with
// quantization metadata.
const Tensor* CreateQuantizedFlatbufferTensor(int size);

// Creates a one-dimensional tensor with no quantization metadata.
const Tensor* CreateMissingQuantizationFlatbufferTensor(int size);

// Creates a vector of flatbuffer buffers.
const flatbuffers::Vector<flatbuffers::Offset<Buffer>>*
CreateFlatbufferBuffers();

// Performs a simple string comparison without requiring standard C library.
int TestStrcmp(const char* a, const char* b);

void PopulateContext(TfLiteTensor* tensors, int tensors_size,
                     TfLiteContext* context);

// Create a TfLiteIntArray from an array of ints.  The first element in the
// supplied array must be the size of the array expressed as an int.
TfLiteIntArray* IntArrayFromInts(const int* int_array);

// Create a TfLiteFloatArray from an array of floats.  The first element in the
// supplied array must be the size of the array expressed as a float.
TfLiteFloatArray* FloatArrayFromFloats(const float* floats);

// Assumes that `src_tensor` is a buffer where each element is a 4-bit value
// stored in 8-bit.
// Returns a new buffer that is packed densely with 2 4-bit values in a byte.
// The packing format is low-bits-first, i.e. the lower nibble of a byte is
// filled first, followed by the upper nibble.
void PackInt4ValuesDenselyInPlace(uint8_t* src_buffer, int buffer_size);

template <typename T>
TfLiteTensor CreateTensor(const T* data, TfLiteIntArray* dims,
                          const bool is_variable = false,
                          TfLiteType type = kTfLiteNoType) {
  TfLiteTensor result;
  result.dims = dims;
  result.params = {};
  result.quantization = {kTfLiteNoQuantization, nullptr};
  result.is_variable = is_variable;
  result.allocation_type = kTfLiteMemNone;
  result.data.data = const_cast<T*>(data);

  if (type == kTfLiteInt4) {
    result.type = kTfLiteInt4;
    PackInt4ValuesDenselyInPlace(tflite::GetTensorData<uint8_t>(&result),
                                 ElementCount(*dims));
    result.bytes = ((ElementCount(*dims) + 1) / 2);
  } else {
    // Const cast is used to allow passing in const and non-const arrays within
    // a single CreateTensor method. A Const array should be used for immutable
    // input tensors and non-const array should be used for mutable and output
    // tensors.
    if (type == kTfLiteNoType) {
      result.type = typeToTfLiteType<T>();
    } else {
      result.type = type;
    }

    result.bytes = ElementCount(*dims) * TfLiteTypeGetSize(result.type);
  }
  return result;
}

template <typename T>
TfLiteTensor CreateQuantizedTensor(const T* data, TfLiteIntArray* dims,
                                   const float scale, const int zero_point = 0,
                                   const bool is_variable = false,
                                   TfLiteType type = kTfLiteNoType) {
  TfLiteTensor result = CreateTensor(data, dims, is_variable, type);
  result.params = {scale, zero_point};
  result.quantization = {kTfLiteAffineQuantization, nullptr};
  return result;
}

template <typename T>
TfLiteTensor CreateQuantizedTensor(const float* input, T* quantized,
                                   TfLiteIntArray* dims, float scale,
                                   int zero_point, bool is_variable = false,
                                   TfLiteType type = kTfLiteNoType) {
  int input_size = ElementCount(*dims);
  tflite::Quantize(input, quantized, input_size, scale, zero_point);
  return CreateQuantizedTensor(quantized, dims, scale, zero_point, is_variable,
                               type);
}

// TODO(ddavis-2015): remove
TfLiteTensor CreateQuantizedBiasTensor(const float* data, int16_t* quantized,
                                       TfLiteIntArray* dims, float input_scale,
                                       float weights_scale,
                                       bool is_variable = false);

// TODO(ddavis-2015): remove
TfLiteTensor CreateQuantizedBiasTensor(const float* data, int32_t* quantized,
                                       TfLiteIntArray* dims, float input_scale,
                                       float weights_scale,
                                       bool is_variable = false);

// TODO(ddavis-2015): remove
TfLiteTensor CreateQuantizedBiasTensor(const float* data,
                                       std::int64_t* quantized,
                                       TfLiteIntArray* dims, float input_scale,
                                       float weights_scale,
                                       bool is_variable = false);

// Creates bias tensor with pre-calculated compressed input data and per-channel
// weights determined by input scale multiplied by weight scale for each
// channel.
template <typename T>
TfLiteTensor CreatePerChannelQuantizedBiasTensor(
    const T* input_data, TfLiteIntArray* dims, float input_scale,
    const TfLiteFloatArray* weight_scales, TfLiteFloatArray* scales,
    TfLiteIntArray* zero_points, TfLiteAffineQuantization* affine_quant,
    int quantized_dimension, bool is_variable = false,
    TfLiteType type = kTfLiteNoType) {
  int num_channels = dims->data[quantized_dimension];
  zero_points->size = num_channels;
  scales->size = num_channels;
  for (int i = 0; i < num_channels; i++) {
    scales->data[i] = input_scale * weight_scales->data[i];
    zero_points->data[i] = 0;
    MicroPrintf("index %d scales %f zero_point %d input scale %f weight %f", i,
                (double)scales->data[i], zero_points->data[i],
                (double)input_scale, (double)weight_scales->data[i]);
  }

  affine_quant->scale = scales;
  affine_quant->zero_point = zero_points;
  affine_quant->quantized_dimension = quantized_dimension;

  TfLiteTensor result = CreateTensor(input_data, dims, is_variable, type);
  result.quantization = {kTfLiteAffineQuantization, affine_quant};
  return result;
}

// Quantizes bias tensor with per-channel weights determined by input
// scale multiplied by weight scale for each channel.
template <typename T>
TfLiteTensor CreatePerChannelQuantizedBiasTensor(
    const float* input, T* quantized, TfLiteIntArray* dims, float input_scale,
    const float* weight_scales, float* scales, int* zero_points,
    TfLiteAffineQuantization* affine_quant, int quantized_dimension,
    bool is_variable = false) {
  int input_size = ElementCount(*dims);
  int num_channels = dims->data[quantized_dimension];
  // First element is reserved for array length
  zero_points[0] = num_channels;
  scales[0] = static_cast<float>(num_channels);
  float* scales_array = &scales[1];
  for (int i = 0; i < num_channels; i++) {
    scales_array[i] = input_scale * weight_scales[i];
    zero_points[i + 1] = 0;
    MicroPrintf("index %d scales %f zero_point %d input scale %f weight %f", i,
                (double)scales_array[i], zero_points[i + 1],
                (double)input_scale, (double)weight_scales[i]);
  }

  SymmetricPerChannelQuantize<T>(input, quantized, input_size, num_channels,
                                 scales_array);

  affine_quant->scale = FloatArrayFromFloats(scales);
  affine_quant->zero_point = IntArrayFromInts(zero_points);
  affine_quant->quantized_dimension = quantized_dimension;

  TfLiteTensor result = CreateTensor(quantized, dims, is_variable);
  result.quantization = {kTfLiteAffineQuantization, affine_quant};
  int64_t data0 = quantized[0];
  MicroPrintf("quantp %p data %f data quantized %lld", affine_quant,
              (double)input[0], data0);
  return result;
}

template <typename T>
TfLiteTensor CreatePerChannelQuantizedTensor(
    const T* quantized, TfLiteIntArray* dims, TfLiteFloatArray* scales,
    TfLiteIntArray* zero_points, TfLiteAffineQuantization* affine_quant,
    int quantized_dimension, bool is_variable = false,
    TfLiteType type = kTfLiteNoType) {
  affine_quant->scale = scales;
  affine_quant->zero_point = zero_points;
  affine_quant->quantized_dimension = quantized_dimension;

  TfLiteTensor result = CreateTensor(quantized, dims, is_variable, type);
  result.quantization = {kTfLiteAffineQuantization, affine_quant};
  return result;
}

TfLiteTensor CreateSymmetricPerChannelQuantizedTensor(
    const float* input, int8_t* quantized, TfLiteIntArray* dims, float* scales,
    int* zero_points, TfLiteAffineQuantization* affine_quant,
    int quantized_dimension, bool is_variable = false,
    TfLiteType tensor_weight_type = kTfLiteNoType);

// Returns the number of tensors in the default subgraph for a tflite::Model.
size_t GetModelTensorCount(const Model* model);

// Derives the asymmetric quantization scaling factor from a min and max range.
template <typename T>
inline float ScaleFromMinMax(const float min, const float max) {
  return (max - min) /
         static_cast<float>((std::numeric_limits<T>::max() * 1.0) -
                            std::numeric_limits<T>::min());
}

// Derives the symmetric quantization scaling factor from a min and max range.
template <typename T>
inline float SymmetricScaleFromMinMax(const float min, const float max) {
  const int32_t kScale =
      std::numeric_limits<typename std::make_signed<T>::type>::max();
  const float range = std::max(std::abs(min), std::abs(max));
  if (range == 0) {
    return 1.0f;
  } else {
    return range / kScale;
  }
}

// Derives the quantization zero point from a min and max range.
template <typename T>
inline int ZeroPointFromMinMax(const float min, const float max) {
  return static_cast<int>(std::numeric_limits<T>::min()) +
         static_cast<int>(roundf(-min / ScaleFromMinMax<T>(min, max)));
}

}  // namespace testing
}  // namespace tflite

#endif  // TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_
