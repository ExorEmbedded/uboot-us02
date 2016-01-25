/*
 *  Copyright Altera Corporation (C) 2013. All rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU General Public License,
 *  version 2, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/reset_manager.h>
#ifndef CONFIG_SPL_BUILD
#include <phy.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include "../../../drivers/net/designware.h"
#endif
#include <i2c.h>

#include <asm/arch/dwmmc.h>

extern int ultisdc_init(u32 regbase, int index);

DECLARE_GLOBAL_DATA_PTR;

/*
 * Reads the hwcfg.txt file from USB stick (root of FATFS partition) if any, parses it
 * and updates the environment variable accordingly.
 * 
 * NOTE: This function is used in case the I2C SEEPROM contents are not valid, in order to get
 *       a temporary and volatile HW configuration from USB to boot properly Linux (even if the I2C SEEPROM is not programmed) 
 */
static int USBgethwcfg(void)
{

  printf("Trying to get the HW cfg from USB stick...\n");

  run_command("usb stop", 0);
  run_command("usb reset", 0);
  run_command("setenv filesize 0", 0);
  run_command_list("if fatload usb 0 ${loadaddr} hwcfg.txt;then env import -t ${loadaddr} ${filesize}; fi", -1, 0);
  run_command("usb stop", 0);

  return 0;
}

/*
 * Initialization function which happen at early stage of c code
 */
int board_early_init_f(void)
{
#ifdef CONFIG_HW_WATCHDOG
	/* disable the watchdog when entering U-Boot */
	watchdog_disable();
#endif
	/* calculate the clock frequencies required for drivers */
	cm_derive_clocks_for_drivers();

	return 0;
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* adress of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = 0x00000100;

	/*
	 * reinitialize the global variable for clock value as after
	 * relocation, the global variable are cleared to zeroes
	 */
	cm_derive_clocks_for_drivers();
	return 0;
}

static void setenv_ethaddr_eeprom(void)
{
	uint addr, alen;
	int linebytes;
	uchar chip, enetaddr[6], temp;

	/* configuration based on dev kit EEPROM */
	chip = 0x51;		/* slave ID for EEPROM */
	alen = 2;		/* dev kit using 2 byte addressing */
	linebytes = 6;		/* emac address stored in 6 bytes address */

#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
	addr = 0x16c;
#elif (CONFIG_EMAC_BASE == CONFIG_EMAC1_BASE)
	addr = 0x174;
#endif

	i2c_read(chip, addr, alen, enetaddr, linebytes);

	/* swapping endian to match board implementation */
	temp = enetaddr[0];
	enetaddr[0] = enetaddr[5];
	enetaddr[5] = temp;
	temp = enetaddr[1];
	enetaddr[1] = enetaddr[4];
	enetaddr[4] = temp;
	temp = enetaddr[2];
	enetaddr[2] = enetaddr[3];
	enetaddr[3] = temp;

	if (is_valid_ether_addr(enetaddr))
		eth_setenv_enetaddr("ethaddr", enetaddr);
	else
		puts("Skipped ethaddr assignment due to invalid "
			"EMAC address in EEPROM\n");
}

#ifdef CONFIG_BOARD_LATE_INIT
extern int i2cgethwcfg (void);
int board_late_init(void)
{
	char args[300];
	char ethaddr[20];
	char eth1addr[20];
	char* tmp;

	// read settings from SEPROM
	if (i2cgethwcfg())
	{
	  // error!
	  printf("Failed to read the HW cfg from the I2C SEEPROM: trying to load it from USB ...\n");
	  USBgethwcfg();
	}

	// set ethernet address
	setenv_addr("setenv_ethaddr_eeprom", (void *)setenv_ethaddr_eeprom);

        // get ethernet addresses
        tmp = getenv("ethaddr");
	if (tmp != NULL)
	{
		memset(ethaddr, 0x00, sizeof(ethaddr));
		strncpy(ethaddr, tmp, sizeof(ethaddr)-1);
	}
        else
		setenv("ethaddr", "FF:FF:FF:FF:FF:FF");

        tmp = getenv("eth1addr");
	if (tmp != NULL)
	{
		memset(eth1addr, 0x00, sizeof(eth1addr));
		strncpy(eth1addr, tmp, sizeof(eth1addr)-1);
	}
       else
		setenv("eth1addr", "FF:FF:FF:FF:FF:FF");

        // set bootargs
	memset(args, 0x00, sizeof(args));
	snprintf(args, sizeof(args)-1, "setenv bootargs console=ttyS0,115200 root=${mmcroot} rw rootwait hw_code=%s hw_dispid=%s touch_type=%s;bootz ${loadaddr} - ${fdtaddr}",
			getenv("hw_code"),
			getenv("hw_dispid"),
			getenv("touch_type")); 
	setenv("mmcboot", args);

	printf("bootargs environment variable set to \"%s\"n", args);

	return 0;
}
#endif

