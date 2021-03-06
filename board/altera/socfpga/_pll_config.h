/*
 * Copyright Altera Corporation (C) 2012-2014. All rights reserved
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of Altera Corporation nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This file is generated by Preloader Generator */

#ifndef _PRELOADER_PLL_CONFIG_H_
#define _PRELOADER_PLL_CONFIG_H_

/* PLL configuration data */
/* Main PLL */
#define CONFIG_HPS_MAINPLLGRP_VCO_DENOM			(0)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_MAINPLLGRP_VCO_NUMER			(41)
#else
#define CONFIG_HPS_MAINPLLGRP_VCO_NUMER			(73)
#endif
#define CONFIG_HPS_MAINPLLGRP_MPUCLK_CNT		(0)
#define CONFIG_HPS_MAINPLLGRP_MAINCLK_CNT		(0)
#define CONFIG_HPS_MAINPLLGRP_DBGATCLK_CNT		(0)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_MAINPLLGRP_MAINQSPICLK_CNT		(2)
#else
#define CONFIG_HPS_MAINPLLGRP_MAINQSPICLK_CNT		(4)
#endif
#define CONFIG_HPS_MAINPLLGRP_MAINNANDSDMMCCLK_CNT	(0)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_MAINPLLGRP_CFGS2FUSER0CLK_CNT	(8)
#else
#define CONFIG_HPS_MAINPLLGRP_CFGS2FUSER0CLK_CNT	(14)
#endif
#define CONFIG_HPS_MAINPLLGRP_MAINDIV_L3MPCLK		(1)
#define CONFIG_HPS_MAINPLLGRP_MAINDIV_L3SPCLK		(1)
#define CONFIG_HPS_MAINPLLGRP_MAINDIV_L4MPCLK		(1)
#define CONFIG_HPS_MAINPLLGRP_MAINDIV_L4SPCLK		(1)
#define CONFIG_HPS_MAINPLLGRP_DBGDIV_DBGATCLK		(0)
#define CONFIG_HPS_MAINPLLGRP_DBGDIV_DBGCLK		(1)
#define CONFIG_HPS_MAINPLLGRP_TRACEDIV_TRACECLK		(0)
/*
 * To tell where is the clock source:
 * 0 = MAINPLL
 * 1 = PERIPHPLL
 */
#define CONFIG_HPS_MAINPLLGRP_L4SRC_L4MP		(1)
#define CONFIG_HPS_MAINPLLGRP_L4SRC_L4SP		(1)

/* Peripheral PLL */
#define CONFIG_HPS_PERPLLGRP_VCO_DENOM			(1)
#define CONFIG_HPS_PERPLLGRP_VCO_NUMER			(79)
/*
 * To tell where is the VCOs source:
 * 0 = EOSC1
 * 1 = EOSC2
 * 2 = F2S
 */
#define CONFIG_HPS_PERPLLGRP_VCO_PSRC			(0)
#define CONFIG_HPS_PERPLLGRP_EMAC0CLK_CNT		(3)
#define CONFIG_HPS_PERPLLGRP_EMAC1CLK_CNT		(3)
#define CONFIG_HPS_PERPLLGRP_PERQSPICLK_CNT		(1)
#define CONFIG_HPS_PERPLLGRP_PERNANDSDMMCCLK_CNT	(4)
#define CONFIG_HPS_PERPLLGRP_PERBASECLK_CNT		(4)
#define CONFIG_HPS_PERPLLGRP_S2FUSER1CLK_CNT		(9)
#define CONFIG_HPS_PERPLLGRP_DIV_USBCLK			(0)
#define CONFIG_HPS_PERPLLGRP_DIV_SPIMCLK		(0)
#define CONFIG_HPS_PERPLLGRP_DIV_CAN0CLK		(1)
#define CONFIG_HPS_PERPLLGRP_DIV_CAN1CLK		(1)
#define CONFIG_HPS_PERPLLGRP_GPIODIV_GPIODBCLK		(6249)
/*
 * To tell where is the clock source:
 * 0 = F2S_PERIPH_REF_CLK
 * 1 = MAIN_CLK
 * 2 = PERIPH_CLK
 */
