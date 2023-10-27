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


/*
struct dai_priv_st dai_priv[MAX_INTF];
*/
struct clk *audio_tx_bclk;
struct clk *audio_tx_mclk;
struct clk *audio_rx_bclk;
struct clk *audio_rx_mclk;


/* Get Stereo channel ID based on I2S/TDM/SPDIF intf and direction */
uint32_t get_stereo_id(struct dai_priv_st **priv,
				struct snd_pcm_substream *substream, int intf)
{

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return priv[intf]->stereo_tx;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		return priv[intf]->stereo_rx;
	else
		return -EINVAL;
}
EXPORT_SYMBOL(get_stereo_id);

/* Get MBOX channel ID based on I2S/TDM/SPDIF intf and direction */
uint32_t get_mbox_id(struct dai_priv_st **priv,
				struct snd_pcm_substream *substream, int intf)
{
//	dev_dbg("%s:%d\n", __func__, __LINE__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return priv[intf]->mbox_tx;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		return priv[intf]->mbox_rx;
	else
		return -EINVAL;
}
EXPORT_SYMBOL(get_mbox_id);

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
	dev_dbg(dev, "%s:%d\n", __func__, __LINE__);
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

	printk("%s:%d:%s\n", __func__, __LINE__,  __clk_get_name(clk));
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
	printk("Set clk rate %d, actual %d", val, clk_get_rate(clk));
	return 0;
}

static void ipq40xx_audio_clk_disable(struct clk **clk, struct device *dev)
{
	dev_dbg(dev, "%s:%d\n", __func__, __LINE__);
	if (*clk) {
		if (__clk_is_enabled(*clk))
			clk_disable_unprepare(*clk);
		devm_clk_put(dev, *clk);
	}
}

static int ipq40xx_audio_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct dai_priv_st **priv = snd_soc_component_get_drvdata(dai->component);
	uint32_t intf = intf_to_index(priv, dai->driver->id);
	int ret = 0;
	struct device *dev = &(priv[intf]->pdev->dev);
	dev_dbg(dev, "%s:%d\n", __func__, __LINE__);
	printk("%s:%d\n", __func__, __LINE__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* Check if the direction is enabled */
		if (priv[intf]->tx_enabled != ENABLE)
			goto error;

		ipq40xx_glb_tx_data_port_en(ENABLE);
		ipq40xx_glb_tx_framesync_port_en(ENABLE);

		ret = ipq40xx_audio_clk_get(&audio_tx_bclk, dev,
						"audio_tx_bclk");
		printk("Return from ipq40xx_audio_clk_get: %d", ret);
		if (!ret && !(priv[intf]->is_txmclk_fixed))
			ret = ipq40xx_audio_clk_get(&audio_tx_mclk, dev,
						"audio_tx_mclk");

	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		/* Check if the direction is enabled */
		if (priv[intf]->rx_enabled != ENABLE)
			goto error;

		ipq40xx_glb_rx_data_port_en(ENABLE);
		ipq40xx_glb_rx_framesync_port_en(ENABLE);

		ret = ipq40xx_audio_clk_get(&audio_rx_bclk, dev,
						"audio_rx_bclk");
		if (!ret)
			ret = ipq40xx_audio_clk_get(&audio_rx_mclk, dev,
						"audio_rx_mclk");
	}
	if (ret)
		return ret;

	if (intf == I2S || intf == I2S1 || intf == I2S2) {
		/* Select I2S mode */
		printk("%s:%d\n", __func__, __LINE__);
		ipq40xx_glb_audio_mode(I2S, substream->stream);
	} else if (intf == TDM) {
		/* Select TDM mode */
		ipq40xx_glb_audio_mode(TDM, substream->stream);

		/* Set TDM Ctrl register */
		ipq40xx_glb_tdm_ctrl_sync_num(TDM_SYNC_NUM, substream->stream);
		ipq40xx_glb_tdm_ctrl_delay(TDM_DELAY, substream->stream);
	}

	printk("%s:%d\n", __func__, __LINE__);
	dev_dbg(dev, "%s:%d\n", __func__, __LINE__);

	return 0;
error:
	pr_err("%s: Direction not enabled\n", __func__);
	return -EFAULT;
}

static int ipq40xx_audio_prepare(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s:%d\n", __func__, __LINE__);
	return 0;
}

