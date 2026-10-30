### Run QEMU
```shell
./scripts/qemu/auto_qemu -serial mon:stdio -serial /dev/ttyUSB0 -s -S
```
### FT232RL pinout
https://components101.com/modules/ft232rl-usb-to-ttl-converter-pinout-features-datasheet-working-application-alternative

### FT232RL set baud rate (after running QEMU)
```c
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <asm/termbits.h>

int main() {
	struct termios2 tio;
	int fd;

	system("stty -F /dev/ttyUSB0 raw");

	fd = open("/dev/ttyUSB0", O_RDONLY);

	ioctl(fd, TCGETS2, &tio);
	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = 416666;
	tio.c_ospeed = 416666;
	ioctl(fd, TCSETS2, &tio);

	return 0;
}
```