#define CONFIG_HPS_PERPLLGRP_SRC_SDMMC			(2)
#define CONFIG_HPS_PERPLLGRP_SRC_NAND			(2)
#define CONFIG_HPS_PERPLLGRP_SRC_QSPI			(1)

/* SDRAM PLL */
#ifdef CONFIG_SOCFPGA_ARRIA5
/* Arria V SDRAM will run at 533MHz while Cyclone V still at 400MHz
 * This if..else... is not required if generated by tools */
#define CONFIG_HPS_SDRPLLGRP_VCO_DENOM			(2)
#define CONFIG_HPS_SDRPLLGRP_VCO_NUMER			(127)
#else
#define CONFIG_HPS_SDRPLLGRP_VCO_DENOM			(0)
#define CONFIG_HPS_SDRPLLGRP_VCO_NUMER			(31)
#endif /* CONFIG_SOCFPGA_ARRIA5 */

/*
 * To tell where is the VCOs source:
 * 0 = EOSC1
 * 1 = EOSC2
 * 2 = F2S
 */
#define CONFIG_HPS_SDRPLLGRP_VCO_SSRC			(0)
#define CONFIG_HPS_SDRPLLGRP_DDRDQSCLK_CNT		(1)
#define CONFIG_HPS_SDRPLLGRP_DDRDQSCLK_PHASE		(0)
#define CONFIG_HPS_SDRPLLGRP_DDR2XDQSCLK_CNT		(0)
#define CONFIG_HPS_SDRPLLGRP_DDR2XDQSCLK_PHASE		(0)
#define CONFIG_HPS_SDRPLLGRP_DDRDQCLK_CNT		(1)
#define CONFIG_HPS_SDRPLLGRP_DDRDQCLK_PHASE		(4)
#define CONFIG_HPS_SDRPLLGRP_S2FUSER2CLK_CNT		(5)
#define CONFIG_HPS_SDRPLLGRP_S2FUSER2CLK_PHASE		(0)

/* Misc */
#ifdef CONFIG_SOCFPGA_ARRIA5
#define CONFIG_HPS_ALTERAGRP_MPUCLK			(0)
#define CONFIG_HPS_ALTERAGRP_MAINCLK			(2)
#else
#define CONFIG_HPS_ALTERAGRP_MAINCLK			(4)
#endif

/* Info for driver */
/* U-Boot derived clock source frequency (OSC1, OSC2 and F2S only) from here */
#define CONFIG_HPS_CLK_OSC1_HZ			(25000000)
#define CONFIG_HPS_CLK_OSC2_HZ			(0)
#define CONFIG_HPS_CLK_F2S_SDR_REF_HZ		(0)
#define CONFIG_HPS_CLK_F2S_PER_REF_HZ		(0)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_CLK_MAINVCO_HZ		(1050000000)
#else
#define CONFIG_HPS_CLK_MAINVCO_HZ		(1850000000)
#endif
#define CONFIG_HPS_CLK_PERVCO_HZ		(1000000000)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_CLK_SDRVCO_HZ		(1066000000)
#else
#define CONFIG_HPS_CLK_SDRVCO_HZ		(800000000)
#endif
#define CONFIG_HPS_CLK_EMAC0_HZ			(250000000)
#define CONFIG_HPS_CLK_EMAC1_HZ			(250000000)
#define CONFIG_HPS_CLK_USBCLK_HZ		(200000000)
#define CONFIG_HPS_CLK_NAND_HZ			(50000000)
#define CONFIG_HPS_CLK_SDMMC_HZ			(200000000)
#ifdef CONFIG_SOCFPGA_ARRIA5
/* The if..else... is not required if generated by tools */
#define CONFIG_HPS_CLK_QSPI_HZ			(350000000)
#else
#define CONFIG_HPS_CLK_QSPI_HZ			(370000000)
#endif
#define CONFIG_HPS_CLK_SPIM_HZ			(200000000)
#define CONFIG_HPS_CLK_CAN0_HZ			(100000000)
#define CONFIG_HPS_CLK_CAN1_HZ			(100000000)
#define CONFIG_HPS_CLK_GPIODB_HZ		(32000)
#define CONFIG_HPS_CLK_L4_MP_HZ			(100000000)
#define CONFIG_HPS_CLK_L4_SP_HZ			(100000000)

#endif /* _PRELOADER_PLL_CONFIG_H_ */


