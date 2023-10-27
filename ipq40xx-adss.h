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

#ifndef IPQ40xx_ADSS_H
#define IPQ40xx_ADSS_H

#include "ipq40xx-common.h"
/* ADSS AUDIO Registers */

#define ADSS_BASE 	0x7700000

/* ADSS_AUDIO_LOCAL_REG Registers */

#define ADSS_GLB_PCM_RST_REG			0x0
#define GLB_PCM_RST_CTRL(x)			(x << 0)

#define ADSS_GLB_PCM_MBOX_CTRL_REG		0x0C

#define ADSS_GLB_CHIP_CTRL_I2S_REG		0x10
#define GLB_CHIP_CTRL_I2S_INTERFACE_EN		(1 << 0)
#define GLB_CHIP_CTRL_I2S_STEREO0_GLB_EN	(1 << 1)
#define GLB_CHIP_CTRL_I2S_STEREO1_GLB_EN	(1 << 2)
#define GLB_CHIP_CTRL_I2S_STEREO2_GLB_EN	(1 << 3)

#define ADSS_GLB_I2S_RST_REG		0x14
#define GLB_I2S_RST_CTRL_MBOX0		(1 << 0)
#define GLB_I2S_RST_CTRL_I2S0		(1 << 1)
#define GLB_I2S_RST_CTRL_MBOX3		(1 << 2)
#define GLB_I2S_RST_CTRL_I2S3		(1 << 3)
#define GLB_I2S_RESET_VAL		0xF

#define ADSS_GLB_CLK_I2S_CTRL_REG	0x18
#define GLB_CLK_I2S_CTRL_TX_BCLK_OE	(1 << 28)
#define GLB_CLK_I2S_CTRL_RX_BCLK_OE	(1 << 27)
#define GLB_CLK_I2S_CTRL_RX_MCLK_OE	(1 << 16)
#define GLB_CLK_I2S_CTRL_TX_MCLK_OE	(1 << 17)

#define ADSS_GLB_TDM_CTRL_REG		0x1C
#define GLB_TDM_CTRL_TX_CHAN_NUM(x)	(x << 0)
#define GLB_TDM_CTRL_TX_CHAN_NUM_MASK	0xF
#define GLB_TDM_CTRL_TX_SYNC_NUM(x)	(x << 4)
#define GLB_TDM_CTRL_TX_SYNC_NUM_MASK	(0x1F << 4)
#define GLB_TDM_CTRL_RX_CHAN_NUM(x)	(x << 16)
#define GLB_TDM_CTRL_RX_CHAN_NUM_MASK	(0xF << 16)
#define GLB_TDM_CTRL_RX_SYNC_NUM(x)	(x << 20)
#define GLB_TDM_CTRL_RX_SYNC_NUM_MASK	(0x1F << 20)
#define GLB_TDM_CTRL_TX_DELAY		(1 << 25)
#define GLB_TDM_CTRL_RX_DELAY		(1 << 26)

#define ADSS_GLB_PWM0_CTRL_REG		0x20

#define ADSS_GLB_PWM1_CTRL_REG		0x24

#define ADSS_GLB_PWM2_CTRL_REG		0x28

#define ADSS_GLB_PWM3_CTRL_REG		0x2C

#define ADSS_GLB_AUDIO_MODE_REG		0x30
#define GLB_AUDIO_MODE_RECV_I2S		(0 << 2)
#define GLB_AUDIO_MODE_RECV_TDM		(1 << 2)
#define GLB_AUDIO_MODE_XMIT_I2S		(0 << 0)
#define GLB_AUDIO_MODE_XMIT_TDM		(1 << 0)
#define GLB_AUDIO_MODE_I2S0_TXD_OE	(7 << 4)
#define GLB_AUDIO_MODE_I2S0_FS_OE	(1 << 7)
#define GLB_AUDIO_MODE_I2S3_FS_OE	(1 << 8)
#define GLB_AUDIO_MODE_I2S3_RXD_OE	(1 << 9)
#define GLB_AUDIO_MODE_SPDIF_OUT_OE	(1 << 10)
#define GLB_AUDIO_MODE_I2S0_RD_SWAP	(0 << 16)
#define GLB_AUDIO_MODE_I2S0_WR_SWAP 	(0 << 17)
#define GLB_AUDIO_MODE_I2S1_RD_SWAP	(0 << 18)
#define GLB_AUDIO_MODE_I2S1_WR_SWAP	(0 << 19)
#define GLB_AUDIO_MODE_I2S2_RD_SWAP	(0 << 20)
#define GLB_AUDIO_MODE_I2S2_WR_SWAP	(0 << 21)
#define GLB_AUDIO_MODE_I2S3_RD_SWAP	(0 << 22)
#define GLB_AUDIO_MODE_I2S3_WR_SWAP	(0 << 23)
#define GLB_AUDIO_MODE_B1K		(1 << 28)

