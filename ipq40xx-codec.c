/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>
#include <sound/control.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <sound/initval.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "ipq40xx-adss.h"
#include "ipq40xx-codec.h"

struct audio_hw_params audio_params;

static const u8 akd4613_reg[AK4613_MAX_REG] = {
	0x0F, 0x07, 0x3F, 0x20, 0x20, 0x55, 0x05, 0x07,
	0x0F, 0x07, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t ipq40xx_compare_hw_params(struct audio_hw_params *curr_params)
{
	if ((curr_params->bit_width == audio_params.bit_width) &&
		(curr_params->freq == audio_params.freq) &&
		(curr_params->channels == audio_params.channels))
		return 0;
	else
		return -EINVAL;
}

/* DFS : Sampling Speed
 *
 * DFS1	DFS0		Sampling Speed Mode (fs)
 * 0	0	Normal Speed Mode	32kHz~48kHz (default)
 * 0	1	Double Speed Mode	64kHz~96kHz
 * 1	0	Quad Speed Mode	128kHz~192kHz
 * 1	1	N/A			-
 */

static int ipq40xx_codec_i2c_set_dfs(struct snd_soc_component *codec, int mode)
{
	uint32_t reg;

	if (mode > QUAD_SPEED) {
		pr_err("%s: %d: Invalid DFS mode", __func__, __LINE__);
		return -EINVAL;
	}

	reg = snd_soc_component_read(codec, AKD4613_04_CTRL2);

	reg &= ~(AKD4613_DFS_MASK);
	reg |= AKD4613_DFS(mode);

	snd_soc_component_write(codec, AKD4613_04_CTRL2, reg);

	return 0;
}

/* CKS : Master Clock Input Frequency Select
 *
 * CKS1	CKS0	Normal Speed	Double Speed	Quad Speed
 * 0	0	256fs		256fs		128fs
 * 0	1	384fs		256fs		128fs
 * 1	0	512fs		256fs		128fs (default)
 * 1	1	512fs		256fs		128fs
 */

static int ipq40xx_codec_i2c_set_cks(struct snd_soc_component *codec,
					int config, int mode)
{
	uint32_t cks_val;
	uint32_t reg;

	if (mode == NORMAL_SPEED) {
		if (config == FS_256)
			cks_val = 0;
		else if (config == FS_384)
			cks_val = 1;
		else if (config == FS_512)
			cks_val = 2;
		else
			cks_val = -EINVAL;
	} else if (mode == DOUBLE_SPEED) {
		if (config == FS_256)
			cks_val = 2;
		else
			cks_val = -EINVAL;
	} else if (mode == QUAD_SPEED) {
		if (config == FS_128)
			cks_val = 2;
		else
			cks_val = -EINVAL;
	} else {
		pr_err("%s: %d: Invalid DFS mode", __func__, __LINE__);
		return -EINVAL;
	}

	if (cks_val < 0) {
		pr_err("%s: %d: Invalid CKS config", __func__, __LINE__);
		return cks_val;
	}

	reg = snd_soc_component_read(codec, AKD4613_04_CTRL2);

	reg &= ~(AKD4613_CKS_MASK);
	reg |= AKD4613_CKS(cks_val);

	snd_soc_component_write(codec, AKD4613_04_CTRL2, reg);

	return 0;
}

static int ipq40xx_codec_i2c_set_tdm_mode(struct snd_soc_component *codec,
						int tdm_mode)
{
	uint32_t reg;

	if (tdm_mode >= TDM_MAX) {
		pr_err("%s: %d: Invalid DFS mode", __func__, __LINE__);
		return -EINVAL;
	}

	reg = snd_soc_component_read(codec, AKD4613_03_CTRL1);

	reg &= ~(AKD4613_TDM_MODE_MASK);
	reg |= AKD4613_TDM_MODE(tdm_mode);

	snd_soc_component_write(codec, AKD4613_03_CTRL1, reg);

	return 0;
}

static int ipq40xx_codec_i2c_set_dif(struct snd_soc_component *codec,
						int dif_val)
{
	uint32_t reg;

	reg = snd_soc_component_read(codec, AKD4613_03_CTRL1);

	reg &= ~(AKD4613_DIF_MASK);
	reg |= AKD4613_DIF(dif_val);

	snd_soc_component_write(codec, AKD4613_03_CTRL1, reg);

	return 0;
}

static void ipq40xx_codec_i2c_write_defaults(struct snd_soc_component *codec)
{
	int i;

	for (i = 0; i < AK4613_MAX_REG; i++)
		snd_soc_component_write(codec, i, akd4613_reg[i]);
	udelay(10);
}

static int ipq40xx_codec_audio_startup(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	struct snd_soc_component *component;

	component = dai->component;

	/* I2S and TDM cannot co-exist. CPU DAI startup would
	 * have already checked this case, by this time.
	 */
	if (!snd_soc_dai_active(dai))
		ipq40xx_codec_i2c_write_defaults(component);

	return 0;
}

static int ipq40xx_codec_audio_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,
					struct snd_soc_dai *dai)
{
	struct snd_soc_component *component;
	int samp_rate = params_rate(params);
	int bit_width = params_format(params);
	int channels = params_channels(params);
	int bit_act;
	int dfs, cks, tdm_mode, dif;
	uint32_t intf = dai->driver->id;
	struct audio_hw_params curr_params;

