/*
 * alc1312.c  --  ALC1312 ALSA SoC audio codec driver
 *
 * Copyright 2015 Realtek Semiconductor Corp.
 * Author: Bard Liao <bardliao@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <asm/div64.h>
#include "alc1312.h"

#include "ipq40xx-adss.h"

#define ALC1312_I2C_ADDR	0x54

extern void rt5616_set_data_bypass(unsigned char onoff);
extern void alc1312_pdb_ctrl(unsigned char onoff);

struct alc1312_init_reg {
	u16 reg;
	u16 val;
	u8 delay;
};

struct alc1312_eq_reg {
	u8 level;
	u16 reg;
	u16 val;
};

static struct alc1312_init_reg init_list[] = {
#if 1
	/////////////////initial
	{ 0x0000, 0xFFFF, 0 },
	//{ 0x0004, 0x1000, 0 },

	// enable RC clock
	{ 0x0004, 0xC000, 0 },

	{ 0x0006, 0x3570, 0 },
	{ 0x0008, 0x3000, 0 },
	{ 0x000A, 0x105E, 0 },
	{ 0x000C, 0x0000, 0 },
	{ 0x0322, 0x400F, 0 },
	{ 0x0326, 0x000F, 0 },
	{ 0x0350, 0x0188, 0 },
	{ 0x0324, 0xC0A0, 0 },
	{ 0x006A, 0x0510, 0 },
	{ 0x006C, 0x2500, 0 },
	{ 0x0102, 0x200C, 0 },	//Soft Volume
	{ 0x0103, 0x033E, 0 },	//Master Volume L = 0dB
	{ 0x0104, 0x033E, 0 },	//Master Volume L = 0dB
	{ 0x0106, 0x9F93, 0 },	//DAGAIN = -12dB -15dB
	{ 0x0300, 0xB803, 0 },  //400K HV32dB
	{ 0x006A, 0x0002, 0 },
	{ 0x006C, 0x6505, 0 },
	{ 0x006A, 0x0300, 0 },
	{ 0x006C, 0x0145, 0 },
	{ 0x006A, 0x0302, 0 },
	{ 0x006C, 0x2802, 0 },
	{ 0x006A, 0x030C, 0 },
	{ 0x006C, 0x0FC0, 0 },
	{ 0x006A, 0x010C, 0 },
	{ 0x006C, 0xC000, 0 },
	// Turn on Efuse & Auto Readback mode
	{ 0x0326, 0x00AF, 0 },
	{ 0x0326, 0x00FF, 0 },
	{ 0x0080, 0x8000, 0 },
	{ 0x0080, 0x8001, 0 },
	{ 0x006A, 0x0220, 0 },
	{ 0x006C, 0x0000, 0 },

	// Turn off Efuse
	{ 0x0326, 0x00AF, 0 },
	{ 0x0326, 0x000F, 500 },
	// Delay 0.5

	// enable PLL
	{ 0x0004, 0x1000, 0 },

	// Turn on PWM
	{ 0x006A, 0x0216, 0 }, // DC detect H_threshold
	{ 0x006C, 0x0470, 0 },
	{ 0x006A, 0x0217, 0 }, // DC detect L_threshold
	{ 0x006C, 0x0400, 0 },
	{ 0x0214, 0x8000, 0 }, // DC detect enable
	{ 0x0350, 0x0108, 0 },

	//L ch 500 LPF, Rch 500HPF
	{ 0x0810, 0x0000, 0},
	{ 0x0811, 0x278C, 0},
	{ 0x0812, 0x0000, 0},
	{ 0x0813, 0x4F17, 0},
	{ 0x0814, 0x0000, 0},
	{ 0x0815, 0x278C, 0},
	{ 0x0816, 0x030D, 0},
	{ 0x0817, 0x00E7, 0},
	{ 0x0818, 0x0073, 0},
	{ 0x0819, 0x9D47, 0},
	{ 0x081A, 0x0079, 0},
	{ 0x081B, 0xA718, 0},
	{ 0x081C, 0x030C, 0},
	{ 0x081D, 0xB1D0, 0},
	{ 0x081E, 0x0079, 0},
	{ 0x081F, 0xA718, 0},
	{ 0x0820, 0x030D, 0},
	{ 0x0821, 0x00E7, 0},
	{ 0x0822, 0x0073, 0},
	{ 0x0823, 0x9D47, 0},
	{ 0x0800, 0x800C, 0}, //Bq1 enable      
#endif
        { 0x0600 ,0x7000, 0},
        { 0x0611 ,0xDF5F, 0},
        { 0x0612 ,0x033E, 0},
        { 0x0613 ,0x033E, 0},
        { 0x0614 ,0x0300, 0},
        { 0x0616 ,0x0506, 0},
        { 0x0621 ,0xDF5F, 0},
        { 0x0622 ,0x033E, 0},
        { 0x0623 ,0x033E, 0},
        { 0x0624 ,0x0300, 0},
        { 0x0626 ,0x0506, 0},
        { 0x0631 ,0xDF5F, 0},
        { 0x0632 ,0x033E, 0},
        { 0x0633 ,0x033E, 0},
        { 0x0634 ,0x0300, 0},
        { 0x0636 ,0x0506, 0},
        { 0x0641 ,0xDF5F, 0},
        { 0x0644 ,0x4040, 0},
        { 0x0645 ,0x4004, 0},
        { 0x0646 ,0x0506, 0},
        { 0x0700 ,0x8000, 0},
        { 0x0701 ,0x0800, 0},
        { 0x0702 ,0x0800, 0},
        { 0x0703 ,0x0800, 0},
        { 0x0704 ,0x0800, 0},
        { 0x0705 ,0x0800, 0},
        { 0x0706 ,0x0800, 0},
        { 0x0707 ,0x0800, 0},
        { 0x0708 ,0x0800, 0},
        { 0x0711 ,0x0000, 0},
        { 0x0712 ,0x0000, 0},
        { 0x0713 ,0x0000, 0},
        { 0x0714 ,0x0000, 0},
        { 0x0715 ,0x0000, 0},
        { 0x0721 ,0x0000, 0},
        { 0x0722 ,0x0000, 0},
        { 0x0723 ,0x0000, 0},
        { 0x0724 ,0x0000, 0},
        { 0x0725 ,0x0000, 0},
        { 0x0731 ,0x0000, 0},
        { 0x0732 ,0x0000, 0},
        { 0x0733 ,0x0000, 0},
        { 0x0734 ,0x0000, 0},
        { 0x0735 ,0x0000, 0},
        { 0x0741 ,0x0000, 0},
        { 0x0742 ,0x0000, 0},
        { 0x0743 ,0x0000, 0},
        { 0x0744 ,0x0000, 0},
        { 0x0745 ,0x0000, 0},
        { 0x0802 ,0x0000, 0},
        { 0x0803 ,0x0000, 0},
        { 0x0806 ,0x0000, 0},
        { 0x0808 ,0x0000, 0},
        { 0x0809 ,0x0000, 0},
        { 0x080A ,0x0000, 0},
        { 0x080B ,0x0000, 0},
        { 0x0810 ,0x0080, 0},
        { 0x0811 ,0x0000, 0},
        { 0x0812 ,0x0000, 0},
        { 0x0813 ,0x0000, 0},
        { 0x0814 ,0x0000, 0},
        { 0x0815 ,0x0000, 0},
        { 0x0816 ,0x0000, 0},
        { 0x0817 ,0x0000, 0},
        { 0x0818 ,0x0000, 0},
        { 0x0819 ,0x0000, 0},
        { 0x081A ,0x0080, 0},
        { 0x081B ,0x0000, 0},
        { 0x081C ,0x0000, 0},
        { 0x081D ,0x0000, 0},
        { 0x081E ,0x0000, 0},
        { 0x081F ,0x0000, 0},
        { 0x0820 ,0x0000, 0},
        { 0x0821 ,0x0000, 0},
        { 0x0822 ,0x0000, 0},
        { 0x0823 ,0x0000, 0},
        { 0x0824 ,0x0002, 0},
        { 0x0825 ,0x2634, 0},
        { 0x0826 ,0x0004, 0},
        { 0x0827 ,0x4C68, 0},
        { 0x0828 ,0x0002, 0},
        { 0x0829 ,0x2634, 0},
        { 0x082A ,0x0333, 0},
        { 0x082B ,0x6791, 0},
        { 0x082C ,0x0055, 0},
        { 0x082D ,0x313F, 0},
        { 0x082E ,0x0080, 0},
        { 0x082F ,0x0000, 0},
        { 0x0830 ,0x0000, 0},
        { 0x0831 ,0x0000, 0},
        { 0x0832 ,0x0000, 0},
        { 0x0833 ,0x0000, 0},
        { 0x0834 ,0x0000, 0},
        { 0x0835 ,0x0000, 0},
        { 0x0836 ,0x0000, 0},
        { 0x0837 ,0x0000, 0},
        { 0x0838 ,0x0080, 0},
        { 0x0839 ,0x4B3C, 0},
        { 0x083A ,0x0311, 0},
        { 0x083B ,0xEA30, 0},
        { 0x083C ,0x006E, 0},
        { 0x083D ,0xFAF6, 0},
        { 0x083E ,0x0311, 0},
        { 0x083F ,0xEA30, 0},
        { 0x0840 ,0x006F, 0},
        { 0x0841 ,0x4633, 0},
        { 0x0842 ,0x0080, 0},
        { 0x0843 ,0x4B3C, 0},
        { 0x0844 ,0x0311, 0},
        { 0x0845 ,0xEA30, 0},
        { 0x0846 ,0x006E, 0},
        { 0x0847 ,0xFAF6, 0},
        { 0x0848 ,0x0311, 0},
        { 0x0849 ,0xEA30, 0},
        { 0x084A ,0x006F, 0},
        { 0x084B ,0x4633, 0},
        { 0x084C ,0x0080, 0},
        { 0x084D ,0x0000, 0},
        { 0x084E ,0x0000, 0},
        { 0x084F ,0x0000, 0},
        { 0x0850 ,0x0000, 0},
        { 0x0851 ,0x0000, 0},
        { 0x0852 ,0x0000, 0},
        { 0x0853 ,0x0000, 0},
        { 0x0854 ,0x0000, 0},
        { 0x0855 ,0x0000, 0},
        { 0x0856 ,0x0077, 0},
        { 0x0857 ,0x3446, 0},
        { 0x0858 ,0x0311, 0},
        { 0x0859 ,0x9775, 0},
        { 0x085A ,0x0077, 0},
        { 0x085B ,0x3446, 0},
        { 0x085C ,0x0312, 0},
        { 0x085D ,0x2F7A, 0},
        { 0x085E ,0x006F, 0},
        { 0x085F ,0x0090, 0},
        { 0x0860 ,0x003A, 0},
        { 0x0861 ,0x812C, 0},
        { 0x0862 ,0x03B6, 0},
        { 0x0863 ,0x1745, 0},
        { 0x0864 ,0x001B, 0},
        { 0x0865 ,0x5452, 0},
        { 0x0866 ,0x033D, 0},
        { 0x0867 ,0x5590, 0},
        { 0x0868 ,0x004E, 0},
        { 0x0869 ,0x9733, 0},
        { 0x086A ,0x0082, 0},
        { 0x086B ,0x8879, 0},
        { 0x086C ,0x0348, 0},
        { 0x086D ,0x4733, 0},
        { 0x086E ,0x0047, 0},
        { 0x086F ,0x2219, 0},
        { 0x0870 ,0x034C, 0},
        { 0x0871 ,0xAA15, 0},
        { 0x0872 ,0x0045, 0},
        { 0x0873 ,0x47B0, 0},
        { 0x0874 ,0x0080, 0},
        { 0x0875 ,0x0000, 0},
        { 0x0876 ,0x0000, 0},
        { 0x0877 ,0x0000, 0},
        { 0x0878 ,0x0000, 0},
        { 0x0879 ,0x0000, 0},
        { 0x087A ,0x0000, 0},
        { 0x087B ,0x0000, 0},
        { 0x087C ,0x0000, 0},
        { 0x087D ,0x0000, 0},
        { 0x087E ,0x0080, 0},
        { 0x087F ,0x0000, 0},
        { 0x0880 ,0x0000, 0},
        { 0x0881 ,0x0000, 0},
        { 0x0882 ,0x0000, 0},
        { 0x0883 ,0x0000, 0},
        { 0x0884 ,0x0000, 0},
        { 0x0885 ,0x0000, 0},
        { 0x0886 ,0x0000, 0},
        { 0x0887 ,0x0000, 0},
        { 0x0888 ,0x000A, 0},
        { 0x0889 ,0x9C43, 0},
        { 0x088A ,0x0015, 0},
        { 0x088B ,0x3886, 0},
        { 0x088C ,0x000A, 0},
        { 0x088D ,0x9C43, 0},
        { 0x088E ,0x037B, 0},
        { 0x088F ,0xEBFE, 0},
        { 0x0890 ,0x002E, 0},
        { 0x0891 ,0x850E, 0},
        { 0x0892 ,0x0080, 0},
        { 0x0893 ,0x0000, 0},
        { 0x0894 ,0x0000, 0},
        { 0x0895 ,0x0000, 0},
        { 0x0896 ,0x0000, 0},
        { 0x0897 ,0x0000, 0},
        { 0x0898 ,0x0000, 0},
        { 0x0899 ,0x0000, 0},
        { 0x089A ,0x0000, 0},
        { 0x089B ,0x0000, 0},
        { 0x089C ,0x0080, 0},
        { 0x089D ,0x0000, 0},
        { 0x089E ,0x0000, 0},
        { 0x089F ,0x0000, 0},
        { 0x08A0 ,0x0000, 0},
        { 0x08A1 ,0x0000, 0},
        { 0x08A2 ,0x0000, 0},
        { 0x08A3 ,0x0000, 0},
        { 0x08A4 ,0x0000, 0},
        { 0x08A5 ,0x0000, 0},
        { 0x08A6 ,0x0080, 0},
        { 0x08A7 ,0x0000, 0},
        { 0x08A8 ,0x0000, 0},
        { 0x08A9 ,0x0000, 0},
        { 0x08AA ,0x0000, 0},
        { 0x08AB ,0x0000, 0},
        { 0x08AC ,0x0000, 0},
        { 0x08AD ,0x0000, 0},
        { 0x08AE ,0x0000, 0},
        { 0x08AF ,0x0000, 0},
        { 0x08B0 ,0x0080, 0},
        { 0x08B1 ,0x0000, 0},
        { 0x08B2 ,0x0000, 0},
        { 0x08B3 ,0x0000, 0},
        { 0x08B4 ,0x0000, 0},
        { 0x08B5 ,0x0000, 0},
        { 0x08B6 ,0x0000, 0},
        { 0x08B7 ,0x0000, 0},
        { 0x08B8 ,0x0000, 0},
        { 0x08B9 ,0x0000, 0},
        { 0x08BA ,0x0080, 0},
        { 0x08BB ,0x0000, 0},
        { 0x08BC ,0x0000, 0},
        { 0x08BD ,0x0000, 0},
        { 0x08BE ,0x0000, 0},
        { 0x08BF ,0x0000, 0},
        { 0x08C0 ,0x0000, 0},
        { 0x08C1 ,0x0000, 0},
        { 0x08C2 ,0x0000, 0},
        { 0x08C3 ,0x0000, 0},
        { 0x08C4 ,0x0080, 0},
        { 0x08C5 ,0x0000, 0},
        { 0x08C6 ,0x0000, 0},
        { 0x08C7 ,0x0000, 0},
        { 0x08C8 ,0x0000, 0},
        { 0x08C9 ,0x0000, 0},
        { 0x08CA ,0x0000, 0},
        { 0x08CB ,0x0000, 0},
        { 0x08CC ,0x0000, 0},
        { 0x08CD ,0x0000, 0},
        { 0x08CE ,0x0080, 0},
        { 0x08CF ,0x0000, 0},
        { 0x08D0 ,0x0000, 0},
        { 0x08D1 ,0x0000, 0},
        { 0x08D2 ,0x0000, 0},
        { 0x08D3 ,0x0000, 0},
        { 0x08D4 ,0x0000, 0},
        { 0x08D5 ,0x0000, 0},
        { 0x08D6 ,0x0000, 0},
        { 0x08D7 ,0x0000, 0},
        { 0x08D8 ,0x0080, 0},
        { 0x08D9 ,0x0000, 0},
        { 0x08DA ,0x0000, 0},
        { 0x08DB ,0x0000, 0},
        { 0x08DC ,0x0000, 0},
        { 0x08DD ,0x0000, 0},
        { 0x08DE ,0x0000, 0},
        { 0x08DF ,0x0000, 0},
        { 0x08E0 ,0x0000, 0},
        { 0x08E1 ,0x0000, 0},
        { 0x08E2 ,0x0080, 0},
        { 0x08E3 ,0x0000, 0},
        { 0x08E4 ,0x0000, 0},
        { 0x08E5 ,0x0000, 0},
        { 0x08E6 ,0x0000, 0},
        { 0x08E7 ,0x0000, 0},
        { 0x08E8 ,0x0000, 0},
        { 0x08E9 ,0x0000, 0},
        { 0x08EA ,0x0000, 0},
        { 0x08EB ,0x0000, 0},
        { 0x08EC ,0x0080, 0},
        { 0x08ED ,0x0000, 0},
        { 0x08EE ,0x0000, 0},
        { 0x08EF ,0x0000, 0},
        { 0x08F0 ,0x0000, 0},
        { 0x08F1 ,0x0000, 0},
        { 0x08F2 ,0x0000, 0},
        { 0x08F3 ,0x0000, 0},
        { 0x08F4 ,0x0000, 0},
        { 0x08F5 ,0x0000, 0},
        { 0x08F6 ,0x0080, 0},
        { 0x08F7 ,0x0000, 0},
        { 0x08F8 ,0x0000, 0},
        { 0x08F9 ,0x0000, 0},
        { 0x08FA ,0x0000, 0},
        { 0x08FB ,0x0000, 0},
        { 0x08FC ,0x0000, 0},
        { 0x08FD ,0x0000, 0},
        { 0x08FE ,0x0000, 0},
        { 0x08FF ,0x0000, 0},
        { 0x0800 ,0xF00C, 0},

};
#define ALC1312_INIT_REG_LEN ARRAY_SIZE(init_list)
#define ALC1312_EQ_REG_LEN ARRAY_SIZE(eq_list)

static int alc1312_reg_init(struct snd_soc_component *component)
{

	int i;
//	component->cache_only = false;
	for (i = 0; i < ALC1312_INIT_REG_LEN; i++) {
		snd_soc_component_write(component, init_list[i].reg, init_list[i].val);
		mdelay(init_list[i].delay);
	}
	//codec->cache_only = true;

	return 0;
}

static struct alc1312_eq_reg eq_list[] = {

//
        { 2 ,0x0002, 0x6505},
        { 2 ,0x0004, 0x5004},
        { 2 ,0x0006, 0x5504},
        { 2 ,0x0008, 0xFFF7},
        { 2 ,0x0009, 0xF800},
        { 2 ,0x000A, 0xC000},
        { 2 ,0x000B, 0xC800},
        { 2 ,0x000D, 0x0003},
        { 2 ,0x000E, 0x8000},
        { 2 ,0x000F, 0x0000},
        { 2 ,0x0010, 0xA833},
        { 2 ,0x0022, 0x0000},
        { 2 ,0x0060, 0x0000},
        { 2 ,0x0062, 0x0000},
        { 2 ,0x0070, 0x0008},
        { 2 ,0x0071, 0x0BFD},
        { 2 ,0x0072, 0x0008},
        { 2 ,0x0073, 0x0FCC},
        { 2 ,0x0074, 0x003F},
        { 2 ,0x0075, 0xFE08},
        { 2 ,0x0076, 0x0020},
        { 2 ,0x0077, 0x00D7},
        { 2 ,0x0078, 0x0000},
        { 2 ,0x0079, 0x0000},
        { 2 ,0x007A, 0x0000},
        { 2 ,0x007B, 0x0000},
        { 2 ,0x007C, 0x0000},
        { 2 ,0x007D, 0x0000},
        { 2 ,0x007E, 0x0000},
        { 2 ,0x007F, 0x0000},
        { 2 ,0x0080, 0x0000},
        { 2 ,0x0088, 0x0000},
        { 2 ,0x0089, 0x0000},
        { 2 ,0x008A, 0x0000},
        { 2 ,0x008B, 0x0000},
        { 2 ,0x008C, 0x0000},
        { 2 ,0x008D, 0x0000},
        { 2 ,0x008E, 0x0000},
        { 2 ,0x008F, 0x0000},
        { 2 ,0x0106, 0x40C0},
        { 2 ,0x0108, 0x0000},
        { 2 ,0x010A, 0xDD5D},
        { 2 ,0x010C, 0xC000},
        { 2 ,0x0200, 0x1106},
        { 2 ,0x0202, 0x0013},
        { 2 ,0x0204, 0x2010},
        { 2 ,0x020A, 0x0000},
        { 2 ,0x020B, 0x0000},
        { 2 ,0x020C, 0x0000},
        { 2 ,0x020D, 0x0000},
        { 2 ,0x020E, 0x0000},
        { 2 ,0x020F, 0x0000},
        { 2 ,0x0210, 0x0000},
        { 2 ,0x0211, 0x0000},
        { 2 ,0x0214, 0x0020},
        { 2 ,0x0216, 0x2000},
        { 2 ,0x0217, 0x0A00},
        { 2 ,0x0220, 0x0030},
        { 2 ,0x0226, 0x0000},
        { 2 ,0x0227, 0x0000},
        { 2 ,0x0228, 0x0000},
        { 2 ,0x0229, 0x0000},
        { 2 ,0x0300, 0x0145},
        { 2 ,0x0302, 0x2C02},
        { 2 ,0x030C, 0x0FC0},
        { 2 ,0x030E, 0xA500},
        { 2 ,0x0310, 0x0100},
        { 2 ,0x0312, 0x0988},
        { 2 ,0x0314, 0xAA00},
        { 2 ,0x0316, 0x00A2},
        { 2 ,0x0350, 0x4002},
        { 2 ,0x0500, 0x066C},
        { 2 ,0x0502, 0x7FFF},
        { 2 ,0x0503, 0x7F00},
        { 2 ,0x0504, 0x0000},
        { 2 ,0x0506, 0x0000},
        { 2 ,0x0507, 0x0000},
        { 2 ,0x0508, 0x0000},
        { 2 ,0x0509, 0x0000},
        { 2 ,0x050A, 0x5F5F},
        { 2 ,0x0510, 0x2500},
        { 2 ,0x0512, 0x3C79},
        { 2 ,0x0514, 0x0001},
        { 2 ,0x0515, 0x47AC},
        { 2 ,0x0516, 0x0000},
        { 2 ,0x0517, 0x51EB},
        { 2 ,0x0518, 0x0000},
        { 2 ,0x051A, 0x0000},
        { 2 ,0x051B, 0x0000},
        { 2 ,0x051C, 0x0000},
        { 2 ,0x051D, 0x0000},
        { 2 ,0x0528, 0x0000},
        { 2 ,0x0600, 0x007D},
        { 2 ,0x0601, 0x6BD7},
        { 2 ,0x0602, 0x00C2},
        { 2 ,0x0603, 0x871F},
        { 2 ,0x0604, 0x0000},
        { 2 ,0x0605, 0x0342},
        { 2 ,0x0606, 0x003E},
        { 2 ,0x0607, 0xB92E},
        { 2 ,0x0608, 0x0066},
        { 2 ,0x0609, 0x784E},
        { 2 ,0x060A, 0x00D5},
        { 2 ,0x060B, 0x3970},
        { 2 ,0x060C, 0x0001},
        { 2 ,0x060D, 0x1390},
        { 2 ,0x060E, 0x0034},
        { 2 ,0x060F, 0x4FB7},
        { 2 ,0x0610, 0x8000},
        { 2 ,0x0611, 0xB245},
        { 2 ,0x0612, 0x7418},
        { 2 ,0x0613, 0xC0FF},
        { 2 ,0x0614, 0x2004},
        { 2 ,0x0615, 0x00FF},
        { 2 ,0x0616, 0x0C00},
        { 2 ,0x0617, 0x4000},
        { 2 ,0x0618, 0xA021},
        { 2 ,0x0621, 0xB245},
        { 2 ,0x0622, 0x7418},
        { 2 ,0x0623, 0xC0FF},
        { 2 ,0x0624, 0x2004},
        { 2 ,0x0625, 0x00FF},
        { 2 ,0x0626, 0x0C20},
        { 2 ,0x0627, 0x4000},
        { 2 ,0x0628, 0xA021},
        { 2 ,0x0631, 0xB245},
        { 2 ,0x0632, 0x7418},
        { 2 ,0x0633, 0xC0FF},
        { 2 ,0x0634, 0x2004},
        { 2 ,0x0635, 0x00FF},
        { 2 ,0x0636, 0x0C20},
        { 2 ,0x0637, 0x4000},
        { 2 ,0x0638, 0xA021},
        { 2 ,0x0641, 0xB255},
        { 2 ,0x0642, 0x7418},
        { 2 ,0x0643, 0xC0FF},
        { 2 ,0x0644, 0x2004},
        { 2 ,0x0645, 0x00FF},
        { 2 ,0x0646, 0x0C20},
        { 2 ,0x0647, 0x4C00},
        { 2 ,0x0648, 0xE021},
        { 2 ,0x0649, 0x3780},
        { 2 ,0x064A, 0x0000},
        { 2 ,0x064B, 0x5F5F},
        { 2 ,0x0700, 0x8000},
        { 2 ,0x0711, 0x0000},
        { 2 ,0x0721, 0x0000},
        { 2 ,0x0731, 0x0000},
        { 2 ,0x0741, 0x0000},

};


static const struct reg_default alc1312_reg[] = {
	{ 0x0000,   0x0    },
	{ 0x0004,   0x1000 },
	{ 0x0006,   0x3570 },
	{ 0x0008,   0x3000 },
	{ 0x000A,   0x105E },
	{ 0x000C,   0x0    },
	{ 0x0080,   0x1    },
	{ 0x0103,   0x33e  },
	{ 0x0104,   0x33e  },
	{ 0x0106,   0x9f93 },
	{ 0x0214,   0x0    },
	{ 0x0300,   0xb803 },
	{ 0x0322,   0x400f },
	{ 0x0324,   0xc0a0 },
	{ 0x0326,   0x000f },
	{ 0x0350,   0x0888 }, /*gpio ctrl*/
	//{ 0x0350,   0x88 },
};

