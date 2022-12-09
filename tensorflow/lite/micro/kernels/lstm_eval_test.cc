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
#include "tensorflow/lite/micro/kernels/lstm_eval.h"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/portable_tensor_utils.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/micro/test_helpers.h"
#include "tensorflow/lite/micro/testing/micro_test.h"

// TODO(b/230666079) enable below tests for xtensa when the xtensa
// kernel is reconciled with reference kernel
#if !defined(XTENSA)
namespace tflite {
namespace testing {
namespace {
/*MODEL SIZE DEFINITONS*/

// Model Size Constant
constexpr int kInputDimension = 2;
constexpr int kStateDimension = 2;
constexpr int kBatchSize = 2;
constexpr int kGateOutputSize = kBatchSize * kStateDimension;
constexpr int kOneTimeInputSize = kBatchSize * kInputDimension;
constexpr int kOneTimeOutputSize = kBatchSize * kStateDimension;
constexpr int kTimeSteps = 3;
constexpr int kInputSize = kOneTimeInputSize * kTimeSteps;
constexpr int kOutputSize = kOneTimeOutputSize * kTimeSteps;

// Number of tensors for the LSTM kernel. 0 input; 1-12 gate parameters; 13-14
// states; 15 output
constexpr int kTensorsNum = 16;
// Test Settings
constexpr float kTestFloatTolerance = 1e-6f;
// LSTM internal setting (e.g., nonlinear activation type)
constexpr TfLiteLSTMParams kModelSettings = {
    /*.activation=*/kTfLiteActTanh,
    /*.cell_clip=*/6, /*.proj_clip=*/3,
    /*.kernel_type=*/kTfLiteLSTMFullKernel,
    /*.asymmetric_quantize_inputs=*/true};

/*TEST DATA */
/*Testing Data*/
// One time data  to test the output of each gate inside the LSTM
struct GateOutputCheckData {
  const float input_data[kOneTimeInputSize] = {
      0.2, 0.3,    // batch1
      -0.98, 0.62  // batch2
  };
  const float hidden_state[kGateOutputSize] = {
      -0.1, 0.2,  // batch1
      -0.3, 0.5   // batch2
  };
  const float cell_state[kGateOutputSize] = {
      -1.3, 6.2,  // batch1
      -7.3, 3.5   // batch2
  };

  // Use the forget gate parameters to test small gate outputs
  // output = sigmoid(W_i*i+W_h*h+b) = sigmoid([[-10,-10],[-20,-20]][0.2,
  // +[[-10,-10],[-20,-20]][-0.1, 0.2]+[1,2]) = sigmoid([-5,-10]) =
  // [6.69285092e-03, 4.53978687e-05] (Batch1)
  // Similarly, we have [0.93086158 0.9945137 ] for batch 2
  const float expected_forget_gate_output[kGateOutputSize] = {
      6.69285092e-3f, 4.53978687e-5f, 0.93086158, 0.9945137};

  // Use the input gate parameters to test small gate outputs
  // output = sigmoid(W_i*i+W_h*h+b) = sigmoid([[10,10],[20,20]][0.2, 0.3]
  // +[[10,10],[20,20]][-0.1, 0.2]+[-1,-2]) = sigmoid([5,10]) =
  // [0.99330715, 0.9999546]
  // Similarly, we have [0.06913842 0.0054863 ] for batch 2
  const float expected_input_gate_output[kGateOutputSize] = {
      0.99330715, 0.9999546, 0.06913842, 0.0054863};

  // Use the output gate parameters to test normnal gate outputs
  // output = sigmoid(W_i*i+W_h*h+b) = sigmoid([[1,1],[1,1]][0.2, 0.3]
  // +[[1,1],[1,1]][-0.1, 0.2]+[0,0]) = sigmoid([0.6,0.6]) =
  // [0.6456563062257954, 0.6456563062257954]
  // Similarly, we have [[0.46008512 0.46008512]] for batch 2
  const float expected_output_gate_output[kGateOutputSize] = {
      0.6456563062257954, 0.6456563062257954, 0.46008512, 0.46008512};

  // Use the cell(modulation) gate parameters to tanh output
  // output = tanh(W_i*i+W_h*h+b) = tanh([[1,1],[1,1]][0.2, 0.3]
  // +[[1,1],[1,1]][-0.1, 0.2]+[0,0]) = tanh([0.6,0.6]) =
  // [0.6456563062257954, 0.6456563062257954]
  // Similarly, we have [-0.1586485 -0.1586485] for batch 2
  const float expected_cell_gate_output[kGateOutputSize] = {
      0.5370495669980353, 0.5370495669980353, -0.1586485, -0.1586485};

  // Cell = forget_gate*cell + input_gate*cell_gate
  // Note -6.80625824 is clipped to -6
  const float expected_updated_cell[kGateOutputSize] = {0.52475447, 0.53730665,
                                                        -6, 3.47992756};

  // Use the updated cell state to update the hidden state
  // tanh(expected_updated_cell) * expected_output_gate_output
  const float expected_updated_hidden[kGateOutputSize] = {
      0.31079388, 0.3169827, -0.46007947, 0.45921249};
};

struct MultiTimeLstmEvalData {
  // batch one using repeated data; batch two mimics
  // random input
  const float input_data[kInputSize] = {
      0.2,   0.3,  0.2,  0.3,  0.2,  0.3,   // batch one
      -0.98, 0.62, 0.01, 0.99, 0.49, -0.32  // batch two
  };

  // Initialize hidden state as zeros
  const float hidden_state[kGateOutputSize] = {};

  // The expected model output after kTimeSteps using the fixed input and
  // parameters
  // TODO(b/253466487): document how the golden values are arrived at
  const float expected_output[kOutputSize] = {
      0.26455893,      0.26870455,      0.47935803,
      0.47937014,      0.58013272,      0.58013278,  // batch1
      -1.41184672e-3f, -1.43329117e-5f, 0.46887168,
      0.46891281,      0.50054074,      0.50054148  // batch2
  };

  const float expected_hidden_state[kGateOutputSize] = {
      0.58013272, 0.58013278,  // batch1
      0.50054074, 0.50054148   // batch2
  };

