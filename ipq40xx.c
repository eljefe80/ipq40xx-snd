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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/jack.h>
#include <asm/io.h>
#include <linux/pinctrl/consumer.h>

#include "ipq40xx-adss.h"

struct ipq40xx_soc_priv {
	struct device *dev;
	struct snd_soc_card card;
        struct snd_soc_codec_conf codec_conf[1];
	struct snd_soc_dai_link *dai_links;
};

static int ipq40xx_init(struct snd_soc_pcm_runtime *rtd) {
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
	return 0;
}
static int ipq40xx_soc_startup(struct snd_pcm_substream *substream) {
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
	return 0;
}

static void ipq40xx_soc_shutdown(struct snd_pcm_substream *) {
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
}

static int ipq40xx_soc_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *param) {
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
	return 0;
}

static const struct snd_soc_ops ipq40xx_soc_ops = {
        .startup        = ipq40xx_soc_startup,
        .shutdown       = ipq40xx_soc_shutdown,
        .hw_params      = ipq40xx_soc_hw_params,
};

static int ipq40xx_soc_probe(struct ipq40xx_soc_priv *priv){
        struct device_node *node = priv->dev->of_node;
	struct device_node *dai_node, *codec_node, *platform_node;
        struct snd_soc_dai_link_component *compnent;
        int comp_count = 6, ret = 0;

	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
        if (!node)
                return 0;
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);

	codec_node = of_parse_phandle(node, "codec", 0);
        if (!codec_node) {
		dev_err(priv->dev, "QCA IP4019 Codec node is not provided\n");
		return -EINVAL;
        }

	dai_node = of_parse_phandle(node, "i2scpu", 0);
        if (!dai_node) {
		dev_err(priv->dev, "QCA IP4019 I2S cpu node is not provided\n");
		return -EINVAL;
        }

	platform_node = of_parse_phandle(node, "i2splatform", 0);
        if (!platform_node) {
		dev_err(priv->dev, "QCA IP4019 I2S platform node is not provided\n");
		return -EINVAL;
        }

        comp_count = 1;
        compnent = devm_kzalloc(priv->dev, comp_count * sizeof(*compnent),
                                GFP_KERNEL);
        priv->dai_links[0].cpus = &compnent[0];
        priv->dai_links[0].num_cpus = 1;
        priv->dai_links[0].codecs = &compnent[1];
        priv->dai_links[0].num_codecs = 1;
        priv->dai_links[0].platforms = &compnent[2];
        priv->dai_links[0].num_platforms = 1;

        priv->dai_links[0].name = "IPQ4019 SOC Playback";
        priv->dai_links[0].stream_name = "I2S";
//	priv->dai_links[0].cpus->dai_name = "qca-cpu-dai";
	priv->dai_links[0].cpus->dai_name = "qca-i2s-dai";
        priv->dai_links[0].cpus->of_node = dai_node;
	priv->dai_links[0].platforms->dai_name = "qca-pcm-i2s",
//	priv->dai_links[0].platforms->name = "qca-pcm-i2s",
        priv->dai_links[0].platforms->of_node = platform_node;
        priv->dai_links[0].codecs->of_node = codec_node;
        priv->dai_links[0].codecs->dai_name = "tas5782m";
        priv->dai_links[0].playback_only = 1;
        priv->dai_links[0].id = 0;
        priv->dai_links[0].dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_INV_MASK;
        priv->dai_links[0].init = ipq40xx_init,
        priv->dai_links[0].ops = &ipq40xx_soc_ops,
        priv->dai_links[0].fully_routed = true,
        of_node_put(platform_node);
        of_node_put(codec_node);
        of_node_put(dai_node);
        return ret;
}

/*
static struct snd_soc_dai_link_component ipq40xx_dai_link_cpus = {
		.dai_name = "qca-i2s-dai",
		.name = "qca-i2s-dai",
		.of_node = of_parse_phandle(node, "", 0),
		//.of_node =
};
static struct snd_soc_dai_link_component ipq40xx_dai_link_codecs = {
		.dai_name = "alc1312_codec.1-001a",
		.name = "alc1312-aif1"
		.of_node = of_parse_phandle(node, "", 0),
		//.of_node =
};
static struct snd_soc_dai_link_component ipq40xx_dai_link_platforms = {
//		.dai_name = "qca-pcm-i2s",
		.name = "qca-pcm-i2s",
//		.dai_name = "7709000.qca-pcm-i2s",
		.dai_name = "qca-i2s-codec-dai",
		.of_node = of_parse_phandle(node, "", 0),
};
*/
static struct snd_soc_dai_link ipq40xx_snd_dai[] = {
	/* Front end DAI Links */
	{
		.name		= "IPQ40xx Media1",
		.stream_name	= "I2S",
//		.cpus		= &ipq40xx_dai_link_cpus,
		.num_cpus	= 1,
//		.codecs		= &ipq40xx_dai_link_codecs,
		.num_codecs	= 1,
//		.platforms	= &ipq40xx_dai_link_platforms,
		.num_platforms	= 1,

		/* Front End DAI Name */
//		.cpu_dai_name	= "qca-i2s-dai",
		/* Platform Driver Name */
//		.platform_name	= "7709000.qca-pcm-i2s",
		/* Codec DAI Name */
//		.codec_dai_name	= "alc1312-aif1",
		/*Codec Driver Name */
//		.codec_name	= "alc1312_codec.1-001a",

	},
#if 0
	{
		.name		= "IPQ40xx Media2",
		.stream_name	= "TDM",
		.cpu_dai_name	= "qca-tdm-dai",
		.platform_name	= "7709000.qca-pcm-tdm",
		.codec_dai_name	= "qca-tdm-codec-dai",
		.codec_name	= "qca_codec.0-0012",
	},
	{
		.name		= "IPQ40xx Media3",
		.stream_name	= "I2S1",
		.cpu_dai_name	= "qca-i2s1-dai",
		.platform_name	= "770b000.qca-pcm-i2s1",
		.codec_dai_name	= "qca-i2s1-codec-dai",
		.codec_name	= "qca_codec.0-0012",
	},
	{
		.name		= "IPQ40xx Media4",
		.stream_name	= "I2S2",
		.cpu_dai_name	= "qca-i2s2-dai",
		.platform_name	= "770d000.qca-pcm-i2s2",
		.codec_dai_name	= "qca-i2s2-codec-dai",
		.codec_name	= "qca_codec.0-0012",
	},
	{
		.name           = "IPQ40xx Media5",
		.stream_name    = "SPDIF",
		.cpu_dai_name   = "qca-spdif-dai",
		.platform_name  = "7707000.qca-pcm-spdif",
		.codec_dai_name = "qca-spdif-codec-dai",
		.codec_name	= "qca_codec.0-0012",
	},
#endif
};