#define ADSS_GLB_AUDIO_MODE2_REG	0x34

#define ADSS_AUDIO_PLL_CONFIG_REG	0x38
#define AUDIO_PLL_CONFIG_REFDIV(x)	(x << 0)
#define AUDIO_PLL_CONFIG_REFDIV_MASK	(0x7 << 0)
#define AUDIO_PLL_CONFIG_PLLPWD		(1 << 5)
#define AUDIO_PLL_CONFIG_POSTPLLDIV(x)	(x << 7)
#define AUDIO_PLL_CONFIG_POSTPLLDIV_MASK	(0x7 << 7)

#define ADSS_AUDIO_PLL_MODULATION_REG		0x3C
#define AUDIO_PLL_MODULATION_TGT_DIV_INT(x)	(x << 1)
#define AUDIO_PLL_MODULATION_TGT_DIV_MASK	(0x1FFFFFFE)
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC(x)	(x << 11)

#define ADSS_AUDIO_PLL_MOD_STEP_REG		0x40

#define ADSS_CURRENT_AUDIO_PLL_MODULATION_REG	0x44

#define ADSS_AUDIO_PLL_CONFIG1_REG		0x48
#define AUDIO_PLL_CONFIG1_SRESET_L(x)		(x << 0)

#define ADSS_AUDIO_ATB_SETTING_REG		0x4C

#define ADSS_AUDIO_RXB_CFG_MUXR_REG		0x104
#define AUDIO_RXB_CFG_MUXR_SRC_SEL(x)		(x << 8)

#define ADSS_AUDIO_RXB_MISC_REG			0x108
#define AUDIO_RXB_MISC_AUTO_SCALE_DIV(x)	(x << 1)

#define ADSS_AUDIO_RXB_CBCR_REG			0x10C

#define ADSS_AUDIO_RXM_CMD_RCGR_REG		0x120
#define AUDIO_RXM_CMD_RCGR_UPDATE		(1 << 0)
#define AUDIO_RXM_CMD_RCGR_ROOT_EN		(1 << 1)

#define ADSS_AUDIO_RXM_CFG_RCGR_REG		0x124
#define AUDIO_RXM_CFG_RCGR_SRC_DIV(x)		(x << 0)
#define AUDIO_RXM_CFG_RCGR_SRC_SEL(x)		(x << 8)

#define ADSS_AUDIO_RXM_MISC_REG			0x128
#define AUDIO_RXM_MISC_AUTO_SCALE_DIV(x)	(x << 4)

#define ADSS_AUDIO_RXM_CBCR_REG			0x12C

#define ADSS_AUDIO_TXB_CFG_MUXR_REG		0x144
#define AUDIO_TXB_CFG_MUXR_SRC_SEL(x)		(x << 8)

#define ADSS_AUDIO_TXB_MISC_REG			0x148
#define AUDIO_TXB_MISC_AUTO_SCALE_DIV(x)	(x << 1)

#define ADSS_AUDIO_TXB_CBCR_REG			0x14C

#define ADSS_AUDIO_TXM_CMD_RCGR_REG		0x160
#define AUDIO_TXM_CMD_RCGR_ROOT_EN		(1 << 1)
#define AUDIO_TXM_CMD_RCGR_UPDATE		(1 << 0)

#define ADSS_AUDIO_TXM_CFG_RCGR_REG		0x164
#define AUDIO_TXM_CFG_RCGR_SRC_DIV(x)		(x << 0)
#define AUDIO_TXM_CFG_RCGR_SRC_SEL(x)		(x << 8)

