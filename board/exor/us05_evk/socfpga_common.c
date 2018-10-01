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

#define GPIO_SWPORTA_DR        0x00
#define GPIO_SWPORTA_DDR       0x04
#define GPIO_INTEN             0x30
#define GPIO_INTMASK           0x34
#define GPIO_INTTYPE_LEVEL     0x38
#define GPIO_INT_POLARITY      0x3c
#define GPIO_INTSTATUS         0x40
#define GPIO_PORTA_DEBOUNCE    0x48
#define GPIO_PORTA_EOI         0x4c
#define GPIO_EXT_PORTA         0x50
#define GPIO0BASE              0xff708000
#define TXENGPIO               22 
#define AGPIO0BASE             0xff210010
#define QSPISELGPIO            2

extern int ultisdc_init(u32 regbase, int index);

DECLARE_GLOBAL_DATA_PTR;

#define ETOP6XX_VAL   116
#define ALTERAKIT_VAL 113
#define US05KIT_VAL   135

void ena_rs232phy(void);

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


#ifdef CONFIG_BOARD_LATE_INIT
extern int i2cgethwcfg (void);
int board_late_init(void)
{
	char args[300];
	char* tmp;
	unsigned long hwcode;
	
	// read settings from SEPROM
	if (i2cgethwcfg())
	{
	  ena_rs232phy();
	  printf("Failed to read the HW cfg from the I2C SEEPROM: trying to load it from USB ...\n");
	  USBgethwcfg();
	}
	
	/* Set the "board_name" env. variable according with the "hw_code" */
	tmp = getenv("hw_code");
	if(!tmp)
	{
	  puts ("WARNING: 'hw_code' environment var not found!\n");
	  return 1;
	}
	hwcode = (simple_strtoul (tmp, NULL, 10))&0xff;
	
	if(hwcode==ETOP6XX_VAL)
	  setenv("board_name", "usom_etop6xx"); 
	else if(hwcode==ALTERAKIT_VAL)
	  setenv("board_name", "usom_us02kit"); 
	else if(hwcode==US05KIT_VAL)
	  if(!getenv("eth3addr"))
	    setenv("board_name", "usom_us05kit");      /* Same hw_code but different board name/dtb file based on number of defined MAC addresses */
	  else
	    setenv("board_name", "usom_us05kit-4mac"); 
	else
	{
	  puts ("WARNING: unknowm carrier hw code; using 'usom_undefined' board name. \n");
	  setenv("board_name", "usom_undefined");
	}
	
	/* Check if file $0030d8$.bin exists on the 1st partition of the SD-card and, if so, skips booting the mainOS */
	run_command("setenv skipbsp1 0", 0);
	run_command("mmc dev 0", 0);
	run_command("mmc rescan", 0);
	run_command("if fatload mmc 0:1 $loadaddr /$0030d8$.bin; then setenv skipbsp1 1; fi", 0);
	
	setenv("bootcmd", CONFIG_BOOTCOMMAND);

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
#ifdef CONFIG_ULTISDC_SDHCI
	ultisdc_init(CONFIG_ULTISDC_BASE, 0);
#endif
#ifdef CONFIG_DWMMC
	altera_dwmmc_init(CONFIG_SDMMC_BASE, CONFIG_DWMMC_BUS_WIDTH, 1);
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

#ifdef CONFIG_HAVEPRGUART
void ena_rs232phy(void)
{
  /* This is actually an empty function, since the console port is TTL, without phy */
  udelay(1000);
}
#else
void ena_rs232phy(void){}
#endif