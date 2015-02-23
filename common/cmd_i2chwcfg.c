 /* This file gives support for I2C SEEPROM HW parameters handling
  *
  * See file CREDITS for list of people who contributed to this
  * project.
  *
  * Author: Giovanni Pavoni Exor S.p.a.
  *
  * --------------- Revision history -----------------------------
  * Author: Giovanni Pavoni Exor S.p.a.
  * Reason: Added SEEPROM format 3 compatibility.
  *			The i2csavehw command generates a minimal SEEPROM format 3 
  *			data structure and allows to change just display id and 
  *			MACid if valid SEEPROM format 3 still present on SEEPROM.
  *
  * Author: Giovanni Pavoni Exor S.p.a.
  * Reason: Integrations for the uSOM platform: added handling of the "hw_code" and
  *			"touch_type" env. variables.
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation; either version 2 of
  * the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
  * MA 02111-1307 USA
  *
  */
 
 
#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <i2c.h>

#ifdef CONFIG_CMD_I2CHWCFG

#define ENET_ADDR_LENGTH 6
/*=======================================================================
 * Layout of I2C memory
 *======================================================================= */
#define FACTORY_SECTION_SIZE_V2	128	/* First part of I2C = factory section for V2 and V1*/

/* Header */
#define SIGNATURE1_POS	0
#define SIGNATURE2_POS	1
#define VERSION_POS		2
#define CKSUM_POS		3

/* Data */
#define DISPID_POS		5
#define JUMPERFLAGSL_POS	14
#define TCHTYPE_POS		21

#define MACID0_POS		24
#define MACID1_POS		25
#define MACID2_POS		26
#define MACID3_POS		27
#define MACID4_POS		28
#define MACID5_POS		29

/* --------------------- Format 3 specific ------------------------------ */
#define FACTORY_SECTION_SIZE_V3	64	/* First part of I2C = factory section for V3*/
#define BL_TIME_CNT		130
#define BL_TIME_CHK		134
#define SYS_TIME_CNT		136
#define SYS_TIME_CHK		140
#define HWPICKPANELCODE_POS     4

#define ZEROCKSUM		0x57
#define SAFE_FACTORY_AREA_POS	184

#define AUXMACID0_POS                   55
#define AUXMACID1_POS                   56
#define AUXMACID2_POS                   57
#define AUXMACID3_POS                   58
#define AUXMACID4_POS                   59
#define AUXMACID5_POS                   60

/*=======================================================================
 * I2C pre-defined / fixed values
 *======================================================================= */
#define SIGNATURE1_VAL	0xaa
#define SIGNATURE2_VAL	0x55
#define VERSION_VAL		3
#define RFU_VAL			0xff

#ifdef CONFIG_CMD_DISPLAYCONFIGXML
  extern int getdisplay(unsigned int lcdid);
#endif

/* ======================================================================
 * Helper function indicating if the buffer contents define a valid 
 * factory section, format >= 3. Returns 1 if valid, 0 otherwise.
 * ====================================================================== */
int isvalidfactory3(unsigned char* buf)
{
  unsigned char cksum;
  int i;
  
  /* Version check; failure if version < 3 */
  if(buf[VERSION_POS] < 3)     
    return 0;
    
  /* Checksum check */  
  cksum = 1;
  for(i = CKSUM_POS + 1; i < FACTORY_SECTION_SIZE_V3; i ++)
    cksum += buf[i];
  cksum -= 0xaa;
  
  if(buf[CKSUM_POS] != cksum)
    return 0;
  
  /* Signature check */
  if( (buf[SIGNATURE1_POS] != SIGNATURE1_VAL) || (buf[SIGNATURE2_POS] != SIGNATURE2_VAL) )
    return 0;
  
  /* It is a valid factory version 3 */
  return 1;
}


/* ======================================================================
 * Helper function returning the factory section area of the selected i2c
 * seeprom device.
 * In case of seeprom format >=3, the proper alignment/handling of the
 * safe factory section area is performed transparently.
 * Returns 1 in case of i2c error.
 * ====================================================================== */
