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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_gpio.h>

#include "ipq40xx-mbox.h"
#include "ipq40xx-stereo.h"
#include "ipq40xx-adss.h"

struct dai_priv_st *dai_priv;
int dai_priv_size;

/*
struct clk *audio_tx_bclk;
struct clk *audio_tx_mclk;
struct clk *audio_rx_bclk;
struct clk *audio_rx_mclk;
*/

uint32_t ipq40xx_get_act_bit_width(uint32_t bit_width)
{
	switch (bit_width) {
	case SNDRV_PCM_FORMAT_S8:
	case SNDRV_PCM_FORMAT_U8:
		return __BIT_8;
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_S16_BE:
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_U16_BE:
		return __BIT_16;
	case SNDRV_PCM_FORMAT_S24_3LE:
	case SNDRV_PCM_FORMAT_S24_3BE:
	case SNDRV_PCM_FORMAT_U24_3LE:
	case SNDRV_PCM_FORMAT_U24_3BE:
		return __BIT_32;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S24_BE:
	case SNDRV_PCM_FORMAT_U24_LE:
	case SNDRV_PCM_FORMAT_U24_BE:
		return __BIT_24;
	case SNDRV_PCM_FORMAT_S32_LE:
	case SNDRV_PCM_FORMAT_S32_BE:
	case SNDRV_PCM_FORMAT_U32_LE:
	case SNDRV_PCM_FORMAT_U32_BE:
		return __BIT_32;
	default:
		return __BIT_INVAL;
	}
}

static int ipq40xx_audio_clk_get(struct clk **clk, struct device *dev,
					const char *id)
{

	*clk = devm_clk_get(dev, id);
	if (IS_ERR(*clk)) {
		dev_err(dev, "%s: Error in %s\n", __func__, id);
		return PTR_ERR(*clk);
	}

	return 0;
}

static int ipq40xx_audio_clk_set(struct clk *clk, struct device *dev,
					uint32_t val)
{
	int ret;

	ret = clk_set_rate(clk, val);
	if (ret != 0) {
		dev_err(dev, "%s: Error in setting %s\n", __func__,
					__clk_get_name(clk));
		return ret;
	}

	ret = clk_prepare_enable(clk);
	if (ret != 0) {
		dev_err(dev, "%s: Error in enable %s\n", __func__,
					__clk_get_name(clk));
		return ret;
	}

	return 0;
}

static int ipq40xx_audio_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{

	struct dai_priv_st *priv = snd_soc_component_get_drvdata(dai->component);
	uint32_t intf = intf_to_index(priv, dai->driver->id);
	int ret = 0;
	struct device *dev = &(priv[intf].pdev->dev);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* Check if the direction is enabled */
		if (priv[intf].tx_enabled != ENABLE)
			goto error;

		ipq40xx_glb_tx_data_port_en(ENABLE);
		ipq40xx_glb_tx_framesync_port_en(ENABLE);

		ret = ipq40xx_audio_clk_get(&(priv[intf].audio_tx_bclk), dev,
						"audio_tx_bclk");

		if (!ret && !(priv[intf].is_txmclk_fixed))
			ret = ipq40xx_audio_clk_get(&(priv[intf].audio_tx_mclk), dev,
						"audio_tx_mclk");

	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		/* Check if the direction is enabled */
		if (priv[intf].rx_enabled != ENABLE)
			goto error;

		ipq40xx_glb_rx_data_port_en(ENABLE);
		ipq40xx_glb_rx_framesync_port_en(ENABLE);

		ret = ipq40xx_audio_clk_get(&(priv[intf].audio_rx_bclk), dev,
						"audio_rx_bclk");
		if (!ret)
			ret = ipq40xx_audio_clk_get(&(priv[intf].audio_rx_mclk), dev,
						"audio_rx_mclk");
	}
	if (ret)
		return ret;

	if (intf == I2S || intf == I2S1 || intf == I2S2) {

		/* Select I2S mode */
		ipq40xx_glb_audio_mode(I2S, substream->stream);

	} else if (intf == TDM) {
		/* Select TDM mode */
		ipq40xx_glb_audio_mode(TDM, substream->stream);

		/* Set TDM Ctrl register */
		ipq40xx_glb_tdm_ctrl_sync_num(TDM_SYNC_NUM, substream->stream);
		ipq40xx_glb_tdm_ctrl_delay(TDM_DELAY, substream->stream);
	}

	return 0;
