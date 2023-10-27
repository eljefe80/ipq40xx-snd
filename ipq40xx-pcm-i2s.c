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
#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <asm/dma.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <linux/pinctrl/consumer.h>

#include "ipq40xx-pcm.h"
#include "ipq40xx-mbox.h"
#include "ipq40xx-adss.h"
#include "ipq40xx-stereo.h"

static struct snd_pcm_hardware ipq40xx_pcm_hardware_playback = {
	.info			=	SNDRV_PCM_INFO_MMAP |
					SNDRV_PCM_INFO_BLOCK_TRANSFER |
					SNDRV_PCM_INFO_MMAP_VALID |
					SNDRV_PCM_INFO_INTERLEAVED |
					SNDRV_PCM_INFO_PAUSE |
					SNDRV_PCM_INFO_RESUME,
	.formats		=	SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
	.rates			=	RATE_16000_96000,
	.rate_min		=	FREQ_16000,
	.rate_max		=	FREQ_96000,
	.channels_min		=	CH_STEREO,
	.channels_max		=	CH_STEREO,
	.buffer_bytes_max	=	IPQ40xx_I2S_BUFF_SIZE,
	.period_bytes_max	=	IPQ40xx_I2S_BUFF_SIZE / 2,
	.period_bytes_min	=	IPQ40xx_I2S_PERIOD_BYTES_MIN,
	.periods_min		=	IPQ40xx_I2S_NO_OF_PERIODS,
	.periods_max		=	IPQ40xx_I2S_NO_OF_PERIODS,
	.fifo_size		=	0,
};

static struct snd_pcm_hardware ipq40xx_pcm_hardware_capture = {
	.info			=	SNDRV_PCM_INFO_MMAP |
					SNDRV_PCM_INFO_BLOCK_TRANSFER |
					SNDRV_PCM_INFO_MMAP_VALID |
					SNDRV_PCM_INFO_INTERLEAVED,
	.formats		=	SNDRV_PCM_FMTBIT_S16 |
					SNDRV_PCM_FMTBIT_S32,
	.rates			=	RATE_16000_96000,
	.rate_min		=	FREQ_16000,
	.rate_max		=	FREQ_96000,
	.channels_min		=	CH_STEREO,
	.channels_max		=	CH_STEREO,
	.buffer_bytes_max	=	IPQ40xx_I2S_BUFF_SIZE,
	.period_bytes_max	=	IPQ40xx_I2S_BUFF_SIZE / 2,
	.period_bytes_min	=	IPQ40xx_I2S_PERIOD_BYTES_MIN,
	.periods_min		=	IPQ40xx_I2S_NO_OF_PERIODS,
	.periods_max		=	IPQ40xx_I2S_NO_OF_PERIODS,
	.fifo_size		=	0,
};

/* Get Stereo channel ID based on I2S/TDM/SPDIF intf and direction */
uint32_t get_stereo_id(struct dai_priv_st *priv,
				struct snd_pcm_substream *substream)
{

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return priv->stereo_tx;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		return priv->stereo_rx;
	else
		return -EINVAL;
}

/* Get MBOX channel ID based on I2S/TDM/SPDIF intf and direction */
uint32_t get_mbox_id(struct dai_priv_st *priv,
				struct snd_pcm_substream *substream)
{
//	dev_dbg("%s:%d\n", __func__, __LINE__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return priv->mbox_tx;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		return priv->mbox_rx;
	else
		return -EINVAL;
}

static size_t ip40xx_dma_buffer_size(struct snd_pcm_hardware *pcm_hw)
{
	printk("%s %d\n", __func__, __LINE__);
	return pcm_hw->buffer_bytes_max +
		(pcm_hw->periods_min * sizeof(struct ipq40xx_mbox_desc));
}

/*
 * The MBOX descriptors and buffers should lie within the same 256MB
 * region. Because, the buffer address pointer (in the descriptor structure)
 * and descriptor base address pointer register share the same MSB 4 bits
 * which is configured in MBOX DMA Policy register.
 *
 * Hence ensure that the entire allocated region falls in a 256MB region.
 */
