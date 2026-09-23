#include "RCOutput.h"
#include <AP_Math/AP_Math.h>
#include <cstdint>

using namespace Embox;


void RCOutput::init() {
    n = 0;
    ngroups = 0;

    for (int i = 0; i < pwm_dev_num(); i++) {
        struct pwm_device *pwm_dev;
	    const struct pwm_desc *pwmd_desc;

        pwm_dev = pwm_dev_by_idx(i);
	    pwmd_desc = pwm_dev->pwmd_desc;

        struct pwm_group &group = groups[ngroups];
        group.dev = pwm_dev;
        group.freq = 50;
        group.mode = MODE_PWM_NORMAL;

	    for (int j = 0; j < pwm_dev_max_chan(pwm_dev); j++) {
		    if (pwmd_desc->pwmd_avail_chan_mask & (1 << j)) {
                channels[n].value = 0;
                channels[n].group = &group;
                channels[n].local_idx = j;
                n++;
		    }
        }
        pwm_set_frequency(pwm_dev, 50);

        ngroups++;
	}
}


void RCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {
    for (uint8_t chan = 0; chan < n; chan++) {
        if ((1U << chan) & chmask) {
            if (channels[chan].group->freq != freq_hz) {
                channels[chan].group->freq = freq_hz;
                pwm_set_frequency(channels[chan].group->dev, freq_hz);
            }
        }
    }
}

uint16_t RCOutput::get_freq(uint8_t chan) {
    if (chan < n) {
        return channels[chan].group->freq;
    }
    return 0;
}

void RCOutput::enable_ch(uint8_t chan)
{
    if (chan < n) {
        pwm_enable(channels[chan].group->dev, channels[chan].local_idx);
    }
}

void RCOutput::disable_ch(uint8_t chan)
{
    if (chan < n) {
        pwm_disable(channels[chan].group->dev, channels[chan].local_idx);
    }
}

void RCOutput::write(uint8_t chan, uint16_t period_us)
{
    if (chan >= n) {
        return;
    }

    channels[chan].value = period_us;
    if (_corked) {
        _pending_mask |= (1U << chan);
    } else if (is_dshot_protocol(channels[chan].group->mode)) {
        dshot_write(chan, period_us);
    } else {
        pwm_set_duty(channels[chan].group->dev, channels[chan].local_idx, channels[chan].value * NSEC_PER_USEC);
    }
}

// mode is switched per group: a pwm_device is one hardware timer with a
// single period, so it cannot run PWM and DShot on different channels at
// once. If chmask only covers part of a group, the whole group still
// switches, matching the hardware constraint.
void RCOutput::set_output_mode(uint32_t chmask, enum output_mode mode)
{
    for (uint8_t chan = 0; chan < n; chan++) {
        if ((1U << chan) & chmask) {
            channels[chan].group->mode = mode;
        }
    }
}

enum AP_HAL::RCOutput::output_mode RCOutput::get_output_mode(uint32_t& chmask)
{
    if (n == 0) {
        chmask = 0;
        return MODE_PWM_NORMAL;
    }

    enum output_mode mode = channels[0].group->mode;
    chmask = 0;
    for (uint8_t chan = 0; chan < n; chan++) {
        if (channels[chan].group->mode == mode) {
            chmask |= (1U << chan);
        }
    }
    return mode;
}

uint16_t RCOutput::read(uint8_t chan)
{
    if (chan < n) {
        return channels[chan].value;
    }
    return 900;
}

void RCOutput::read(uint16_t* period_us, uint8_t len)
{
    len = MIN(len, n);
    for (uint8_t i = 0; i < len; i++) {
        period_us[i] = channels[i].value;
    }
}

void RCOutput::cork(void)
{
    _corked = true;
}

void RCOutput::push(void)
{
    if (!_corked) {
        return;
    }
    for (uint8_t chan = 0; chan < n; chan++) {
        if ((1U << chan) & _pending_mask) {
            if (is_dshot_protocol(channels[chan].group->mode)) {
                dshot_write(chan, channels[chan].value);
            } else {
                pwm_set_duty(channels[chan].group->dev, channels[chan].local_idx, channels[chan].value * NSEC_PER_USEC);
            }
        }
    }
    _pending_mask = 0;
    _corked = false;
}
