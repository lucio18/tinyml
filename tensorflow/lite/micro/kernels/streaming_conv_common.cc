/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/c_api_types.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/padding.h"
#include "tensorflow/lite/micro/kernels/conv.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {

const int kStreamingConvInputTensor = 0;
const int kStreamingConvWeightsTensor = 1;
const int kStreamingConvBiasTensor = 2;
const int kStreamingConvOutputTensor = 0;

// Conv is quantized along dimension 0:
// https://www.tensorflow.org/lite/performance/quantization_spec
const int kStreamingConvQuantizedDimension = 0;

void* StreamingConvInit(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataStreamingConv));
}

TfLiteStatus CalculateOpDataStreamingConv(TfLiteContext* context, TfLiteNode* node,
                                 const TfLiteConvParams& params, int width,
                                 int height, int filter_width,
                                 int filter_height, int* out_width,
                                 int* out_height, const TfLiteType data_type,
                                 OpDataConv* data) {
  bool has_bias = node->inputs->size == 3;
  // Check number of inputs/outputs
  TF_LITE_ENSURE(context, has_bias || node->inputs->size == 2);
  TF_LITE_ENSURE_EQ(context, node->outputs->size, 1);

  // Matching GetWindowedOutputSize in TensorFlow.
  auto padding = params.padding;
  data->padding = ComputePaddingHeightWidth(
      params.stride_height, params.stride_width, params.dilation_height_factor,
      params.dilation_width_factor, height, width, filter_height, filter_width,
      padding, out_height, out_width);

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kStreamingConvInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter =
      micro_context->AllocateTempInputTensor(node, kStreamingConvWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor* bias =
      micro_context->AllocateTempInputTensor(node, kStreamingConvBiasTensor);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kStreamingConvOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  // Note that quantized inference requires that all tensors have their
  // parameters set. This is usually done during quantized training.
  if (data_type != kTfLiteFloat32) {
    int output_channels = filter->dims->data[kStreamingConvQuantizedDimension];

    TF_LITE_ENSURE_STATUS(tflite::PopulateConvolutionQuantizationParams(
        context, input, filter, bias, output, params.activation,
        &data->output_multiplier, &data->output_shift,
        &data->output_activation_min, &data->output_activation_max,
        data->per_channel_output_multiplier, data->per_channel_output_shift,
        output_channels));
  }

  data->input_zero_point = input->params.zero_point;
  data->filter_zero_point = filter->params.zero_point;
  data->output_zero_point = output->params.zero_point;

  micro_context->DeallocateTempTfLiteTensor(output);
  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(filter);
  if (bias != nullptr) {
    micro_context->DeallocateTempTfLiteTensor(bias);
  }

  return kTfLiteOk;
}

TfLiteStatus StreamingConvReshapeOutputTensor(TfLiteContext* context, TfLiteNode* node,
                                     const TfLiteTensor* input,
                                     const TfLiteTensor* filter,
                                     TfLiteTensor* output, int height,
                                     int width) {
  const int filter_output_channels = filter->dims->data[0];
  const int batches = input->dims->data[0];

  // relocate output tensor dims so they can be updated
  TfLiteEvalTensor* output_eval =
      tflite::micro::GetEvalOutput(context, node, kStreamingConvOutputTensor);
  TF_LITE_ENSURE_STATUS(tflite::micro::CreateWritableTensorDimsWithCopy(
      context, output, output_eval));

  output->dims->data[0] = batches;
  output->dims->data[1] = height;
  output->dims->data[2] = width;
  output->dims->data[3] = filter_output_channels;

  return kTfLiteOk;
}

TfLiteStatus StreamingConvPrepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpDataStreamingConv* sdata = static_cast<OpDataStreamingConv*>(node->user_data);
  OpDataConv* data = &(sdata->op_data);
  const auto& params =
      *(static_cast<const TfLiteConvParams*>(node->builtin_data));
  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kStreamingConvOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);
  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kStreamingConvInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter =
      micro_context->AllocateTempInputTensor(node, kStreamingConvWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);

  TF_LITE_ENSURE_EQ(context, input->type, output->type);
  TF_LITE_ENSURE_MSG(
      context,
      (input->type == kTfLiteFloat32 && filter->type == kTfLiteFloat32) ||
          (input->type == kTfLiteInt16 && filter->type == kTfLiteInt8) ||
          (input->type == kTfLiteInt8 &&
           (filter->type == kTfLiteInt4 || filter->type == kTfLiteInt8)),
      "Hybrid models are not supported on TFLite Micro.");

  // Check dimensionality of input, filter, output
  TF_LITE_ENSURE_EQ(context, input->dims->size, 4);
  TF_LITE_ENSURE_EQ(context, filter->dims->size, 4);
  TF_LITE_ENSURE_EQ(context, output->dims->size, 4);

  // Check input channels matching filter
  const int input_channels = input->dims->data[3];
  const int filter_input_channels = filter->dims->data[3];
  TF_LITE_ENSURE(context, filter_input_channels > 0);
  TF_LITE_ENSURE_EQ(context, input_channels % filter_input_channels, 0);

  const int input_width = input->dims->data[2];
  const int input_height = input->dims->data[1];
  const int filter_width = filter->dims->data[2];
  const int filter_height = filter->dims->data[1];
  int output_width = 0;
  int output_height = 0;


  TFLITE_DCHECK_EQ(input_width, 1); /* iw is 1 for streaming conv */

  // Dynamically allocate per-channel quantization parameters.
  const int num_channels = filter->dims->data[kStreamingConvQuantizedDimension];
  data->per_channel_output_multiplier =
      static_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));
  data->per_channel_output_shift =
      static_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));
  sdata->input_state =
      static_cast<void*>(context->AllocatePersistentBuffer(
          context, input_height * input_channels * filter_width  * sizeof(int16_t)));

  memset(sdata->input_state, 0, input_height * input_channels * filter_width  * sizeof(int16_t));

  // All per-channel quantized tensors need valid zero point and scale arrays.
  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);

    const auto* affine_quantization =
        static_cast<TfLiteAffineQuantization*>(filter->quantization.params);
    TFLITE_DCHECK(affine_quantization != nullptr);
    TFLITE_DCHECK(affine_quantization->scale != nullptr);
    TFLITE_DCHECK(affine_quantization->zero_point != nullptr);

    TF_LITE_ENSURE(context,
                   affine_quantization->scale->size == 1 ||
                       affine_quantization->scale->size ==
                           filter->dims->data[kStreamingConvQuantizedDimension]);
  }

  TF_LITE_ENSURE_STATUS(CalculateOpDataStreamingConv(
      context, node, params, filter_width, input_height, filter_width,
      filter_height, &output_width, &output_height, input->type, data));

  // compute output tensor shape and relocate shape data
  TF_LITE_ENSURE_STATUS(StreamingConvReshapeOutputTensor(
      context, node, input, filter, output, output_height, output_width));

  if (filter->type == kTfLiteInt4) {
    int filter_size =
        RuntimeShape(filter->dims->size,
                     reinterpret_cast<const int32_t*>(filter->dims->data))
            .FlatSize();
    context->RequestScratchBufferInArena(context, filter_size,
                                         &data->filter_buffer_index);
  }

  micro_context->DeallocateTempTfLiteTensor(filter);
  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);
  return kTfLiteOk;
}
}  // namespace tflite
