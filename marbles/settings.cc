// Copyright 2015 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Settings storage.

#include "marbles/settings.h"

#include <algorithm>

#include "stmlib/system/storage.h"

namespace marbles {

using namespace std;

const Scale preset_scales[6] = {
  // C major
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 },  // C
      { 0.0833f, 16 },   // C#
      { 0.1667f, 96 },   // D
      { 0.2500f, 24 },   // D#
      { 0.3333f, 128 },  // E
      { 0.4167f, 64 },   // F
      { 0.5000f, 8 },    // F#
      { 0.5833f, 192 },  // G
      { 0.6667f, 16 },   // G#
      { 0.7500f, 96 },   // A
      { 0.8333f, 24 },   // A#
      { 0.9167f, 128 },  // B
    }
  },
  
  // C minor
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 },  // C
      { 0.0833f, 16 },   // C#
      { 0.1667f, 96 },   // D
      { 0.2500f, 128 },  // Eb
      { 0.3333f, 8 },    // E
      { 0.4167f, 64 },   // F
      { 0.5000f, 4 },    // F#
      { 0.5833f, 192 },  // G
      { 0.6667f, 96 },   // G#
      { 0.7500f, 16 },   // A
      { 0.8333f, 128 },  // Bb
      { 0.9167f, 16 },   // B
    }
  },
  
  // C Harmonic minor
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 },  // C
      { 0.0833f, 4 },    // C#
      { 0.1667f, 96 },   // D
      { 0.2500f, 192 },    // Eb
      { 0.3333f, 4 },    // E
      { 0.4167f, 64 },  // F
      { 0.5000f, 32 },    // F#
      { 0.5833f, 192 },  // G
      { 0.6667f, 128 },    // G#
      { 0.7500f, 4 },   // A
      { 0.8333f, 16 },    // Bb
      { 0.9167f, 192 },    // B
    }
  },

  // C, Eb, F#, A Diminished whole half
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 }, // C
      { 0.0833f, 0 }, // C#
      { 0.1667f, 255 }, // D
      { 0.2500f, 192 }, // Eb
      { 0.3333f, 0 }, // E
      { 0.4167f, 192 }, // F
      { 0.5000f, 255 }, // F#
      { 0.5833f, 0 }, // G
      { 0.6667f, 255 }, // G#
      { 0.7500f, 192 }, // A
      { 0.8333f, 0 }, // Bb
      { 0.9167f, 255 }, // B
    }
  },

  // Dorian
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 }, // C
      { 0.0833f, 4 },   // C#
      { 0.1667f, 192 }, // D
      { 0.22500, 224 },  // Eb
      { 0.3333f, 4 },   // E
      { 0.4167f, 160 }, // F
      { 0.5000f, 8 },   // F#
      { 0.5833f, 224 }, // G
      { 0.6667f, 4 },   // G#
      { 0.7500f, 224 },  // A
      { 0.8333f, 192 }, // Bb
      { 0.9167f, 4 },   // B
    }
  },
  
  // Raag Shri
  {
    1.0f,
    12,
    {
      { 0.0000f, 255 }, // ** Sa
      { 0.0752f, 4 },   //    Komal Re
      { 0.1699f, 128 }, // ** Re
      { 0.2630f, 64 },  // ** Komal Ga
      { 0.3219f, 4 },   //    Ga
      { 0.4150f, 128 }, // ** Ma
      { 0.4918f, 4 },   //    Tivre Ma
      { 0.5850f, 192 }, // ** Pa
      { 0.6601f, 4 },   //    Komal Dha
      { 0.7549f, 64 },  // ** Dha
      { 0.8479f, 128 }, // ** Komal Ni
      { 0.9069f, 4 },   //    Ni
    }
  }
};

#define FIX_OUTLIER(destination, expected_value) if (fabsf(destination / expected_value - 1.0f) > 0.1f) { destination = expected_value; }

void Settings::ResetScale(int i) {
  persistent_data_.scale[i] = preset_scales[i];
}


