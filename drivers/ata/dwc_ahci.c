// SPDX-License-Identifier: GPL-2.0+
/*
 * DWC SATA platform driver
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * Author: Mugunthan V N <mugunthanvnm@ti.com>
 */

#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <sata.h>
#ifdef CONFIG_ARCH_OMAP2PLUS
#include <asm/arch/sata.h>
#endif
#include <asm/io.h>
#include <generic-phy.h>
#include <linux/printk.h>

struct dwc_ahci_priv {
	void *base;
	void *wrapper_base;
};

static int dwc_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ahci_bind_scsi(dev, &scsi_dev);
}

static int dwc_ahci_of_to_plat(struct udevice *dev)
{
	struct dwc_ahci_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	priv->base = map_physmem(dev_read_addr(dev), sizeof(void *),
				 MAP_NOCACHE);

	addr = devfdt_get_addr_index(dev, 1);
	if (addr != FDT_ADDR_T_NONE) {
		priv->wrapper_base = map_physmem(addr, sizeof(void *),
						 MAP_NOCACHE);
	} else {
		priv->wrapper_base = NULL;
	}

	return 0;
}

static int dwc_ahci_probe(struct udevice *dev)
{
	struct dwc_ahci_priv *priv = dev_get_priv(dev);
	int ret;
	struct phy phy;

	ret = generic_phy_get_by_name(dev, "sata-phy", &phy);
	if (ret) {
		pr_err("can't get the phy from DT\n");
		return ret;
	}

	ret = generic_phy_init(&phy);
	if (ret) {
		pr_debug("unable to initialize the sata phy\n");
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_debug("unable to power on the sata phy\n");
		return ret;
	}

#ifdef CONFIG_ARCH_OMAP2PLUS
	if (priv->wrapper_base) {
		u32 val = TI_SATA_IDLE_NO | TI_SATA_STANDBY_NO;

		/* Enable SATA module, No Idle, No Standby */
		writel(val, priv->wrapper_base + TI_SATA_SYSCONFIG);
	}
#endif

	return ahci_probe_scsi(dev, (ulong)priv->base);
}

static const struct udevice_id dwc_ahci_ids[] = {
	{ .compatible = "snps,dwc-ahci" },
	{ }
};

U_BOOT_DRIVER(dwc_ahci) = {
	.name	= "dwc_ahci",
	.id	= UCLASS_AHCI,
	.of_match = dwc_ahci_ids,
	.bind	= dwc_ahci_bind,
	.of_to_plat = dwc_ahci_of_to_plat,
	.ops	= &scsi_ops,
	.probe	= dwc_ahci_probe,
	.priv_auto	= sizeof(struct dwc_ahci_priv),
};
