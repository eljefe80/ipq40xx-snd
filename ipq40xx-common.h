#ifndef IPQ40xx_COMMON_H
#define IPQ40xx_COMMON_H

#define MAX_STEREO_ENTRIES	4

struct dai_priv_st {
	int stereo_tx;
	int stereo_rx;
	int mbox_tx;
	int mbox_rx;
	int tx_enabled;
	int rx_enabled;
	int is_txmclk_fixed;
	int interface;
	struct platform_device *pdev;
};

/* Enumerations */

enum dir {
	PLAYBACK,
	CAPTURE
};

enum cfg {
	DISABLE,
	ENABLE
};

enum intf {
	I2S,
	TDM,
	SPDIF,
	I2S1,
	I2S2,
	MAX_INTF
};

enum channels {
	CH_STEREO = 2,
	CH_3_1 = 4,
	CH_5_1 = 6,
	CH_7_1 = 8
};

enum ipq40xx_samp_freq {
        FREQ_8000 = 8000,
        FREQ_11025 = 11025,
        FREQ_16000 = 16000,
        FREQ_22050 = 22050,
        FREQ_32000 = 32000,
        FREQ_44100 = 44100,
        FREQ_48000 = 48000,
        FREQ_64000 = 64000,
        FREQ_88200 = 88200,
        FREQ_96000 = 96000,
        FREQ_176400 = 176400,
        FREQ_192000 = 192000,
};

enum stereo_ch {
	STEREO0,
	STEREO1,
	STEREO2,
	STEREO3
};

enum bit_width {
        __BIT_8 = 8,
        __BIT_16 = 16,
        __BIT_24 = 24,
        __BIT_32 = 32,
        __BIT_INVAL = -1
};

/* Supported Channels */
#define RATE_16000_96000 \
		(SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |\
		SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |\
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_64000 |\
		SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

static inline uint32_t intf_to_index(struct dai_priv_st *priv, int intf){
	for (int i = 0; i < MAX_INTF; i++)
		if (priv[i]->interface == intf)
			return i;
	return -EINVAL;
}

#endif