#ifdef CONFIG_PM
static void alc1312_index_sync(struct snd_soc_component *component)
{
	const u16 *reg_cache = component->reg_cache;
	int i;

	/* Sync back cached values if they're different from the
	 * hardware default.
	 */
	for (i = 1; i < component->val_bytes; i++) {
		if (reg_cache[i] == alc1312_reg[i].reg)
			continue;
		snd_soc_component_write(component, i, reg_cache[i]);
	}
}
#endif

/**
 * alc1312_index_write - Write private register.
 * @codec: SoC audio codec device.
 * @reg: Private register index.
 * @value: Private register Data.
 *
 * Modify private register for advanced setting. It can be written through
 * private index (0x6a) and data (0x6c) register.
 *
 * Returns 0 for success or negative error code.
 */
static int alc1312_index_write(struct snd_soc_component *component,
		unsigned int reg, unsigned int value)
{
	int ret;
	//printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	ret = snd_soc_component_write(component, ALC1312_PRIV_INDEX, reg);
	if (ret < 0) {
		dev_err(component->dev, "Failed to set private addr: %d\n", ret);
		goto err;
	}
	ret = snd_soc_component_write(component, ALC1312_PRIV_DATA, value);
	if (ret < 0) {
		dev_err(component->dev, "Failed to set private value: %d\n", ret);
		goto err;
	}
	return 0;

err:
	return ret;
}