  const float expected_cell_state[kGateOutputSize] = {
      0.89740515, 0.8974053,  // batch1
      0.80327607, 0.80327785  // batch2
  };
};

// TODO(b/253466487): move the struct declaration (generalized, with setters)
// into a .h and defines the global variables here
constexpr GateOutputCheckData kGateOutputData;
constexpr MultiTimeLstmEvalData kMultiTimeEvalData;

/*MODEL PARAMETERS*/

// Struct that holds the weight/bias information for a standard gate (i.e. no
// modification such as layer normalization, peephole, etc.)
template <typename WeightType, typename BiasType>
struct GateParameters {
  WeightType activation_weight[kStateDimension * kInputDimension];
  WeightType recurrent_weight[kStateDimension * kStateDimension];
  BiasType fused_bias[kStateDimension];
  // Quantized model folded the zero point of activations into biases:
  // bias + zero_point * weight.
  BiasType activation_zp_folded_bias[kStateDimension];
  BiasType recurrent_zp_folded_bias[kStateDimension];
};

// Parameters for different gates
// negative large weights for forget gate to make it really forget
constexpr GateParameters<float, float> kForgetGateParameters = {
    /*.activation_weight=*/{-10, -10, -20, -20},
    /*.recurrent_weight=*/{-10, -10, -20, -20},
    /*.fused_bias=*/{1, 2},
    /*activation_zp_folded_bias=*/{0, 0},
    /*recurrent_zp_folded_bias=*/{0, 0}};
// positive large weights for input gate to make it really remember
constexpr GateParameters<float, float> kInputGateParameters = {
    /*.activation_weight=*/{10, 10, 20, 20},
    /*.recurrent_weight=*/{10, 10, 20, 20},
    /*.fused_bias=*/{-1, -2},
    /*activation_zp_folded_bias=*/{0, 0},
    /*recurrent_zp_folded_bias=*/{0, 0}};
// all ones to test the behavior of tanh at normal range (-1,1)
constexpr GateParameters<float, float> kCellGateParameters = {
    /*.activation_weight=*/{1, 1, 1, 1},
    /*.recurrent_weight=*/{1, 1, 1, 1},
    /*.fused_bias=*/{0, 0},
    /*activation_zp_folded_bias=*/{0, 0},
    /*recurrent_zp_folded_bias=*/{0, 0}};
// all ones to test the behavior of sigmoid at normal range (-1. 1)
constexpr GateParameters<float, float> kOutputGateParameters = {
    /*.activation_weight=*/{1, 1, 1, 1},
    /*.recurrent_weight=*/{1, 1, 1, 1},
    /*.fused_bias=*/{0, 0},
    /*activation_zp_folded_bias=*/{0, 0},
    /*recurrent_zp_folded_bias=*/{0, 0}};

// A base class that holds all the tensors for evaluation
template <typename ActivationType, typename WeightType, typename BiasType,
          typename CellType>
class ModelContents {
 public:
  ModelContents(const GateParameters<WeightType, BiasType> forget_gate_params,
                const GateParameters<WeightType, BiasType> input_gate_params,
                const GateParameters<WeightType, BiasType> cell_gate_params,
                const GateParameters<WeightType, BiasType> output_gate_params)
      : forget_gate_params_(forget_gate_params),
        input_gate_params_(input_gate_params),
        cell_gate_params_(cell_gate_params),
        output_gate_params_(output_gate_params) {
    InitializeTensors();
  }
  // Provide interface to set the input tensor values for flexible testing
  void SetInputTensorData(const ActivationType* data) {
    std::memcpy(input_, data, kInputSize * sizeof(ActivationType));
    SetTensor(0, input_, input_size_);
  }

  // Provide interface to set the hidden state tensor values for flexible
  // testing
  void SetHiddenStateTensorData(const ActivationType* data) {
    std::memcpy(hidden_state_, data, kGateOutputSize * sizeof(ActivationType));
    SetTensor(13, hidden_state_, state_size_);
  }

  TfLiteEvalTensor* GetTensor(int tensor_index) {
    return &tensors_[tensor_index];
  }
  const ActivationType* GetHiddenState() const { return hidden_state_; }
  const CellType* GetCellState() const { return cell_state_; }
  const ActivationType* GetOutput() const { return output_; }

  CellType* ScratchBuffers() { return scratch_buffers_; }

  // TODO(b/253466487): make all getters constant after refactor the
  // IntegerLstmParameter
  GateParameters<WeightType, BiasType>& ForgetGateParams() {
    return forget_gate_params_;
  }
  GateParameters<WeightType, BiasType>& InputGateParams() {
    return input_gate_params_;
  }
  GateParameters<WeightType, BiasType>& CellGateParams() {
    return cell_gate_params_;
  }
  GateParameters<WeightType, BiasType>& OutputGateParams() {
    return output_gate_params_;
  }

 private:
  void InitializeTensors() {
    // Input Tensor
    SetTensor(0, input_, input_size_);
    // Forget Gate Tensors
    SetTensor(1, forget_gate_params_.activation_weight,
              activation_weight_size_);
    SetTensor(2, forget_gate_params_.recurrent_weight, recurrent_weight_size_);
    SetTensor(3, forget_gate_params_.fused_bias, bias_size_);
    // Input Gate Tensors
    SetTensor(4, input_gate_params_.activation_weight, activation_weight_size_);
    SetTensor(5, input_gate_params_.recurrent_weight, recurrent_weight_size_);
    SetTensor(6, input_gate_params_.fused_bias, bias_size_);
    // Cell Gate Tensors
    SetTensor(7, cell_gate_params_.activation_weight, activation_weight_size_);
    SetTensor(8, cell_gate_params_.recurrent_weight, recurrent_weight_size_);
    SetTensor(9, cell_gate_params_.fused_bias, bias_size_);
    // Output Gate Tensors
    SetTensor(10, output_gate_params_.activation_weight,
              activation_weight_size_);
    SetTensor(11, output_gate_params_.recurrent_weight, recurrent_weight_size_);
    SetTensor(12, output_gate_params_.fused_bias, bias_size_);
    // State Tensors
    SetTensor(13, hidden_state_, state_size_);
    SetTensor(14, cell_state_, state_size_);
    // Output Tensor
    SetTensor(15, output_, output_size_);
  }

  template <typename T>
  void SetTensor(const int index, const T* data, int* dims) {
    tensors_[index].data.data = const_cast<T*>(data);
    tensors_[index].dims = IntArrayFromInts(dims);
    tensors_[index].type = typeToTfLiteType<T>();
  }

  GateParameters<WeightType, BiasType> forget_gate_params_;
  GateParameters<WeightType, BiasType> input_gate_params_;
  GateParameters<WeightType, BiasType> cell_gate_params_;
  GateParameters<WeightType, BiasType> output_gate_params_;

  // Not const since IntArrayFromInts takes int *; the first element of the
  // array must be the size of the array
  int input_size_[4] = {3, kBatchSize, kTimeSteps, kInputDimension};
  int output_size_[4] = {3, kBatchSize, kTimeSteps, kStateDimension};
  int activation_weight_size_[3] = {2, kStateDimension, kInputDimension};
  int recurrent_weight_size_[3] = {2, kStateDimension, kStateDimension};
  int bias_size_[3] = {2, kBatchSize, kStateDimension};
  int state_size_[3] = {2, kBatchSize, kStateDimension};

  // 0 input; 1-12 gate parameters; 13-14 states; 15 output
  TfLiteEvalTensor tensors_[kTensorsNum];

