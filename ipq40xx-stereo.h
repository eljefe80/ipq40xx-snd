#ifndef IPQ40xx_STEREO_H
#define IPQ40xx_STEREO_H

#include "ipq40xx-common.h"
/* ADSS_STEREO0_AUDIO_STEREO_REG Registers */

#define STEREO0_OFFSET			0x0
#define STEREO1_OFFSET			0x2000
#define STEREO2_OFFSET			0x4000
#define STEREO3_OFFSET			0x6000

#define ADSS_STEREOn_STEREO0_CONFIG_REG			0x0
#define STEREOn_CONFIG_MIC_SWAP				(1 << 24)
#define STEREOn_CONFIG_SPDIF_ENABLE			(1 << 23)
#define STEREOn_CONFIG_ENABLE				(1 << 21)
#define STEREOn_CONFIG_MIC_RESET			(1 << 20)
#define STEREOn_CONFIG_RESET				(1 << 19)
#define STEREOn_CONFIG_I2S_DELAY			(0 << 18)
#define STEREOn_CONFIG_PCM_SWAP				(1 << 17)
#define STEREOn_CONFIG_MIC_WORD_SIZE_32			(1 << 16)
#define STEREOn_CONFIG_MIC_WORD_SIZE_16			(0 << 16)
#define STEREOn_CONFIG_STEREO_MODE			(0 << 14)
#define STEREOn_CONFIG_MONO_MODE			(1 << 14)
#define STEREOn_CONFIG_STEREO_MONO_MASK			(3 << 14)
#define STEREOn_CONFIG_DATA_WORD_SIZE(x)		(x << 12)
#define STEREOn_CONFIG_DATA_WORD_SIZE_MASK		(3 << 12)
#define STEREOn_CONFIG_I2S_WORD_SIZE_32			(1 << 11)
#define STEREOn_CONFIG_I2S_WORD_SIZE_16			(0 << 11)
#define STEREOn_CONFIG_MCK_SEL				(1 << 10)
#define STEREOn_CONFIG_SAMPLE_CNT_CLEAR_TYPE		(1 << 9)
#define STEREOn_CONFIG_MASTER				(1 << 8)


/* SPDIF clocks */
#define AUDIO_SPDIFINFAST	49152000

enum bit_width {
	__BIT_8 = 8,
	__BIT_16 = 16,
	__BIT_24 = 24,
	__BIT_32 = 32,
	__BIT_INVAL = -1
};

/* Stereo APIs */
void ipq40xx_stereo_config_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_mic_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_enable(uint32_t enable, uint32_t stereo_offset);
int ipq40xx_cfg_bit_width(uint32_t bit_width, uint32_t stereo_offset);
void ipq40xx_config_master(uint32_t enable, uint32_t stereo_offset);
int ipq40xx_audio_stereo_probe(struct platform_device *pdev);
#endif