/*
 * alc1350.h  --  ALC1312 ALSA SoC audio driver
 *
 * Copyright 2011 Realtek Microelectronics
 * Author: Johnny Hsu <johnnyhsu@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ALC1312_H__
#define __ALC1312_H__

/* Private Register Control */
#define ALC1312_PRIV_INDEX			0x6a
#define ALC1312_PRIV_DATA			0x6c

/* Debug String Length */
#define ALC1312_REG_DISP_LEN 	23


struct alc1312_priv {
	struct snd_soc_component *component;
	struct regmap *regmap;

	int sysclk;
	int sysclk_src;
	int lrck;
	int bclk;
	int master;

	int pll_src;
	int pll_in;
	int pll_out;
};
struct alc1312_setup_data {
	int i2c_address;
	int i2c_bus;
};


#endif /* __ALC1312_H__ */