  // states are initialized to zero
  ActivationType hidden_state_[kGateOutputSize] = {};
  CellType cell_state_[kGateOutputSize] = {};
  // input is defined in the ModelContent (const across all derived models)
  ActivationType input_[kInputSize] = {};
  ActivationType output_[kOutputSize] = {};
  // scratch buffers (4; used for floating point model only)
  CellType scratch_buffers_[4 * kGateOutputSize] = {};
};

/*QUANTIZATION SETTINGS*/
// A struct that holds quantization parameters for a LSTM Tensor
struct TensorQuantizationParameters {
  // all the effective
  const double scale;
  const int zero_point;
  const bool symmetry;
};

struct GateQuantizationParameters {
  TensorQuantizationParameters activation_weight;
  TensorQuantizationParameters recurrent_weight;
  TensorQuantizationParameters bias;
};

// A struct that holds the quantization settings for the model
struct ModelQuantizationParameters {
  TfLiteType activation_type;
  TfLiteType cell_type;
  TfLiteType bias_type;
  double nonlinear_activation_input_scale;
  double nonlinear_activation_output_scale;
  // Quantization parameters for input/output
  TensorQuantizationParameters input_quantization_parameters;
  TensorQuantizationParameters output_quantization_parameters;
  // Quantization parameters for internal states
  TensorQuantizationParameters hidden_quantization_parameters;
  TensorQuantizationParameters cell_quantization_parameters;
  // Quantization parameters for gates
  GateQuantizationParameters forget_gate_quantization_parameters;
  GateQuantizationParameters input_gate_quantization_parameters;
  GateQuantizationParameters cell_gate_quantization_parameters;
  GateQuantizationParameters output_gate_quantization_parameters;
};

// TODO(b/253466487): using initialization functions to make it more readable
// (and general)
constexpr ModelQuantizationParameters kInt8QuantizationSettings = {
    /*activation_type=*/kTfLiteInt8,
    /*cell_type=*/kTfLiteInt16,
    /*bias_type=*/kTfLiteInt32,
    /*nonlinear_input_scale=*/0.00024414062,   // std::pow(2.0f, -12.0f)
    /*nonlinear_output_scale=*/0.00003051757,  // std::pow(2.0f, -15.0f)
    /*Input=*/{/*scale=*/0.00784313725490196, /*zp=*/0, /*symmetry=*/false},
    /*output=*/{/*scale=*/0.004705882165580988, /*zp=*/-21, /*symmetry=*/false},
    /*hidden=*/{/*scale=*/0.004705882165580988, /*zp=*/-21, /*symmetry=*/false},
    /*cell=*/{/*scale=*/0.00024414062, /*zp=*/0, /*symmetry=*/true},

    /*forget_gate=*/
    {/*activation_weight=*/{/*scale=*/0.15748031496062992, /*zp=*/0,
                            /*symmetry=*/true},
     /*recurrent_weight*/
     {/*scale=*/0.15748031496062992, /*zp=*/0, /*symmetry=*/true},
     /*bias=*/{/*scale=*/0.0012351397251814111, /*zp=*/0, /*symmetry=*/true}},

    /*input_gate=*/
    {/*activation_weight=*/{/*scale=*/0.15748031496062992, /*zp=*/0,
                            /*symmetry=*/true},
     /*recurrent_weight*/
     {/*scale=*/0.15748031496062992, /*zp=*/0, /*symmetry=*/true},
     /*bias=*/{/*scale=*/0.0012351397251814111, /*zp=*/0, /*symmetry=*/true}},

    /*cell_gate=*/
    {/*activation_weight=*/{/*scale=*/0.007874015748031496, /*zp=*/0,
                            /*symmetry=*/true},
     /*recurrent_weight*/
     {/*scale=*/0.007874015748031496, /*zp=*/0, /*symmetry=*/true},
     /*bias=*/{/*scale=*/6.175698625907056e-5, /*zp=*/0, /*symmetry=*/true}},

    /*output_gate=*/
    {/*activation_weight=*/{/*scale=*/0.1, /*zp=*/0, /*symmetry=*/true},
     /*recurrent_weight*/ {/*scale=*/0.1, /*zp=*/0, /*symmetry=*/true},
     /*bias=*/{/*scale=*/0.1, /*zp=*/0, /*symmetry=*/true}}};

// A function that converts floating point gate parameters to the
// corresponding quantized version
template <typename WeightType, typename BiasType>
GateParameters<WeightType, BiasType> CreateQuantizedGateParameters(
    const GateParameters<float, float>& gate_parameters,
    const TensorQuantizationParameters& input_quantization_params,
    const TensorQuantizationParameters& output_quantization_params,
    const GateQuantizationParameters& gate_quantization_params) {
  GateParameters<WeightType, BiasType> quantized_gate_params;
  tflite::SymmetricQuantize(gate_parameters.activation_weight,
                            quantized_gate_params.activation_weight,
                            kStateDimension * kInputDimension,
                            gate_quantization_params.activation_weight.scale);
  tflite::SymmetricQuantize(gate_parameters.recurrent_weight,
                            quantized_gate_params.recurrent_weight,
                            kStateDimension * kStateDimension,
                            gate_quantization_params.recurrent_weight.scale);
  tflite::SymmetricQuantize(gate_parameters.fused_bias,
                            quantized_gate_params.fused_bias, kStateDimension,
                            gate_quantization_params.bias.scale);

  // Copy the bias values to prepare zero_point folded bias precomputation (bias
  // has same scale as input_scale*input_weight_scale)
  std::memcpy(quantized_gate_params.activation_zp_folded_bias,
              quantized_gate_params.fused_bias,
              kStateDimension * sizeof(BiasType));
  // Pre-calculate bias - zero_point * weight (a constant).
  tflite::tensor_utils::MatrixScalarMultiplyAccumulate(
      quantized_gate_params.activation_weight,
      -1 * input_quantization_params.zero_point, kStateDimension,
      kInputDimension, quantized_gate_params.activation_zp_folded_bias);

  // Initialize the folded bias to zeros for accumulation
  for (size_t i = 0; i < kStateDimension; i++) {
    quantized_gate_params.recurrent_zp_folded_bias[i] = 0;
  }
  // Calculate : -zero_point * weight since it is a constant
  tflite::tensor_utils::MatrixScalarMultiplyAccumulate(
      quantized_gate_params.recurrent_weight,
      -1 * output_quantization_params.zero_point, kStateDimension,
      kStateDimension, quantized_gate_params.recurrent_zp_folded_bias);

  return quantized_gate_params;
}

// Class that contains all the information to run quantized LSTM inference
// ActivationType: data type for activation tensors (e.g., int8_t)
// WeightType: data type for weight tensors (e.g., int8_t)
// BiasType: data type for biase tensors (e.g., int32_t). Note bias should have
// the same data type as the MatMul accumulator
// CellType: data type for the long-term state (cell). Note at least 16 bits are
// required for precision consideration
template <typename ActivationType, typename WeightType, typename BiasType,
          typename CellType>
class QuantizedModelContents
    : public ModelContents<ActivationType, WeightType, BiasType, CellType> {
 public:
  QuantizedModelContents(
      const ModelQuantizationParameters quantization_settings,
      const GateParameters<float, float> forget_gate_params,
      const GateParameters<float, float> input_gate_params,
      const GateParameters<float, float> cell_gate_params,
      const GateParameters<float, float> output_gate_params)
      : ModelContents<ActivationType, WeightType, BiasType, CellType>(
            CreateQuantizedGateParameters<WeightType, BiasType>(
                forget_gate_params,
                quantization_settings.input_quantization_parameters,
                quantization_settings.output_quantization_parameters,
                quantization_settings.forget_gate_quantization_parameters),
            CreateQuantizedGateParameters<WeightType, BiasType>(
                input_gate_params,
                quantization_settings.input_quantization_parameters,
                quantization_settings.output_quantization_parameters,
                quantization_settings.input_gate_quantization_parameters),
            CreateQuantizedGateParameters<WeightType, BiasType>(
                cell_gate_params,
                quantization_settings.input_quantization_parameters,
                quantization_settings.output_quantization_parameters,
                quantization_settings.cell_gate_quantization_parameters),
            CreateQuantizedGateParameters<WeightType, BiasType>(
                output_gate_params,
                quantization_settings.input_quantization_parameters,
                quantization_settings.output_quantization_parameters,
                quantization_settings.output_gate_quantization_parameters)),
        quantization_settings_(quantization_settings) {
    AssembleEvalualtionParams();
  }

  const IntegerLstmParameter& EvaluationParameters() const {
    return evaluation_params_;
  }

  const ModelQuantizationParameters& QuantizationSettings() const {
    return quantization_settings_;
  }

  // Sets the input to a quantized model from the underlying floating point
  // values. The quantization of the floating point values uses the quantization
  // parameters specified while creating the ModelQuantization object.
  void QuantizeInputTensor(const float* data) {
    ActivationType quantized_input_data[kInputSize] = {};
    Quantize(data, quantized_input_data, kInputSize,
             quantization_settings_.input_quantization_parameters.scale,
             quantization_settings_.input_quantization_parameters.zero_point);
    this->SetInputTensorData(quantized_input_data);
  }

  // Sets the hidden state to a quantized model from the underlying floating
  // point values. The quantization of the floating point values uses the
  // quantization parameters specified while creating the ModelQuantization
  // object.
  void QuantizeHiddenStateTensor(const float* data) {
    ActivationType quantized_hidden_state_data[kGateOutputSize] = {};
    Quantize(data, quantized_hidden_state_data, kGateOutputSize,
             quantization_settings_.hidden_quantization_parameters.scale,
             quantization_settings_.hidden_quantization_parameters.zero_point);
    this->SetHiddenStateTensorData(quantized_hidden_state_data);
  }

 private:
  void AssembleEvalualtionParams() {
    double effective_scale;
    // TODO(b/260006407): QuantizeMultiplier takes int as the output shift
    // type, but the shift type is stored as int32_t inside the
    // IntegerLstmParameter. Hexagon compilation requires the exact match of the
    // two. Considering make shift type to be int inside the
    // IntegerLstmParameter.
    int buffer_shift_output;
    // TODO(b/253466487): Considering refactoring IntegerLstmParameter to
    // distribute the calculation of gate quantization parameters (e.g.,
    // effective scale) to gate level. Forget Gate
    effective_scale =
        quantization_settings_.input_quantization_parameters.scale *
        quantization_settings_.forget_gate_quantization_parameters
            .activation_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_input_to_forget_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_input_to_forget_scale_b = buffer_shift_output;
    effective_scale =
        quantization_settings_.output_quantization_parameters.scale *
        quantization_settings_.forget_gate_quantization_parameters
            .recurrent_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(
        effective_scale,
        &evaluation_params_.effective_recurrent_to_forget_scale_a,
        &buffer_shift_output);
    evaluation_params_.effective_recurrent_to_forget_scale_b =
        buffer_shift_output;
    // Set effective bias
    evaluation_params_.input_to_forget_effective_bias =
        this->ForgetGateParams().activation_zp_folded_bias;
    evaluation_params_.recurrent_to_forget_effective_bias =
        this->ForgetGateParams().recurrent_zp_folded_bias;

    // input gate
    effective_scale =
        quantization_settings_.input_quantization_parameters.scale *
        quantization_settings_.input_gate_quantization_parameters
            .activation_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_input_to_input_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_input_to_input_scale_b = buffer_shift_output;
    effective_scale =
        quantization_settings_.output_quantization_parameters.scale *
        quantization_settings_.input_gate_quantization_parameters
            .recurrent_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_recurrent_to_input_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_recurrent_to_input_scale_b =
        buffer_shift_output;
    // Set effective bias
    evaluation_params_.input_to_input_effective_bias =
        this->InputGateParams().activation_zp_folded_bias;
    evaluation_params_.recurrent_to_input_effective_bias =
        this->InputGateParams().recurrent_zp_folded_bias;

    // cell gate
    effective_scale =
        quantization_settings_.input_quantization_parameters.scale *
        quantization_settings_.cell_gate_quantization_parameters
            .activation_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_input_to_cell_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_input_to_cell_scale_b = buffer_shift_output;
    effective_scale =
        quantization_settings_.output_quantization_parameters.scale *
        quantization_settings_.cell_gate_quantization_parameters
            .recurrent_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_recurrent_to_cell_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_recurrent_to_cell_scale_b =
        buffer_shift_output;
    // Set effective bias
    evaluation_params_.input_to_cell_effective_bias =
        this->CellGateParams().activation_zp_folded_bias;
    evaluation_params_.recurrent_to_cell_effective_bias =
        this->CellGateParams().recurrent_zp_folded_bias;

    // output gate
    effective_scale =
        quantization_settings_.input_quantization_parameters.scale *
        quantization_settings_.output_gate_quantization_parameters
            .activation_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_input_to_output_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_input_to_output_scale_b = buffer_shift_output;
    effective_scale =
        quantization_settings_.output_quantization_parameters.scale *
        quantization_settings_.output_gate_quantization_parameters
            .recurrent_weight.scale /
        quantization_settings_.nonlinear_activation_input_scale;
    QuantizeMultiplier(
        effective_scale,
        &evaluation_params_.effective_recurrent_to_output_scale_a,
        &buffer_shift_output);
    evaluation_params_.effective_recurrent_to_output_scale_b =
        buffer_shift_output;
    // Set effective bias
    evaluation_params_.input_to_output_effective_bias =
        this->OutputGateParams().activation_zp_folded_bias;
    evaluation_params_.recurrent_to_output_effective_bias =
        this->OutputGateParams().recurrent_zp_folded_bias;

    // hidden state (no projection, output is the hidden state)
    effective_scale =
        quantization_settings_.nonlinear_activation_output_scale *
        quantization_settings_.nonlinear_activation_output_scale /
        quantization_settings_.hidden_quantization_parameters.scale;
    QuantizeMultiplier(effective_scale,
                       &evaluation_params_.effective_hidden_scale_a,
                       &buffer_shift_output);
    evaluation_params_.effective_hidden_scale_b = buffer_shift_output;
    evaluation_params_.hidden_zp =
        quantization_settings_.hidden_quantization_parameters.zero_point;

    // cell state. Note, cell_scale is actually not a scale. 2^-cell_scale is
    // the true scale for cell
    int buffer_cell_scale;
    tflite::CheckedLog2(
        quantization_settings_.cell_quantization_parameters.scale,
        &buffer_cell_scale);
    evaluation_params_.cell_scale = buffer_cell_scale;

    evaluation_params_.quantized_cell_clip = static_cast<int16_t>(std::min(
        std::max(static_cast<double>(kModelSettings.cell_clip) /
                     quantization_settings_.cell_quantization_parameters.scale,
                 -32768.0),
        32767.0));
  }

  // Quantization settings for every tensor inside the model
  const ModelQuantizationParameters quantization_settings_;
  // All the information that required to invoke the quantized kernel
  IntegerLstmParameter evaluation_params_;
};

/*TEST HELPER FUNCTIONS*/
template <typename T>
void ValidateResultGoldens(const T* golden, const T* output_data,
                           const int output_len, const float tolerance) {
  for (int i = 0; i < output_len; ++i) {
    TF_LITE_MICRO_EXPECT_NEAR(golden[i], output_data[i], tolerance);
  }
}

void TestGateOutputFloat(const GateParameters<float, float>& gate_params,
                         TfLiteFusedActivation activation_type,
                         const float* input_data, const float* hidden_state,
                         const float* expected_vals) {
  float gate_output[kGateOutputSize] = {};
  tflite::lstm_internal::CalculateLstmGateFloat(
      input_data, gate_params.activation_weight,
      /*aux_input=*/nullptr, /*aux_input_to_gate_weights*/ nullptr,
      hidden_state, gate_params.recurrent_weight,
      /*cell_state=*/nullptr, /*cell_to_gate_weights=*/nullptr,
      /*layer_norm_coefficients=*/nullptr, gate_params.fused_bias, kBatchSize,
      kInputDimension, kInputDimension, kStateDimension, kStateDimension,
      /*activation=*/activation_type, gate_output,
      /*is_input_all_zeros=*/false,
      /*is_aux_input_all_zeros=*/true);
  ValidateResultGoldens(expected_vals, gate_output, kGateOutputSize,
                        kTestFloatTolerance);
}

// TODO(b/253466487): Clean up the input parameters, which requires refactor
// IntegerLstmParameter
template <typename ActivationType, typename BiasType, typename CellType>
void TestGateOutputQuantized(
    const GateParameters<int8_t, BiasType>& gate_params,
    const ModelQuantizationParameters& quantization_settings,
    int32_t effective_input_to_gate_scale_a,
    int32_t effective_input_to_gate_scale_b,
    int32_t effective_recurrent_to_gate_scale_a,
    int32_t effective_recurrent_to_gate_scale_b,
    TfLiteFusedActivation nonlinear_type, const float* expected_vals,
    float tolerance) {
  // Quantize the  floating point input
  ActivationType quantized_input[kOneTimeInputSize] = {};
  Quantize(kGateOutputData.input_data, quantized_input, kOneTimeInputSize,
           quantization_settings.input_quantization_parameters.scale,
           quantization_settings.input_quantization_parameters.zero_point);
  // Quantize the  floating point hidden state
  ActivationType quantized_hidden_state[kGateOutputSize] = {};
  Quantize(kGateOutputData.hidden_state, quantized_hidden_state,
           kGateOutputSize,
           quantization_settings.hidden_quantization_parameters.scale,
           quantization_settings.hidden_quantization_parameters.zero_point);

  CellType gate_output[kGateOutputSize] = {};
  BiasType scratch_buffer[kGateOutputSize] = {};

  tflite::lstm_internal::CalculateLstmGateInteger8x8_16(
      // Input and weights
      quantized_input, gate_params.activation_weight,
      gate_params.activation_zp_folded_bias, effective_input_to_gate_scale_a,
      effective_input_to_gate_scale_b,
      // Output state and weights
      quantized_hidden_state, gate_params.activation_weight,
      gate_params.recurrent_zp_folded_bias, effective_recurrent_to_gate_scale_a,
      effective_recurrent_to_gate_scale_b,
      // Cell state and weights
      nullptr, nullptr, 0, 0,
      // Layer normalization parameters (layer norm LSTM)
      nullptr, nullptr, 0, 0, 0,
      // Array sizes
      kBatchSize, kInputDimension, kStateDimension, kStateDimension,
      nonlinear_type,
      // Output
      gate_output,
      // Parameters for performance optimizations
      // Scratch arrays
      scratch_buffer);

  float gate_output_float[kGateOutputSize] = {};
  Dequantize(gate_output, kGateOutputSize,
             quantization_settings.nonlinear_activation_output_scale, 0,
             gate_output_float);

  ValidateResultGoldens(expected_vals, gate_output_float, kGateOutputSize,
                        tolerance);
}

void TestCellUpdateFloat() {
  // copy the data since it will be updated
  float cell_state[kGateOutputSize] = {};
  std::memcpy(cell_state, kGateOutputData.cell_state,
              kGateOutputSize * sizeof(float));

  float forget_gate[kGateOutputSize] = {};
  std::memcpy(forget_gate, kGateOutputData.expected_forget_gate_output,
              kGateOutputSize * sizeof(float));

  tflite::lstm_internal::UpdateLstmCellFloat(
      kBatchSize, kStateDimension, cell_state,
      kGateOutputData.expected_input_gate_output, forget_gate,
      kGateOutputData.expected_cell_gate_output,
      /*use_cifg=*/false,
      /*clip=*/kModelSettings.cell_clip);

  ValidateResultGoldens(kGateOutputData.expected_updated_cell, cell_state,
                        kGateOutputSize, kTestFloatTolerance);
}

template <typename ActivationType, typename BiasType, typename CellType>
void TestCellUpdateQuantized(
    const ModelQuantizationParameters& quantization_settings,
    const int32_t cell_scale_shift, const CellType quantized_cell_clip,
    const float tolerance) {
  CellType quantized_cell_state[kGateOutputSize] = {};
  tflite::Quantize(
      kGateOutputData.cell_state, quantized_cell_state, kGateOutputSize,
      quantization_settings.cell_quantization_parameters.scale,
      quantization_settings.cell_quantization_parameters.zero_point);

  CellType quantized_forget_gate[kGateOutputSize] = {};
  tflite::Quantize(kGateOutputData.expected_forget_gate_output,
                   quantized_forget_gate, kGateOutputSize,
                   quantization_settings.nonlinear_activation_output_scale, 0);

  CellType quantized_input_gate[kGateOutputSize] = {};
  tflite::Quantize(kGateOutputData.expected_input_gate_output,
                   quantized_input_gate, kGateOutputSize,
                   quantization_settings.nonlinear_activation_output_scale, 0);

  CellType quantized_cell_gate[kGateOutputSize] = {};
  tflite::Quantize(kGateOutputData.expected_cell_gate_output,
                   quantized_cell_gate, kGateOutputSize,
                   quantization_settings.nonlinear_activation_output_scale, 0);

  tflite::lstm_internal::UpdateLstmCellInteger(
      kBatchSize, kStateDimension, quantized_cell_state, cell_scale_shift,
      quantized_input_gate, quantized_forget_gate, quantized_cell_gate, false,
      quantized_cell_clip);

  float cell_state_float[kGateOutputSize] = {};
  Dequantize(quantized_cell_state, kGateOutputSize,
             quantization_settings.cell_quantization_parameters.scale,
             quantization_settings.cell_quantization_parameters.zero_point,
             cell_state_float);

  ValidateResultGoldens(kGateOutputData.expected_updated_cell, cell_state_float,
                        kGateOutputSize, tolerance);
}

void TestHiddenStateUpdateFloat() {
  // If no projection layer, hidden state dimension == output dimension ==
  // cell state dimension
  float output[kGateOutputSize] = {};
  float scratch[kGateOutputSize] = {};

  tflite::lstm_internal::CalculateLstmOutputFloat(
      kBatchSize, kStateDimension, kStateDimension,
      kGateOutputData.expected_updated_cell,
      kGateOutputData.expected_output_gate_output, kTfLiteActTanh, nullptr,
      nullptr, 0, output, scratch);

  ValidateResultGoldens(kGateOutputData.expected_updated_hidden, output,
                        kGateOutputSize, kTestFloatTolerance);
}

template <typename ActivationType, typename BiasType, typename CellType>
void TestHiddenStateUpdateQuantized(
    const ModelQuantizationParameters& quantization_settings,
    const IntegerLstmParameter& evaluation_params, const float tolerance) {
  CellType quantized_cell_state[kGateOutputSize] = {};
  tflite::Quantize(
      kGateOutputData.expected_updated_cell, quantized_cell_state,
      kGateOutputSize, quantization_settings.cell_quantization_parameters.scale,
      quantization_settings.cell_quantization_parameters.zero_point);

  CellType quantized_output_gate[kGateOutputSize] = {};
  tflite::Quantize(kGateOutputData.expected_output_gate_output,
                   quantized_output_gate, kGateOutputSize,
                   quantization_settings.nonlinear_activation_output_scale, 0);

  // scratches
  int16_t scratch0[kGateOutputSize] = {};
  int8_t scratch1[kGateOutputSize] = {};
  int32_t scratch2[kGateOutputSize] = {};

  // output (updated hidden state)
  int8_t output_state[kGateOutputSize] = {};

  tflite::lstm_internal::CalculateLstmOutputInteger8x8_16(
      kBatchSize, kStateDimension, kStateDimension, quantized_cell_state,
      evaluation_params.cell_scale, quantized_output_gate,
      evaluation_params.effective_hidden_scale_a,
      evaluation_params.effective_hidden_scale_b, evaluation_params.hidden_zp,
      /*projection_weights=*/nullptr, /*proj_scale_a=*/0, 0, 0,
      /*output_state_zp=*/evaluation_params.hidden_zp,
      evaluation_params.quantized_proj_clip, output_state, scratch0, scratch1,
      scratch2);

  float output_state_float[kGateOutputSize] = {};
  Dequantize(output_state, kGateOutputSize,
             quantization_settings.hidden_quantization_parameters.scale,
             quantization_settings.hidden_quantization_parameters.zero_point,
             output_state_float);

  ValidateResultGoldens(kGateOutputData.expected_updated_hidden,
                        output_state_float, kGateOutputSize, tolerance);
}

void TestOneStepLSTMFloat() {
  // scratch buffers
  float forget_gate_scratch[kGateOutputSize] = {};
  float input_gate_scratch[kGateOutputSize] = {};
  float cell_gate_scratch[kGateOutputSize] = {};
  float output_gate_scratch[kGateOutputSize] = {};

  // initialize hidden and cell state (will be updated, copy the value)
  float hidden_state[kGateOutputSize] = {};
  std::memcpy(hidden_state, kGateOutputData.hidden_state,
              kGateOutputSize * sizeof(float));

  float cell_state[kGateOutputSize] = {};
  std::memcpy(cell_state, kGateOutputData.cell_state,
              kGateOutputSize * sizeof(float));

  float output[kGateOutputSize] = {};

  tflite::lstm_internal::LstmStepFloat(
      kGateOutputData.input_data, kInputGateParameters.activation_weight,
      kForgetGateParameters.activation_weight,
      kCellGateParameters.activation_weight,
      kOutputGateParameters.activation_weight,
      /*aux_input_ptr=*/nullptr, /*aux_input_to_input_weights_ptr=*/nullptr,
      /*aux_input_to_forget_weights_ptr=*/nullptr,
      /*aux_input_to_cell_weights_ptr=*/nullptr,
      /*aux_input_to_output_weights_ptr=*/nullptr,
      kInputGateParameters.recurrent_weight,
      kForgetGateParameters.recurrent_weight,
      kCellGateParameters.recurrent_weight,
      kOutputGateParameters.recurrent_weight,
      /*cell_to_input_weights_ptr=*/nullptr,
      /*cell_to_forget_weights_ptr=*/nullptr,
      /*cell_to_output_weights_ptr=*/nullptr,
      /*input_layer_norm_coefficients_ptr=*/nullptr,
      /*forget_layer_norm_coefficients_ptr=*/nullptr,
      /*cell_layer_norm_coefficients_ptr=*/nullptr,
      /*output_layer_norm_coefficients_ptr=*/nullptr,
      kInputGateParameters.fused_bias, kForgetGateParameters.fused_bias,
      kCellGateParameters.fused_bias, kOutputGateParameters.fused_bias,
      /*projection_weights_ptr=*/nullptr, /*projection_bias_ptr=*/nullptr,
      &kModelSettings, kBatchSize, kStateDimension, kInputDimension,
      kInputDimension, kStateDimension,
      /*output_batch_leading_dim=*/0, hidden_state, cell_state,
      input_gate_scratch, forget_gate_scratch, cell_gate_scratch,
      output_gate_scratch, output);

  ValidateResultGoldens(kGateOutputData.expected_updated_hidden, hidden_state,
                        kGateOutputSize, kTestFloatTolerance);
  ValidateResultGoldens(kGateOutputData.expected_updated_cell, cell_state,
                        kGateOutputSize, kTestFloatTolerance);
}

template <typename ActivationType, typename BiasType, typename CellType>
void TestOneStepLSTMQuantized(
    QuantizedModelContents<ActivationType, int8_t, BiasType, CellType>&
        model_contents,
    const float hidden_state_tolerance, const float cell_state_tolerance) {
  auto quantization_settings = model_contents.QuantizationSettings();
  auto evaluation_params = model_contents.EvaluationParameters();

  ActivationType quantized_input[4] = {};
  tflite::Quantize(
      kGateOutputData.input_data, quantized_input, 4,
      quantization_settings.input_quantization_parameters.scale,
      quantization_settings.input_quantization_parameters.zero_point);

  // initialize hidden and cell state (will be updated)
  ActivationType quantized_hidden_state[kGateOutputSize] = {};
  Quantize(kGateOutputData.hidden_state, quantized_hidden_state,
           kGateOutputSize,
           quantization_settings.hidden_quantization_parameters.scale,
           quantization_settings.hidden_quantization_parameters.zero_point);

  CellType quantized_cell_state[kGateOutputSize] = {};
  tflite::Quantize(
      kGateOutputData.cell_state, quantized_cell_state, kGateOutputSize,
      quantization_settings.cell_quantization_parameters.scale,
      quantization_settings.cell_quantization_parameters.zero_point);

  ActivationType output[kGateOutputSize] = {};
  // Scratch buffers
  CellType scratch0[kGateOutputSize] = {};
  CellType scratch1[kGateOutputSize] = {};
  CellType scratch2[kGateOutputSize] = {};
  CellType scratch3[kGateOutputSize] = {};
  ActivationType scratch4[kOutputSize] = {};
  BiasType scratch5[kGateOutputSize] = {};

  tflite::lstm_internal::LstmStepInteger8x8_16(
      quantized_input, model_contents.InputGateParams().activation_weight,
      evaluation_params.effective_input_to_input_scale_a,
      evaluation_params.effective_input_to_input_scale_b,
      model_contents.ForgetGateParams().activation_weight,
      evaluation_params.effective_input_to_forget_scale_a,
      evaluation_params.effective_input_to_forget_scale_b,
      model_contents.CellGateParams().activation_weight,
      evaluation_params.effective_input_to_cell_scale_a,
      evaluation_params.effective_input_to_cell_scale_b,
      model_contents.OutputGateParams().activation_weight,
      evaluation_params.effective_input_to_output_scale_a,
      evaluation_params.effective_input_to_output_scale_b,
      model_contents.InputGateParams().recurrent_weight,
      evaluation_params.effective_recurrent_to_input_scale_a,
      evaluation_params.effective_recurrent_to_input_scale_b,
      model_contents.ForgetGateParams().recurrent_weight,
      evaluation_params.effective_recurrent_to_forget_scale_a,
      evaluation_params.effective_recurrent_to_forget_scale_b,
      model_contents.CellGateParams().recurrent_weight,
      evaluation_params.effective_recurrent_to_cell_scale_a,
      evaluation_params.effective_recurrent_to_cell_scale_b,
      model_contents.OutputGateParams().recurrent_weight,
      evaluation_params.effective_recurrent_to_output_scale_a,
      evaluation_params.effective_recurrent_to_output_scale_b,
      /*cell_to_input_weight_ptr=*/nullptr,
      /*effective_cell_to_input_scale_a=*/0,
      /*effective_cell_to_input_scale_b=*/0,
      /*cell_to_forget_weight_ptr=*/nullptr,
      /*effective_cell_to_forget_scale_a=*/0,
      /*effective_cell_to_forget_scale_b=*/0,
      /*cell_to_output_weight_ptr=*/nullptr,
      /*effective_cell_to_output_scale_a=*/0,
      /*effective_cell_to_output_scale_b=*/0,
      /*projection_weight_ptr=*/nullptr, /*effective_proj_scale_a=*/0,
      /*effective_proj_scale_b=*/0, evaluation_params.hidden_zp,
      evaluation_params.effective_hidden_scale_a,
      evaluation_params.effective_hidden_scale_b,
      /*layer_norm_input_weight_ptr=*/nullptr,
      /*layer_norm_input_scale_a=*/0, /*layer_norm_input_scale_b=*/0,
      /*layer_norm_forget_weight_ptr=*/nullptr,
      /*layer_norm_forget_scale_a=*/0, /*layer_norm_forget_scale_b=*/0,
      /*layer_norm_cell_weight_ptr=*/nullptr,
      /*layer_norm_cell_scale_a=*/0, /*layer_norm_cell_scale_b=*/0,
      /*layer_norm_output_weight_ptr=*/nullptr,
      /*layer_norm_output_scale_a=*/0, /*layer_norm_output_scale_b=*/0,
      /*input_gate_bias_ptr=*/nullptr, /*forget_gate_bias_ptr=*/nullptr,
      /*cell_gate_bias_ptr=*/nullptr, /*output_gate_bias_ptr=*/nullptr,
      evaluation_params.quantized_cell_clip,
      evaluation_params.quantized_proj_clip, evaluation_params.cell_scale,
      /*input_variance_guard=*/0, /*forget_variance_guard=*/0,
      /*cell_variance_guard=*/0, /*output_variance_guard=*/0,
      evaluation_params.input_to_forget_effective_bias,
      evaluation_params.recurrent_to_forget_effective_bias,
      evaluation_params.input_to_cell_effective_bias,
      evaluation_params.recurrent_to_cell_effective_bias,
      evaluation_params.input_to_output_effective_bias,
      evaluation_params.recurrent_to_output_effective_bias,
      evaluation_params.input_to_input_effective_bias,
      evaluation_params.recurrent_to_input_effective_bias,
      evaluation_params.projection_effective_bias, kBatchSize, kInputDimension,
      kStateDimension, kStateDimension, quantized_hidden_state,
      quantization_settings.output_quantization_parameters.zero_point,
      quantized_cell_state, output, scratch0, scratch1, scratch2, scratch3,
      scratch4, scratch5);

  float dequantized_hidden_state[kGateOutputSize] = {};
  Dequantize(quantized_hidden_state, kGateOutputSize,
             quantization_settings.hidden_quantization_parameters.scale,
             quantization_settings.hidden_quantization_parameters.zero_point,
             dequantized_hidden_state);

  float dequantized_cell_state[kGateOutputSize] = {};
  Dequantize(quantized_cell_state, kGateOutputSize,
             quantization_settings.cell_quantization_parameters.scale,
             quantization_settings.cell_quantization_parameters.zero_point,
             dequantized_cell_state);

  ValidateResultGoldens(kGateOutputData.expected_updated_hidden,
                        dequantized_hidden_state, kGateOutputSize,
                        hidden_state_tolerance);
  ValidateResultGoldens(kGateOutputData.expected_updated_cell,
                        dequantized_cell_state, kGateOutputSize,
                        cell_state_tolerance);
}

void TestLSTMEvalFloat() {
  ModelContents<float, float, float, float> float_model_contents(
      kForgetGateParameters, kInputGateParameters, kCellGateParameters,
      kOutputGateParameters);

  float_model_contents.SetInputTensorData(kMultiTimeEvalData.input_data);

  tflite::EvalFloatLstm(
      float_model_contents.GetTensor(0), float_model_contents.GetTensor(4),
      float_model_contents.GetTensor(1), float_model_contents.GetTensor(7),
      float_model_contents.GetTensor(10), float_model_contents.GetTensor(5),
      float_model_contents.GetTensor(2), float_model_contents.GetTensor(8),
      float_model_contents.GetTensor(11),
      /*cell_to_input_weights=*/nullptr,
      /*cell_to_forget_weights=*/nullptr,
      /*cell_to_output_weights=*/nullptr,
      /*input_layer_norm_coefficients=*/nullptr,
      /*forget_layer_norm_coefficients=*/nullptr,
      /*cell_layer_norm_coefficients=*/nullptr,
      /*output_layer_norm_coefficients=*/nullptr,
      /*aux_input=*/nullptr,
      /*aux_input_to_input_weights=*/nullptr,
      /*aux_input_to_forget_weights=*/nullptr,
      /*aux_input_to_cell_weights=*/nullptr,
      /*aux_input_to_output_weights=*/nullptr,
      float_model_contents.GetTensor(6), float_model_contents.GetTensor(3),
      float_model_contents.GetTensor(9), float_model_contents.GetTensor(12),
      /*projection_weights=*/nullptr,
      /*projection_bias=*/nullptr, &kModelSettings,
      /*forward_sequence=*/true, /*time_major=*/false,
      /*output_offset=*/0, float_model_contents.ScratchBuffers(),
      float_model_contents.GetTensor(13), float_model_contents.GetTensor(14),
      float_model_contents.GetTensor(15));

  // Validate hidden state. See previous test for the calculation
  ValidateResultGoldens(kMultiTimeEvalData.expected_hidden_state,
                        float_model_contents.GetHiddenState(), kGateOutputSize,
                        kTestFloatTolerance);
  // Validate cell state. See previous test for the calculation
  ValidateResultGoldens(kMultiTimeEvalData.expected_cell_state,
                        float_model_contents.GetCellState(), kGateOutputSize,
                        kTestFloatTolerance);
  // Validate output . See previous test for the calculation
  ValidateResultGoldens(kMultiTimeEvalData.expected_output,
                        float_model_contents.GetOutput(), kOutputSize,
                        kTestFloatTolerance);
}

template <typename ActivationType, typename BiasType, typename CellType>
void TestLSTMEvalQuantized(const float hidden_state_tolerance,
                           const float cell_state_tolerance) {
  // TODO(b/253466487): make it constant after refactor the IntegerLstmParameter
  QuantizedModelContents<int8_t, int8_t, int32_t, int16_t>
      quantized_model_content(kInt8QuantizationSettings, kForgetGateParameters,
                              kInputGateParameters, kCellGateParameters,
                              kOutputGateParameters);
  // Scratch buffers
  CellType scratch0[kGateOutputSize] = {};
  CellType scratch1[kGateOutputSize] = {};
  CellType scratch2[kGateOutputSize] = {};
  CellType scratch3[kGateOutputSize] = {};
  ActivationType scratch4[kOutputSize] = {};
  BiasType scratch5[kGateOutputSize] = {};

  auto quantization_settings = quantized_model_content.QuantizationSettings();

  // Quantize Input and Hidden State
  quantized_model_content.QuantizeInputTensor(kMultiTimeEvalData.input_data);
  quantized_model_content.QuantizeHiddenStateTensor(
      kMultiTimeEvalData.hidden_state);

  EvalInteger8x8_16Lstm(
      quantized_model_content.GetTensor(0),
      quantized_model_content.GetTensor(4),
      quantized_model_content.GetTensor(1),
      quantized_model_content.GetTensor(7),
      quantized_model_content.GetTensor(10),
      quantized_model_content.GetTensor(5),
      quantized_model_content.GetTensor(2),
      quantized_model_content.GetTensor(8),
      quantized_model_content.GetTensor(11),
      /*cell_to_input_weights=*/nullptr,
      /*cell_to_forget_weights=*/nullptr,
      /*cell_to_output_weights=*/nullptr,
      /*input_layer_norm_coefficients=*/nullptr,
      /*forget_layer_norm_coefficients=*/nullptr,
      /*cell_layer_norm_coefficients=*/nullptr,
      /*output_layer_norm_coefficients=*/nullptr,
      quantized_model_content.GetTensor(6),
      quantized_model_content.GetTensor(3),
      quantized_model_content.GetTensor(9),
      quantized_model_content.GetTensor(12),
      /*projection_weights=*/nullptr,
      /*projection_bias=*/nullptr, &kModelSettings,
      /*forward_sequence=*/true, /*time_major=*/false,
      &quantized_model_content.EvaluationParameters(),
      quantization_settings.output_quantization_parameters.zero_point,
      quantized_model_content.GetTensor(13),
      quantized_model_content.GetTensor(14),
      quantized_model_content.GetTensor(15), scratch0, scratch1, scratch2,
      scratch3, scratch4, scratch5);

  float dequantized_hidden_state[kGateOutputSize] = {};
  Dequantize(quantized_model_content.GetHiddenState(), kGateOutputSize,
             quantization_settings.hidden_quantization_parameters.scale,
             quantization_settings.hidden_quantization_parameters.zero_point,
             dequantized_hidden_state);

  ValidateResultGoldens(kMultiTimeEvalData.expected_hidden_state,
                        dequantized_hidden_state, kGateOutputSize,
                        hidden_state_tolerance);

  float dequantized_cell_state[kGateOutputSize] = {};
  Dequantize(quantized_model_content.GetCellState(), kGateOutputSize,
             quantization_settings.cell_quantization_parameters.scale,
             quantization_settings.cell_quantization_parameters.zero_point,
             dequantized_cell_state);
  ValidateResultGoldens(kMultiTimeEvalData.expected_cell_state,
                        dequantized_cell_state, kGateOutputSize,
                        cell_state_tolerance);

  float dequantized_output[kOutputSize] = {};
  Dequantize(quantized_model_content.GetOutput(), kOutputSize,
             quantization_settings.output_quantization_parameters.scale,
             quantization_settings.output_quantization_parameters.zero_point,
             dequantized_output);
  ValidateResultGoldens(kMultiTimeEvalData.expected_output, dequantized_output,
                        kOutputSize, hidden_state_tolerance);
}

}  // namespace
}  // namespace testing
}  // namespace tflite
#endif  // !defined(XTENSA)