int getfactorysection(unsigned char i2cslaveaddr, unsigned char* buf)
{
  unsigned char safefactory_buf[FACTORY_SECTION_SIZE_V3];
  int i;
  
  i2c_init(CONFIG_SYS_I2C_SPEED, i2cslaveaddr);
  
  /* Reads the factory section from seeprom */
  if (i2c_read(i2cslaveaddr, 0, 1, buf, FACTORY_SECTION_SIZE_V3) != 0)
  {
    puts ("I2C error reading the factory section\n");
    return 1;
  }
  
  /* Reads the safe factory section from seeprom */
  if (i2c_read(i2cslaveaddr, SAFE_FACTORY_AREA_POS, 1, safefactory_buf, FACTORY_SECTION_SIZE_V3) != 0)
  {
    puts ("I2C error reading the factory section\n");
    return 1;
  }
  
  /* Performs synchronization/handling of the safe factory section if we detect a format >= 3 */
  if(isvalidfactory3(buf))   //If we have a valid v3 factory section area...
  {
    /* ...copy factory section to safe factory section, wherever any difference is found  */
    for(i = 0; i < FACTORY_SECTION_SIZE_V3; i ++)
      if(buf[i] != safefactory_buf[i])
      {
	i2c_write (i2cslaveaddr, SAFE_FACTORY_AREA_POS + i, 1, &(buf[i]), 1);
	udelay(20000);
      }
	
  }
  else                       // else: The factory section area is not a valid version 3 one ...
  {
    if(isvalidfactory3(safefactory_buf)) //...if the safe factory section is a valid version 3 one
    {                                    //...copy the safe area to the factory area (recovery)
      for(i = 0; i < FACTORY_SECTION_SIZE_V3; i ++)
	if(buf[i] != safefactory_buf[i])
	{
	  buf[i]= safefactory_buf[i];
	  if(i2c_write (i2cslaveaddr, i, 1, &(buf[i]), 1))
	  {
	    puts ("I2C error writing the factory section\n");
	    return 1;
	  }
	  udelay(20000);
	}
    }
    else // In this case neither the factory section nor the safe factory section areas are valid format 3... assume an old version 2 is 
    {    // used. Just read the factory section assuming a version 2 size and leave the upper levels to validate its contents.
      if (i2c_read(i2cslaveaddr, 0, 1, buf, FACTORY_SECTION_SIZE_V2) != 0)
      {
	puts ("I2C error reading the factory section:2\n");
	return 1;
      }
    }
  }
  
  return 0;
}


/* ======================================================================
 * Perform HW cfg load from I2C SEEPROM to env vars
 * ====================================================================== */
int i2cgethwcfg (void)
{
  unsigned char buf[FACTORY_SECTION_SIZE_V2];
  unsigned char adpbuf[FACTORY_SECTION_SIZE_V2];
  unsigned char cksum;
  int i, j;
  char label[40];
  
  puts ("Loading configuration from I2C SEEPROM\n");
  
  /* Reads the I2C contents */
  if(getfactorysection(CONFIG_SYS_I2C_EEPROM_ADDR, buf))
  {
	puts ("ERROR: I2C SEEPROM read error\n");	
	return 1;
  }
  
  /* Checksum check */
  if(buf[VERSION_POS] < 3) 
	j = FACTORY_SECTION_SIZE_V2;
  else
	j = FACTORY_SECTION_SIZE_V3;
  
  cksum = 1;
  for(i = CKSUM_POS + 1; i < j; i ++)
	cksum += buf[i];
  cksum -= 0xaa;
  
  if(buf[CKSUM_POS] != cksum)
  {
	puts ("ERROR: I2C SEEPROM checksum error: HW cfg not valid\n");
	return 1;
  }
  
  /* Signature check */
  if( (buf[SIGNATURE1_POS] != SIGNATURE1_VAL) || (buf[SIGNATURE2_POS] != SIGNATURE2_VAL) )
  {
	puts ("ERROR: I2C SEEPROM invalid signature: HW cfg not valid\n");
	return 1;
  }
	
  /* Get display number and touch type */
#ifdef CONFIG_SYS_I2C_ADPADD
  /* If display ID=0xff, get them from the ADP board */
  if(buf[DISPID_POS] == 0xff)
  {
    if(getfactorysection(CONFIG_SYS_I2C_ADPADD, adpbuf))
      return 1;
    
    buf[DISPID_POS] = adpbuf[DISPID_POS];
    buf[TCHTYPE_POS] = adpbuf[TCHTYPE_POS];
  }
#endif
  sprintf(label, "%u", buf[DISPID_POS]);
  setenv("hw_dispid", label); 
  sprintf(label, "%u", buf[TCHTYPE_POS]);
  setenv("touch_type", label); 
  #ifdef CONFIG_CMD_DISPLAYCONFIGXML
  getdisplay(buf[DISPID_POS]);
#endif
  
  /* *** Now gets datas for version >= 2 of the SEEPROM *** */
  if(buf[VERSION_POS] < 2) 
  {
	puts ("I2C SEEPROM WARNING: old version 1 found\n");
	return 1;
  }

  /* get eth mac ID */
  eth_setenv_enetaddr("ethaddr", &(buf[MACID0_POS]));
  
  /* get boardhwcode */
  sprintf(label, "%u", buf[HWPICKPANELCODE_POS]);
  setenv("hw_code", label); 
  
  /* get jumperflagsl */
  sprintf(label, "%u", buf[JUMPERFLAGSL_POS]);
  setenv("jumperflagsl", label); 

  /* get 2nd eth mac ID */
  eth_setenv_enetaddr("eth1addr", &(buf[AUXMACID0_POS]));
 
  return 0;
}
  

/* ======================================================================
 * Performs the HW cfg store to I2C SEEPROM
 * ====================================================================== */