/**
 * alc1312_index_read - Read private register.
 * @codec: SoC audio codec device.
 * @reg: Private register index.
 *
 * Read advanced setting from private register. It can be read through
 * private index (0x6a) and data (0x6c) register.
 *
 * Returns private register value or negative error code.
 */
static unsigned int alc1312_index_read(struct snd_soc_component *component,
					unsigned int reg)
{
	int ret;
	ret = snd_soc_component_write(component, ALC1312_PRIV_INDEX, reg);
	if (ret < 0) {
		dev_err(component->dev, "Failed to set private addr: %d\n", ret);
		return ret;
	}
	return snd_soc_component_read(component, ALC1312_PRIV_DATA);
}

/**
 * alc1312_index_update_bits - update private register bits
 * @codec: audio codec
 * @reg: Private register index.
 * @mask: register mask
 * @value: new value
 *
 * Writes new register value.
 *
 * Returns 1 for change, 0 for no change, or negative error code.
 */
/*
static int alc1312_index_update_bits(struct snd_soc_component *component,
	unsigned int reg, unsigned int mask, unsigned int value)
{
	unsigned int old, new;
	int change, ret;
	ret = alc1312_index_read(component, reg);
	if (ret < 0) {
		dev_err(component->dev, "Failed to read private reg: %d\n", ret);
		goto err;
	}

	old = ret;
	new = (old & ~mask) | (value & mask);
	change = old != new;
	if (change) {
		ret = alc1312_index_write(codec, reg, new);
		if (ret < 0) {
			dev_err(component->dev,
				"Failed to write private reg: %d\n", ret);
			goto err;
		}
	}
	return change;

err:
	return ret;
}
*/
extern unsigned int serial_in_i2c(unsigned int addr, int offset);
extern unsigned int serial_out_i2c(unsigned int addr, int offset, int value);