static int ipq40xx_mbox_buf_is_aligned(void *c_ptr, ssize_t size)
{
	u32 ptr = (u32)c_ptr;

	printk("%s %d\n", __func__, __LINE__);
	return (ptr & 0xF0000000) == ((ptr + size - 1) & 0xF0000000);
}

static struct device *ss2dev(struct snd_pcm_substream *substream)
{
	printk("%s %d\n", __func__, __LINE__);
	return substream->pcm->card->dev;
}

static int ipq40xx_pcm_preallocate_dma_buffer(struct snd_pcm *pcm,
						int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_pcm_hardware *pcm_hw;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size;
	u8 *area;
	dma_addr_t addr;
	u32 num_periods;
	struct device_node *np;

	printk("%s %d\n", __func__, __LINE__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		pcm_hw = &ipq40xx_pcm_hardware_playback;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		pcm_hw = &ipq40xx_pcm_hardware_capture;
	else
		return -EINVAL;

	np = of_node_get(pcm->card->dev->of_node);
	if (!(of_property_read_u32(np, "ipq,i2s-no-of-periods",
						&num_periods))) {
		pcm_hw->periods_min = num_periods;
		pcm_hw->periods_max = num_periods;
		pcm_hw->buffer_bytes_max =
			IPQ40xx_I2S_PERIOD_BYTES_MIN * num_periods;
		pcm_hw->period_bytes_max =
			(IPQ40xx_I2S_PERIOD_BYTES_MIN * num_periods) / 2;
	} else {
		dev_dbg(pcm->card->dev, "i2s-no-of-periods property not available\n");
	}

	size = ip40xx_dma_buffer_size(pcm_hw);
	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	/*
	 * Currently payload uses uncached memory.
	 * TODO: Eventually we will move to cached memory for payload
	 * and dma_map_single() will be used for Invalidating/Flushing
	 * the buffers.
	 */

	area = dma_alloc_coherent(pcm->card->dev, size, &addr, GFP_KERNEL);

	if (!area) {
		dev_info(ss2dev(substream), "Alloc coherent memory failed\n");
		return -ENOMEM;
	}

	if (!ipq40xx_mbox_buf_is_aligned(area, size)) {
//		dev_info(ss2dev(substream),
			 printk("First allocation %p not within 256M region\n", area);

		buf->area = dma_alloc_coherent(pcm->card->dev, size,
						&buf->addr, GFP_KERNEL);
		/*
		 * If we are here, the previously allocated buffer is not
		 * usable for the driver. Have to free it anyway regardless
		 * of the success/failure of the second allocation.
		 */
		dma_free_coherent(pcm->card->dev, size, area, addr);
		if (!buf->area) {
//			dev_info(ss2dev(substream),
				 printk("Second Alloc coherent memory failed\n");
			return -ENOMEM;
		}
	} else {
		buf->area = area;
		buf->addr = addr;
	}

	buf->bytes = pcm_hw->buffer_bytes_max;
	printk("%s %d\n", __func__, __LINE__);

	return 0;
}

static void ipq40xx_pcm_free_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream;
	struct snd_pcm_hardware *pcm_hw;
	struct snd_dma_buffer *buf;
	size_t size;

	printk("%s %d\n", __func__, __LINE__);
	substream = pcm->streams[stream].substream;
	buf = &substream->dma_buffer;
	switch (stream) {
	case SNDRV_PCM_STREAM_PLAYBACK:
		pcm_hw = &ipq40xx_pcm_hardware_playback;
		break;
	case SNDRV_PCM_STREAM_CAPTURE:
		pcm_hw = &ipq40xx_pcm_hardware_capture;
		break;
	}

	size = ip40xx_dma_buffer_size(pcm_hw);

	dma_free_coherent(pcm->card->dev, size, buf->area, buf->addr);

	buf->addr = 0;
	printk("%s %d\n", __func__, __LINE__);
}