	bit_act = ipq40xx_get_act_bit_width(bit_width);
	if (bit_act == __BIT_INVAL)
		return -EINVAL;

	curr_params.freq = samp_rate;
	curr_params.channels = channels;
	curr_params.bit_width = bit_act;

	component = dai->component;

	/*
	 * Since CLKS in the codec are shared by I2S TX and RX channels,
	 * Rx and Tx when used simulatneoulsy will have to use the same channel,
	 * sampling frequency and bit widths. So compare the settings and then
	 * update the codec settings.
	 */

	if (snd_soc_dai_active(dai) > 1) {
		if (ipq40xx_compare_hw_params(&curr_params)) {
			/* Playback and capture settings do not match */
			pr_err("\nPlayback and capture settings do not match\n");
			return -EINVAL;
		} else {
			/* Settings match, codec settings are already done*/
			return 0;
		}
	}

	audio_params.freq = samp_rate;
	audio_params.channels = channels;
	audio_params.bit_width = bit_act;

	if (intf == I2S) {
		/* default values */
		dfs = NORMAL_SPEED;
		cks = FS_512;
		tdm_mode = STEREO;
		dif = DIF_I2S_MODE;

	} else if (intf == TDM) {
		/* Codec settings for 8 channels */
		dfs = DOUBLE_SPEED;
		cks = FS_256;
		tdm_mode = TDM_256;
		dif = DIF_LR_MODE3;
	} else {
		pr_err("\n%s Invalid interface\n", __func__);
		return -EINVAL;
	}

	ipq40xx_codec_i2c_set_dfs(component, dfs);
	ipq40xx_codec_i2c_set_cks(component, cks, dfs);
	ipq40xx_codec_i2c_set_tdm_mode(component, tdm_mode);
	ipq40xx_codec_i2c_set_dif(component, dif);
	udelay(10);

	return 0;
}

static int ipq40xx_codec_audio_prepare(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s:%d\n", __func__, __LINE__);
	return 0;
}

static void ipq40xx_codec_audio_shutdown(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s:%d\n", __func__, __LINE__);
}

static struct snd_soc_dai_ops ipq40xx_codec_audio_ops = {
	.startup	= ipq40xx_codec_audio_startup,
	.hw_params	= ipq40xx_codec_audio_hw_params,
	.prepare	= ipq40xx_codec_audio_prepare,
	.shutdown	= ipq40xx_codec_audio_shutdown,
};