static int ipq40xx_audio_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,
					struct snd_soc_dai *dai)
{
	struct dai_priv_st **priv = snd_soc_component_get_drvdata(dai->component);
	uint32_t intf = intf_to_index(priv, dai->driver->id);
	uint32_t stereo_id = get_stereo_id(priv, substream, intf);
	uint32_t mbox_id = get_mbox_id(priv, substream, intf);
	uint32_t bit_width, channels, rate;
	uint32_t bit_act;
	int ret;
	uint32_t mclk, bclk;
	struct device *dev = &(priv[intf]->pdev->dev);

	dev_dbg(dev, "%s:%d\n", __func__, __LINE__);
	bit_width = params_format(params);
	channels = params_channels(params);
	rate = params_rate(params);

	bit_act = ipq40xx_get_act_bit_width(bit_width);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	if (intf == TDM) {

		/* Set TDM number of channels */
		printk("Keen %s %d\r\n",__func__,__LINE__);
		ipq40xx_glb_tdm_ctrl_ch_num((channels-1), substream->stream);
		mclk = bclk = rate * bit_act * channels;
	} else {
		printk("Keen %s %d\r\n",__func__,__LINE__);
		//bclk = rate * bit_act * channels;
		bclk = rate * bit_act * channels;
		/* Keen fixed 16bit/64fs */
		if(bit_act == __BIT_16 )
		{
			printk("<3> Keen Fixed clk %s %d \r\n",__FUNCTION__,__LINE__);
			bclk = bclk*2;
		}
		mclk = bclk * MCLK_MULTI;
		printk("<3> Keen bclk=%d mclk=%d %s %d \r\n",bclk,mclk,__FUNCTION__,__LINE__);
	}

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_glb_clk_enable_oe(substream->stream);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (!(priv[intf]->is_txmclk_fixed)) {
			ret = ipq40xx_audio_clk_set(audio_tx_mclk, dev, mclk);
			if (ret)
				return ret;
		}

		ret = ipq40xx_audio_clk_set(audio_tx_bclk, dev, bclk);
		if (ret)
			return ret;
		//alc1312_pdb_ctrl(1);
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = ipq40xx_audio_clk_set(audio_rx_mclk, dev, mclk);
		if (ret)
			return ret;

		ret = ipq40xx_audio_clk_set(audio_rx_bclk, dev, bclk);
		if (ret)
			return ret;
	}

	printk("Keen %s %d\r\n",__func__,__LINE__);
	return 0;
}

static void ipq40xx_audio_shutdown(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	printk("Keen %s %d\r\n",__func__,__LINE__);
#if 0 /* Keen@foxconn bclk always enable */ 
	uint32_t intf = dai->driver->id;
	struct device *dev = &(dai_priv[intf].pdev->dev);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		alc1312_pdb_ctrl(0);
		ipq40xx_glb_tx_data_port_en(DISABLE);
		ipq40xx_glb_tx_framesync_port_en(DISABLE);

		/* Disable the clocks */
		ipq40xx_audio_clk_disable(&audio_tx_bclk, dev);
		ipq40xx_audio_clk_disable(&audio_tx_mclk, dev);
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ipq40xx_glb_rx_data_port_en(DISABLE);
		ipq40xx_glb_rx_framesync_port_en(DISABLE);

		/* Disable the clocks */
		ipq40xx_audio_clk_disable(&audio_rx_bclk, dev);
		ipq40xx_audio_clk_disable(&audio_rx_mclk, dev);
	}
	/* Disable the I2S Stereo block */
	ipq40xx_stereo_config_enable(DISABLE, get_stereo_id(substream, intf));
#endif
      /* Keen@foxconn playback control */
      //alc1312_pdb_ctrl(0);
}

static int ipq40xx_audio_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	dev_dbg(dai->dev, "%s:%d\n", __func__, __LINE__);
	return 0;
}

static struct snd_soc_dai_ops ipq40xx_audio_ops = {
	.startup	= ipq40xx_audio_startup,
	.prepare	= ipq40xx_audio_prepare,
	.hw_params	= ipq40xx_audio_hw_params,
	.shutdown	= ipq40xx_audio_shutdown,
	.set_fmt	= ipq40xx_audio_set_fmt,
};

