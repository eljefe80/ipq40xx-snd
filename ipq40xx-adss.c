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
#include <linux/reset.h>
#include <linux/spinlock.h>

#include "ipq40xx-adss.h"

void __iomem *adss_audio_local_base;
struct reset_control *audio_blk_rst;
static spinlock_t i2s_ctrl_lock;
static spinlock_t tdm_ctrl_lock;
static spinlock_t glb_mode_lock;

/* I2S Interface Enable */
void ipq40xx_glb_i2s_interface_en(int enable)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&i2s_ctrl_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_CHIP_CTRL_I2S_REG);
	cfg &= ~(GLB_CHIP_CTRL_I2S_INTERFACE_EN);
	if (enable)
		cfg |= GLB_CHIP_CTRL_I2S_INTERFACE_EN;
	writel(cfg, adss_audio_local_base + ADSS_GLB_CHIP_CTRL_I2S_REG);
	spin_unlock_irqrestore(&i2s_ctrl_lock, flags);
	mdelay(5);
}

/* Enable Stereo0/Stereo1/Stereo2 channel */
void ipq40xx_glb_stereo_ch_en(int enable, int stereo_ch)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&i2s_ctrl_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_CHIP_CTRL_I2S_REG);
	if (stereo_ch == STEREO0) {
		cfg &= ~(GLB_CHIP_CTRL_I2S_STEREO0_GLB_EN);
		cfg |= GLB_CHIP_CTRL_I2S_STEREO0_GLB_EN;
	} else if (stereo_ch == STEREO1) {
		cfg &= ~(GLB_CHIP_CTRL_I2S_STEREO1_GLB_EN);
		cfg |= GLB_CHIP_CTRL_I2S_STEREO1_GLB_EN;
	} else if (stereo_ch == STEREO2) {
		cfg &= ~(GLB_CHIP_CTRL_I2S_STEREO2_GLB_EN);
		cfg |= GLB_CHIP_CTRL_I2S_STEREO2_GLB_EN;
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_CHIP_CTRL_I2S_REG);
	spin_unlock_irqrestore(&i2s_ctrl_lock, flags);
}

/* I2S Module Reset */
void ipq40xx_glb_i2s_reset(uint32_t reset)
{
	writel(GLB_I2S_RESET_VAL, adss_audio_local_base + ADSS_GLB_I2S_RST_REG);
	mdelay(5);
	writel(0x0, adss_audio_local_base + ADSS_GLB_I2S_RST_REG);
}

/* Enable I2S/TDM and Playback/Capture Audio Mode */
void ipq40xx_glb_audio_mode(int mode, int dir)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	if (mode == I2S && dir == PLAYBACK) {
		cfg &= ~(1);
		cfg |= GLB_AUDIO_MODE_XMIT_I2S;
	} else if (mode == I2S && dir == CAPTURE) {
		cfg &= ~(4);
		cfg |= GLB_AUDIO_MODE_RECV_I2S;
	} else if (mode == TDM && dir == PLAYBACK) {
		cfg &= ~(1);
		cfg |= GLB_AUDIO_MODE_XMIT_TDM;
	} else if (mode == TDM && dir == CAPTURE) {
		cfg &= ~(4);
		cfg |= GLB_AUDIO_MODE_RECV_TDM;
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

/* I2S0 TX Data Port Enable */
/* Todo : Check if bits 6:4 configures only
	  I2S0 or other channels as well */
void ipq40xx_glb_tx_data_port_en(uint32_t enable)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	cfg &= ~(GLB_AUDIO_MODE_I2S0_TXD_OE);
	if (enable)
		cfg |= GLB_AUDIO_MODE_I2S0_TXD_OE;
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

/* I2S3 RX Data Port Enable */
void ipq40xx_glb_rx_data_port_en(uint32_t enable)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	cfg &= ~(GLB_AUDIO_MODE_I2S3_RXD_OE);
	if (enable)
		cfg |= GLB_AUDIO_MODE_I2S3_RXD_OE;
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

/* Cross 1K Boundary */
void ipq40xx_glb_audio_mode_B1K(void)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg =  readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	cfg &= ~(GLB_AUDIO_MODE_B1K);
	cfg |= GLB_AUDIO_MODE_B1K;
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

/* Frame Sync Port Enable for I2S0 TX */
void ipq40xx_glb_tx_framesync_port_en(uint32_t enable)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg =  readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	cfg &= ~(GLB_AUDIO_MODE_I2S0_FS_OE);
	if (enable)
		cfg |= GLB_AUDIO_MODE_I2S0_FS_OE;
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

/* Frame Sync Port Enable for I2S3 RX */
void ipq40xx_glb_rx_framesync_port_en(uint32_t enable)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&glb_mode_lock, flags);
	cfg =  readl(adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	cfg &= ~(GLB_AUDIO_MODE_I2S3_FS_OE);
	if (enable)
		cfg |= GLB_AUDIO_MODE_I2S3_FS_OE;
	writel(cfg, adss_audio_local_base + ADSS_GLB_AUDIO_MODE_REG);
	spin_unlock_irqrestore(&glb_mode_lock, flags);
}