static bool alc1312_volatile_register(
	struct device *codec, unsigned int reg)
{

	return 1;

}

static bool alc1312_readable_register(
	struct device *dev, unsigned int reg)
{
	switch (reg) {
	case 0x0000 ... 0x000D:
	case 0x0020 ... 0x0022:
	case 0x006a ... 0x0080:
	case 0x00f0 ... 0x00f4:
	case 0x0100 ... 0x0114:
	case 0x0214:
	case 0x0300 ... 0x0352:
	case 0x0500 ... 0x052a:
	case 0x0600 ... 0x0646:
	case 0x0700 ... 0x0745:
	case 0x0800 ... 0x08ff:
		return 1;
	default:
		return 0;
	}
}

static int alc1312_index_readable_register(
	struct snd_soc_component *component, unsigned int reg)
{
	switch (reg) {
	case 0x0002 ... 0x0022:
	case 0x0060 ... 0x0062:
	case 0x0070 ... 0x008f:
	case 0x0106 ... 0x010c:
	case 0x0200 ... 0x0229:
	case 0x0300 ... 0x0316:
	case 0x0350:
	case 0x0500 ... 0x051d:
	case 0x0528:
	case 0x0600 ... 0x064b:
	case 0x0700:
	case 0x0711:
	case 0x0721:
	case 0x0731:
	case 0x0741:
		return 1;
	default:
		return 0;
	}
}