static struct snd_soc_dai_driver ipq40xx_cpu_dais[] = {
	{
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
static const struct snd_soc_component_driver ipq40xx_i2s_component = {
	.name           = "qca-cpu-dai",
};

static const struct of_device_id ipq40xx_cpu_dai_id_table[] = {
	{ .compatible = "qca,ipq40xx-cpu" },
/*
	{ .compatible = "qca,ipq40xx-i2s", .data = (void *)I2S},
	{ .compatible = "qca,ipq40xx-tdm", .data = (void *)TDM},
	{ .compatible = "qca,ipq40xx-spdif", .data = (void *)SPDIF},
	{ .compatible = "qca,ipq40xx-i2s1", .data = (void *)I2S1},
	{ .compatible = "qca,ipq40xx-i2s2", .data = (void *)I2S2},
*/
	{},
};
MODULE_DEVICE_TABLE(of, ipq40xx_cpu_dai_id_table);

static int ipq40xx_dai_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device_node *np = NULL;
	struct dai_priv_st** priv;
	int ret, tmp, num_plats, i, offset;
	int intf;

	printk("Keen %s %d\r\n",__func__,__LINE__);
	match = of_match_device(ipq40xx_cpu_dai_id_table, &pdev->dev);
	if (!match) {
		ret = -ENODEV;
		goto error;
	}
	printk("Keen %s %d\r\n",__func__,__LINE__);
	ret = snd_soc_register_component(&pdev->dev, &ipq40xx_i2s_component,
			 ipq40xx_cpu_dais, ARRAY_SIZE(ipq40xx_cpu_dais));
	if (ret) {
		dev_err(&pdev->dev,
			"%s: error registering soc dais\n", __func__);
		return ret;
	}
	printk("Keen %s %d\r\n",__func__,__LINE__);
	np = of_node_get(pdev->dev.of_node);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	if (!of_get_property(np, "platforms", &tmp))
		goto error_node;
	/* There should be 5 values for each Platform */
	printk("Keen %s %d\r\n",__func__,__LINE__);
	num_plats = tmp / (sizeof(u32) * 5);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	priv = kmalloc(num_plats * sizeof(struct dai_priv_st), GFP_KERNEL);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	platform_set_drvdata(pdev, priv);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	for (i = 0; i < num_plats; i++) {
		offset = i * 5;
		if (of_property_read_u32_index(np, "platforms", offset, &(priv[i]->interface)))
			goto error_node;

		if (of_property_read_u32_index(np, "platforms", offset + 1, &(priv[i]->mbox_tx)))
			goto error_node;

		if (of_property_read_u32_index(np, "platforms", offset + 2, &(priv[i]->stereo_tx)))
			goto error_node;

		if (of_property_read_u32_index(np, "platforms", offset + 3, &(priv[i]->mbox_rx)))
			goto error_node;

		if (of_property_read_u32_index(np, "platforms", offset + 3, &(priv[i]->stereo_rx)))
			goto error_node;

		/* TX is enabled only when both DMA and Stereo TX channel
		* is specified in the DTSi
		*/
	printk("Keen %s %d\r\n",__func__,__LINE__);
		if ((priv[i]->mbox_tx >= 0)
			|| (priv[i]->stereo_tx >= 0)) {
			priv[i]->tx_enabled = ENABLE;
		}
	printk("Keen %s %d\r\n",__func__,__LINE__);
		/* RX is enabled only when both DMA and Stereo RX channel
		* is specified in the DTSi, except in case of SPDIF RX
		*/

		if ((priv[i]->mbox_rx < 0)) {
			if (priv[i]->interface == SPDIF) {
				priv[i]->rx_enabled = ENABLE;
				priv[i]->stereo_rx = MAX_STEREO_ENTRIES;
			} else if (priv[i]->stereo_rx >= 1) {
				priv[i]->rx_enabled = ENABLE;
			}
		}
	printk("Keen %s %d\r\n",__func__,__LINE__);
		/* Either TX or Rx should have been enabled for a DMA/Stereo Channel */
		if (!(priv[i]->tx_enabled || priv[i]->rx_enabled)) {
			pr_err("%s: error reading critical device"
					" node properties\n", np->name);
			ret = -EFAULT;
			goto error_node;

		}
/*
	if (of_property_read_u32(np, "ipq,txmclk-fixed",
					&priv[i].is_txmclk_fixed))
		pr_debug("%s: ipq,txmclk-fixed not enabled\n", __func__);
*/
			priv[i]->pdev = pdev;

	}

	printk("Keen %s %d\r\n",__func__,__LINE__);
	of_node_put(pdev->dev.of_node);
	ipq40xx_audio_adss_probe(pdev);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	return 0;
error_node:
	of_node_put(pdev->dev.of_node);
error:
	snd_soc_unregister_component(&pdev->dev);
	return ret;
}

static int ipq40xx_dai_remove(struct platform_device *pdev)
{
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