static const struct snd_soc_dapm_route ipq4019_audio_map[] = {
	{"Amp", NULL, "DAC"},
        {"DAC", NULL, "AIF1RX"},
};
static const struct snd_soc_dapm_widget ipq4019_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN("AIF1RX", "IPQ4019 I2S", 0, SND_SOC_NOPM, 0, 0),
//	SND_SOC_DAPM_AIF_IN("AIF1RX", "AIF1 Playback", 0, SND_SOC_NOPM, 0, 0),
        /* Output Lines */
        SND_SOC_DAPM_DAC("DAC", NULL, SND_SOC_NOPM, 0, 0),
        SND_SOC_DAPM_OUTPUT("Amp"),
};
static struct snd_soc_card snd_soc_card_qca = {
	.name			= "ipq40xx_snd_card",
	.dai_link		= ipq40xx_snd_dai,
	.num_links		= ARRAY_SIZE(ipq40xx_snd_dai),
	.owner			= THIS_MODULE,
	.num_dapm_routes	= ARRAY_SIZE(ipq4019_audio_map),
	.dapm_routes		= ipq4019_audio_map,
	.num_dapm_widgets	= ARRAY_SIZE(ipq4019_dapm_widgets),
	.dapm_widgets		= ipq4019_dapm_widgets,
};

static const struct of_device_id ipq40xx_audio_id_table[] = {
	{
		.compatible	= "qca,ipq40xx-audio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, ipq40xx_audio_id_table);


static int ipq40xx_audio_probe(struct platform_device *pdev)
{
	int ret;
	struct ipq40xx_soc_priv *priv;
	struct snd_soc_card *card;
	struct dev_pin_info *pins;
	struct pinctrl_state *pin_state;

        priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
        if (!priv)
                return -ENOMEM;
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
        priv->dai_links = devm_kcalloc(&pdev->dev, 3,
                                       sizeof(*priv->dai_links), GFP_KERNEL);
        priv->dev = &pdev->dev;
        card = &priv->card;
	card->dev = &pdev->dev;
        card->name = "IPQ4019 Audio Interface";
	pins = card->dev->pins;
        card->owner = THIS_MODULE;
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
/*
 * If the sound card registration fails, then the audio TLMM change
 * is also reverted. Due to this, the pins are seen to toggle causing
 * pop noise. To avoid this, the pins are set to GPIO state and moved
 * to audio functionality only when the sound card registration is
 * successful.
 */
        gpio_direction_output(28, 1);
        /* 20us sleep required after pulling the reset gpio to LOW */
        usleep_range(20, 30);
        gpio_set_value(28, 1);
        /* 20us sleep required after pulling the reset gpio to HIGH */
        usleep_range(20, 30);
	pin_state = pinctrl_lookup_state(pins->p, "default");
	if (IS_ERR(pin_state)) {
		pr_err("audio pinctrl state not available\n");
		return PTR_ERR(pin_state);
	}
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
        ipq40xx_soc_probe(priv);

        card->dai_link = priv->dai_links;
        card->num_links = 1;

        card->codec_conf = priv->codec_conf;
        card->num_configs = 0;

        snd_soc_card_set_drvdata(card, priv);

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret) {
		pr_err("snd_soc_register_card() failed:%d\n", ret);
		printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
		return ret;
	}
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);

	ipq40xx_audio_adss_init();
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);

	pinctrl_select_state(pins->p, pin_state);
	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);

	return ret;
}

static int ipq40xx_audio_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	printk("<3> Keen %s %d \r\n",__FUNCTION__,__LINE__);
	snd_soc_unregister_card(card);
	card->dev = NULL;

	return 0;
}

static struct platform_driver ipq40xx_audio_driver = {
	.driver = {
		.name = "ipq40xx_audio",
		.owner = THIS_MODULE,
		.of_match_table = ipq40xx_audio_id_table,
                .pm = &snd_soc_pm_ops,
	},
	.probe = ipq40xx_audio_probe,
	.remove = ipq40xx_audio_remove,
};
module_platform_driver(ipq40xx_audio_driver);

MODULE_ALIAS("platform:ipq40xx_audio");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("ALSA SoC IPQ40xx Machine Driver");