#define ADSS_AUDIO_TXM_MISC_REG			0x168
#define AUDIO_TXM_MISC_AUTO_SCALE_DIV(x)	(x << 4)

#define ADSS_AUDIO_TXM_CBCR_REG			0x16C

#define ADSS_AUDIO_SAMPLE_CBCR_REG		0x18C

#define ADSS_AUDIO_PCM_CMD_RCGR_REG		0x1A0

#define ADSS_AUDIO_PCM_CFG_RCGR_REG		0x1A4
#define AUDIO_PCM_CFG_RCGR_SRC_SEL(x)		(x << 8)
#define AUDIO_PCM_CFG_RGCR_SRC_DIV(x)		(x << 0)

#define ADSS_AUDIO_PCM_MISC_REG			0x1A8
#define AUDIO_PCM_MISC_AUTO_SCALE_DIV(x)	(x << 4)

#define ADSS_AUDIO_PCM_CBCR_REG			0x1AC

#define ADSS_AUDIO_XO_CBCR_REG			0x1CC

#define ADSS_AUDIO_AHB_CBCR_REG			0x200

#define ADSS_AUDIO_AHB_I2S0_CBCR_REG		0x204

#define ADSS_AUDIO_AHB_I2S3_CBCR_REG		0x208

#define ADSS_AUDIO_AHB_MBOX0_CBCR_REG		0x20C

#define DSS_AUDIO_AHB_MBOX3_CBCR_REG		0x210

#define AADSS_PCM_OFFSET_REG		0x08

#define AADSS_PCM_START_REG		0x0C

#define AADSS_PCM_INT_STATUS_REG	0x10

#define AADSS_PCM_INT_ENABLE_REG	0x14

#define AADSS_PCM_RX_DATA_8BIT_REG	0x18

#define AADSS_PCM_TX_DATA_8BIT_REG	0x1C

#define AADSS_PCM_DIVIDER_REG		0x20

#define AADSS_PCM_TH_REG		0x24

#define AADSS_PCM_FIFO_CNT_REG		0x28

#define AADSS_PCM_FIFO_ERR_SLOT_REG	0x2C

#define AADSS_PCM_RX_DATA_16BIT_REG	0x30

#define AADSS_PCM_TX_DATA_16BIT_REG	0x34



#define TDM_SYNC_NUM		2
#define TDM_DELAY		0
#define MCLK_MULTI		4

/* I2S Parameters */
#define IPQ40xx_I2S_NO_OF_PERIODS	(130)
#define IPQ40xx_I2S_PERIOD_BYTES_MIN	ALIGN(4032, L1_CACHE_BYTES)
#define IPQ40xx_I2S_BUFF_SIZE		(IPQ40xx_I2S_PERIOD_BYTES_MIN * \
						IPQ40xx_I2S_NO_OF_PERIODS)


/* ADSS APIs */
void ipq40xx_glb_audio_mode(int mode, int dir);
void ipq40xx_glb_tx_data_port_en(uint32_t enable);
void ipq40xx_glb_rx_data_port_en(uint32_t enable);
void ipq40xx_glb_audio_mode_B1K(void);
void ipq40xx_glb_tx_framesync_port_en(uint32_t enable);
void ipq40xx_glb_rx_framesync_port_en(uint32_t enable);
void ipq40xx_glb_tdm_ctrl_ch_num(uint32_t val, uint32_t dir);
void ipq40xx_glb_tdm_ctrl_sync_num(uint32_t val, uint32_t dir);
void ipq40xx_glb_tdm_ctrl_delay(uint32_t delay, uint32_t dir);
void ipq40xx_glb_clk_enable_oe(uint32_t dir);
int ipq40xx_audio_adss_probe(struct platform_device *pdev);

/* APIs in DAI driver */
uint32_t get_mbox_id(struct dai_priv_st *priv,
				struct snd_pcm_substream *substream);
uint32_t get_stereo_id(struct dai_priv_st *priv,
				struct snd_pcm_substream *substream);
uint32_t ipq40xx_get_act_bit_width(uint32_t bit_width);

void ipq40xx_stereo_config_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_mic_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_enable(uint32_t enable, uint32_t stereo_offset);
int ipq40xx_audio_stereo_probe(struct platform_device *pdev);
#endif