static irqreturn_t ipq40xx_pcm_irq(int intrsrc, void *data)
{
	struct snd_pcm_substream *substream = data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv =
		(struct ipq40xx_pcm_rt_priv *)runtime->private_data;

//	printk("%s %d\n", __func__, __LINE__);
	if (pcm_rtpriv->mmap_flag)
		pcm_rtpriv->curr_pos =
			ipq40xx_mbox_get_played_offset_set_own(
					pcm_rtpriv->channel);
	else
		pcm_rtpriv->curr_pos =
			ipq40xx_mbox_get_played_offset(pcm_rtpriv->channel);

	snd_pcm_period_elapsed(substream);
//	printk("%s %d\n", __func__, __LINE__);

	return IRQ_HANDLED;
}

static snd_pcm_uframes_t ipq40xx_pcm_i2s_pointer(struct snd_soc_component *component,
				struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv;
	snd_pcm_uframes_t ret;

//	printk("%s %d\n", __func__, __LINE__);
	pcm_rtpriv = runtime->private_data;

	ret = bytes_to_frames(runtime, pcm_rtpriv->curr_pos);
//	printk("%s %d\n", __func__, __LINE__);
	return ret;
}

static int ipq40xx_pcm_i2s_copy(struct snd_soc_component *component,
				struct snd_pcm_substream *substream, int chan,
				snd_pcm_uframes_t hwoff, void __user *ubuf,
				snd_pcm_uframes_t frames)
{
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv = runtime->private_data;
	char *hwbuf;
	u32 offset, size;
	u32 period_size, i, no_of_descs;

	printk("%s %d\n", __func__, __LINE__);
	offset = frames_to_bytes(runtime, hwoff);
	size = frames_to_bytes(runtime, frames);
	period_size = pcm_rtpriv->period_size;

	hwbuf = buf->area + offset;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* At the EOF, the size of userdata to be copied might be
		 * greater/lesser than one period size. Since each descriptor
		 * transfers one period of data, the buffer is padded with 0s
		 * if the size to be copied is not a multiple of period size.
		 */
		if (size % period_size)
			memset(hwbuf + size, 0,
					period_size - (size % period_size));
		if (copy_from_user(hwbuf, ubuf, size))
			return -EFAULT;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (copy_to_user(ubuf, hwbuf, size))
			return -EFAULT;
	}

	no_of_descs = (size + (period_size - 1)) / period_size;

	for (i = 0; i < no_of_descs; i++) {
		ipq40xx_mbox_desc_own(pcm_rtpriv->channel,
					(offset / period_size), 1);
		offset += period_size;
	}

	if (pcm_rtpriv->dma_started)
		ipq40xx_mbox_dma_resume(pcm_rtpriv->channel);

	printk("%s %d\n", __func__, __LINE__);
	return 0;
}

static int ipq40xx_pcm_i2s_mmap(struct snd_soc_component *component,
				struct snd_pcm_substream *substream,
				struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv =
		(struct ipq40xx_pcm_rt_priv *)runtime->private_data;
	printk("%s %d\n", __func__, __LINE__);

	pcm_rtpriv->mmap_flag = 1;

	printk("%s %d\n", __func__, __LINE__);
	return dma_mmap_coherent(substream->pcm->card->dev, vma,
		runtime->dma_area, runtime->dma_addr, runtime->dma_bytes);
}

static int ipq40xx_pcm_hw_free(struct snd_soc_component *component,
				struct snd_pcm_substream *substream)
{
	printk("%s %d\n", __func__, __LINE__);
	snd_pcm_set_runtime_buffer(substream, NULL);
	printk("%s %d\n", __func__, __LINE__);
	return 0;
}


static int ipq40xx_pcm_i2s_prepare(struct snd_soc_component *component,
				struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv;

	uint32_t ret;

	printk("%s %d\n", __func__, __LINE__);
	pcm_rtpriv = runtime->private_data;

	ret = ipq40xx_mbox_dma_prepare(pcm_rtpriv->channel);
	if (ret) {
		pr_err("%s: %d: Error in dma prepare : channel : %d\n",
				__func__, __LINE__, pcm_rtpriv->channel);
		ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
		return ret;
	}

	/* Set the ownership bits only if mmap is used */
	if (pcm_rtpriv->mmap_flag == 1)
		ipq40xx_mbox_get_elapsed_size(pcm_rtpriv->channel);

	pcm_rtpriv->last_played = NULL;

	printk("%s %d\n", __func__, __LINE__);
	return ret;
}

