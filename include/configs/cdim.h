#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE   0x80000000
#define CFG_SYS_SDRAM_SIZE		0x08000000

// Note: U-Boot will not parse clock property in the device tree

#define CFG_SYS_NS16550_CLK  100000000

#define CFG_EXTRA_ENV_SETTINGS \
    "autostart=yes\0"

#endif /* __CONFIG_H */