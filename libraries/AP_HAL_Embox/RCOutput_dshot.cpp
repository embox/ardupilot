/*
  DShot support for AP_HAL_Embox.
*/
#include "RCOutput.h"
#include <AP_Math/AP_Math.h>
#include <AP_HAL/AP_HAL.h>
#include <cstdint>
#include <cstdio>

using namespace Embox;

/*
  build a 16 bit dshot packet from an 11 bit throttle/command value:
  11 bits value, 1 bit telemetry request, 4 bit checksum
*/
uint16_t RCOutput::create_dshot_packet(uint16_t value, bool telem_request)
{
    uint16_t packet = (value << 1);

    if (telem_request) {
        packet |= 1;
    }

    uint16_t csum = 0;
    uint16_t csum_data = packet;
    for (uint8_t i = 0; i < 3; i++) {
        csum ^= csum_data;
        csum_data >>= 4;
    }
    csum &= 0xf;

    return (packet << 4) | csum;
}

/*
  encode period_us (as passed to write(), 1000-2000 like a normal PWM pulse)
  into a dshot packet and log it. No hardware transmission yet.
*/
void RCOutput::dshot_write(uint8_t chan, uint16_t period_us)
{
    uint16_t value = 0;

    if (period_us != 0) {
        uint16_t pwm = constrain_int16(period_us, 1000, 2000);
        value = MIN(2 * (pwm - 1000), 1999);
        // dshot values are from 48 to 2047, 0 means off
        value += DSHOT_ZERO_THROTTLE;
    }

    const uint16_t packet = create_dshot_packet(value, false);

    char bits[17];
    for (int8_t b = 15; b >= 0; b--) {
        bits[15 - b] = ((packet >> b) & 1) ? '1' : '0';
    }
    bits[16] = '\0';

    printf("[DSHOT] ch=%u period_us=%u value=%u packet=0x%04x bits=%s\n",
           chan, period_us, value, packet, bits);
}
