#include <nucleo_f767zi.conf.h>

CONFIG {
	uarts[2].status = DISABLED;
	uarts[3].status = ENABLED;
	uarts[6].status = ENABLED;

	spis[1].status = ENABLED;
	spis[2].status = ENABLED;
}
