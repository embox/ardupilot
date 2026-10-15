/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SPIDevice.h"

#include <assert.h>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <drivers/spi.h>

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/utility/OwnPtr.h>

extern const AP_HAL::HAL& hal;

namespace Embox {

#define MHZ (1000U*1000U)
#define KHZ (1000U)

// TODO
#define BUS_NUM 3

SPIBus SPIDeviceManager::businfo[BUS_NUM];

SPIDevice::SPIDevice(SPIBus &bus, struct spi_device *device)
    : bus(bus)
    , dev(device)
{
    set_device_bus(device->spid_bus_num);
    device->spid_flags |= SPI_CS_ACTIVE;
    device->spid_flags |= SPI_CS_INACTIVE;
    // TODO
    _speed = 0;
}

bool SPIDevice::set_speed(AP_HAL::Device::Speed speed)
{
    switch (speed) {
    case AP_HAL::Device::SPEED_HIGH:
        _speed = 0;
        break;
    case AP_HAL::Device::SPEED_LOW:
        _speed = 0;
        break;
    }

    return true;
}

bool SPIDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    uint32_t len = (send_len > recv_len)?send_len:recv_len + 1;
    uint8_t *buffer = (uint8_t *)calloc(len, sizeof(uint8_t));
    if (!buffer) {
        return false;
    }
    if (send_len > 0) {
        memcpy(buffer, send, send_len);
    }
    bool res = transfer_fullduplex(buffer, buffer, len);
    if (res && recv_len > 0) {
        memcpy(recv, buffer+1, recv_len);
    }
    free(buffer);
    return res;
}

bool SPIDevice::transfer_fullduplex(const uint8_t *send, uint8_t *recv,
                                    uint32_t len)
{
    return spi_transfer(dev, (uint8_t *)send, recv, len)?false:true;
}

AP_HAL::Semaphore *SPIDevice::get_semaphore()
{
    return &bus.semaphore;
}

AP_HAL::Device::PeriodicHandle SPIDevice::register_periodic_callback(
    uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    return bus.register_periodic_callback(period_usec, cb, this);
}

bool SPIDevice::adjust_periodic_callback(
    AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return bus.adjust_timer(h, period_usec);
}


AP_HAL::OwnPtr<AP_HAL::SPIDevice>
SPIDeviceManager::get_device(const char *name)
{
    struct spi_device *dev = spi_dev_by_name(name);
    if (!dev) {
        return nullptr;
    }

    if (dev->spid_bus_num < 0 ||
        static_cast<unsigned>(dev->spid_bus_num) >= ARRAY_SIZE(businfo)) {
        return nullptr;
    }

    return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(
        NEW_NOTHROW SPIDevice(businfo[dev->spid_bus_num], dev));
}
}