#if 0
/*
 * read alc1312 register cache
 */
static inline unsigned int alc1312_read_reg_cache(struct snd_soc_component *component,
	unsigned int reg)
{
	u16 *cache = component->reg_cache;
	if (reg < 1 || reg > (ARRAY_SIZE(alc1312_reg) + 1))
		return -1;
	return cache[reg];
}

/*
 * write alc1312 register cache
 */

static inline void alc1312_write_reg_cache(struct snd_soc_component *component,
	unsigned int reg, unsigned int value)
{
	u16 *cache = component->reg_cache;
	if (reg < 0 || reg > 0xfc)
		return;
	cache[reg] = value;
}

static int alc1312_write(struct snd_soc_component *component, unsigned int reg, unsigned int val)
{
	//panic_printk("write 0x%02x 0x%04x\n",reg,val);

	alc1312_write_reg_cache(component, reg, val);

	serial_out_i2c(ALC1312_I2C_ADDR, reg, val);

	return 0;
}

static unsigned int alc1312_read(struct snd_soc_component *component, unsigned int reg)
{
	int ret=0;
	if(alc1312_volatile_register(snd_soc_component_get_drvdata(component), reg))
	{
		ret=serial_in_i2c(ALC1312_I2C_ADDR, reg);
		//panic_printk("from i2c read 0x%02x 0x%04x\n",reg,ret);
		return ret;
	}
	ret = alc1312_read_reg_cache(component, reg);
	//panic_printk("from cache read 0x%02x 0x%04x\n",reg,ret);
	return ret;
}
#endif

