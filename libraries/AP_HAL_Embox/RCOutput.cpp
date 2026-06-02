#include "RCOutput.h"
#include "AP_Common/AP_Common.h"
#include "AP_GPS/AP_GPS_GSOF.h"
#include <AP_Math/AP_Math.h>
#include <cstdint>

using namespace Embox;


void RCOutput::init() {
    n = 0;
	
    for (int i = 0; i < pwm_dev_num(); i++) {
        struct pwm_device *pwm_dev;
	    const struct pwm_desc *pwmd_desc;
        
        pwm_dev = pwm_dev_by_idx(i);
	    pwmd_desc = pwm_dev->pwmd_desc;

	    for (int j = 0; j < pwm_dev_max_chan(pwm_dev); j++) {
		    if (pwmd_desc->pwmd_avail_chan_mask & (1 << j)) {
                channels[n].dev = pwm_dev;
                channels[n].ch = j;
                channels[n].value = 0;
                channels[n].freq = 50;
                n++;
		    }
        }
        pwm_set_frequency(pwm_dev, 50);  
	}
}

void RCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {
    for (uint8_t i = 0; i < n; i++) {
        if ((1U << i) & chmask) {
            pwm_set_frequency(channels[i].dev, freq_hz);
            channels[i].freq = freq_hz;
        }
    }
}

uint16_t RCOutput::get_freq(uint8_t chan) {
    return channels[chan].freq;
}

void RCOutput::enable_ch(uint8_t chan)
{
    if (chan < n) {
        pwm_enable(channels[chan].dev, channels[chan].ch);
    }
}

void RCOutput::disable_ch(uint8_t chan)
{
    if (chan < n) {
        pwm_disable(channels[chan].dev, channels[chan].ch);
    }
}

void RCOutput::write(uint8_t chan, uint16_t period_us)
{
    if (chan < n) {
        channels[chan].value = period_us * NSEC_PER_USEC;
        if (_corked) {
            _pending_mask |= (1U << chan);
        } else { 
            pwm_set_duty(channels[chan].dev, channels[chan].ch, channels[chan].value);
        }
    }
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
    for (uint8_t i = 0; i < n; i++) {
        period_us[i] = channels[i].value / NSEC_PER_USEC;
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
    for (uint8_t i = 0; i < n; i++) {
        if ((1U << i) & _pending_mask) {
            pwm_set_duty(channels[i].dev, channels[i].ch, channels[i].value);
        }
    }
    _pending_mask = 0;
    _corked = false;
}