void Settings::Init() {
  freshly_baked_ = false;
  
  // Set default values for all calibration and state settings.
  // This settings will be written to flash memory the first time the module
  // is powered on, or if corrupted data is found in the flash sector,
  // following a major firmware upgrade.
  CalibrationData& c = persistent_data_.calibration_data;
  fill(&c.adc_scale[0], &c.adc_scale[ADC_CHANNEL_LAST], -2.0f);
  fill(&c.adc_offset[0], &c.adc_offset[ADC_CHANNEL_LAST], +1.0f);
  fill(&c.dac_scale[0], &c.dac_scale[DAC_CHANNEL_LAST], -6212.8f);
  fill(&c.dac_offset[0], &c.dac_offset[DAC_CHANNEL_LAST], 32768.0f);
  c.adc_offset[ADC_CHANNEL_T_RATE] = 60.0f;
  c.adc_scale[ADC_CHANNEL_T_RATE] = -120.0f;

  // calibration data
  c.dac_scale[0] = -6492.5022f; c.dac_offset[0] = 33325.3813f;
  c.dac_scale[1] = -6169.5050f; c.dac_offset[1] = 32394.4365f;
  c.dac_scale[2] = -6214.8645f; c.dac_offset[2] = 32456.6353f;
  c.dac_scale[3] = -6204.9336f; c.dac_offset[3] = 32380.1917f;
  
  for (size_t i = 0; i < kNumScales; ++i) {
    ResetScale(i);
  }

  state_.t_deja_vu = 0;
  state_.t_model = 0;
  state_.t_range = 1;
  state_.t_pulse_width_mean = 128;
  state_.t_pulse_width_std = 0;

  state_.x_deja_vu = 0;
  state_.x_control_mode = 0;
  state_.x_register_mode = 0;
  state_.x_range = 2;
  state_.x_scale = 0;
  
  state_.y_spread = 128;
  state_.y_bias = 128;
  state_.y_steps = 0;
  state_.y_divider = 128;
  state_.y_range = 2;
  
  state_.color_blind = 0;
  state_.explicit_reset = 0;
  
  freshly_baked_ = !chunk_storage_.Init(&persistent_data_, &state_);
  
  if (!freshly_baked_) {
    CONSTRAIN(state_.t_model, 0, 5);
    CONSTRAIN(state_.t_range, 0, 2);
    CONSTRAIN(state_.x_control_mode, 0, 2);
    CONSTRAIN(state_.x_range, 0, 2);
    CONSTRAIN(state_.x_scale, 0, 5);
    CONSTRAIN(state_.y_range, 0, 2);
    CONSTRAIN(state_.t_deja_vu, DEJA_VU_OFF, DEJA_VU_LOCKED);
    CONSTRAIN(state_.x_deja_vu, DEJA_VU_OFF, DEJA_VU_LOCKED);
    
    CalibrationData& c = persistent_data_.calibration_data;
    for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
      if (i == ADC_CHANNEL_T_RATE) {
        FIX_OUTLIER(c.adc_scale[i], -120.0f);
        FIX_OUTLIER(c.adc_offset[i], 60.0f);
      } else {
        FIX_OUTLIER(c.adc_scale[i], -2.0f);
        FIX_OUTLIER(c.adc_offset[i], +1.0f);
      }
    }
    for (size_t i = 0; i < DAC_CHANNEL_LAST; ++i) {
      FIX_OUTLIER(c.dac_scale[i], -6212.8f);
      FIX_OUTLIER(c.dac_offset[i], 32768.0f);
    }
  }
}

void Settings::SavePersistentData() {
  chunk_storage_.SavePersistentData();
}

void Settings::SaveState() {
  chunk_storage_.SaveState();
}

/* static */
void Settings::ProgramOptionBytes() {
  FLASH_Unlock();
  FLASH_OB_Unlock();
  FLASH_OB_BORConfig(OB_BOR_OFF);
  FLASH_OB_Launch();
  FLASH_OB_Lock();
}

}  // namespace marbles
