// Copyright (c) 2025 HIGH CODE LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SUBGHZ_SPECTRUM_H
#define SUBGHZ_SPECTRUM_H

#include <stdint.h>
#include <stdbool.h>

#define SPECTRUM_SAMPLES    80

typedef struct {
    uint32_t center_freq;
    uint32_t span_hz;
    uint32_t start_freq;
    uint32_t step_hz;
    float dbm_values[SPECTRUM_SAMPLES];
    uint64_t timestamp;
} subghz_spectrum_line_t;

void subghz_spectrum_start(uint32_t center_freq, uint32_t span_hz);
void subghz_spectrum_stop(void);
bool subghz_spectrum_get_line(subghz_spectrum_line_t* out_line);

#endif // SUBGHZ_SPECTRUM_H