TF_LITE_MICRO_TESTS_BEGIN
// TODO(b/230666079) enable below tests for xtensa when the xtensa
// kernel is reconciled with reference kernel
#if !defined(XTENSA)
TF_LITE_MICRO_TEST(CheckGateOutputFloat) {
  // Forget gate
  tflite::testing::TestGateOutputFloat(
      tflite::testing::kForgetGateParameters, kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.input_data,
      tflite::testing::kGateOutputData.hidden_state,
      tflite::testing::kGateOutputData.expected_forget_gate_output);
  // Input gate
  tflite::testing::TestGateOutputFloat(
      tflite::testing::kInputGateParameters, kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.input_data,
      tflite::testing::kGateOutputData.hidden_state,
      tflite::testing::kGateOutputData.expected_input_gate_output);
  // output gate
  tflite::testing::TestGateOutputFloat(
      tflite::testing::kOutputGateParameters, kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.input_data,
      tflite::testing::kGateOutputData.hidden_state,
      tflite::testing::kGateOutputData.expected_output_gate_output);
  // cell (modulation) gate
  tflite::testing::TestGateOutputFloat(
      tflite::testing::kCellGateParameters,
      tflite::testing::kModelSettings.activation,
      tflite::testing::kGateOutputData.input_data,
      tflite::testing::kGateOutputData.hidden_state,
      tflite::testing::kGateOutputData.expected_cell_gate_output);
}

