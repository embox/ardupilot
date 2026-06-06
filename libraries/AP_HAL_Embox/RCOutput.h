#pragma once

#include "Util.h"
#include "AP_HAL_Embox.h"
#include <cstdint>
#include <drivers/pwm.h>

namespace Embox {

    struct pwm_channel {
        struct pwm_device* dev;
        uint32_t value;
        uint16_t freq;
        uint8_t ch;
    };

    class RCOutput : public AP_HAL::RCOutput {
        friend class Embox::Util;

        void init() override;
        void set_freq(uint32_t chmask, uint16_t freq_hz) override;
        uint16_t get_freq(uint8_t ch) override;
        void enable_ch(uint8_t ch) override;
        void disable_ch(uint8_t ch) override;
        void write(uint8_t ch, uint16_t period_us) override;
        uint16_t read(uint8_t ch) override;
        void read(uint16_t* period_us, uint8_t len) override;
        void cork(void) override;
        void push(void) override;

    private:
        struct pwm_device* dev;
        struct pwm_channel channels[16];
        uint8_t n;
        uint16_t _pending_mask = 0;
        bool _corked = false;
        bool safety_on = true;
    };
} // namespace Embox