error:
	pr_err("%s: Direction not enabled\n", __func__);
	return -EFAULT;
}

/*
static int ipq40xx_audio_prepare(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s:%d\n", __func__, __LINE__);
	return 0;
}
*/

static int ipq40xx_audio_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,
					struct snd_soc_dai *dai)
{
	struct dai_priv_st *priv = snd_soc_component_get_drvdata(dai->component);
	uint32_t intf = intf_to_index(priv, dai->driver->id);
	uint32_t bit_width, channels, rate;
	uint32_t bit_act;
	int ret;
	uint32_t mclk, bclk;
	struct device *dev = &(priv[intf].pdev->dev);

	bit_width = params_format(params);
	channels = params_channels(params);
	rate = params_rate(params);

	bit_act = ipq40xx_get_act_bit_width(bit_width);

	if (intf == TDM) {

		/* Set TDM number of channels */
		ipq40xx_glb_tdm_ctrl_ch_num((channels-1), substream->stream);
		mclk = bclk = rate * bit_act * channels;
	} else {
		bclk = rate * bit_act * channels;
		if(bit_act == __BIT_16 )
			bclk = bclk*2;

		mclk = bclk * MCLK_MULTI;
	}

	ipq40xx_glb_clk_enable_oe(substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (!(priv[intf].is_txmclk_fixed)) {
			ret = ipq40xx_audio_clk_set(priv[intf].audio_tx_mclk,
									dev, mclk);
			if (ret)
				return ret;
		}

		ret = ipq40xx_audio_clk_set(priv[intf].audio_tx_bclk,
									dev, bclk);
		if (ret)
			return ret;
		
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = ipq40xx_audio_clk_set(priv[intf].audio_rx_mclk,
									dev, mclk);
		if (ret)
			return ret;

		ret = ipq40xx_audio_clk_set(priv[intf].audio_rx_bclk,
									dev, bclk);
		if (ret)
			return ret;
	}

	return 0;
}

static struct snd_soc_dai_ops ipq40xx_audio_ops = {
	.startup	= ipq40xx_audio_startup,
	.hw_params	= ipq40xx_audio_hw_params,
};

static int ipq40xx_audio_probe(struct snd_soc_dai* dai){
	for(int i=0; i<dai_priv_size; i++)
		if (dai_priv[i].interface == dai->driver->id)
		{
			snd_soc_dai_set_drvdata(dai, dai_priv);
			return 0;
		}

	return -EINVAL;
}

