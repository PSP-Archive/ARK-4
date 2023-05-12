#include <psptypes.h>
#include <string.h>

#include "sysreg.h"
#include "syscon.h"
#include "kirk.h"

int sceSysconInit(void)
{
	sceSysregSpiClkSelect(0,1);

	sceSysregSpiClkEnable(0);

	//sceSysregSpiIoEnable(0)
	REG32(0xbc100078) |= (0x1000000<<0);

	// init SPI
	REG32(0xbe580000) = 0xcf;
	REG32(0xbe580004) = 0x04;
	REG32(0xbe580014) = 0;
	REG32(0xbe580024) = 0;

	// sceGpioPortClear
	REG32(0xbe24000c) = 0x08;

	// GPIO3 OUT
	REG32(0xbe240000) |= 0x08;
	REG32(0xBE240040) &= ~0x08;
	REG32(0xBC10007C) |= 0x08;
	
	// GPIO4 IN
	REG32(0xbe240000) &= ~0x10;
	REG32(0xBE240040) |= 0x10;
	REG32(0xBC10007C) |= 0x10;

	// GpioSetIntrMode(4,3)
	REG32(0xbe240010) &= ~0x10;
	REG32(0xbe240014) &= ~0x10;
	REG32(0xbe240018) |=  0x10;
	REG32(0xbe240024) = 0x10;

	return 0;
}

int Syscon_cmd(u8 *tx_buf,u8 *rx_buf)
{
	vu32 dmy;
	int result = 0;
	u16 wdata;
	u8 bdata;
	int cnt;
	u8 sum;
	int i;

retry:
	// calc & set TX sum
	sum = 0;
	cnt = tx_buf[1];
	for(i=0;i<cnt;i++)
		sum += tx_buf[i];
	tx_buf[cnt] = ~sum;

	// padding TX buf
	for(i=cnt+1;i<0x10;i++)
		tx_buf[i] = 0xff;

	// clear RX buf
	for(i=0x0f;i>=0;i--)
		rx_buf[i]=0xff;


	// sceGpioPortRead();
	dmy = REG32(0xbe240004);

	// sceGpioPortClear(8)
	REG32(0xbe24000c) = 0x08;

	if(REG32(0xbe58000c) & 4)
	{
		while(REG32(0xbe58000c) & 4)
		{
			dmy = REG32(0xbe580008);
		}
	}
	
	dmy = REG32(0xbe58000c);
	REG32(0xbe580020) = 3;

	for(i=0;i<(cnt+1);i+=2)
	{
		dmy = REG32(0xbe58000c);
		REG32(0xbe580008) = (tx_buf[i]<<8) | tx_buf[i + 1];
	}

	REG32(0xbe580004) = 6; // RX mode ?

	// sceGpioPortSet(8)
	REG32(0xbe240008) = 0x08;

	while((REG32(0xbe240020) & 0x10) == 0);

	REG32(0xbe240024) = 0x10;

//--------------------------------------------------------------
//receive
//--------------------------------------------------------------

	if( (REG32(0xbe58000c) & 4)==0)
	{
		rx_buf[0] = 0xff;
		result = -1;

		for(cnt=0x0f;cnt;cnt--);
	}

	if( (REG32(0xbe58000c) & 1)==0)
	{
		result = -1;
	}

	if( REG32(0xbe580018) & 1)
	{
		REG32(0xbe580020) = 1;
	}

	for(i=0;i<0x10;i+=2)
	{
		if( (REG32(0xbe58000c) & 4)==0)
			break;

		wdata = REG32(0xbe580008);
		bdata = wdata>>8;
		if(i == 0)
		{
			result = bdata;
		}
		rx_buf[i] = bdata;
		rx_buf[i + 1] = wdata & 0xff;
	}

	REG32(0xbe580004) = 4;

	REG32(0xbe24000c) = 0x08;

	if(result>0)
	{
		cnt = rx_buf[1];
		if(cnt < 3)
		{
			result = -2;
		}
		else
		{
			sum = 0;
			for(i=0;i<cnt;i++)
				sum += rx_buf[i];

			if( (sum^0xff) != rx_buf[cnt])
			{
				result = -2; // check sum error
			}
		}
	}

	switch(rx_buf[2])
	{
	case 0x80:
	case 0x81:
		goto retry;
	}

	return result;
}

int sceSysconCommonWrite(u32 param,u8 cmd,u8 tx_len)
{
	u8 tx_buf[0x10],rx_buf[0x10];

	tx_buf[0] = cmd;
	tx_buf[1] = tx_len;
	tx_buf[2] = (u8)param;
	tx_buf[3] = (u8)(param>>8);
	tx_buf[4] = (u8)(param>>16);
	tx_buf[5] = (u8)(param>>24);
	return Syscon_cmd(tx_buf,rx_buf);
}

int sceSysconCommonRead(u32 *param,u8 cmd)
{
	u8 tx_buf[0x10],rx_buf[0x10];
	int result;

	tx_buf[0] = cmd;
	tx_buf[1] = 2;

	result = Syscon_cmd(tx_buf,rx_buf);
	if(result >= 0 && param)
	{
		switch(rx_buf[1])
		{
		case 4: *param = rx_buf[3]; break;
		case 5: *param = rx_buf[3]|(rx_buf[4]<<8); break;
		case 6: *param = rx_buf[3]|(rx_buf[4]<<8)|(rx_buf[5]<<16); break;
		default:
			*param = rx_buf[3]|(rx_buf[4]<<8)|(rx_buf[5]<<16)|(rx_buf[6]<<24);
		}
	}
	return result;
}