TF_LITE_MICRO_TEST(CheckGateOutputInt8) {
  // TODO(b/253466487): make it constant after refactor the IntegerLstmParameter
  tflite::testing::QuantizedModelContents<int8_t, int8_t, int32_t, int16_t>
      model_contents_int8(tflite::testing::kInt8QuantizationSettings,
                          tflite::testing::kForgetGateParameters,
                          tflite::testing::kInputGateParameters,
                          tflite::testing::kCellGateParameters,
                          tflite::testing::kOutputGateParameters);
  auto evaluation_params = model_contents_int8.EvaluationParameters();

  // Different gate has different weights, resulting different quantization
  // prediction precisions
  float tolerance;

  // Forget Gate
  // Quantization performs badly here due to integer overflow!!!
  tolerance = 1e-1f;
  tflite::testing::TestGateOutputQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.ForgetGateParams(),
      model_contents_int8.QuantizationSettings(),
      evaluation_params.effective_input_to_forget_scale_a,
      evaluation_params.effective_input_to_forget_scale_b,
      evaluation_params.effective_recurrent_to_forget_scale_a,
      evaluation_params.effective_recurrent_to_forget_scale_b,
      kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.expected_forget_gate_output, tolerance);

  // Input Gate
  tolerance = 1e-1f;
  tflite::testing::TestGateOutputQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.InputGateParams(),
      model_contents_int8.QuantizationSettings(),
      evaluation_params.effective_input_to_input_scale_a,
      evaluation_params.effective_input_to_input_scale_b,
      evaluation_params.effective_recurrent_to_input_scale_a,
      evaluation_params.effective_recurrent_to_input_scale_b, kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.expected_input_gate_output, tolerance);

  // Output Gate
  tolerance = 1e-2f;
  tflite::testing::TestGateOutputQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.OutputGateParams(),
      model_contents_int8.QuantizationSettings(),
      evaluation_params.effective_input_to_output_scale_a,
      evaluation_params.effective_input_to_output_scale_b,
      evaluation_params.effective_recurrent_to_output_scale_a,
      evaluation_params.effective_recurrent_to_output_scale_b,
      kTfLiteActSigmoid,
      tflite::testing::kGateOutputData.expected_output_gate_output, tolerance);

  // Cell Gate (tanh activation)
  tolerance = 1e-2f;
  tflite::testing::TestGateOutputQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.CellGateParams(),
      model_contents_int8.QuantizationSettings(),
      evaluation_params.effective_input_to_cell_scale_a,
      evaluation_params.effective_input_to_cell_scale_b,
      evaluation_params.effective_recurrent_to_cell_scale_a,
      evaluation_params.effective_recurrent_to_cell_scale_b,
      tflite::testing::kModelSettings.activation,
      tflite::testing::kGateOutputData.expected_cell_gate_output, tolerance);
}

