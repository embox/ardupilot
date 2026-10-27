/*
    RC input pass trough to RC output
    Max RC channels 14
    Max update rate 10 Hz
    Attention: If your board has safety switch,
    don't forget to push it to enable the PWM output.
*/

#include <AP_HAL/AP_HAL.h>

// we need a boardconfig created so that the io processor's enable
// parameter is available
#if CONFIG_HAL_BOARD == HAL_BOARD_CHIBIOS
#include <AP_BoardConfig/AP_BoardConfig.h>
#include <AP_IOMCU/AP_IOMCU.h>
AP_BoardConfig BoardConfig;
#endif
#if CONFIG_HAL_BOARD == HAL_BOARD_EMBOX
#include <AP_SerialManager/AP_SerialManager.h>

static AP_SerialManager serial_manager;
#endif


void setup();
void loop();

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

#define MAX_CHANNELS 14

static uint8_t max_channels = 0;
static uint16_t last_value[MAX_CHANNELS];

void setup(void)
{
    hal.console->printf("Starting RCInputToRCOutput test\n");

#if CONFIG_HAL_BOARD == HAL_BOARD_CHIBIOS
    BoardConfig.init();
#endif
#if CONFIG_HAL_BOARD == HAL_BOARD_EMBOX
    // Use SERIAL1 for RC input; the RC protocol decoder selects the UART baud rate.
    serial_manager.set_protocol_and_baud(1, AP_SerialManager::SerialProtocol_RCIN, 416666);
    serial_manager.init();
#endif

    hal.rcout->set_output_mode(0x3FFF, AP_HAL::RCOutput::MODE_PWM_DSHOT600);
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        hal.rcout->enable_ch(i);
    }
}

void loop(void)
{
    bool changed = false;
    uint8_t nchannels = hal.rcin->num_channels();

    if (nchannels > MAX_CHANNELS) {
       nchannels = MAX_CHANNELS;
    }

    for (uint8_t i = 0; i < nchannels; i++) {
        uint16_t v = hal.rcin->read(i);
        if (last_value[i] != v) {
            hal.rcout->write(i, v);
            changed = true;
            last_value[i] = v;
        }
        if (i > max_channels) {
            max_channels = i;
        }
    }
    if (changed) {
        for (uint8_t i = 0; i < max_channels; i++) {
            hal.console->printf("%2u:%04u ", (unsigned)(i + 1), (unsigned)last_value[i]);
        }
        hal.console->printf("\n");
    }
    hal.scheduler->delay(100);
}

AP_HAL_MAIN();
