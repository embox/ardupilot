#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <AP_HAL/AP_HAL.h>
#include <drivers/gpio.h>

#include "GPIO.h"
#include "Util.h"

extern const AP_HAL::HAL &hal;

using namespace Embox;

struct gpio_entry {
    uint8_t pin_num;
    uint8_t gpio_port;
    uint8_t gpio_pin;
};

#define GPIO_PORT_QUANTITY    (10)
#define GPIO_PORT_WIDTH       (16)

static struct gpio_entry _gpio_tab[GPIO_PORT_QUANTITY * GPIO_PORT_WIDTH];

/*
  map a user pin number to a GPIO table entry
 */
static struct gpio_entry *gpio_by_pin_num(uint8_t pin_num)
{
    if ( pin_num >= ARRAY_SIZE(_gpio_tab) ) {
        return NULL;
    }
    _gpio_tab[pin_num].pin_num = pin_num;
    _gpio_tab[pin_num].gpio_port = pin_num / GPIO_PORT_WIDTH;
    _gpio_tab[pin_num].gpio_pin = pin_num % GPIO_PORT_WIDTH;

    return &_gpio_tab[pin_num];
}

DigitalSource::DigitalSource(uint8_t v): _v(v) {
}

void DigitalSource::mode(uint8_t output) {
}

uint8_t DigitalSource::read() {
	return _v;
}

void DigitalSource::write(uint8_t value) {
	_v = value;
}

void DigitalSource::toggle() {
	_v = !_v;
}

void GPIO::init() {
}

void GPIO::pinMode(uint8_t pin, uint8_t output) {
	struct gpio_entry *gpio_entry;
	gpio_entry = gpio_by_pin_num(pin);
	
	gpio_setup_mode(gpio_entry->gpio_port, (1 << gpio_entry->gpio_pin), output == HAL_GPIO_OUTPUT ? GPIO_MODE_OUT : GPIO_MODE_IN);
}

uint8_t GPIO::read(uint8_t pin) {
	struct gpio_entry *gpio_entry;
	gpio_entry = gpio_by_pin_num(pin);

	return !!gpio_get(gpio_entry->gpio_port, (1 << gpio_entry->gpio_pin));
}

void GPIO::write(uint8_t pin, uint8_t value) {
	struct gpio_entry *gpio_entry;
	gpio_entry = gpio_by_pin_num(pin);
	gpio_set(gpio_entry->gpio_port, (1 << gpio_entry->gpio_pin), value);
}

void GPIO::toggle(uint8_t pin) {
	struct gpio_entry *gpio_entry;
	gpio_entry = gpio_by_pin_num(pin);

	gpio_toggle(gpio_entry->gpio_port, (1 << gpio_entry->gpio_pin));
}

AP_HAL::DigitalSource *GPIO::channel(uint16_t pin) {
	return NEW_NOTHROW DigitalSource(0);
}

bool GPIO::usb_connected(void) {
	return false;
}