static struct snd_soc_dai_driver ipq40xx_cpu_dais[] = {
	{
		.probe = ipq40xx_audio_probe,
		.playback = {
			.stream_name = "Playback",
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= CH_STEREO,
			.channels_max	= CH_STEREO,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.capture = {
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= CH_STEREO,
			.channels_max	= CH_STEREO,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.ops = &ipq40xx_audio_ops,
		.id = I2S,
		.name = "qca-i2s-dai"
	},
	{
		.probe = ipq40xx_audio_probe,
		.playback = {
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= CH_STEREO,
			.channels_max	= CH_7_1,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.capture = {
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= CH_STEREO,
			.channels_max	= CH_7_1,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.ops = &ipq40xx_audio_ops,
		.id = TDM,
		.name = "qca-tdm-dai"
	},
	{
		.probe = ipq40xx_audio_probe,
		.playback = {
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= 2,
			.channels_max	= 2,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.ops = &ipq40xx_audio_ops,
		.id = I2S1,
		.name = "qca-i2s1-dai"
	},
	{
		.probe = ipq40xx_audio_probe,
		.playback = {
			.rates		= RATE_16000_96000,
			.formats	= SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
			.channels_min	= 2,
			.channels_max	= 2,
			.rate_min	= FREQ_16000,
			.rate_max	= FREQ_96000,
		},
		.ops = &ipq40xx_audio_ops,
		.id = I2S2,
		.name = "qca-i2s2-dai"
	}
};

static int ipq40xx_parse_of(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret = 0, i, offset;

	np = of_node_get(pdev->dev.of_node);
	if (!of_get_property(np, "platforms", &dai_priv_size))
		goto error_node;

	/* There should be 5 values for each Platform */
	dai_priv_size = dai_priv_size / (sizeof(u32) * 5);

	dai_priv = kmalloc_array(dai_priv_size, 
					sizeof(struct dai_priv_st), GFP_KERNEL);

	if (!dai_priv){
		ret = -ENOMEM;
		goto error_node;
	}
	
	platform_set_drvdata(pdev, dai_priv);
	
	for (i = 0; i < dai_priv_size; i++) {
		offset = i * 5;
	
		if (of_property_read_u32_index(np, "platforms",
						offset, &dai_priv[i].interface)){
			ret = -EFAULT;
			goto error_node;
		}

		if (of_property_read_u32_index(np, "platforms",
						offset + 1, &dai_priv[i].mbox_tx)){
			ret = -EFAULT;
			goto error_node;
		}

		if (of_property_read_u32_index(np, "platforms",
						offset + 2, &dai_priv[i].stereo_tx)){
			ret = -EFAULT;
			goto error_node;
		}

		if (of_property_read_u32_index(np, "platforms",
						offset + 3, &dai_priv[i].mbox_rx)){
			ret = -EFAULT;
			goto error_node;
		}

		if (of_property_read_u32_index(np, "platforms",
						offset + 4, &dai_priv[i].mbox_rx)){
			ret = -EFAULT;
			goto error_node;
		}

		/* TX is enabled only when both DMA and Stereo TX channel
		* is specified in the DTSi
		*/

		if ((dai_priv[i].mbox_tx >= 0)
			|| (dai_priv[i].stereo_tx >= 0))
			dai_priv[i].tx_enabled = ENABLE;

		/* RX is enabled only when both DMA and Stereo RX channel
		* is specified in the DTSi, except in case of SPDIF RX
		*/

		if ((dai_priv[i].mbox_rx < 0)) {
			if (dai_priv[i].interface == SPDIF) {
				dai_priv[i].rx_enabled = ENABLE;
				dai_priv[i].stereo_rx = MAX_STEREO_ENTRIES;
			} else if (dai_priv[i].stereo_rx >= 1) {
				dai_priv[i].rx_enabled = ENABLE;
			}
		}

		/* Either TX or Rx should have been enabled for a DMA/Stereo Channel */
		if (!(dai_priv[i].tx_enabled || dai_priv[i].rx_enabled)) {
			pr_err("%s: error reading critical device"
					" node properties\n", np->name);
			ret = -EFAULT;
			goto error_node;
		}

		if (of_property_read_u32(np, "ipq,txmclk-fixed",
					&dai_priv[i].is_txmclk_fixed))
			pr_debug("%s: ipq,txmclk-fixed not enabled\n", __func__);

		dai_priv[i].pdev = pdev;

	}
	return 0;

error_node:
	of_node_put(pdev->dev.of_node);
	return ret;
}

static const struct snd_soc_component_driver ipq40xx_i2s_component = {
	.name           = "qca-cpu-dai",
};

static const struct of_device_id ipq40xx_cpu_dai_id_table[] = {
	{ .compatible = "qca,ipq40xx-cpu" },
	{},
};

MODULE_DEVICE_TABLE(of, ipq40xx_cpu_dai_id_table);

static int ipq40xx_dai_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	int ret;

	match = of_match_device(ipq40xx_cpu_dai_id_table, &pdev->dev);
	if (!match) {
		ret = -ENODEV;
		goto error;
	}

	ret = snd_soc_register_component(&pdev->dev, &ipq40xx_i2s_component,
			 ipq40xx_cpu_dais, ARRAY_SIZE(ipq40xx_cpu_dais));

	if (ret) {
		dev_err(&pdev->dev,
			"%s: error registering soc dais\n", __func__);
		return ret;
	}

	ret = ipq40xx_parse_of(pdev);

	if (ret) {
		dev_err(&pdev->dev,
			"%s: error parsing soc dais\n", __func__);
		return ret;
	}

	ipq40xx_audio_adss_probe(pdev);

	return 0;

error:
	snd_soc_unregister_component(&pdev->dev);
	return ret;
}

static int ipq40xx_dai_remove(struct platform_device *pdev)
{
	ipq40xx_audio_adss_remove(pdev);
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static struct platform_driver ipq40xx_dai_driver = {
	.probe = ipq40xx_dai_probe,
	.remove = ipq40xx_dai_remove,
	.driver = {
		.name = "qca-cpu-dai",
		.owner = THIS_MODULE,
		.of_match_table = ipq40xx_cpu_dai_id_table,
	},
};

module_platform_driver(ipq40xx_dai_driver);

MODULE_ALIAS("platform:qca-cpu-dai");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("IPQ40xx CPU DAI DRIVER");