#if 0 /*Bard: keep it as a sample code*/
static const DECLARE_TLV_DB_SCALE(out_vol_tlv, -4650, 150, 0);
static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -65625, 375, 0);
static const DECLARE_TLV_DB_SCALE(in_vol_tlv, -3450, 150, 0);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -17625, 375, 0);
static const DECLARE_TLV_DB_SCALE(adc_bst_tlv, 0, 1200, 0);

/* {0, +20, +24, +30, +35, +40, +44, +50, +52} dB */
static unsigned int bst_tlv[] = {
	TLV_DB_RANGE_HEAD(7),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(2000, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(2400, 0, 0),
	3, 5, TLV_DB_SCALE_ITEM(3000, 500, 0),
	6, 6, TLV_DB_SCALE_ITEM(4400, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(5000, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(5200, 0, 0),
};


static const char *alc1312_input_mode[] = {
	"Single ended", "Differential"};

static const struct soc_enum alc1312_in1_mode_enum = SOC_ENUM_SINGLE(ALC1312_IN1_IN2,
	ALC1312_IN_SFT1, ARRAY_SIZE(alc1312_input_mode), alc1312_input_mode);
#endif


static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -6475, 37, 1);


static const char *alc1312_init_type_mode[] = {
	"nothing", "init"
};

static const SOC_ENUM_SINGLE_DECL(alc1312_init_type_enum, 0, 0, alc1312_init_type_mode);

static int alc1312_init_type_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = 0;

	return 0;
}

static int alc1312_init_type_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	int init = ucontrol->value.integer.value[0];

	if (init)
		alc1312_reg_init(component);

	return 0;
}

static const struct snd_kcontrol_new alc1312_snd_controls[] = {

	//SOC_DOUBLE_TLV("DAC Playback Volume", 0x12,
	//		8, 0, 0xff, 0, dac_vol_tlv),
	SOC_DOUBLE_TLV("DAC Playback Volume", 0x0106,
			8, 0, 0xAf, 0, dac_vol_tlv),

	SOC_ENUM_EXT("AMP Init Control",  alc1312_init_type_enum,
		alc1312_init_type_get, alc1312_init_type_put),

        SOC_SINGLE("Playback L mute/umute", 0x0102,
                                BIT(14), 1, 1),

        SOC_SINGLE("Playback R mute/umute", 0x0102,
                                BIT(15), 1, 1),

        /* OUTPUT Control */
 //       SOC_DOUBLE("OUT Playback Switch", RT5640_OUTPUT,
//                RT5640_L_MUTE_SFT, RT5640_R_MUTE_SFT, 1, 1),

};

static const struct snd_soc_dapm_widget alc1312_dapm_widgets[] = {
	/* Audio Interface */
	SND_SOC_DAPM_AIF_IN("AIF1RX", "AIF1 Playback", 0, SND_SOC_NOPM, 0, 0),

	/* DACs */
	SND_SOC_DAPM_DAC("DAC", NULL, SND_SOC_NOPM, 0, 0),

	/* Output Lines */
	SND_SOC_DAPM_OUTPUT("Amp"),
};

static const struct snd_soc_dapm_route alc1312_dapm_routes[] = {
	{"DAC", NULL, "AIF1RX"},
	{"Amp", NULL, "DAC"},
};

static int alc1312_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	//struct alc1312_priv *alc1312 = snd_soc_codec_get_drvdata(codec);
	printk("enter %s\n",__func__);


	return 0;
}

static int alc1312_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	//struct alc1312_priv *alc1312 = snd_soc_codec_get_drvdata(codec);
	printk("enter %s\n",__func__);
	return 0;
}

static int alc1312_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct snd_soc_component *component = dai->component;
	//struct alc1312_priv *alc1312 = snd_soc_codec_get_drvdata(codec);
	printk("enter %s\n",__func__);

	return 0;
}

static int alc1312_set_dai_sysclk(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = dai->component;
	//struct alc1312_priv *alc1312 = snd_soc_codec_get_drvdata(codec);
	printk("enter %s\n",__func__);


	return 0;
}

/*
static int alc1312_set_dai_mute(struct snd_soc_dai *dai, int mute)
{

        snd_soc_component_update_bits(dai->component, 0x0102, BIT(14)|BIT(15),
                            (mute ? (BIT(14)|BIT(15)) : 0));
	return 0;
}
*/
/**
 * alc1312_index_show - Dump private registers.
 * @dev: codec device.
 * @attr: device attribute.
 * @buf: buffer for display.
 *
 * To show non-zero values of all private registers.
 *
 * Returns buffer length.
 */