void ipq40xx_glb_clk_enable_oe(uint32_t dir)
{
	uint32_t cfg;

	cfg = readl(adss_audio_local_base + ADSS_GLB_CLK_I2S_CTRL_REG);

	if (dir == PLAYBACK) {
		cfg |= (GLB_CLK_I2S_CTRL_TX_BCLK_OE |
			GLB_CLK_I2S_CTRL_TX_MCLK_OE);
	} else {
		cfg |= (GLB_CLK_I2S_CTRL_RX_BCLK_OE |
			GLB_CLK_I2S_CTRL_RX_MCLK_OE);
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_CLK_I2S_CTRL_REG);
}

/* Channel Number Per Frame for Transmitter/Receiver
 * Real value = val + 1
 */
void ipq40xx_glb_tdm_ctrl_ch_num(uint32_t val, uint32_t dir)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&tdm_ctrl_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);

	if (dir == PLAYBACK) {
		cfg &= ~(GLB_TDM_CTRL_TX_CHAN_NUM_MASK);
		cfg |= GLB_TDM_CTRL_TX_CHAN_NUM(val);
	} else if (dir == CAPTURE) {
		cfg &= ~(GLB_TDM_CTRL_RX_CHAN_NUM_MASK);
		cfg |= GLB_TDM_CTRL_RX_CHAN_NUM(val);
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);
	spin_unlock_irqrestore(&tdm_ctrl_lock, flags);
}

/* FSYNC Hi Duration for Transmitter/Receiver */
void ipq40xx_glb_tdm_ctrl_sync_num(uint32_t val, uint32_t dir)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&tdm_ctrl_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);

	if (dir == PLAYBACK) {
		cfg &= ~(GLB_TDM_CTRL_TX_SYNC_NUM_MASK);
		cfg |= GLB_TDM_CTRL_TX_SYNC_NUM(val);
	} else if (dir == CAPTURE) {
		cfg &= ~(GLB_TDM_CTRL_RX_SYNC_NUM_MASK);
		cfg |= GLB_TDM_CTRL_RX_SYNC_NUM(val);
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);
	spin_unlock_irqrestore(&tdm_ctrl_lock, flags);
}

/* Serial Data Delay for transmitter/receiver */
void ipq40xx_glb_tdm_ctrl_delay(uint32_t delay, uint32_t dir)
{
	uint32_t cfg;
	unsigned long flags;

	spin_lock_irqsave(&tdm_ctrl_lock, flags);
	cfg = readl(adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);

	if (dir == PLAYBACK) {
		cfg &= ~(GLB_TDM_CTRL_TX_DELAY);
		if (delay)
			cfg |= GLB_TDM_CTRL_TX_DELAY;
	} else if (dir == CAPTURE) {
		cfg &= ~(GLB_TDM_CTRL_RX_DELAY);
		if (delay)
			cfg |= GLB_TDM_CTRL_RX_DELAY;
	}
	writel(cfg, adss_audio_local_base + ADSS_GLB_TDM_CTRL_REG);
	spin_unlock_irqrestore(&tdm_ctrl_lock, flags);
}

void ipq40xx_audio_adss_init(void)
{
	spin_lock_init(&i2s_ctrl_lock);
	spin_lock_init(&tdm_ctrl_lock);
	spin_lock_init(&glb_mode_lock);

	/* I2S in reset */
	ipq40xx_glb_i2s_reset(1);

	/* Enable I2S interface */
	ipq40xx_glb_i2s_interface_en(ENABLE);

	ipq40xx_glb_audio_mode_B1K();
}

static const struct of_device_id ipq40xx_audio_adss_id_table[] = {
	{ .compatible = "qca,ipq40xx-audio-adss" },
	{},
};

MODULE_DEVICE_TABLE(of, ipq40xx_audio_adss_id_table);

int ipq40xx_audio_adss_probe(struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	adss_audio_local_base = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(adss_audio_local_base))
		return PTR_ERR(adss_audio_local_base);

	audio_blk_rst = devm_reset_control_get(&pdev->dev, "blk_rst");
	if (IS_ERR(audio_blk_rst))
		return PTR_ERR(audio_blk_rst);

	ipq40xx_audio_adss_init();
	return 0;
}

int ipq40xx_audio_adss_remove(struct platform_device *pdev)
{
	ipq40xx_glb_i2s_interface_en(DISABLE);
	return 0;
}

MODULE_ALIAS("platform:ipq40xx-adss");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("IPQ40xx AUDIO ADSS driver");