static struct snd_soc_dai_driver ipq40xx_codec_dais[] = {
	{
		.name = "qca-i2s-codec-dai",
		.playback = {
			.stream_name = "qca-i2s-playback",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
		.capture = {
			.stream_name = "qca-i2s-capture",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
		.ops = &ipq40xx_codec_audio_ops,
		.id = I2S,
	},
	{
		.name = "qca-tdm-codec-dai",
		.playback = {
			.stream_name = "qca-tdm-playback",
			.channels_min = CH_STEREO,
			.channels_max = CH_7_1,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
		.capture = {
			.stream_name = "qca-tdm-capture",
			.channels_min = CH_STEREO,
			.channels_max = CH_7_1,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
		.ops = &ipq40xx_codec_audio_ops,
		.id = TDM,
	},
	{
		.name = "qca-i2s1-codec-dai",
		.playback = {
			.stream_name = "qca-i2s1-playback",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
	},
	{
		.name = "qca-i2s2-codec-dai",
		.playback = {
			.stream_name = "qca-i2s2-playback",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S32,
		},
	},
	{
		.name = "qca-spdif-codec-dai",
		.playback = {
			.stream_name = "qca-spdif-playback",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S24_3LE,
		},
		.capture = {
			.stream_name = "qca-spdif-capture",
			.channels_min = CH_STEREO,
			.channels_max = CH_STEREO,
			.rates = RATE_16000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16 |
				SNDRV_PCM_FMTBIT_S24_3LE,
		},
	},
};

static int ipq40xx_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	return -ENOTSUPP;
}

static const struct snd_kcontrol_new vol_ctrl  = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "playback volume",
	.access = (SNDRV_CTL_ELEM_ACCESS_TLV_READ |
		SNDRV_CTL_ELEM_ACCESS_READWRITE),
	.info = ipq40xx_info,
};

unsigned int ipq40xx_codec_i2c_read(struct snd_soc_component *component,
					unsigned int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(component->control_data, (u8)(reg & 0xFF));
	if (ret < 0)
		pr_err("\ti2c read error %s(%d)\n", __func__, ret);

	return ret;
}


static int ipq40xx_codec_probe(struct snd_soc_component *codec)
{
	int ret;

	ret = snd_soc_component_set_cache_io(codec, 8, 8, SND_SOC_I2C);
	if (ret != 0)
		dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);

	return ret;
}

static const struct snd_soc_component_driver ipq40xx_codec = {
	.probe = ipq40xx_codec_probe,
	.num_controls = 0,
/*	.reg_cache_size = ARRAY_SIZE(akd4613_reg),
	.reg_word_size = sizeof(u8),
	.reg_cache_default = akd4613_reg,
*/
};

static const struct of_device_id ipq40xx_codec_id_table[] = {
	{ .compatible = "qca,ipq40xx-codec" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, ipq40xx_codec_id_table);

static int ipq40xx_codec_i2c_probe(struct i2c_client *i2c,
					const struct i2c_device_id *id)
{
	int ret;

	ret = snd_soc_register_component(&i2c->dev,
			&ipq40xx_codec, ipq40xx_codec_dais,
			ARRAY_SIZE(ipq40xx_codec_dais));
	if (ret < 0) {
		pr_err("\nsnd_soc_register_codec failed (%d)\n", ret);
	}

	return ret;
}

static int ipq40xx_codec_i2c_remove(struct i2c_client *client)
{
	snd_soc_unregister_component(&client->dev);
	return 0;
}

static const struct of_device_id ipq40xx_codec_of_match[] = {
	{ .compatible = "qca,ipq40xx-codec" },
	{},
};

static const struct i2c_device_id ipq40xx_codec_i2c_id[] = {
	{ "qca_codec", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ipq40xx_codec_i2c_id);

static struct i2c_driver ipq40xx_codec_i2c_driver = {
	.driver = {
		.name = "qca_codec",
		.owner = THIS_MODULE,
		.of_match_table = ipq40xx_codec_of_match,
	},
	.probe = ipq40xx_codec_i2c_probe,
	.remove = ipq40xx_codec_i2c_remove,
	.id_table = ipq40xx_codec_i2c_id,
};

static int ipq40xx_codec_init(void)
{
	int ret;

	ret = i2c_add_driver(&ipq40xx_codec_i2c_driver);
	if (ret < 0)
		pr_err("%s: %d: Failed to add I2C driver", __func__, __LINE__);

	return ret;
}
module_init(ipq40xx_codec_init);

static void ipq40xx_codec_exit(void)
{
	i2c_del_driver(&ipq40xx_codec_i2c_driver);
}
module_exit(ipq40xx_codec_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("IPQ40xx Codec Driver");
