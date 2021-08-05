/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/lite/micro/kernels/conv_test_data.h"

#include "tensorflow/lite/c/common.h"

namespace tflite {
namespace testing {
namespace conv {

const int8_t kInput1x32x32x3[1 * 32 * 32 * 3] = {
    27,   32,   33,   31,   31,   35,   32,   33,   35,   30,   37,   36,
    31,   37,   37,   31,   37,   37,   31,   36,   42,   32,   38,   39,
    34,   39,   40,   36,   38,   40,   32,   42,   41,   36,   41,   42,
    35,   41,   42,   38,   44,   45,   40,   46,   43,   40,   44,   48,
    34,   39,   40,   39,   43,   44,   49,   53,   60,   42,   48,   49,
    36,   44,   43,   35,   45,   44,   37,   46,   45,   38,   46,   45,
    39,   45,   45,   37,   45,   44,   41,   46,   46,   41,   46,   46,
    38,   46,   46,   40,   47,   47,   38,   48,   47,   38,   46,   46,
    29,   32,   33,   30,   32,   34,   31,   32,   34,   29,   35,   34,
    30,   36,   34,   30,   37,   36,   32,   37,   37,   31,   37,   36,
    31,   37,   37,   33,   38,   39,   34,   40,   39,   34,   41,   41,
    42,   48,   47,   26,   27,   27,   -20,  -17,  -19,  -4,   -3,   0,
    1,    1,    6,    -4,   -5,   0,    -31,  -33,  -31,  -8,   -7,   -7,
    53,   59,   62,   37,   44,   42,   37,   46,   44,   37,   45,   43,
    38,   46,   43,   39,   45,   44,   39,   46,   46,   38,   47,   46,
    42,   47,   47,   40,   48,   47,   42,   47,   48,   41,   49,   47,
    31,   33,   35,   33,   33,   36,   31,   37,   35,   30,   37,   36,
    31,   37,   36,   33,   36,   38,   32,   38,   37,   33,   39,   37,
    34,   40,   40,   33,   40,   40,   37,   38,   41,   40,   47,   47,
    4,    7,    2,    -34,  -34,  -34,  34,   40,   38,   -48,  -42,  -47,
    -65,  -61,  -62,  -55,  -47,  -49,  7,    14,   14,   21,   26,   26,
    -24,  -24,  -31,  49,   56,   58,   39,   47,   46,   38,   47,   45,
    39,   47,   44,   39,   47,   45,   39,   48,   45,   40,   48,   47,
    40,   47,   47,   39,   46,   46,   40,   47,   47,   41,   47,   48,
    30,   31,   32,   29,   34,   34,   31,   34,   36,   30,   36,   36,
    31,   36,   34,   32,   36,   34,   30,   37,   37,   34,   37,   35,
    34,   37,   38,   34,   37,   38,   36,   39,   39,   43,   45,   44,
    -68,  -69,  -78,  50,   57,   56,   -70,  -69,  -71,  -116, -117, -114,
    -105, -105, -102, -99,  -99,  -97,  -106, -105, -97,  62,   72,   70,
    -30,  -28,  -27,  10,   14,   10,   42,   53,   51,   40,   48,   47,
    42,   48,   48,   42,   48,   47,   41,   49,   47,   42,   48,   47,
    42,   49,   49,   43,   49,   49,   43,   49,   49,   41,   49,   47,
    33,   34,   37,   31,   36,   36,   31,   38,   37,   32,   37,   38,
    34,   36,   36,   35,   37,   36,   33,   39,   39,   33,   38,   38,
    35,   38,   37,   37,   39,   37,   38,   41,   42,   22,   25,   25,
    -46,  -49,  -50,  61,   64,   64,   -66,  -63,  -63,  -113, -113, -113,
    -100, -100, -97,  -97,  -96,  -95,  -112, -110, -103, 51,   65,   62,
    10,   17,   17,   -40,  -38,  -43,  46,   58,   55,   40,   47,   46,
    42,   46,   47,   39,   48,   47,   39,   49,   48,   38,   50,   49,
    42,   48,   48,   42,   48,   48,   42,   49,   49,   44,   48,   48,
    31,   34,   35,   31,   36,   35,   31,   38,   36,   32,   38,   36,
    33,   37,   38,   34,   38,   39,   36,   36,   39,   36,   37,   39,
    37,   38,   40,   37,   41,   42,   38,   42,   42,   41,   45,   45,
    -37,  -42,  -45,  44,   53,   55,   5,    3,    4,    -95,  -97,  -90,
    -89,  -92,  -94,  -86,  -85,  -82,  -38,  -62,  -62,  63,   35,   26,
    -19,  -19,  -23,  2,    -25,  -28,  69,   26,   27,   64,   21,   21,
    65,   21,   23,   62,   24,   21,   62,   26,   24,   56,   32,   29,
    44,   46,   44,   42,   50,   51,   43,   49,   49,   42,   48,   48,
    31,   33,   35,   32,   37,   35,   32,   37,   35,   33,   38,   39,
    37,   38,   39,   38,   39,   41,   39,   41,   41,   38,   40,   43,
    37,   41,   43,   38,   43,   45,   38,   46,   45,   45,   50,   50,
    10,   -1,   -5,   2,    9,    5,    42,   34,   34,   2,    -19,  -25,
    -64,  -73,  -88,  -39,  -34,  -37,  37,   -13,  -14,  3,    -22,  -20,
    -15,  -28,  -28,  67,   21,   24,   63,   26,   28,   61,   23,   28,
    60,   22,   23,   62,   24,   22,   61,   25,   23,   64,   26,   26,
    65,   23,   20,   45,   41,   37,   42,   49,   50,   42,   49,   48,
    31,   32,   34,   32,   33,   35,   32,   35,   34,   31,   38,   38,
    43,   45,   44,   41,   46,   44,   37,   44,   42,   39,   34,   32,
    45,   31,   27,   50,   30,   22,   46,   20,   13,   38,   5,    -4,
    33,   -9,   -14,  -2,   -42,  -49,  -41,  -50,  -49,  -44,  -54,  -51,
    -71,  -82,  -96,  -57,  -56,  -66,  -18,  -17,  -10,  -25,  -46,  -49,
    28,   -20,  -15,  36,   -6,   -5,   55,   18,   22,   56,   13,   17,
    56,   10,   14,   56,   13,   16,   52,   14,   12,   52,   17,   13,
    52,   15,   14,   50,   13,   4,    46,   53,   52,   44,   50,   49,
    27,   34,   35,   28,   36,   33,   37,   37,   31,   62,   50,   45,
    -26,  -26,  -24,  -127, -124, -123, 32,   -1,   0,    66,   26,   21,
    54,   16,   15,   49,   17,   14,   39,   7,    5,    23,   -19,  -24,
    0,    -43,  -49,  17,   -15,  -11,  43,   24,   32,   42,   31,   48,
    60,   62,   77,   79,   81,   90,   81,   73,   85,   82,   71,   79,
    66,   44,   51,   39,   -1,   7,    31,   -20,  -15,  33,   -15,  -11,
    39,   0,    1,    40,   1,    1,    43,   2,    3,    45,   7,    6,
    42,   -4,   -5,   33,   -13,  -23,  49,   55,   53,   43,   53,   53,
    56,   24,   21,   65,   19,   19,   83,   40,   38,   -85,  -95,  -95,
    -127, -127, -127, -34,  -30,  -21,  -59,  -60,  -59,  50,   16,   19,
    55,   15,   12,   42,   2,    -1,   25,   -4,   -5,   14,   4,    7,
    16,   17,   20,   0,    5,    4,    -9,   -10,  -6,   -6,   -7,   -1,
    -12,  -13,  -8,   -1,   -1,   5,    -4,   -3,   3,    11,   10,   13,
    29,   27,   27,   62,   64,   63,   99,   99,   103,  74,   59,   67,
    12,   -32,  -25,  9,    -44,  -51,  15,   -45,  -46,  15,   -50,  -54,
    19,   -39,  -49,  38,   31,   26,   45,   48,   48,   50,   34,   31,
    63,   22,   26,   69,   34,   37,   9,    -11,  -12,  -127, -127, -127,
    -109, -110, -110, -126, -127, -125, -57,  -52,  -46,  -25,  -40,  -47,
    66,   26,   27,   4,    -7,   -8,   -12,  -6,   -7,   -40,  -39,  -42,
    -46,  -47,  -50,  -21,  -21,  -21,  -28,  -28,  -30,  -13,  -11,  -17,
    -11,  -10,  -13,  -21,  -19,  -26,  -9,   -6,   -13,  -23,  -22,  -24,
    -8,   -1,   -6,   -4,   -1,   1,    -7,   -7,   -7,   25,   21,   19,
    77,   80,   80,   39,   11,   26,   -19,  -72,  -74,  19,   -21,  -24,
    51,   25,   23,   57,   13,   12,   56,   7,    7,    54,   8,    6,
    54,   22,   26,   55,   25,   30,   19,   -25,  -28,  -51,  -107, -114,
    -105, -114, -116, -114, -114, -109, -53,  -57,  -64,  24,   26,   27,
    -31,  -28,  -30,  -45,  -44,  -48,  -53,  -55,  -58,  -29,  -29,  -30,
    -23,  -21,  -25,  2,    3,    -1,   2,    2,    2,    3,    3,    1,
    -1,   1,    -1,   -22,  -20,  -22,  24,   27,   22,   0,    3,    -4,
    8,    11,   4,    -9,   -6,   -12,  -3,   3,    -2,   -1,   2,    1,
    0,    -1,   4,    5,    6,    7,    40,   38,   43,   52,   7,    8,
    60,   15,   19,   56,   6,    8,    48,   -3,   -2,   45,   -5,   -6,
    38,   -8,   -2,   37,   -6,   1,    22,   -11,  -6,   -25,  -65,  -69,
    -58,  -97,  -94,  -76,  -110, -111, -47,  -64,  -65,  -65,  -69,  -73,
    -47,  -49,  -51,  -37,  -35,  -38,  -20,  -20,  -20,  29,   32,   28,
    49,   54,   54,   65,   71,   72,   51,   59,   60,   32,   42,   45,
    47,   44,   48,   7,    -9,   -4,   83,   96,   95,   55,   63,   64,
    59,   58,   56,   43,   47,   44,   15,   16,   13,   2,    3,    -1,
    0,    3,    -2,   -4,   2,    2,    -18,  -18,  -18,  4,    0,    0,
    49,   5,    9,    50,   -8,   -1,   38,   -21,  -17,  33,   -18,  -18,
    18,   -34,  -41,  20,   -25,  -28,  23,   -7,   -10,  20,   -9,   -11,
    -13,  -56,  -55,  -64,  -91,  -96,  -48,  -58,  -61,  -54,  -55,  -58,
    -32,  -29,  -31,  12,   15,   15,   60,   66,   65,   7,    13,   18,
    45,   57,   57,   62,   16,   21,   69,   58,   60,   47,   29,   33,
    36,   4,    9,    21,   12,   15,   44,   23,   28,   75,   82,   81,
    69,   12,   22,   87,   95,   95,   31,   37,   42,   53,   62,   59,
    22,   24,   19,   1,    1,    -1,   -10,  -6,   -8,   -14,  -20,  -17,
    -34,  -40,  -41,  44,   -13,  -8,   21,   -34,  -30,  12,   -45,  -42,
    5,    -59,  -65,  7,    -48,  -53,  12,   -33,  -40,  17,   -21,  -25,
    0,    -37,  -36,  -53,  -68,  -70,  -54,  -60,  -62,  -11,  -6,   -5,
    22,   29,   29,   59,   68,   67,   66,   67,   65,   74,   70,   72,
    74,   82,   82,   69,   88,   88,   40,   12,   17,   72,   78,   78,
    51,   50,   51,   11,   0,    4,    84,   95,   95,   47,   22,   26,
    59,   63,   65,   79,   87,   88,   86,   90,   93,   78,   55,   57,
    66,   74,   72,   41,   47,   43,   13,   14,   12,   -6,   -2,   -5,
    -28,  -35,  -26,  -23,  -42,  -44,  19,   -39,  -33,  -4,   -68,  -64,
    7,    -50,  -55,  -11,  -78,  -77,  -3,   -54,  -59,  5,    -41,  -47,
    -45,  -65,  -70,  -50,  -63,  -68,  -25,  -20,  -17,  8,    15,   16,
    36,   41,   45,   30,   38,   43,   66,   40,   40,   59,   32,   37,
    69,   85,   82,   39,   16,   20,   23,   29,   33,   76,   84,   85,
    35,   38,   39,   -4,   -1,   0,    82,   89,   89,   46,   55,   58,
    45,   27,   31,   74,   84,   83,   76,   82,   83,   73,   45,   48,
    88,   97,   100,  30,   38,   37,   40,   43,   41,   9,    12,   7,
    12,   15,   16,   -56,  -62,  -53,  2,    -45,  -42,  -11,  -64,  -61,
    39,   3,    -3,   -37,  -103, -104, -28,  -88,  -87,  -32,  -83,  -84,
    -67,  -86,  -91,  -46,  -52,  -49,  11,   18,   21,   53,   59,   58,
    50,   58,   59,   46,   56,   59,   68,   79,   78,   66,   77,   76,
    70,   77,   77,   34,   18,   22,   71,   77,   79,   77,   84,   85,
    -30,  -27,  -29,  -73,  -76,  -82,  83,   90,   90,   77,   85,   86,
    51,   32,   36,   64,   72,   72,   77,   83,   84,   77,   86,   90,
    83,   91,   92,   82,   93,   94,   75,   82,   82,   26,   33,   29,
    18,   21,   20,   -27,  -29,  -14,  -47,  -75,  -76,  -25,  -77,  -78,
    43,   0,    2,    39,   0,    -3,   -69,  -108, -107, -95,  -118, -118,
    -37,  -56,  -62,  -12,  -10,  -6,   28,   34,   39,   61,   73,   72,
    70,   52,   50,   57,   15,   20,   63,   76,   77,   63,   73,   71,
    68,   75,   75,   46,   40,   41,   51,   48,   49,   63,   73,   74,
    27,   31,   32,   -9,   -7,   -2,   66,   73,   76,   75,   85,   87,
    37,   16,   19,   77,   86,   85,   81,   85,   86,   80,   88,   87,
    79,   73,   76,   77,   43,   45,   88,   98,   98,   65,   71,   72,
    30,   36,   31,   27,   28,   38,   -62,  -64,  -59,  37,   29,   29,
    34,   -12,  -13,  32,   -14,  -20,  24,   -19,  -22,  -52,  -76,  -81,
    -22,  -36,  -41,  6,    13,   19,   45,   51,   54,   43,   50,   51,
    44,   48,   51,   66,   70,   68,   64,   70,   70,   64,   71,   71,
    66,   72,   72,   67,   77,   76,   40,   29,   32,   21,   -2,   3,
    58,   65,   66,   -15,  -9,   -5,   52,   45,   48,   35,   10,   15,
    71,   80,   80,   75,   83,   82,   98,   102,  103,  95,   101,  101,
    93,   100,  99,   94,   103,  100,  81,   88,   88,   71,   77,   77,
    63,   71,   67,   37,   42,   50,   -68,  -75,  -64,  29,   -10,  -5,
    28,   -22,  -18,  38,   4,    4,    37,   -2,   -2,   -10,  -30,  -43,
    -23,  -35,  -37,  37,   44,   48,   40,   46,   48,   34,   41,   43,
    35,   46,   48,   67,   76,   75,   64,   70,   70,   64,   70,   70,
    64,   70,   70,   65,   71,   71,   68,   76,   76,   70,   73,   72,
    -21,  -30,  -30,  -66,  -74,  -87,  50,   41,   42,   71,   82,   81,
    70,   77,   77,   80,   89,   88,   118,  124,  124,  120,  123,  125,
    121,  124,  125,  121,  126,  127,  115,  120,  123,  98,   104,  104,
    85,   93,   91,   45,   50,   54,   -70,  -72,  -58,  56,   15,   19,
    30,   -25,  -18,  29,   -12,  -9,   10,   -40,  -40,  58,   37,   33,
    -30,  -42,  -44,  36,   45,   47,   40,   48,   51,   68,   78,   78,
    56,   12,   16,   57,   43,   49,   65,   72,   72,   63,   71,   70,
    64,   70,   70,   64,   70,   70,   64,   70,   70,   68,   74,   74,
    11,   14,   10,   -50,  -52,  -53,  79,   86,   86,   68,   76,   77,
    67,   75,   76,   81,   84,   89,   116,  124,  123,  121,  126,  126,
    122,  126,  127,  112,  101,  104,  97,   97,   96,   77,   84,   85,
    77,   83,   82,   37,   44,   46,   -74,  -78,  -62,  52,   -3,   0,
    24,   -23,  -21,  15,   -39,  -36,  -4,   -59,  -61,  7,    -18,  -22,
    -32,  -40,  -50,  27,   34,   36,   48,   54,   58,   67,   75,   76,
    59,   61,   59,   62,   69,   67,   62,   69,   69,   64,   70,   70,
    65,   71,   71,   66,   72,   72,   63,   69,   69,   58,   64,   64,
    33,   33,   35,   -29,  -24,  -24,  75,   83,   82,   72,   79,   79,
    70,   76,   79,   71,   76,   80,   78,   83,   86,   79,   84,   86,
    79,   89,   88,   83,   93,   92,   63,   70,   70,   82,   89,   92,
    60,   67,   70,   22,   25,   30,   -64,  -66,  -57,  34,   -27,  -22,
    14,   -45,  -46,  18,   -29,  -30,  9,    -29,  -29,  -15,  -52,  -50,
    -26,  -29,  -44,  -13,  -9,   -5,   41,   46,   51,   60,   66,   68,
    -11,  -7,   0,    72,   78,   78,   65,   72,   73,   64,   70,   70,
    66,   72,   72,   68,   74,   74,   66,   75,   75,   13,   18,   24,
    24,   25,   30,   14,   19,   25,   36,   40,   47,   38,   43,   47,
    74,   79,   81,   70,   75,   80,   71,   78,   78,   73,   80,   81,
    70,   62,   63,   80,   81,   82,   53,   59,   60,   66,   68,   72,
    26,   36,   38,   -35,  -34,  -25,  -45,  -62,  -61,  11,   -47,  -44,
    35,   -6,   -2,   36,   -5,   0,    19,   -26,  -24,  -23,  -78,  -78,
    -44,  -62,  -73,  -38,  -47,  -47,  45,   51,   59,   58,   62,   67,
    78,   88,   88,   57,   25,   28,   57,   19,   21,   69,   76,   79,
    66,   73,   72,   68,   73,   74,   66,   72,   72,   54,   60,   60,
    45,   50,   54,   46,   50,   54,   45,   47,   52,   45,   50,   54,
    74,   78,   82,   71,   76,   81,   73,   79,   83,   75,   83,   84,
    72,   33,   36,   74,   69,   66,   81,   89,   88,   66,   73,   76,
    -12,  -6,   -2,   -73,  -71,  -67,  -13,  -58,  -59,  -7,   -67,  -66,
    43,   6,    5,    47,   6,    7,    38,   -5,   -4,   14,   -35,  -35,
    -38,  -79,  -83,  -29,  -43,  -48,  -19,  -17,  -10,  36,   44,   49,
    58,   64,   68,   73,   84,   84,   33,   42,   47,   70,   76,   76,
    71,   83,   83,   68,   75,   75,   69,   74,   75,   72,   77,   79,
    65,   71,   73,   66,   71,   74,   62,   68,   69,   75,   81,   81,
    69,   78,   78,   71,   77,   79,   70,   70,   74,   70,   72,   76,
    35,   41,   45,   87,   95,   92,   70,   75,   79,   26,   35,   39,
    -45,  -51,  -48,  -50,  -72,  -72,  -10,  -56,  -63,  9,    -11,  -18,
    66,   27,   30,   67,   28,   31,   56,   19,   21,   33,   -13,  -11,
    -20,  -76,  -80,  -38,  -57,  -54,  -40,  -54,  -58,  5,    10,   16,
    49,   54,   59,   61,   66,   68,   8,    15,   20,   73,   77,   78,
    49,   -31,  -18,  77,   88,   87,   67,   74,   74,   70,   82,   81,
    63,   71,   70,   63,   68,   69,   63,   71,   73,   69,   80,   79,
    70,   77,   76,   77,   75,   76,   59,   4,    9,    75,   85,   83,
    57,   64,   63,   54,   64,   63,   63,   72,   76,   -50,  -53,  -46,
    -35,  -51,  -51,  30,   26,   18,   60,   72,   71,   61,   72,   73,
    62,   24,   28,   54,   12,   14,   50,   6,    9,    31,   -19,  -13,
    -7,   -67,  -66,  -65,  -105, -110, -37,  -50,  -47,  -31,  -45,  -47,
    -4,   3,    7,    41,   46,   52,   65,   70,   71,   63,   70,   70,
    76,   88,   83,   19,   25,   30,   74,   85,   85,   56,   -10,  0,
    67,   62,   65,   79,   89,   89,   60,   37,   40,   66,   25,   29,
    69,   76,   79,   9,    14,   19,   80,   93,   91,   63,   69,   70,
    70,   78,   80,   57,   67,   71,   -58,  -58,  -52,  -21,  -40,  -41,
    59,   64,   60,   60,   71,   72,   52,   41,   44,   48,   19,   21,
    49,   4,    5,    45,   -4,   -2,   42,   -17,  -11,  27,   -21,  -19,
    4,    -45,  -45,  -28,  -74,  -77,  -61,  -105, -105, -36,  -50,  -47,
    -19,  -38,  -35,  6,    9,    13,   28,   30,   37,   41,   46,   50,
    64,   66,   70,   40,   41,   45,   74,   82,   82,   72,   73,   70,
    76,   79,   80,   -20,  -18,  -9,   76,   83,   81,   74,   81,   79,
    76,   82,   81,   37,   41,   41,   68,   74,   77,   66,   72,   75,
    8,    13,   20,   -56,  -59,  -55,  -15,  -33,  -40,  25,   5,    3,
    40,   15,   15,   42,   -1,   0,    46,   1,    4,    49,   1,    6,
    38,   -19,  -18,  34,   -30,  -30,  32,   -17,  -17,  29,   -11,  -11,
    26,   -13,  -16,  5,    -40,  -47,  -28,  -82,  -88,  -64,  -104, -113,
    -51,  -67,  -63,  -13,  -24,  -23,  -3,   -9,   -5,   29,   34,   38,
    22,   30,   35,   45,   52,   57,   45,   50,   54,   38,   43,   47,
    64,   71,   72,   42,   46,   49,   60,   66,   67,   35,   41,   42,
    55,   60,   64,   54,   64,   65,   11,   16,   23,   -34,  -30,  -28,
    -54,  -61,  -66,  -17,  -44,  -51,  15,   -24,  -29,  43,   4,    2,
    43,   11,   9,    41,   6,    11,   41,   2,    5,    43,   -2,   1,
    30,   -31,  -32,  33,   -12,  -13,  45,   13,   11,   48,   18,   17,
    44,   10,   9,    33,   -2,   -5,   6,    -48,  -51,  -34,  -88,  -98,
    -62,  -101, -104, -69,  -91,  -88,  -38,  -53,  -50,  -11,  -24,  -24,
    -7,   -16,  -13,  8,    9,    13,   30,   34,   38,   28,   35,   38,
    25,   31,   35,   21,   26,   32,   20,   25,   31,   18,   22,   28,
    -1,   2,    6,    -45,  -48,  -46,  -53,  -64,  -67,  -43,  -67,  -77,
    9,    -22,  -30,  25,   -15,  -18,  40,   -7,   -7,   39,   -7,   -5,
    33,   -12,  -9,   29,   -10,  -8,   30,   -8,   -4,   32,   -12,  -10,
    44,   7,    10,   53,   12,   12,   59,   17,   19,   61,   22,   21,
    54,   15,   14,   51,   8,    9,    39,   -5,   -7,   7,    -41,  -50,
    -46,  -92,  -102, -62,  -95,  -94,  -85,  -108, -113, -90,  -112, -116,
    -29,  -32,  -44,  3,    -1,   -2,   -35,  -45,  -38,  -25,  -33,  -28,
    -30,  -37,  -32,  -39,  -46,  -46,  -49,  -60,  -62,  -57,  -71,  -74,
    -67,  -87,  -93,  -39,  -65,  -75,  -8,   -36,  -42,  25,   -9,   -13,
    52,   19,   21,   49,   13,   16,   31,   -10,  -7,   35,   -13,  -6,
    38,   -15,  -6,   32,   -20,  -17,  23,   -23,  -22,  24,   -26,  -23,
    63,   25,   22,   66,   30,   30,   64,   25,   28,   62,   22,   28,
    61,   21,   26,   55,   17,   17,   52,   13,   13,   42,   2,    4,
    7,    -42,  -42,  -56,  -99,  -98,  -56,  -84,  -86,  -20,  -45,  -61,
    -39,  -79,  -90,  -57,  -99,  -102, -49,  -96,  -102, -47,  -91,  -94,
    -49,  -87,  -92,  -47,  -86,  -88,  -28,  -70,  -76,  -5,   -41,  -45,
    19,   -9,   -11,  35,   12,   9,    42,   17,   16,   44,   15,   17,
    40,   13,   15,   27,   -10,  -7,   5,    -53,  -51,  3,    -51,  -56,
    6,    -55,  -59,  16,   -41,  -41,  19,   -31,  -29,  14,   -41,  -39};

const int8_t kFilter8x3x3x3[8 * 3 * 3 * 3] = {
    -82, -59,  -112, -88, -69,  -114, -50, -60,  -99, -67, -63,  -100, -75,
    -86, -124, -51,  -46, -94,  -94,  -88, -127, -77, -71, -109, -28,  -41,
    -93, -31,  68,   -34, -54,  104,  -49, -32,  62,  -30, -38,  80,   -41,
    -67, 127,  -63,  -36, 71,   -36,  -12, 26,   -15, -26, 51,   -27,  -6,
    18,  -12,  114,  86,  55,   109,  80,  49,   103, 75,  50,   112,  84,
    55,  108,  79,   50,  102,  72,   48,  127,  95,  64,  124,  91,   61,
    116, 85,   59,   -93, 24,   72,   -98, 25,   76,  -24, 7,    18,   -120,
    34,  87,   -127, 35,  94,   -29,  10,  21,   -49, 21,  27,   -54,  17,
    35,  2,    -3,   1,   44,   73,   10,  38,   68,  17,  -12,  -15,  5,
    -70, -127, -16,  -53, -115, -25,  6,   10,   -11, 24,  55,   2,    19,
    54,  10,   -11,  -16, 2,    97,   114, 86,   99,  113, 87,   105,  123,
    92,  103,  120,  93,  104,  118,  92,  109,  125, 99,  99,   117,  97,
    100, 115,  92,   108, 127,  102,  -48, -78,  -20, -79, -127, -33,  -38,
    -56, -13,  -13,  -9,  -8,   -29,  -24, -10,  -16, -14, -2,   22,   28,
    7,   36,   43,   13,  15,   16,   11,  -31,  -64, -18, -30,  -57,  -15,
    4,   -7,   1,    -78, -127, -32,  -69, -119, -31, -3,  -4,   -2,   -49,
    -61, -15,  -42,  -63, -18,  -2,   8,   0};
const int32_t kBiasQuantized8[8] = {-4166945, 70595,  203077, 315159,
                                    55295,    184082, 75855,  233991};

const int8_t kGoldenOutput1x16x16x8[1 * 16 * 16 * 8] = {
    -128, -21,  -81,  67,  -20,  -109, -29,  4,   -128, -19,  -81,  68,
    -19,  -109, -31,  3,   -128, -19,  -80,  68,  -20,  -109, -32,  2,
    -128, -19,  -80,  68,  -20,  -109, -32,  1,   -128, -19,  -80,  68,
    -19,  -109, -33,  1,   -128, -18,  -80,  69,  -18,  -109, -36,  -1,
    -128, -19,  -86,  68,  -36,  -112, -46,  9,   -128, -20,  -92,  68,
    13,   -115, -47,  20,  -128, -21,  -94,  68,  -19,  -116, -52,  27,
    -128, -20,  -87,  68,  29,   -112, -43,  18,  -128, -17,  -80,  69,
    -36,  -108, -41,  0,   -128, -17,  -78,  69,  -20,  -108, -37,  -3,
    -128, -18,  -77,  68,  -21,  -107, -37,  -3,  -128, -18,  -77,  69,
    -20,  -107, -38,  -4,  -128, -18,  -77,  69,  -22,  -107, -38,  -4,
    -128, -19,  -82,  69,  -18,  -110, -31,  -4,  -128, -20,  -81,  67,
    -19,  -109, -30,  3,   -128, -19,  -81,  68,  -19,  -109, -31,  2,
    -128, -19,  -80,  68,  -20,  -109, -31,  2,   -128, -19,  -80,  68,
    -20,  -109, -33,  1,   -128, -20,  -79,  68,  -19,  -108, -33,  1,
    -128, -19,  -88,  67,  -19,  -113, -34,  0,   -128, -19,  -99,  66,
    -13,  -118, -1,   26,  -128, -19,  -120, 66,  32,   -128, 2,    64,
    -128, -20,  -124, 67,  8,    -128, 13,   76,  -128, -19,  -98,  68,
    1,    -118, -17,  31,  -128, -18,  -89,  67,  -12,  -113, -33,  25,
    -128, -17,  -76,  69,  -22,  -107, -37,  -5,  -128, -17,  -77,  68,
    -22,  -107, -38,  -4,  -128, -17,  -77,  69,  -22,  -107, -38,  -5,
    -128, -18,  -76,  69,  -23,  -107, -38,  -5,  -128, -19,  -82,  68,
    -18,  -110, -31,  -5,  -128, -19,  -81,  68,  -20,  -109, -31,  2,
    -128, -19,  -80,  68,  -21,  -109, -32,  1,   -128, -20,  -79,  67,
    -20,  -109, -32,  1,   -128, -21,  -79,  67,  -20,  -108, -32,  0,
    -128, -20,  -79,  67,  -21,  -108, -33,  -1,  -128, -20,  -86,  67,
    -20,  -113, -12,  -1,  -128, -21,  -93,  66,  -15,  -115, -10,  21,
    -128, -23,  -113, 65,  -16,  -126, 77,   46,  -128, -23,  -120, 65,
    -9,   -128, 74,   71,  -128, -26,  -97,  60,  -26,  -118, -12,  29,
    -128, -27,  -89,  61,  -11,  -114, 1,    28,  -128, -32,  -77,  58,
    -15,  -108, -42,  -2,  -128, -30,  -78,  58,  -16,  -108, -40,  0,
    -128, -26,  -77,  60,  -18,  -108, -40,  -2,  -128, -20,  -77,  66,
    -24,  -107, -41,  -4,  -128, -19,  -82,  68,  -17,  -110, -32,  -5,
    -128, -20,  -81,  67,  -20,  -109, -32,  3,   -128, -20,  -82,  67,
    -14,  -110, -34,  0,   -128, -20,  -90,  67,  -62,  -113, -67,  16,
    -128, -24,  -80,  64,  -25,  -109, -38,  1,   -128, -25,  -80,  61,
    -20,  -110, -39,  2,   -128, -28,  -87,  58,  -10,  -113, -42,  9,
    -128, -29,  -94,  56,  -15,  -116, -13,  29,  -128, -26,  -96,  61,
    37,   -118, 24,   31,  -128, -23,  -94,  62,  30,   -116, 55,   40,
    -128, -33,  -87,  59,  25,   -113, 26,   20,  -128, -41,  -85,  53,
    -8,   -112, -19,  16,  -128, -42,  -83,  52,  -26,  -111, -42,  9,
    -128, -40,  -82,  52,  -20,  -111, -38,  8,   -128, -38,  -82,  52,
    -18,  -111, -38,  7,   -128, -29,  -81,  54,  -22,  -110, -42,  9,
    -128, -19,  -81,  68,  -18,  -110, -32,  -7,  -128, -31,  -80,  58,
    -17,  -109, -33,  1,   -128, -24,  -107, 60,  0,    -123, -46,  32,
    -128, -23,  -121, 66,  -27,  -128, 24,   76,  -128, -34,  -92,  57,
    -9,   -115, -39,  32,  -128, -35,  -86,  53,  -16,  -113, -29,  10,
    -128, -30,  -97,  58,  -25,  -117, -17,  24,  -128, -27,  -97,  62,
    -46,  -117, -15,  28,  -128, -26,  -91,  66,  -4,   -114, -44,  23,
    -128, -24,  -87,  68,  15,   -112, -71,  17,  -128, -25,  -85,  66,
    10,   -111, -71,  13,  -128, -29,  -83,  63,  -38,  -110, -39,  5,
    -128, -36,  -82,  59,  -88,  -110, -14,  -2,  -128, -41,  -90,  52,
    25,   -115, -9,   19,  -128, -43,  -94,  47,  1,    -117, -21,  34,
    -128, -32,  -84,  52,  -13,  -111, -16,  15,  -128, -24,  -83,  63,
    -20,  -111, -39,  -1,  -128, -38,  -85,  54,  -23,  -112, -36,  6,
    -128, -32,  -116, 54,  -12,  -128, 61,   50,  -128, -24,  -127, 63,
    -12,  -128, 68,   87,  -128, -24,  -106, 63,  -47,  -122, -6,   44,
    -128, -22,  -102, 63,  8,    -120, -17,  38,  -128, -20,  -99,  66,
    6,    -118, 30,   38,  -128, -20,  -90,  66,  -8,   -114, 35,   21,
    -128, -20,  -89,  66,  -16,  -114, 18,   17,  -128, -20,  -89,  65,
    -15,  -113, 14,   25,  -128, -18,  -84,  66,  -14,  -111, 22,   9,
    -128, -18,  -89,  66,  -4,   -113, 8,    15,  -128, -20,  -89,  66,
    -16,  -113, -27,  20,  -128, -27,  -90,  65,  9,    -114, -30,  14,
    -128, -37,  -90,  56,  -63,  -114, -5,   20,  -128, -43,  -84,  50,
    -21,  -112, -32,  8,   -128, -38,  -89,  49,  -17,  -114, -24,  16,
    -128, -41,  -96,  49,  -13,  -118, -21,  31,  -128, -37,  -101, 53,
    -33,  -120, 23,   28,  -128, -33,  -116, 54,  -24,  -127, 56,   61,
    -128, -22,  -108, 63,  -15,  -123, 45,   53,  -128, -20,  -89,  66,
    -18,  -113, 33,   24,  -128, -20,  -76,  67,  -31,  -107, -9,   0,
    -128, -25,  -72,  65,  2,    -105, -45,  -12, -128, -26,  -76,  64,
    -22,  -107, -35,  -6,  -128, -28,  -79,  62,  -19,  -108, -26,  11,
    -128, -25,  -70,  65,  -9,   -104, -57,  -14, -128, -24,  -71,  64,
    -22,  -105, -30,  -14, -128, -20,  -78,  66,  -30,  -108, 1,    -4,
    -128, -19,  -88,  66,  -9,   -113, 8,    10,  -128, -25,  -96,  65,
    -11,  -117, -5,   27,  -128, -43,  -96,  53,  -6,   -118, -26,  31,
    -128, -41,  -97,  48,  -13,  -118, -13,  34,  -128, -46,  -106, 44,
    -8,   -123, 9,    46,  -128, -38,  -109, 48,  -17,  -124, -10,  45,
    -128, -26,  -110, 59,  -12,  -124, 28,   57,  -128, -20,  -89,  68,
    -22,  -113, 15,   23,  -128, -21,  -74,  69,  -14,  -106, -41,  -4,
    -128, -24,  -68,  63,  -2,   -103, -53,  -12, -128, -23,  -73,  67,
    -9,   -105, -54,  -12, -128, -22,  -76,  67,  -12,  -107, -45,  -13,
    -128, -21,  -84,  67,  -44,  -110, -51,  21,  -128, -23,  -69,  67,
    -21,  -104, -40,  -21, -128, -21,  -68,  67,  -16,  -103, -64,  -13,
    -128, -25,  -64,  65,  -14,  -101, -54,  -21, -128, -18,  -70,  69,
    -18,  -104, -28,  -19, -128, -20,  -86,  66,  -19,  -112, 5,    5,
    -128, -31,  -102, 64,  -16,  -120, 16,   34,  -128, -41,  -104, 49,
    -25,  -121, 5,    49,  -128, -42,  -99,  48,  -43,  -120, 33,   24,
    -128, -36,  -116, 51,  23,   -128, 52,   69,  -128, -24,  -98,  63,
    -31,  -118, 31,   43,  -128, -20,  -76,  69,  -28,  -106, -40,  -5,
    -128, -24,  -71,  64,  -13,  -105, -45,  -7,  -128, -17,  -67,  70,
    -22,  -102, -59,  -20, -128, -23,  -72,  67,  -21,  -105, -38,  -13,
    -128, -21,  -77,  67,  -23,  -108, -54,  -9,  -128, -21,  -86,  67,
    -51,  -112, 15,   22,  -128, -23,  -71,  67,  -32,  -105, -67,  -17,
    -128, -22,  -67,  66,  -15,  -102, -46,  -13, -128, -20,  -60,  69,
    -21,  -99,  -60,  -30, -128, -23,  -61,  65,  0,    -100, -61,  -25,
    -128, -18,  -70,  69,  -35,  -104, -31,  -22, -128, -25,  -94,  67,
    -21,  -116, 15,   5,   -128, -31,  -103, 59,  -47,  -120, 24,   42,
    -128, -41,  -92,  50,  -21,  -116, -12,  21,  -128, -30,  -99,  53,
    -25,  -119, 30,   28,  -128, -22,  -88,  66,  -29,  -113, -13,  23,
    -128, -20,  -76,  69,  -11,  -107, -37,  -6,  -128, -20,  -72,  67,
    -30,  -105, -55,  -9,  -128, -19,  -68,  69,  -22,  -103, -55,  -18,
    -128, -19,  -69,  69,  -20,  -103, -51,  -19, -128, -23,  -77,  65,
    -48,  -108, -21,  -13, -128, -21,  -90,  65,  23,   -114, -22,  36,
    -128, -23,  -69,  66,  -31,  -104, -30,  -13, -128, -18,  -60,  69,
    -27,  -98,  -65,  -25, -128, -20,  -50,  69,  -36,  -94,  -73,  -47,
    -128, -20,  -52,  68,  -39,  -95,  -74,  -46, -128, -19,  -60,  69,
    -44,  -99,  -62,  -36, -128, -22,  -86,  69,  -22,  -113, -24,  -15,
    -128, -34,  -100, 60,  -27,  -119, 3,    39,  -128, -45,  -98,  48,
    -13,  -119, -5,   31,  -128, -32,  -101, 52,  1,    -120, -17,  34,
    -128, -20,  -90,  66,  -23,  -113, -28,  27,  -128, -22,  -75,  69,
    -21,  -107, -52,  -13, -128, -23,  -74,  64,  -45,  -105, -42,  -7,
    -128, -18,  -68,  69,  -21,  -103, -54,  -18, -128, -19,  -67,  69,
    -23,  -103, -53,  -19, -128, -19,  -75,  69,  -23,  -107, -54,  -13,
    -128, -20,  -85,  67,  -29,  -111, -1,   20,  -128, -19,  -68,  69,
    -36,  -103, -68,  -18, -128, -20,  -63,  69,  -23,  -100, -70,  -23,
    -128, -19,  -59,  69,  -6,   -98,  -99,  -31, -128, -20,  -61,  68,
    -15,  -100, -88,  -31, -128, -19,  -67,  69,  -20,  -102, -73,  -23,
    -128, -22,  -92,  69,  -19,  -115, -38,  1,   -128, -37,  -102, 57,
    -17,  -120, -2,   45,  -128, -42,  -91,  50,  -29,  -116, 10,   20,
    -128, -37,  -104, 50,  -2,   -122, 18,   37,  -128, -23,  -101, 61,
    -5,   -119, -13,  48,  -128, -21,  -78,  70,  -33,  -107, -46,  -3,
    -128, -26,  -73,  65,  -31,  -106, -37,  -11, -128, -22,  -69,  66,
    -13,  -103, -55,  -12, -128, -19,  -66,  69,  -21,  -102, -55,  -21,
    -128, -20,  -72,  69,  -28,  -105, -25,  -15, -128, -22,  -75,  69,
    -29,  -106, -16,  -6,  -128, -21,  -70,  69,  -19,  -104, -35,  -9,
    -128, -21,  -65,  69,  -23,  -101, -60,  -23, -128, -23,  -68,  68,
    -25,  -103, -61,  -23, -128, -24,  -68,  63,  -12,  -103, -55,  -15,
    -128, -20,  -78,  69,  -33,  -108, -55,  -18, -128, -25,  -107, 66,
    -2,   -123, -1,   43,  -128, -37,  -102, 51,  -10,  -120, 17,   46,
    -128, -40,  -81,  52,  -33,  -110, -21,  3,   -128, -43,  -96,  50,
    -27,  -118, 7,    17,  -128, -32,  -112, 53,  -21,  -125, 15,   57,
    -128, -23,  -94,  66,  -11,  -115, -42,  34,  -128, -20,  -76,  70,
    -27,  -107, -58,  -8,  -128, -24,  -72,  68,  -10,  -105, -46,  -9,
    -128, -24,  -70,  64,  0,    -104, -62,  -8,  -128, -21,  -69,  67,
    -33,  -103, -66,  -18, -128, -20,  -68,  69,  -18,  -103, -54,  -19,
    -128, -20,  -69,  67,  -34,  -103, -65,  -16, -128, -23,  -69,  67,
    -34,  -104, -64,  -17, -128, -23,  -69,  64,  -3,   -103, -48,  -15,
    -128, -18,  -74,  69,  -12,  -106, -65,  -16, -128, -22,  -93,  67,
    -18,  -116, -31,  20,  -128, -26,  -90,  62,  -16,  -114, 49,   23,
    -128, -24,  -86,  62,  -74,  -112, 2,    0,   -128, -44,  -87,  49,
    -18,  -113, -34,  14,  -128, -45,  -93,  49,  -11,  -117, -3,   22,
    -128, -40,  -110, 48,  -18,  -125, 49,   48,  -128, -30,  -113, 57,
    -7,   -125, 5,    61,  -128, -23,  -93,  66,  -13,  -115, -47,  29,
    -128, -20,  -77,  69,  -14,  -107, -67,  0,   -128, -20,  -73,  69,
    -27,  -105, -48,  -8,  -128, -26,  -70,  64,  -47,  -104, -43,  -14,
    -128, -22,  -72,  68,  7,    -105, -52,  -6,  -128, -24,  -70,  64,
    -50,  -104, -46,  -15, -128, -19,  -73,  69,  -30,  -105, -41,  -12,
    -128, -19,  -81,  70,  -40,  -109, -85,  -9,  -128, -22,  -97,  67,
    21,   -118, -45,  27,  -128, -29,  -93,  59,  -37,  -115, 9,    31,
    -128, -31,  -81,  59,  3,    -110, -54,  5,   -128, -35,  -87,  55,
    -9,   -113, -29,  10,  -128, -44,  -88,  47,  -11,  -114, 1,    21,
    -128, -38,  -85,  52,  -29,  -112, -4,   10,  -128, -39,  -92,  50,
    -27,  -116, 18,   16,  -128, -39,  -112, 46,  -14,  -126, 57,   50,
    -128, -31,  -117, 57,  9,    -128, 4,    67,  -128, -25,  -105, 63,
    -17,  -121, -46,  47,  -128, -22,  -91,  67,  -2,   -114, -50,  20,
    -128, -21,  -88,  68,  -35,  -112, -63,  12,  -128, -21,  -89,  69,
    -25,  -113, -70,  12,  -128, -21,  -94,  68,  -30,  -116, -71,  18,
    -128, -22,  -99,  66,  6,    -119, -57,  32,  -128, -26,  -101, 60,
    24,   -120, 19,   44,  -128, -35,  -94,  54,  -27,  -117, 28,   27,
    -128, -41,  -89,  50,  -24,  -114, -16,  19,  -128, -41,  -89,  52,
    -12,  -114, -24,  19,  -128, -37,  -91,  52,  -17,  -115, -19,  21,
    -128, -37,  -85,  55,  -39,  -112, -37,  8,   -128, -38,  -84,  55,
    -32,  -111, -42,  8,   -128, -37,  -86,  54,  -31,  -113, -32,  10,
    -128, -37,  -95,  52,  -36,  -117, 10,   19,  -128, -35,  -110, 52,
    -10,  -125, 71,   51,  -128, -28,  -110, 56,  -21,  -124, 71,   55,
    -128, -29,  -105, 57,  37,   -122, 21,   48,  -128, -32,  -106, 57,
    30,   -123, 32,   52,  -128, -30,  -107, 58,  25,   -123, 38,   51,
    -128, -30,  -105, 57,  -9,   -122, 49,   45,  -128, -30,  -98,  57,
    -59,  -118, 33,   30,  -128, -33,  -90,  57,  -46,  -114, -13,  18,
    -128, -37,  -90,  56,  -15,  -115, -27,  16,  -128, -42,  -96,  50,
    12,   -118, -6,   33,  -128, -43,  -95,  49,  7,    -117, -4,   32,
    -128, -37,  -95,  53,  -7,   -117, -1,   30};

}  // namespace conv
}  // namespace testing
}  // namespace tflite