/* EMAC related setup and only supported in U-Boot */
#if !defined(CONFIG_SOCFPGA_VIRTUAL_TARGET) && \
!defined(CONFIG_SPL_BUILD)

/*
 * DesignWare Ethernet initialization
 * This function overrides the __weak  version in the driver proper.
 * Our Micrel Phy needs slightly non-conventional setup
 */
int designware_board_phy_init(struct eth_device *dev, int phy_addr,
		int (*mii_write)(struct eth_device *, u8, u8, u16),
		int (*dw_reset_phy)(struct eth_device *))
{
	struct dw_eth_dev *priv = dev->priv;
	struct phy_device *phydev;
	struct mii_dev *bus;

	if ((*dw_reset_phy)(dev) < 0)
		return -1;

	bus = mdio_get_current_dev();
	phydev = phy_connect(bus, phy_addr, dev,
		priv->interface);

	/* Micrel PHY is connected to EMAC1 */
	if (strcasecmp(phydev->drv->name, "Micrel ksz9021") == 0 &&
		((phydev->drv->uid & phydev->drv->mask) ==
		(phydev->phy_id & phydev->drv->mask))) {

		printf("Configuring PHY skew timing for %s\n",
			phydev->drv->name);

		/* min rx data delay */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW,
			getenv_ulong(CONFIG_KSZ9021_DATA_SKEW_ENV, 16,
				CONFIG_KSZ9021_DATA_SKEW_VAL)) < 0)
			return -1;
		/* min tx data delay */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW,
			getenv_ulong(CONFIG_KSZ9021_DATA_SKEW_ENV, 16,
				CONFIG_KSZ9021_DATA_SKEW_VAL)) < 0)
			return -1;
		/* max rx/tx clock delay, min rx/tx control */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_CLOCK_SKEW,
			getenv_ulong(CONFIG_KSZ9021_CLK_SKEW_ENV, 16,
				CONFIG_KSZ9021_CLK_SKEW_VAL)) < 0)
			return -1;

		if (phydev->drv->config)
			phydev->drv->config(phydev);
	}
	return 0;
}
#endif

/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
#if !defined(CONFIG_SOCFPGA_VIRTUAL_TARGET) && \
!defined(CONFIG_SPL_BUILD)

	/* Initialize EMAC */

	/*
	 * Putting the EMAC controller to reset when configuring the PHY
	 * interface select at System Manager
	*/
	emac0_reset_enable(1);
	emac1_reset_enable(1);

	/* Clearing emac0 PHY interface select to 0 */
	clrbits_le32(CONFIG_SYSMGR_EMAC_CTRL,
		(SYSMGR_EMACGRP_CTRL_PHYSEL_MASK <<
#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB));
#elif (CONFIG_EMAC_BASE == CONFIG_EMAC1_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB));
#endif

	/* configure to PHY interface select choosed */
	setbits_le32(CONFIG_SYSMGR_EMAC_CTRL,
#if (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_GMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_MII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RGMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII <<
#endif
#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB));
	/* Release the EMAC controller from reset */
	emac0_reset_enable(0);
#elif (CONFIG_EMAC_BASE == CONFIG_EMAC1_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB));
	/* Release the EMAC controller from reset */
	emac1_reset_enable(0);
#endif

	/* initialize and register the emac */
	int rval = designware_initialize(0, CONFIG_EMAC_BASE,
		CONFIG_EPHY_PHY_ADDR,
#if (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_GMII)
		PHY_INTERFACE_MODE_GMII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_MII)
		PHY_INTERFACE_MODE_MII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RGMII)
		PHY_INTERFACE_MODE_RGMII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RMII)
		PHY_INTERFACE_MODE_RMII);
#endif
	debug("board_eth_init %d\n", rval);
	return rval;
#else
	return 0;
#endif
}

/*
 * Initializes MMC controllers.
 * to override, implement board_mmc_init()
 */
int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_DWMMC
	printf("initializing dwmmc...\n");
	altera_dwmmc_init(CONFIG_SDMMC_BASE, CONFIG_DWMMC_BUS_WIDTH, 0);
#endif
#ifdef CONFIG_ULTISDC_SDHCI
	printf("initializing ultisdc...\n");
	ultisdc_init(CONFIG_ULTISDC_BASE, 1);
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD
 /*
  * Init the ULTISDC for the dedicated/custom SPL u-boot loading sequence. 
  */
 int spl_board_mmc_initialize(void)
 {
   return ultisdc_init(CONFIG_ULTISDC_BASE, 1);
 }
 
 /*
  * Init the EMMC for the dedicated/custom SPL u-boot loading sequence. 
  */
 int spl_board_emmc_initialize(void)
 {
   return altera_dwmmc_init(CONFIG_SDMMC_BASE, CONFIG_DWMMC_BUS_WIDTH, 0);
 }
#endif
