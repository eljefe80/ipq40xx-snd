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

#define ADSS_AUDIO_SPDIF_MISC_REG		0x150
#define AUDIO_SPDIF_MISC_AUTO_SCALE_DIV_MASK	(0xF << 1)
#define AUDIO_SPDIF_MISC_AUTO_SCALE_DIV(x)	(x << 1)

#define ADSS_AUDIO_SPDIF_CBCR_REG		0x154

#define ADSS_AUDIO_SPDIFDIV2_MISC_REG		0x158
#define AUDIO_SPDIFDIV2_MISC_AUTO_SCALE_DIV_MASK	(0xF << 1)
#define AUDIO_SPDIFDIV2_MISC_AUTO_SCALE_DIV(x)	(x << 1)

#define ADSS_AUDIO_SPDIFDIV2_CBCR_REG		0x15C

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

#define ADSS_AUDIO_SPDIFINFAST_CMD_RCGR_REG	0x1E0
#define AUDIO_SPDIFINFAST_CMD_RCGR_ROOT_EN	(1 << 1)
#define AUDIO_SPDIFINFAST_CMD_RCGR_UPDATE	(1 << 0)

#define ADSS_AUDIO_SPDIFINFAST_CFG_RCGR_REG	0x1E4
#define AUDIO_SPDIFINFAST_CFG_RCGR_SRC_SEL(x)	(x << 8)
#define AUDIO_SPDIFINFAST_CFG_RCGR_SRC_DIV(x)	(x << 0)

#define ADSS_AUDIO_SPDIFINFAST_CBCR_REG		0x1EC

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


/* ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG Registers */

#define ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE	ADSS_BASE + 0x6000

#define AADSS_MBOXSPDIFIN_MBOX_FIFO0_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x0

#define AADSS_MBOXSPDIFIN_MBOX_FIFO_STATUS0_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x08

#define AADSS_MBOXSPDIFIN_MBOX_DMA_POLICY_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x10

#define AADSS_MBOXSPDIFIN_MBOX0_DMA_RX_DESCRIPTOR_BASE_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x18

#define AADSS_MBOXSPDIFIN_MBOX0_DMA_RX_CONTROL_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x1C

#define AADSS_MBOXSPDIFIN_MBOX0_DMA_TX_DESCRIPTOR_BASE_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x20

#define AADSS_MBOXSPDIFIN_MBOX0_DMA_TX_CONTROL_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x24

#define AADSS_MBOXSPDIFIN_MBOX_FRAME_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x38

#define AADSS_MBOXSPDIFIN_FIFO_TIMEOUT_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x40

#define AADSS_MBOXSPDIFIN_MBOX_INT_STATUS_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x44

#define AADSS_MBOXSPDIFIN_MBOX_INT_ENABLE_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x4C

#define AADSS_MBOXSPDIFIN_MBOX_FIFO_RESET_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x58

#define AADSS_MBOXSPDIFIN_MBOX_DEBUG_CHAIN0_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x60

#define AADSS_MBOXSPDIFIN_MBOX_DEBUG_CHAIN1_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x64

#define AADSS_MBOXSPDIFIN_MBOX_DEBUG_CHAIN0_SIGNALS_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x68

#define AADSS_MBOXSPDIFIN_MBOX_DEBUG_CHAIN1_SIGNALS_REG \\
	ADSS_MBOXSPDIFIN_AUDIO_MBOX_REG_BASE + 0x6C

/* ADSS_SPDIFIN_AUDIO_SPDIF_BASE Registers */

#define ADSS_SPDIFIN_SPDIF_CTRL_REG			(0x00)
#define SPDIF_CTRL_INTREQ_MASK				(1 << 31)
#define SPDIF_CTRL_BEGIN_MASK				(1 << 30)
#define SPDIF_CTRL_LOCK_MASK				(1 << 29)
#define SPDIF_CTRL_SYNCERR_MASK				(1 << 28)
#define SPDIF_CTRL_AFULL_MASK				(1 << 27)
#define SPDIF_CTRL_FULL_MASK				(1 << 26)
#define SPDIF_CTRL_AEMPTY_MASK				(1 << 25)
#define SPDIF_CTRL_EMPTY_MASK				(1 << 24)
#define SPDIF_CTRL_OVRERR_MASK				(1 << 23)
#define SPDIF_CTRL_UNDERR_MASK				(1 << 22)
#define SPDIF_CTRL_PARITY_MASK				(1 << 21)
#define SPDIF_CTRL_USE_FIFO_IF				(1 << 19)
#define SPDIF_CTRL_SETPREAMBB				(1 << 18)
#define SPDIF_CTRL_DUPLICATE				(1 << 17)
#define SPDIF_CTRL_CHANNEL_MODE				(1 << 16)
#define SPDIF_CTRL_VALIDITYCHECK			(1 << 15)
#define SPDIF_CTRL_PARITYGEN				(1 << 14)
#define SPDIF_CTRL_PARITYCHECK				(1 << 13)
#define SPDIF_CTRL_TR_MODE				(1 << 12)
#define SPDIF_CTRL_CLK_ENABLE				(1 << 11)
#define SPDIF_CTRL_FIFO_ENABLE				(1 << 10)
#define SPDIF_CTRL_SPDIF_ENABLE				(1 << 9)
#define SPDIF_CTRL_SFR_ENABLE				(1 << 8)
#define SPDIF_CTRL_TSAMPLERATE				(1 << 7)

#define ADSS_SPDIFIN_STEREO0_VOLUME			(0x04)

#define ADSS_SPDIFIN_FIFO_CTRL_REG			(0x08)

#define ADSS_SPDIFIN_START_REG_REG			(0x0C)

#define ADSS_SPDIFIN_SELFIFO_REG			(0x10)


#define TDM_SYNC_NUM		2
#define TDM_DELAY		0
#define MCLK_MULTI		4

/* I2S Parameters */
#define IPQ40xx_I2S_NO_OF_PERIODS	(130)
#define IPQ40xx_I2S_PERIOD_BYTES_MIN	ALIGN(4032, L1_CACHE_BYTES)
#define IPQ40xx_I2S_BUFF_SIZE		(IPQ40xx_I2S_PERIOD_BYTES_MIN * \
						IPQ40xx_I2S_NO_OF_PERIODS)

/* SPDIF clocks */
#define AUDIO_SPDIFINFAST       49152000

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
void ipq40xx_spdifin_cfg(void);
void ipq40xx_glb_clk_enable_oe(uint32_t dir);
int ipq40xx_audio_adss_probe(struct platform_device *pdev);

/* APIs in DAI driver */
uint32_t get_mbox_id(struct snd_pcm_substream *substream, int intf);
uint32_t get_stereo_id(struct snd_pcm_substream *substream, int intf);
uint32_t ipq40xx_get_act_bit_width(uint32_t bit_width);

void ipq40xx_stereo_config_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_mic_reset(uint32_t reset, uint32_t stereo_offset);
void ipq40xx_stereo_config_enable(uint32_t enable, uint32_t stereo_offset);
int ipq40xx_audio_stereo_probe(struct platform_device *pdev);
#endif
