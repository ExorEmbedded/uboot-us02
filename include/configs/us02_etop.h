/*
 *  Copyright Altera Corporation (C) 2012-2013. All rights reserved
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

#define CONFIG_SOCFPGA_CYCLONE5

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../../board/exor/us02_etop/build.h"
#include "../../board/exor/us02_etop/pinmux_config.h"
#include "../../board/exor/us02_etop/pll_config.h"
#include "../../board/exor/us02_etop/sdram/sdram_config.h"
#include "../../board/exor/us02_etop/reset_config.h"
#include "socfpga_common.h"
#ifdef CONFIG_SPL_BUILD
#include "../../board/exor/us02_etop/iocsr_config_cyclone5.h"
#endif

#if (CONFIG_PRELOADER_BOOT_FROM_SDMMC == 1)
#define CONFIG_SYS_EXOR_USOM
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION	1
#endif

/*
 * Memory allocation (MALLOC)
 */
/* Room required on the stack for the environment data */
#ifdef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SIZE
#endif
#define CONFIG_ENV_SIZE			8192
/* Size of DRAM reserved for malloc() use */
#ifdef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_MALLOC_LEN
#endif
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 256 * 1024)

/* Bootcounter using the M41T83 I2C RTC NVRAM */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_I2C
#define CONFIG_BOOTCOUNT_ALEN 1
#define CONFIG_SYS_I2C_RTC_ADDR 0x68
#undef  CONFIG_SYS_BOOTCOUNT_ADDR
#define CONFIG_SYS_BOOTCOUNT_ADDR 0x19

/* Bootdelay setting
 */
#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY 0
#define CONFIG_ZERO_BOOTDELAY_CHECK

/* Environment setting for EMMC 
 */
#ifdef CONFIG_ENV_IS_IN_MMC

#ifdef CONFIG_SYS_MMC_ENV_DEV
#undef CONFIG_SYS_MMC_ENV_DEV
#endif
#define CONFIG_SYS_MMC_ENV_DEV		1	/* device 1: EMMC */

#ifdef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_OFFSET
#endif
#define CONFIG_ENV_OFFSET		0	

#ifdef CONFIG_SYS_MMC_ENV_PART 
#undef CONFIG_SYS_MMC_ENV_PART
#endif
#define CONFIG_SYS_MMC_ENV_PART         2  /* BOOT2 partition */

#endif


/*
 * Console setup
 */
#define CONFIG_HAVEPRGUART
/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT		"US02 # "

/* EMAC controller and PHY used */
#define CONFIG_EMAC_BASE		CONFIG_EMAC1_BASE
#define CONFIG_EPHY_PHY_ADDR		CONFIG_EPHY1_PHY_ADDR
#define CONFIG_PHY_INTERFACE_MODE	SOCFPGA_PHYSEL_ENUM_MII

/* ULTISDC FPGA CORE */
#define CONFIG_ULTISDC_SDHCI
#define CONFIG_ULTISDC_BASE		0xFF230000
#define CONFIG_SDHCI
#define CONFIG_MMC_SDHCI_IO_ACCESSORS

#define CONFIG_CMD_FS_GENERIC

#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS \
	"altbootcmd="CONFIG_SYS_ALT_BOOTCOMMAND"\0"\
	"bootlimit=3\0" \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdtaddr=0x00000100\0" \
	"boot_fdt=yes\0" \
	"skipbsp1=0\0" \
	"bootpart=0:1\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=usom_undefined.dtb\0" \
	"console=/dev/null\0" \
	"rs232_txen=0\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"rootpath=/export/rootfs\0" \
	"nfsopts=nolock\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"hw_dispid=${hw_dispid} " \
		"hw_code=${hw_code} " \
		"board_name=${board_name} " \
		"touch_type=${touch_type} " \
		"ethaddr=${ethaddr} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"hw_dispid=${hw_dispid} " \
		"hw_code=${hw_code} " \
		"board_name=${board_name} " \
		"touch_type=${touch_type} " \
		"ethaddr=${ethaddr} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadfdt=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"mmcloados=run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootenv; then " \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"if run loadimage; then " \
				"run mmcloados;" \
			"fi;" \
		"fi;\0" \
	"usbargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"hw_dispid=${hw_dispid} " \
		"hw_code=${hw_code} " \
		"board_name=${board_name} " \
		"touch_type=${touch_type} " \
		"ethaddr=${ethaddr} " \
		"root=${usbroot} " \
		"rootfstype=${usbrootfstype}\0" \
	"usbroot=/dev/sda2 rw\0" \
	"usbrootfstype=ext4 rootwait\0" \
	"usbloados=run usbargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run usbloadfdt; then " \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"usbloadimage=load usb 0 ${loadaddr} ${bootdir}/${bootfile}\0" \
	"usbloadfdt=load usb 0 ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"usbboot=mmc dev ${mmcdev}; " \
		"if usb reset; then " \
			"if run usbloadimage; then " \
				"run usbloados;" \
			"fi;" \
			"usb stop;" \
		"fi;\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"tftp ${loadaddr} ${bootfile}; " \
		"tftp ${fdtaddr} ${fdtfile}; " \
		"run netargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"findfdt="\
		"if test $board_name = usom_etop6xx; then " \
			"setenv fdtfile usom_etop6xx.dtb; fi; " \
		"if test $board_name = usom_us02kit; then " \
			"setenv fdtfile usom_us02kit.dtb; fi; " \
		"if test $board_name = usom_undefined; then " \
			"setenv fdtfile usom_undefined.dtb; fi; \0" 
	
#define CONFIG_BOOTCOMMAND \
	"setenv mmcdev 0; " \
	"run findfdt; " \
	"echo Try booting Linux from SD-card...;" \
	"run mmcboot;" \
	"if test $skipbsp1 = 0; then " \
	"echo Try booting Linux from EMMC, main BSP...;" \
	"setenv mmcdev 1; " \
	"setenv bootpart 1:3; " \
	"setenv mmcroot /dev/mmcblk1p3 ro; " \
	"run mmcboot;" \
	"fi; " \
	"echo Try booting Linux from USB stick...;" \
	"run usbboot;" \
	"echo Try booting Linux from EMMC, recovery BSP...;" \
	"setenv mmcdev 1; " \
	"setenv bootpart 1:2; " \
	"setenv mmcroot /dev/mmcblk1p2 ro; " \
	"run mmcboot;" 

#define CONFIG_SYS_ALT_BOOTCOMMAND \
	"i2c mw 68 19 0; " \
	"setenv mmcdev 0; " \
	"run findfdt; " \
	"echo Try booting Linux from SD-card...;" \
	"run mmcboot;" \
	"echo Try booting Linux from USB stick...;" \
	"run usbboot;" \
	"echo Try booting Linux from EMMC, recovery BSP...;" \
	"setenv mmcdev 1; " \
	"setenv bootpart 1:2; " \
	"setenv mmcroot /dev/mmcblk1p2 ro; " \
	"run mmcboot;" 

#endif	/* __CONFIG_H */