int do_i2csavehw ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  unsigned char buf[FACTORY_SECTION_SIZE_V3];
  unsigned long i;
  char* tmp;
  unsigned char dispid;
  unsigned char hwcode;
  unsigned char tchtype;
  u8 hw_addr[ENET_ADDR_LENGTH];
  unsigned char cksum;
  unsigned char n;

  if (argc>1) 
  {
	puts ("ERROR: Too many input parameters!\n");
	return 1;
  }
  
  /* Initializes the buffer ... uses still existing SEEPROM datas, if valid */

  /* Reads the I2C contents ...*/
  i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1, buf, FACTORY_SECTION_SIZE_V3);
  /* ...and verifies chksum for format 3*/
  cksum = 1;
  for(i = CKSUM_POS + 1; i < FACTORY_SECTION_SIZE_V3; i ++)
	cksum += buf[i];
  cksum -= 0xaa;
  
  if(buf[CKSUM_POS] != cksum)
  {
	puts ("WARNING: No SEEPROM format 3 found ... initializing  ...\n");
	for(i=0; i < FACTORY_SECTION_SIZE_V3; i++)
	  buf[i] = RFU_VAL;
  }
  
  /* Sets the pre-defined fields of buffer */
  buf[SIGNATURE1_POS] = SIGNATURE1_VAL;
  buf[SIGNATURE2_POS] = SIGNATURE2_VAL;
  buf[VERSION_POS] = VERSION_VAL;
  
  /* Now copy into buffer the datas from env vars */ 
  
  /* Display ID handling */
  tmp = getenv("hw_dispid");
  if(!tmp)
  {
	puts ("ERROR: 'hw_dispid' environment var not found!\n");
	return 1;
  }
  i = simple_strtoul (tmp, NULL, 10);
  dispid = (unsigned char)(i & 0xff);
  buf[DISPID_POS] = dispid;
  
  /* Eth mac address handling */
  if (eth_getenv_enetaddr("ethaddr", hw_addr))
  {
	// hw_addr is in the format 00:11:22:33:44:55     
	buf[MACID0_POS] = hw_addr[0];
	buf[MACID1_POS] = hw_addr[1];
	buf[MACID2_POS] = hw_addr[2];
	buf[MACID3_POS] = hw_addr[3];
	buf[MACID4_POS] = hw_addr[4];
	buf[MACID5_POS] = hw_addr[5];
  }
  else
  {
	puts ("ERROR: 'ethaddr' environment var not found!\n");
	return 1;
  }

  /* Second th mac address handling */
  if (eth_getenv_enetaddr("eth1addr", hw_addr))
  {
	buf[AUXMACID0_POS] = hw_addr[0];
	buf[AUXMACID1_POS] = hw_addr[1];
	buf[AUXMACID2_POS] = hw_addr[2];
	buf[AUXMACID3_POS] = hw_addr[3];
	buf[AUXMACID4_POS] = hw_addr[4];
	buf[AUXMACID5_POS] = hw_addr[5];
  }
  else
  {
	puts ("WARNING: 'eth1addr' environment var not found!\n");
  }

  /* hw_code handling */
  tmp = getenv("hw_code");
  if(!tmp)
  {
    puts ("WARNING: 'hw_code' environment var not found!\n");
  }
  else
  {
    i = simple_strtoul (tmp, NULL, 10);
    hwcode = (unsigned char)(i & 0xff);
    buf[HWPICKPANELCODE_POS] = hwcode;
  }
  
  /* touch_type handling */
  tmp = getenv("touch_type");
  if(!tmp)
  {
    puts ("WARNING: 'touch_type' environment var not found!\n");
  }
  else
  {
    i = simple_strtoul (tmp, NULL, 10);
    tchtype = (unsigned char)(i & 0xff);
    buf[TCHTYPE_POS] = tchtype;
  }

  /* Calculate and copies checksum into buffer */
  cksum = 1;
  for(i = CKSUM_POS + 1; i < FACTORY_SECTION_SIZE_V3; i ++)
	cksum += buf[i];
  cksum -= 0xaa;
  
  buf[CKSUM_POS] = cksum;

  /* Now write buffer into SEEPROM */
  puts ("Writing HW configuration to I2C SEEPROM ...\n");
  for(n=0; n < FACTORY_SECTION_SIZE_V3; n++)
  {
      i2c_write (CONFIG_SYS_I2C_EEPROM_ADDR, n, 1, &(buf[n]), 1);
      udelay(20000);
  }
  
  return 0;
}


/* ====================================================================== */

U_BOOT_CMD(
	i2csavehw,      2,      0,      do_i2csavehw,
	" Stores display id and MAC id into I2C SEEPROM (format v3) keeping other params unchanged.\n",
	" If no valid SEEPROM 3 datas are found, SEEPROM is initialized with defaults\n"
);


#endif	/* CONFIG_CMD_I2CHWCFG */