static ssize_t alc1312_index_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct alc1312_priv *alc1312 = i2c_get_clientdata(client);
	struct snd_soc_component *component = alc1312->component;
	unsigned int val;
	int cnt = 0, i;
	//component->cache_only = false;
	cnt += sprintf(buf, "ALC1312 index register\n");
	for (i = 0; i < 0x741; i++) {
		if (cnt + ALC1312_REG_DISP_LEN >= PAGE_SIZE)
			break;
		if (alc1312_index_readable_register(component, i)) {
			val = alc1312_index_read(component, i);
			if (!val)
				continue;
			cnt += snprintf(buf + cnt, ALC1312_REG_DISP_LEN,
					"%02x: %04x\n", i, val);
		}
	}

	if (cnt >= PAGE_SIZE)
		cnt = PAGE_SIZE - 1;
	//codec->cache_only = true;

	return cnt;
}
static DEVICE_ATTR(index_reg, 0444, alc1312_index_show, NULL);

static ssize_t alc1312_component_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct alc1312_priv *alc1312 = i2c_get_clientdata(client);
	struct snd_soc_component *component = alc1312->component;
	unsigned int val;
	int cnt = 0, i;
	component->cache_only = false;
	cnt += sprintf(buf, "ALC1312 codec register\n");
	for (i = 0; i <= 0x8ff; i++) {
		if (cnt + 22 >= PAGE_SIZE)
			break;
		if (alc1312_readable_register(snd_soc_component_get_drvdata(component), i)) {
			val = snd_soc_component_read(component, i);

			cnt += snprintf(buf + cnt, 22,
					"%04x: %04x\n", i, val);
		}
	}

	if (cnt >= PAGE_SIZE)
		cnt = PAGE_SIZE - 1;
	//codec->cache_only = true;

	return cnt;
}

static ssize_t alc1312_component_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct alc1312_priv *alc1312 = i2c_get_clientdata(client);
	struct snd_soc_component *component = alc1312->component;
	unsigned int val = 0, addr = 0;
	int i;
//	pr_debug("register \"%s\" count=%d\n", buf, count);
	for (i = 0; i < count; i++) {	/*address */
		if (*(buf + i) <= '9' && *(buf + i) >= '0')
			addr = (addr << 4) | (*(buf + i) - '0');
		else if (*(buf + i) <= 'f' && *(buf + i) >= 'a')
			addr = (addr << 4) | ((*(buf + i) - 'a') + 0xa);
		else if (*(buf + i) <= 'F' && *(buf + i) >= 'A')
			addr = (addr << 4) | ((*(buf + i) - 'A') + 0xa);
		else
			break;
	}

	for (i = i + 1; i < count; i++) {
		if (*(buf + i) <= '9' && *(buf + i) >= '0')
			val = (val << 4) | (*(buf + i) - '0');
		else if (*(buf + i) <= 'f' && *(buf + i) >= 'a')
			val = (val << 4) | ((*(buf + i) - 'a') + 0xa);
		else if (*(buf + i) <= 'F' && *(buf + i) >= 'A')
			val = (val << 4) | ((*(buf + i) - 'A') + 0xa);
		else
			break;
	}
	printk("addr=0x%x val=0x%x, i=%d, count=%d\n", addr, val, i, count);

	if (i == count) {
		pr_debug("0x%02x = 0x%04x\n", addr,
		snd_soc_component_read(component, addr));
	} else {
		snd_soc_component_write(component, addr, val);
	}

	return count;
}

static DEVICE_ATTR(codec_reg, 0644, alc1312_component_show, alc1312_component_store);


static int alc1312_write_eq_param(struct snd_soc_component *component)
{
	int i;
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	for (i = 0; i < ARRAY_SIZE(eq_list); i++) {
		if (eq_list[i].level == 1) //MX-
			snd_soc_component_write(component, eq_list[i].reg, eq_list[i].val);
		else if (eq_list[i].level == 2)//PR-
			alc1312_index_write(component, eq_list[i].reg, eq_list[i].val);
	}

	return 0;
}


static void alc1312_sync_cache(struct snd_soc_component *component)
{
	const u16 *reg_cache = component->regmap;
	int i;

	printk("<3> Keen %s %d %d\r\n",__FUNCTION__,__LINE__,component->val_bytes);
	/* Sync back cached values if they're different from the
	 * hardware default.
	 */
	for (i = 1; i < component->val_bytes; i++) {
		if (reg_cache[i] == alc1312_reg[i].reg)
			continue;
		snd_soc_component_write(component, i, reg_cache[i]);
	}
}

static int alc1312_set_bias_level(struct snd_soc_component *component,
			enum snd_soc_bias_level level)
{
/*
	static int init_once = 0;
	/////////////////power on
	WrL1 0350 0188

	/////////////////power off
	WrL1 0350 0088

*/
	printk("enter %s, level=%d \n",__func__, level);
	switch (level) {
	case SND_SOC_BIAS_ON:
		//if (init_once==0) {
		//	alc1312_reg_init(codec);
		//	init_once = 1;
		//}
//		snd_soc_write(codec, 0x0350, 0x0188);
		printk("enter %s, SND_SOC_BIAS_ON \n",__func__);
//		rt5616_set_data_bypass(1);
		alc1312_pdb_ctrl(1);
		break;

	case SND_SOC_BIAS_PREPARE:
		printk("enter %s, SND_SOC_BIAS_PREPARE \n",__func__);

		break;

	case SND_SOC_BIAS_STANDBY:
		printk("enter %s, SND_SOC_BIAS_STANDBY \n",__func__);
//		rt5616_set_data_bypass(0);
		if (SND_SOC_BIAS_OFF == component->dapm.bias_level) {

		}
		break;

	case SND_SOC_BIAS_OFF:
//		snd_soc_write(codec, 0x0350, 0x0088);
//		rt5616_set_data_bypass(0);
		break;

	default:
		break;
	}
	component->dapm.bias_level = level;

	return 0;
}

void alc1312_pdb_ctrl(unsigned char onoff)
{
        gpio_set_value_cansleep(28, onoff);
        //gpio_set_value_cansleep(63, onoff);
        //gpio_set_value_cansleep(65, onoff);

}
EXPORT_SYMBOL(alc1312_pdb_ctrl);

static int alc1312_init(struct snd_soc_component *component)
{
	//struct alc1312_priv *alc1312 = snd_soc_codec_get_drvdata(codec);
	//int ret;
	unsigned val;
	val = snd_soc_component_read(component, 0x007C);
        printk("Device id =0x%x\r\n",val);
        
        if(val != 0x10EC)
          return 0;

	printk("enter %s\n",__func__);

	alc1312_reg_init(component);

	printk("<3> Keen EQ init %s %d\r\n",__FUNCTION__,__LINE__);
	alc1312_write_eq_param(component);

	//alc1312_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	//rt5616_set_data_bypass(1);
	return 0;
}