static int ipq40xx_pcm_i2s_close(struct snd_soc_component *component,
				struct snd_pcm_substream *substream)
{
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv;
	uint32_t ret;
	printk("%s %d\n", __func__, __LINE__);
	pcm_rtpriv = substream->runtime->private_data;
	pcm_rtpriv->mmap_flag = 0;

	ret = ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
	if (ret) {
		pr_err("%s: %d: Error in dma release\n",
					__func__, __LINE__);
	}

	kfree(pcm_rtpriv);
	printk("%s %d\n", __func__, __LINE__);

	return 0;
}

static int ipq40xx_pcm_i2s_trigger(struct snd_soc_component *component,
				struct snd_pcm_substream *substream, int cmd)
{
	printk("Keen %s %d\r\n",__func__,__LINE__);
	int ret;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv =
				substream->runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *dai = asoc_rtd_to_cpu(rtd, 0);
	struct dai_priv_st *priv = snd_soc_dai_get_drvdata(dai);

//	printk("%s %d\n", __func__, __LINE__);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* Enable the I2S Stereo block for operation */
		ipq40xx_stereo_config_enable(ENABLE,
				get_stereo_id(priv, substream));
		ret = ipq40xx_mbox_dma_start(pcm_rtpriv->channel);
		if (ret) {
			pr_err("%s: %d: Error in dma start\n",
				__func__, __LINE__);
			ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
		}
		pcm_rtpriv->dma_started = 1;
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ret = ipq40xx_mbox_dma_resume(pcm_rtpriv->channel);
		if (ret) {
			pr_err("%s: %d: Error in dma resume\n",
				__func__, __LINE__);
			ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
		}
		pcm_rtpriv->dma_started = 1;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* Disable the I2S Stereo block */
		ipq40xx_stereo_config_enable(DISABLE,
				get_stereo_id(priv, substream));
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ret = ipq40xx_mbox_dma_stop(pcm_rtpriv->channel);
		if (ret) {
			pr_err("%s: %d: Error in dma stop\n",
				__func__, __LINE__);
			ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
		}
		pcm_rtpriv->dma_started = 0;
		break;
	default:
		ret = -EINVAL;
		break;
	}
//	printk("%s %d\n", __func__, __LINE__);
	return ret;
}

static int ipq40xx_pcm_i2s_hw_params(struct snd_soc_component *component,
				struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *hw_params)
{
	printk("Keen %s %d\r\n",__func__,__LINE__);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv =
				substream->runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *dai = asoc_rtd_to_cpu(rtd, 0);
	uint32_t bit_width, rate;
	struct dai_priv_st *priv = snd_soc_dai_get_drvdata(dai);
	int ret;
	int stereo_id = get_stereo_id(priv, substream)
	unsigned int period_size, sample_size, sample_rate, frames, channels;
	pr_debug("%s %d\n", __func__, __LINE__);

	pcm_rtpriv = runtime->private_data;
	printk("Keen %s %d\r\n",__func__,__LINE__);
	bit_width = params_format(hw_params);
	channels = params_channels(hw_params);
	rate = params_rate(hw_params);

	ipq40xx_config_master(ENABLE, stereo_id);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ret = ipq40xx_cfg_bit_width(bit_width, stereo_id);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	if (ret) {
		pr_err("%s: BitWidth %d not supported\n",
			__FUNCTION__, bit_width);
		return ret;
	}

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_enable(DISABLE, stereo_id);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_reset(ENABLE, stereo_id);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_mic_reset(ENABLE, stereo_id);

	mdelay(5);

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ret = ipq40xx_mbox_fifo_reset(get_mbox_id(priv, substream));
	if (ret) {
		pr_err("%s: %d: Error in dma fifo reset\n",
					__func__, __LINE__);
		return ret;
	}

	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_reset(DISABLE, stereo_id);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_mic_reset(DISABLE, stereo_id);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	ipq40xx_stereo_config_enable(ENABLE, stereo_id);
	ret = ipq40xx_mbox_form_ring(pcm_rtpriv->channel,
			runtime->dma_addr,
			runtime->dma_area,
/*			substream->dma_buffer.addr,
			substream->dma_buffer.area,*/
			params_period_bytes(hw_params),
			params_buffer_bytes(hw_params),
			(substream->stream == SNDRV_PCM_STREAM_CAPTURE));
	if (ret) {
		pr_err("%s: %d: Error dma form ring\n",
				__func__, __LINE__);
		ipq40xx_mbox_dma_release(pcm_rtpriv->channel);
		return ret;
	}

	period_size = params_period_bytes(hw_params);
	sample_size = snd_pcm_format_size(params_format(hw_params), 1);
	sample_rate = params_rate(hw_params);
	channels = params_channels(hw_params);
	frames = period_size / (sample_size * channels);
	pcm_rtpriv->period_size = params_period_bytes(hw_params);

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);

	runtime->dma_bytes = params_buffer_bytes(hw_params);
	printk("%s %d\n", __func__, __LINE__);
	return ret;
}

