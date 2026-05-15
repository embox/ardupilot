#include "RCOutput.h"
#include <AP_Math/AP_Math.h>

using namespace Embox;

void RCOutput::init() {
    dev = pwm_dev_by_id(0); 
}

void RCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {
    pwm_set_period(dev, USEC_PER_SEC / freq_hz);
    frec = freq_hz;
}

uint16_t RCOutput::get_freq(uint8_t chan) {
    return frec;
}

void RCOutput::enable_ch(uint8_t chan)
{
    pwm_enable(dev, 1 << chan);
}

void RCOutput::disable_ch(uint8_t chan)
{
    pwm_disable(dev, 1 << chan);
}

void RCOutput::write(uint8_t chan, uint16_t period_us)
{
    if (chan < ARRAY_SIZE(value)) {
        value[chan] = period_us;
        if (_corked) {
            _pending_mask |= (1U << chan);
        } else { 
            pwm_set_duty(dev, chan, period_us);
        }
    }
}

uint16_t RCOutput::read(uint8_t chan)
{
    if (chan < ARRAY_SIZE(value)) {
        return value[chan];
    }
    return 900;
}

void RCOutput::read(uint16_t* period_us, uint8_t len)
{
    len = MIN(len, ARRAY_SIZE(value));
    memcpy(period_us, value, len*sizeof(value[0]));
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
    for (uint8_t i=0; i<4; i++) {
        if ((1U<<i) & _pending_mask) {
            pwm_set_duty(dev, i, value[i]);
        }
    }
    _pending_mask = 0;
    _corked = false;
}