static int alc1312_probe(struct snd_soc_component *component)
{
	struct alc1312_priv *alc1312 = snd_soc_component_get_drvdata(component);
	int ret = 0;

	printk("enter %s\n",__func__);
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	component->dapm.idle_bias_off = 1;
/*
	ret = snd_soc_codec_set_cache_io(codec, 16, 16, SND_SOC_I2C);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
		return ret;
	}

	codec->cache_only = false;		// no cache
*/
	alc1312->component = component;

	alc1312_init(component);

	ret = device_create_file(component->dev, &dev_attr_index_reg);
	if (ret != 0) {
		dev_err(component->dev,
			"Failed to create index_reg sysfs files: %d\n", ret);
		return ret;
	}
	ret = device_create_file(component->dev, &dev_attr_codec_reg);
	if (ret != 0) {
		dev_err(component->dev,
			"Failed to create codex_reg sysfs files: %d\n", ret);
		return ret;
	}
	return 0;

}

static void alc1312_remove(struct snd_soc_component *component)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	//if (codec->control_data)
	alc1312_set_bias_level(component, SND_SOC_BIAS_OFF);
}

#ifdef CONFIG_PM
static int alc1312_suspend(struct snd_soc_component *component)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	alc1312_set_bias_level(component, SND_SOC_BIAS_OFF);
	return 0;
}

static int alc1312_resume(struct snd_soc_component *component)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	codec->cache_only = false;
//	codec->cache_sync = 1;
	snd_soc_cache_sync(component);
	alc1312_index_sync(component);
	return 0;
}
#else
#define alc1312_suspend NULL
#define alc1312_resume NULL
#endif

#define ALC1312_STEREO_RATES SNDRV_PCM_RATE_48000 //SNDRV_PCM_RATE_8000_96000
#define ALC1312_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_BE)

struct snd_soc_dai_ops alc1312_aif_dai_ops = {
	.hw_params = alc1312_hw_params,
	.prepare = alc1312_prepare,
	.set_fmt = alc1312_set_dai_fmt,
	.set_sysclk = alc1312_set_dai_sysclk,
//	.digital_mute   = alc1312_set_dai_mute,
};

struct snd_soc_dai_driver alc1312_dai[] = {
	{
		.name = "alc1312-aif1",
		.id = I2S,
		.playback = {
			.stream_name = "AIF1 Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = ALC1312_STEREO_RATES,
			.formats = ALC1312_FORMATS,
		},
		.ops = &alc1312_aif_dai_ops,
	},
};

static struct snd_soc_component_driver soc_codec_dev_alc1312 = {
	.probe = alc1312_probe,
	.remove = alc1312_remove,
	.suspend = alc1312_suspend,
	.resume = alc1312_resume,
	.set_bias_level = alc1312_set_bias_level,
	.controls = alc1312_snd_controls,
	.num_controls = ARRAY_SIZE(alc1312_snd_controls),
	.dapm_widgets = alc1312_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(alc1312_dapm_widgets),
	.dapm_routes = alc1312_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(alc1312_dapm_routes),
};

static const struct of_device_id ipq40xx_codec_of_match[] = {
        { .compatible = "qca,ipq40xx-codec-alc1312" },
        {},
};

static const struct i2c_device_id alc1312_i2c_id[] = {
	{ "alc1312", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, alc1312_i2c_id);

static const struct regmap_config alc1312_regmap_config = {
	.reg_bits = 16,
	.val_bits = 16,

	.readable_reg = alc1312_readable_register,
	.volatile_reg = alc1312_volatile_register,
	.max_register = 0x8FF,
	.reg_defaults = alc1312_reg,
	.num_reg_defaults = ARRAY_SIZE(alc1312_reg),
	.cache_type = REGCACHE_RBTREE,
};


static int alc1312_i2c_probe(struct i2c_client *i2c,
		    const struct i2c_device_id *id)
{
	struct alc1312_priv *alc1312;
	int ret;
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	alc1312 = kzalloc(sizeof(struct alc1312_priv), GFP_KERNEL);
	if (NULL == alc1312)
		return -ENOMEM;

	i2c_set_clientdata(i2c, alc1312);

	//alc1312->regmap = devm_regmap_init_i2c(i2c, &alc1312_regmap_config);
	//if (IS_ERR(alc1312->regmap))
	//	return PTR_ERR(alc1312->regmap);

	ret = snd_soc_register_component(&i2c->dev, &soc_codec_dev_alc1312,
			alc1312_dai, ARRAY_SIZE(alc1312_dai));
	if (ret < 0)
		kfree(alc1312);

	return ret;
}


static int alc1312_i2c_remove(struct i2c_client *i2c)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	snd_soc_unregister_component(&i2c->dev);
	kfree(i2c_get_clientdata(i2c));
	return 0;
}

void alc1312_i2c_shutdown(struct i2c_client *client)
{
	struct alc1312_priv *alc1312 = i2c_get_clientdata(client);
	struct snd_soc_component *component = alc1312->component;

	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	if (component != NULL)
		alc1312_set_bias_level(component, SND_SOC_BIAS_OFF);
}

struct i2c_driver alc1312_i2c_driver = {
	.driver = {
		.name = "alc1312_codec",
		.owner = THIS_MODULE,
		.of_match_table = ipq40xx_codec_of_match,
	},
	.id_table = alc1312_i2c_id,
	.probe = 	alc1312_i2c_probe,
	.remove = 	alc1312_i2c_remove,
	.shutdown = alc1312_i2c_shutdown,
};


static int __init alc1312_modinit(void)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	return i2c_add_driver(&alc1312_i2c_driver);
}
module_init(alc1312_modinit);

static void __exit alc1312_modexit(void)
{
	printk("<3> Keen %s %d\r\n",__FUNCTION__,__LINE__);
	i2c_del_driver(&alc1312_i2c_driver);
}
module_exit(alc1312_modexit);


MODULE_DESCRIPTION("ASoC ALC1312 driver");
MODULE_AUTHOR("Bard Liao <bardliao@realtek.com>");
MODULE_LICENSE("GPL");