static int ipq40xx_pcm_i2s_open(struct snd_soc_component *component,
				struct snd_pcm_substream *substream)
{
	printk("Keen %s %d\r\n",__func__,__LINE__);
	int ret;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct ipq40xx_pcm_rt_priv *pcm_rtpriv;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
	printk("Keen %s %d\r\n",__func__,__LINE__);
    struct snd_soc_dai *dai = asoc_rtd_to_cpu(rtd, 0);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	struct dai_priv_st *priv = snd_soc_dai_get_drvdata(dai);
	printk("Keen %s %d\r\n",__func__,__LINE__);
	printk("%s %d\n", __func__, __LINE__);

	pcm_rtpriv = kmalloc(sizeof(struct ipq40xx_pcm_rt_priv), GFP_KERNEL);
	if (!pcm_rtpriv) {
		return -ENOMEM;
	}

	snd_printd("%s: 0x%xB allocated at 0x%08x\n",
			__FUNCTION__, sizeof(*pcm_rtpriv), (u32) pcm_rtpriv);
	pcm_rtpriv->last_played = NULL;
	pcm_rtpriv->dev = substream->pcm->card->dev;
	pcm_rtpriv->channel = get_mbox_id(priv, substream);
	substream->runtime->private_data = pcm_rtpriv;
	pcm_rtpriv->mmap_flag = 0;
	pcm_rtpriv->dma_started = 0;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		runtime->dma_bytes =
			ipq40xx_pcm_hardware_playback.buffer_bytes_max;
		snd_soc_set_runtime_hwparams(substream,
				&ipq40xx_pcm_hardware_playback);
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		runtime->dma_bytes =
				ipq40xx_pcm_hardware_capture.buffer_bytes_max;
		snd_soc_set_runtime_hwparams(substream,
					&ipq40xx_pcm_hardware_capture);

	} else {
		pr_err("%s: Invalid stream\n", __func__);
		ret = -EINVAL;
		goto error;
	}
	ret = ipq40xx_mbox_dma_init(pcm_rtpriv->dev,
		pcm_rtpriv->channel, ipq40xx_pcm_irq, substream);
	if (ret) {
		pr_err("%s: %d: Error initializing dma\n",
					__func__, __LINE__);
		goto error;
	}

	ret = snd_pcm_hw_constraint_integer(runtime,
			SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0) {
		pr_err("%s: snd_pcm_hw_constraint_integer failed\n", __func__);
		goto error;
	}
	return 0;
error:
	kfree(pcm_rtpriv);
	return ret;
}


static void ipq40xx_asoc_pcm_i2s_free(struct snd_soc_component *component,
					struct snd_pcm *pcm)
{
	ipq40xx_pcm_free_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
	ipq40xx_pcm_free_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
}

static int ipq40xx_asoc_pcm_i2s_new(struct snd_soc_component *component,
					struct snd_soc_pcm_runtime *prtd)
{
	struct snd_card *card = prtd->card->snd_card;
	struct snd_pcm *pcm = prtd->pcm;
	size_t size;
	int ret = 0;

