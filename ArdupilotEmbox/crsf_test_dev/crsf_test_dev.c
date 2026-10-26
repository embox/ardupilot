#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <drivers/char_dev.h>

static uint8_t crsf_frame[] = {
    // Device Address: Flight Controller
    0xC8,
    // Frame Length: 24
    0x18,
    // Type: CRSF_FRAMETYPE_RC_CHANNELS_PACKED
    0x16,
    // Payload: 16 channels 1500us
    0xE0,
    0x03,
    0x1F,
    0xF8,
    0xC0,
    0x07,
    0x3E,
    0xF0,
    0x81,
    0x0F,
    0x7C,
    0xE0,
    0x03,
    0x1F,
    0xF8,
    0xC0,
    0x07,
    0x3E,
    0xF0,
    0x81,
    0x0F,
    0x7C,
    // CRC8
    0xAD,
};

static ssize_t crsf_test_dev_read(struct char_dev *cdev, void *buf,
    size_t nbyte, int flags) {
	if (nbyte >= sizeof(crsf_frame)) {
		memcpy(buf, crsf_frame, sizeof(crsf_frame));
		return sizeof(crsf_frame);
	}
	return 0;
}

static ssize_t crsf_test_dev_write(struct char_dev *cdev, const void *buf,
    size_t nbyte, int flags) {
	return nbyte;
}

static int crsf_test_dev_ioctl(struct char_dev *cdev, int request, void *data) {
	return 0;
}

static int crsf_test_dev_status(struct char_dev *cdev, int mask) {
	int res;

	res = 0;

	if (mask & POLLIN) {
		res += sizeof(crsf_frame);
	}

	return res;
}

static const struct char_dev_ops crsf_test_dev_ops = {
    .read = crsf_test_dev_read,
    .write = crsf_test_dev_write,
    .ioctl = crsf_test_dev_ioctl,
    .status = crsf_test_dev_status,
};

static struct char_dev crsf_test_dev_cdev = CHAR_DEV_INIT("crsf_test_dev",
    &crsf_test_dev_ops);

CHAR_DEV_REGISTER(&crsf_test_dev_cdev);
