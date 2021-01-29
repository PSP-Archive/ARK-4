/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <stdio.h>
#include <string.h>
#include <pspinit.h>
#include "systemctrl.h"
#include "macros.h"

static const int g_cpu_list[] = {
	20, 75, 100, 133, 166, 222, 266, 300, 333
};

static int g_fake_pll = 222;
static int g_fake_cpu = 222;
static int g_fake_bus = 111;

typedef struct _PowerFuncRedir {
	u32 nid;
	void *fp;
} PowerFuncRedir;

static u32 g_scePowerSetClockFrequency_orig;

static int myPowerGetPllClockFrequencyInt(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_pll;
}

static float myPowerGetPllClockFrequencyFloat(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_pll;
}

static int myPowerGetCpuClockFrequency(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_cpu;
}

static float myPowerGetCpuClockFrequencyFloat(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_cpu;
}

static int myPowerGetBusClockFrequency(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_bus;
}

static float myPowerGetBusClockFrequencyFloat(void)
{
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return g_fake_bus;
}

static int myPowerSetClockFrequency(int pllfreq, int cpufreq, int busfreq)
{
	g_fake_pll = pllfreq;
	g_fake_cpu = cpufreq;
	g_fake_bus = busfreq;
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return 0;
}

static int myPowerSetCpuClockFrequency(int cpufreq)
{
	g_fake_cpu = cpufreq;
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);

	return 0;
}

static int myPowerSetBusClockFrequency(int busfreq)
{
	g_fake_bus = busfreq;
	printk("%s: %d/%d/%d\n", __func__, g_fake_pll, g_fake_cpu, g_fake_bus);
	
	return 0;
}

static inline u32 find_power_function(u32 nid)
{
	return sctrlHENFindFunction("scePower_Service", "scePower", nid);
}

static PowerFuncRedir g_power_func_redir[] = {
	{ 0x737486F2, myPowerSetClockFrequency },
	{ 0x545A7F3C, myPowerSetClockFrequency },
	{ 0xEBD177D6, myPowerSetClockFrequency },
	{ 0xA4E93389, myPowerSetClockFrequency },
	{ 0x469989AD, myPowerSetClockFrequency },
	{ 0x843FBF43, myPowerSetCpuClockFrequency },
	{ 0xB8D7B3FB, myPowerSetBusClockFrequency },
	{ 0x34F9C463, myPowerGetPllClockFrequencyInt },
	{ 0xEA382A27, myPowerGetPllClockFrequencyFloat },
	{ 0xFEE03A2F, myPowerGetCpuClockFrequency },
	{ 0xB1A52C83, myPowerGetCpuClockFrequencyFloat },
	{ 0x478FE6F5, myPowerGetBusClockFrequency },
	{ 0x9BADB3EB, myPowerGetBusClockFrequencyFloat },
};

void SetSpeed(int cpuspd, int busspd)
{
	int (*_scePowerSetClockFrequency)(int, int, int);
	u32 fp;
	int i;

	for(i=0; i<NELEMS(g_cpu_list); ++i) {
		if(cpuspd == g_cpu_list[i])
			break;
	}

	if(i >= NELEMS(g_cpu_list)) {
		return;
	}

	fp = find_power_function(0x737486F2);
	g_scePowerSetClockFrequency_orig = fp;
	_scePowerSetClockFrequency = (void*)fp;
	_scePowerSetClockFrequency(cpuspd, cpuspd, busspd);

	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_VSH) {
		hookImportByNID(sceKernelFindModuleByName("vsh_module"), "scePower", 0x469989AD, NULL);
		return;
	}

	for(i=0; i<NELEMS(g_power_func_redir); ++i) {
		fp = find_power_function(g_power_func_redir[i].nid);

		if(fp != 0) {
			REDIRECT_FUNCTION(g_power_func_redir[i].fp, fp);
		} else {
			printk("%s: scePower_%08X not found\n", __func__, (uint)g_power_func_redir[i].nid);
		}
	}
}

void sctrlHENSetSpeed(int cpuspd, int busspd)
{
	int (*_scePowerSetClockFrequency)(int, int, int);
	g_scePowerSetClockFrequency_orig = find_power_function(0x545A7F3C); /* scePowerSetClockFrequency */
	_scePowerSetClockFrequency = (void *) g_scePowerSetClockFrequency_orig;
	_scePowerSetClockFrequency(cpuspd, cpuspd, busspd);
}
