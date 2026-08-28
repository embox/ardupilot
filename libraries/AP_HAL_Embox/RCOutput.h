#pragma once

#include "Util.h"
#include "AP_HAL_Embox.h"
#include <cstdint>
#include <drivers/pwm.h>

namespace Embox {

    #define GROUPS_NUM 4
    #define CHANNELS_NUM 16

    struct pwm_group {
        struct pwm_device* dev;
        uint16_t freq;
        AP_HAL::RCOutput::output_mode mode;
    };

    struct pwm_channel {
        struct pwm_group* group;
        uint8_t local_idx;         // index within the group, 0..3
        uint16_t value;             // per-channel pulse width, us
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
        void set_output_mode(uint32_t chmask, enum output_mode mode) override;
        enum output_mode get_output_mode(uint32_t& chmask) override;

    private:
        struct pwm_group groups[GROUPS_NUM];
        struct pwm_channel channels[CHANNELS_NUM];
        uint8_t ngroups;
        uint8_t n;  // total channel count across all groups

        uint16_t _pending_mask = 0;
        bool _corked = false;
        bool safety_on = true;

        // DShot support (RCOutput_dshot.cpp). No real hardware backend yet in
        // Embox, so dshot_write() logs the encoded frame instead of sending it.
        static uint16_t create_dshot_packet(uint16_t value, bool telem_request);
        void dshot_write(uint8_t chan, uint16_t period_us);
    };
} // namespace Embox