TF_LITE_MICRO_TEST(CheckCellUpdateFloat) {
  tflite::testing::TestCellUpdateFloat();
}

TF_LITE_MICRO_TEST(CheckCellUpdateInt8) {
  const tflite::testing::QuantizedModelContents<int8_t, int8_t, int32_t,
                                                int16_t>
      model_contents_int8(tflite::testing::kInt8QuantizationSettings,
                          tflite::testing::kForgetGateParameters,
                          tflite::testing::kInputGateParameters,
                          tflite::testing::kCellGateParameters,
                          tflite::testing::kOutputGateParameters);

  auto evaluation_params = model_contents_int8.EvaluationParameters();
  // Very high precision. The error is introduced by the
  // quantization error of the clip value (~1e-5), but cannot actually reach
  // the precision due to integer overflow for the elements
  const float tolerance = 1e-3f;
  tflite::testing::TestCellUpdateQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.QuantizationSettings(), evaluation_params.cell_scale,
      evaluation_params.quantized_cell_clip, tolerance);
}

TF_LITE_MICRO_TEST(CheckOutputCalculationFloat) {
  // Not testing projection here, output is the updated hidden state
  tflite::testing::TestHiddenStateUpdateFloat();
}

TF_LITE_MICRO_TEST(CheckOutputCalculationInt8) {
  const tflite::testing::QuantizedModelContents<int8_t, int8_t, int32_t,
                                                int16_t>
      model_contents_int8(tflite::testing::kInt8QuantizationSettings,
                          tflite::testing::kForgetGateParameters,
                          tflite::testing::kInputGateParameters,
                          tflite::testing::kCellGateParameters,
                          tflite::testing::kOutputGateParameters);

  // Theoritical error floor = quantization scale = 0.004705882165580988
  const float tolerance = 1e-2;
  tflite::testing::TestHiddenStateUpdateQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8.QuantizationSettings(),
      model_contents_int8.EvaluationParameters(), tolerance);
}