u32 Syscon_wait(u32 usec)
{
	u32 i;
	vu32 dmy = 0;

	while(usec--)
	{
		for(i=0;i<10;i++)
		{
			dmy ^= REG32(0xbe240000);
		}
	}
	return dmy;
}

/*
  LED power controll

*/
int sceSysconCtrlLED(int sel,int is_on)
{
	u32 param;

	param = (is_on==0) ? 0xf0 : 0x00;
	if(sel==1)
	{
		param += 0x90;
	}
	else
	{
		param += 0x50;
		if(sel!=0)
		{
			param = (is_on==0) ? 0 : 0xf0;
			param = (-is_on);
			param += 0x30;
		}
	}
	return sceSysconCommonWrite(param,0x47,0x03);
}

int pspSysconGetCtrl2(u32 *ctrl,u8 *vol1,u8 *vol2)
{
	u8 tx_buf[0x10],rx_buf[0x10];
	int result;

	tx_buf[0] = 0x08;
	tx_buf[1] = 2;
	result = Syscon_cmd(tx_buf,rx_buf);
	*ctrl = rx_buf[3]|(rx_buf[4]<<8)|(rx_buf[5]<<16)|(rx_buf[6]<<24);
	*vol1  = rx_buf[7];
	*vol2  = rx_buf[8];
	return result;
}

int pspSysconSendAuth(u8 key, u8 *data)
{
	u8 tx_buf[0x10],rx_buf[0x10];

	tx_buf[0] = 0x30;
	tx_buf[1] = 2 + 1 + 8;
	tx_buf[2] = key;
	memcpy(&tx_buf[3], data, 8);
	int result = Syscon_cmd(tx_buf,rx_buf);
	if (result < 0)
		return result;

	tx_buf[0] = 0x30;
	tx_buf[1] = 2 + 1 + 8;
	tx_buf[2] = key + 1;
	memcpy(&tx_buf[3], &data[8], 8);
	result = Syscon_cmd(tx_buf,rx_buf);
	if (result < 0)
		return result;
		
	return 0;
	
}

int pspSysconRecvAuth(u8 key, u8 *data)
{
	u8 tx_buf[0x10],rx_buf[0x10];

	tx_buf[0] = 0x30;
	tx_buf[1] = 2 + 1;
	tx_buf[2] = key;
	int result = Syscon_cmd(tx_buf,rx_buf);
	if (result < 0)
		return result;
	
	memcpy(data, &rx_buf[4], 8);

	tx_buf[0] = 0x30;
	tx_buf[1] = 2 + 1;
	tx_buf[2] = key + 1;
	result = Syscon_cmd(tx_buf,rx_buf);
	if (result < 0)
		return result;
	
	memcpy(&data[8], &rx_buf[4], 8);
		
	return 0;
}

int seed_gen1(u8 *random_key, u8 *random_key_dec_resp_dec)
{
	memset(random_key, 0xAA, 16);
	
	u8 random_key_dec[16];
	int ret = kirkDecryptAes(random_key_dec, random_key, 16, 0x69);
	if (ret)
		return ret;
	
	ret = pspSysconSendAuth(0x80, random_key_dec);
	if (ret)
		return ret;
	
	u8 random_key_dec_resp[16];
	ret = pspSysconRecvAuth(0, random_key_dec_resp);
	if (ret)
		return ret;
	
	ret = kirkDecryptAes(random_key_dec_resp_dec, random_key_dec_resp, 16, 0x14);
	if (ret)
		return ret;
		
	u8 random_key_dec_resp_dec_swapped[16];
	memcpy(random_key_dec_resp_dec_swapped, &random_key_dec_resp_dec[8], 8);
	memcpy(&random_key_dec_resp_dec_swapped[8], random_key_dec_resp_dec, 8);
	
	u8 seed_dec_resp_dec_hi_low_swapped_dec[16];
	ret = kirkDecryptAes(seed_dec_resp_dec_hi_low_swapped_dec, random_key_dec_resp_dec_swapped, 16, 0x69);
	if (ret)
		return ret;

	ret = pspSysconSendAuth(0x82, seed_dec_resp_dec_hi_low_swapped_dec);
	if (ret)
		return ret;

	return 0;
}

void xor(u8 *dest, u8 *src_a, u8 *src_b)
{
	for (int i = 0; i < 16; i++)
		dest[i] = src_a[i] ^ src_b[i];
}

int seed_gen2(u8 *rand_xor, u8 *key_86, u8 *random_key, u8 *random_key_dec_resp_dec)
{
	u8 random_key_xored[16];
	xor(random_key_xored, random_key, rand_xor);
	
	int ret = kirkDecryptAes(random_key_xored, random_key_xored, 16, 0x15);
	if (ret)
		return ret;
		
	u8 random_key_dec_resp_dec_xored[16];
	xor(random_key_dec_resp_dec_xored, random_key_dec_resp_dec, random_key_xored);
	
	ret = pspSysconSendAuth(0x84, random_key_dec_resp_dec_xored);
	if (ret)
		return ret;
		
	ret = pspSysconSendAuth(0x86, key_86);
	if (ret)
		return ret;

	u8 resp_2[16];
	ret = pspSysconRecvAuth(2, resp_2);
	if (ret)
		return ret;

	u8 resp_4[16];
	ret = pspSysconRecvAuth(4, resp_4);
	if (ret)
		return ret;
	
	return 0;
}