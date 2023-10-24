#ifndef IPQ40xx_COMMON_H
#define IPQ40xx_COMMON_H
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

enum stereo_ch {
	STEREO0,
	STEREO1,
	STEREO2,
	STEREO3
};

/* Supported Channels */
#define RATE_16000_96000 \
		(SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |\
		SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |\
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_64000 |\
		SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)