TF_LITE_MICRO_TEST(CheckOneStepLSTMFloat) {
  tflite::testing::TestOneStepLSTMFloat();
}

TF_LITE_MICRO_TEST(CheckOneStepLSTMInt8) {
  // TODO(b/253466487): make it constant after refactor the IntegerLstmParameter
  tflite::testing::QuantizedModelContents<int8_t, int8_t, int32_t, int16_t>
      model_contents_int8(tflite::testing::kInt8QuantizationSettings,
                          tflite::testing::kForgetGateParameters,
                          tflite::testing::kInputGateParameters,
                          tflite::testing::kCellGateParameters,
                          tflite::testing::kOutputGateParameters);

  const float hidden_state_tolerance = 1e-2;
  // cell state degrade due to integer overflow
  const float cell_state_tolerance = 1e-1;
  tflite::testing::TestOneStepLSTMQuantized<int8_t, int32_t, int16_t>(
      model_contents_int8, hidden_state_tolerance, cell_state_tolerance);
}

TF_LITE_MICRO_TEST(TestLSTMEvalFloat) { tflite::testing::TestLSTMEvalFloat(); }

TF_LITE_MICRO_TEST(TestLSTMEvalInt8) {
  const float hidden_state_tolerance = 1e-2;
  // cell state degrade due to integer overflow
  const float cell_state_tolerance = 1e-1;
  tflite::testing::TestLSTMEvalQuantized<int8_t, int32_t, int16_t>(
      hidden_state_tolerance, cell_state_tolerance);
}
#endif  // !defined(XTENSA)
TF_LITE_MICRO_TESTS_END