	printk("%s %d\n", __func__, __LINE__);
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &card->dev->coherent_dma_mask;

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		size = ip40xx_dma_buffer_size(&ipq40xx_pcm_hardware_playback);
		snd_pcm_set_managed_buffer_all(pcm, SNDRV_DMA_TYPE_DEV,
				       card->dev, size, size);
/*		ret = ipq40xx_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_PLAYBACK);
*/
		if (ret) {
			pr_err("%s: %d: Error allocating dma buf\n",
						__func__, __LINE__);
			return -ENOMEM;
		}
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = ipq40xx_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_CAPTURE);
		if (ret) {
			pr_err("%s: %d: Error allocating dma buf\n",
						__func__, __LINE__);
			ipq40xx_pcm_free_dma_buffer(pcm,
					SNDRV_PCM_STREAM_PLAYBACK);
			return -ENOMEM;
		}
	}

	printk("%s %d\n", __func__, __LINE__);
	return ret;
}

static int ipq40xx_pcm_lib_ioctl(struct snd_soc_component *component,
				struct snd_pcm_substream *substream, unsigned int cmd,  void *arg) {
	printk("%s %d %i\n", __func__, __LINE__, cmd);
	return snd_pcm_lib_ioctl(substream, cmd, arg);
}


static struct snd_soc_component_driver ipq40xx_asoc_pcm_i2s_platform = {
        .open           = ipq40xx_pcm_i2s_open,
        .hw_params      = ipq40xx_pcm_i2s_hw_params,
        .hw_free        = ipq40xx_pcm_hw_free,
        .trigger        = ipq40xx_pcm_i2s_trigger,
        .ioctl          = ipq40xx_pcm_lib_ioctl,
        .close          = ipq40xx_pcm_i2s_close,
        .prepare        = ipq40xx_pcm_i2s_prepare,
        .mmap           = ipq40xx_pcm_i2s_mmap,
        .pointer        = ipq40xx_pcm_i2s_pointer,
        .copy_user      = ipq40xx_pcm_i2s_copy,
		.pcm_construct	= ipq40xx_asoc_pcm_i2s_new,
		.pcm_destruct	= ipq40xx_asoc_pcm_i2s_free,
};

static const struct of_device_id ipq40xx_pcm_i2s_id_table[] = {
	{ .compatible = "qca,ipq40xx-pcm-i2s" },
	{ .compatible = "qca,ipq40xx-pcm-i2s1" },
	{ .compatible = "qca,ipq40xx-pcm-i2s2" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, ipq40xx_pcm_i2s_id_table);

static int ipq40xx_pcm_i2s_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct dev_pin_info *pins;
	struct pinctrl_state *pin_state;

	printk("%s %d %s\n", __func__, __LINE__,__FILE__);

	pins = pdev->dev.pins;
	pin_state = pinctrl_lookup_state(pins->p, "audio");
	ret = devm_snd_soc_register_component(&pdev->dev,
			&ipq40xx_asoc_pcm_i2s_platform, NULL, 0);

	if (ret)
		dev_err(&pdev->dev, "%s: Failed to register i2s pcm device\n",
								__func__);
	pinctrl_select_state(pins->p, pin_state);

	ipq40xx_mbox_probe(pdev);
	ipq40xx_audio_stereo_probe(pdev);
	printk("%s %d %s\n", __func__, __LINE__,__FILE__);
	return ret;
}

static int ipq40xx_pcm_i2s_driver_remove(struct platform_device *pdev)
{
	printk("%s %d\n", __func__, __LINE__);
	snd_soc_unregister_component(&pdev->dev);
	printk("%s %d\n", __func__, __LINE__);

	return 0;
}

static struct platform_driver ipq40xx_pcm_i2s_driver = {
	.probe = ipq40xx_pcm_i2s_driver_probe,
	.remove = ipq40xx_pcm_i2s_driver_remove,
	.driver = {
		.name = "qca-pcm-i2s",
		.owner = THIS_MODULE,
		.of_match_table = ipq40xx_pcm_i2s_id_table,
	},
};

module_platform_driver(ipq40xx_pcm_i2s_driver);

MODULE_ALIAS("platform:qca-pcm-i2s");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("IPQ40xx PCM I2S Platform Driver");
