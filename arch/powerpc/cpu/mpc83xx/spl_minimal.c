// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <asm-offsets.h>
#include <clock_legacy.h>
#include <mpc83xx.h>
#include <system-constants.h>
#include <time.h>
#include <asm/global_data.h>

#include "lblaw/lblaw.h"
#include "elbc/elbc.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * im)
{
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *)SYS_INIT_SP_ADDR;

	/* global data region was cleared in start.S */

	/* system performance tweaking */

#ifndef CONFIG_ACR_PIPE_DEP_UNSET
	/* Arbiter pipeline depth */
	im->arbiter.acr = (im->arbiter.acr & ~ACR_PIPE_DEP) |
			  CONFIG_ACR_PIPE_DEP;
#endif

#ifndef CONFIG_ACR_RPTCNT_UNSET
	/* Arbiter repeat count */
	im->arbiter.acr = (im->arbiter.acr & ~(ACR_RPTCNT)) |
			  CONFIG_ACR_RPTCNT;
#endif

#ifdef CONFIG_SYS_SPCR_OPT
	/* Optimize transactions between CSB and other devices */
	im->sysconf.spcr = (im->sysconf.spcr & ~SPCR_OPT) |
			   (CONFIG_SYS_SPCR_OPT << SPCR_OPT_SHIFT);
#endif

	/* Enable Time Base & Decrementer (so we will have udelay()) */
	im->sysconf.spcr |= SPCR_TBEN;

	/* DDR control driver register */
#ifdef CFG_SYS_DDRCDR
	im->sysconf.ddrcdr = CFG_SYS_DDRCDR;
#endif
	/* Output buffer impedance register */
#ifdef CFG_SYS_OBIR
	im->sysconf.obir = CFG_SYS_OBIR;
#endif

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CFG_SYS_NAND_BR_PRELIM)  \
	&& defined(CFG_SYS_NAND_OR_PRELIM) \
	&& defined(CFG_SYS_NAND_LBLAWBAR_PRELIM) \
	&& defined(CFG_SYS_NAND_LBLAWAR_PRELIM)
	set_lbc_br(0, CFG_SYS_NAND_BR_PRELIM);
	set_lbc_or(0, CFG_SYS_NAND_OR_PRELIM);
	im->sysconf.lblaw[0].bar = CFG_SYS_NAND_LBLAWBAR_PRELIM;
	im->sysconf.lblaw[0].ar = CFG_SYS_NAND_LBLAWAR_PRELIM;
#else
#error CFG_SYS_NAND_BR_PRELIM, CFG_SYS_NAND_OR_PRELIM, CFG_SYS_NAND_LBLAWBAR_PRELIM & CFG_SYS_NAND_LBLAWAR_PRELIM must be defined
#endif
}

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */
unsigned long get_tbclk(void)
{
	return (gd->bus_clk + 3L) / 4L;
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

ulong get_bus_freq(ulong dummy)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u8 spmf = (im->clk.spmr & SPMR_SPMF) >> SPMR_SPMF_SHIFT;

	return get_board_sys_clk() * spmf;
}
