#!/bin/bash -e
# Copyright 2022 The TensorFlow Authors. All Rights Reserved.
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

# Measures the size of specified binaries and append the report to a log.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -eq 0 ]
then
  SAVE_TO_DIR=$(pwd)/report
else
  SAVE_TO_DIR=${1}
fi

if [ $# -lt 1 ]
then
  SOURCE_RELATIVE_ROOT_DIR=../../../core_software/tflite_micro
else
  SOURCE_RELATIVE_ROOT_DIR=${2}
fi

echo $SAVE_TO_DIR
echo $SOURCE_RELATIVE_ROOT_DIR