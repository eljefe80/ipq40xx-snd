/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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
 */

#ifndef _IPQ40XX_MBOX_H_
#define _IPQ40XX_MBOX_H_

#include <linux/sound.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include "ipq40xx-adss.h"

#define ADSS_MBOX_INVALID_PCM			(0xFFFFFFFF)
#define ADSS_MBOX_REG_BASE			(0x7700000 + 0x6000)
#define ADSS_MBOX_RANGE				(0xFA000)
#define ADSS_MBOX_SPDIF_IRQ			(163 + 32)
#define ADSS_MBOX0_IRQ				(156 + 32)
#define ADSS_MBOX1_IRQ				(157 + 32)
#define ADSS_MBOX2_IRQ				(158 + 32)
#define ADSS_MBOX3_IRQ				(159 + 32)

#define ADSS_MBOX_STEREO_AUDIO_BASE			ADSS_BASE + 0x8000

/* ADSS_MBOX_STEREO_AUDIO_BASE + 0x0 */
#define ADSS_MBOX0_AUDIO_BASE 				0x0
#define ADSS_MBOX1_AUDIO_BASE 				0x2000
#define ADSS_MBOX2_AUDIO_BASE 				0x4000
#define ADSS_MBOX3_AUDIO_BASE 				0x6000

#define ADSS_MBOXn_MBOX_FIFO0_REG			0x0
#define MBOX_FIFO_RESET_TX_INIT				(1 << 0)
#define MBOX_FIFO_RESET_RX_INIT				(1 << 2)

#define ADSS_MBOXn_MBOX_FIFO_STATUS0_REG		0x08

#define ADSS_MBOXn_MBOX_DMA_POLICY_REG			0x10
#define MBOX_DMA_POLICY_SW_RESET			(1 << 31)
#define MBOX_DMA_POLICY_TX_INT_TYPE			(1 << 17)
#define MBOX_DMA_POLICY_RX_INT_TYPE			(1 << 16)
#define MBOX_DMA_POLICY_RXD_16BIT_SWAP			(1 << 10)
#define MBOX_DMA_POLICY_RXD_END_SWAP			(1 << 8)
#define ADSS_MBOX_DMA_POLICY_SRAM_AC(x)		(((x >> 28) & 0xf) << 12)
#define ADSS_MBOX_DMA_POLICY_TX_FIFO_THRESHOLD(x)	(((x & 0xf) << 4))

#define ADSS_MBOXn_MBOXn_DMA_RX_DESCRIPTOR_BASE_REG	0x18

#define ADSS_MBOXn_MBOXn_DMA_RX_CONTROL_REG		0x1C
#define ADSS_MBOXn_DMA_RX_CONTROL_STOP			(1 << 0)
#define ADSS_MBOXn_DMA_RX_CONTROL_START			(1 << 1)
#define ADSS_MBOXn_DMA_RX_CONTROL_RESUME		(1 << 2)

#define ADSS_MBOXn_MBOXn_DMA_TX_DESCRIPTOR_BASE_REG	0x20

#define ADSS_MBOXn_MBOXn_DMA_TX_CONTROL_REG		0x24
#define ADSS_MBOXn_DMA_TX_CONTROL_STOP			(1 << 0)
#define ADSS_MBOXn_DMA_TX_CONTROL_START			(1 << 1)
#define ADSS_MBOXn_DMA_TX_CONTROL_RESUME		(1 << 2)

#define ADSS_MBOXn_MBOX_FRAME_REG			0x38

#define ADSS_MBOXn_FIFO_TIMEOUT_REG			0x40

#define ADSS_MBOXn_MBOX_INT_STATUS_REG			0x44
#define MBOX_INT_STATUS_TX_DMA_COMPLETE			(1 << 6)
#define MBOX_INT_STATUS_RX_DMA_COMPLETE			(1 << 10)

#define ADSS_MBOXn_MBOX_INT_ENABLE_REG			0x4C
#define MBOX_INT_ENABLE_RX_DMA_COMPLETE			(1 << 10)
#define MBOX_INT_STATUS_RX_UNDERFLOW			(1 << 4)
#define MBOX_INT_STATUS_RX_FIFO_UNDERFLOW		(1 << 12)
#define MBOX_INT_ENABLE_TX_DMA_COMPLETE			(1 << 6)
#define MBOX_INT_STATUS_TX_OVERFLOW			(1 << 5)
#define MBOX_INT_STATUS_TX_FIFO_OVERFLOW		(1 << 13)

#define ADSS_MBOXn_MBOX_FIFO_RESET_REG			0x58
#define MBOX_FIFO_RESET_TX_INIT				(1 << 0)
#define MBOX_FIFO_RESET_RX_INIT				(1 << 2)

#define ADSS_MBOXn_MBOX_DEBUG_CHAIN0_REG		0x60

#define ADSS_MBOXn_MBOX_DEBUG_CHAIN1_REG		0x64

#define ADSS_MBOXn_MBOX_DEBUG_CHAIN0_SIGNALS_REG	0x68

#define ADSS_MBOXn_MBOX_DEBUG_CHAIN1_SIGNALS_REG	0x6C

/* When the mailbox operation is started, the mailbox would get one descriptor
 * for the current data transfer and prefetch one more descriptor. When less
 * than 3 descriptors are configured, then it is possible that before the CPU
 * handles the interrupt, the mailbox could check the pre fetched descriptor
 * and stop the DMA transfer.
 * To handle this, the design is use multiple descriptors, but they would
 * point to the same buffer address. This way  more number of descriptors
 * would satisfy the mbox requirement, and reusing the buffer address would
 * satisfy the upper layer's buffer requirement
 *
 * The value of 5 of repetition times was derived from trial and error testing
 * for minimum number of repetitions that would result in MBOX operations
 * without stopping.
 */
#define MBOX_MIN_DESC_NUM       3
#define MBOX_DESC_REPEAT_NUM    5

enum {
	ADSS_MBOX_NR_CHANNELS = 5,
};

//extern
static struct ipq40xx_mbox_rt_priv *mbox_rtime[ADSS_MBOX_NR_CHANNELS];

struct ipq40xx_mbox_desc {

	unsigned int	length 	: 12,	/* bit 11-00 */
			size	: 12,	/* bit 23-12 */
			vuc	: 1,	/* bit 24 */
			ei	: 1,	/* bit 25 */
			rsvd1	: 4,	/* bit 29-26 */
			EOM	: 1,	/* bit 30 */
			OWN	: 1, 	/* bit 31 */
			BufPtr	: 28,   /* bit 27-00 */
			rsvd2	:  4,   /* bit 31-28 */
			NextPtr	: 28,   /* bit 27-00 */
			rsvd3	:  4;   /* bit 31-28 */

	unsigned int vuc_dword[36];

}; // __packed;

struct ipq40xx_mbox_rt_dir_priv {
	/* Desc array in virtual space */
	struct ipq40xx_mbox_desc *dma_virt_head;

	/* Desc array for DMA */
	dma_addr_t dma_phys_head;
	struct device *dev;
	unsigned int ndescs;
	irq_handler_t callback;
	void *dai_priv;
	unsigned long status;
	uint32_t channel_id;
	uint32_t err_stats;
	uint32_t last_played_is_null;
	u32 write;
	u32 read;
};

struct ipq40xx_mbox_rt_priv {
	int irq_no;
	u32 io_resource;
	volatile void __iomem *mbox_reg_base;
	struct ipq40xx_mbox_rt_dir_priv dir_priv[2];
	int mbox_started;
};

/* Replaces struct ath_i2s_softc */
struct ipq40xx_pcm_pltfm_priv {
	struct snd_pcm_substream *playback;
	struct snd_pcm_substream *capture;
};

/* platform data */
//extern struct snd_soc_platform_driver ipq40xx_soc_platform;

int ipq40xx_mbox_fifo_reset(int channel_id);
int ipq40xx_mbox_dma_start(int channel_id);
int ipq40xx_mbox_dma_stop(int channel_id);
int ipq40xx_mbox_dma_prepare(int channel_id);
int ipq40xx_mbox_dma_resume(int channel_id);
int ipq40xx_mbox_form_ring(int channel_id, dma_addr_t baseaddr, u8 *base,
				int period_bytes, int bufsize, int own_bit);
int ipq40xx_mbox_dma_release(int channel);
int ipq40xx_mbox_dma_init(struct device *dev, int channel_id,
	irq_handler_t callback, void *private_data);
void ipq40xx_mbox_desc_own(u32 channel_id, int desc_no, int own);

uint32_t ipq40xx_mbox_get_played_offset(u32 channel_id);
uint32_t ipq40xx_mbox_get_played_offset_set_own(u32 channel_id);
static int ipq40xx_mbox_probe(struct platform_device *pdev);
static inline uint32_t ipq40xx_convert_id_to_channel(uint32_t id)
{
//	printk("Requested channel %d from id %d",((id)/2), id);
	return ((id)/2);
}

static inline uint32_t ipq40xx_convert_id_to_dir(uint32_t id)
{
//	printk("Requested direction %d from id %d",((id)/2), id);
	return ((id)%2);
}

static inline int ipq40xx_mbox_interrupt_enable(int channel_id,
						unsigned int mask)
{
	volatile void __iomem *mbox_reg;
	unsigned int val;
	uint32_t index;
	index = ipq40xx_convert_id_to_channel(channel_id);

	if (!mbox_rtime[index])
		return -ENOMEM;

	mbox_reg = mbox_rtime[index]->mbox_reg_base;

	val = readl(mbox_reg + ADSS_MBOXn_MBOX_INT_ENABLE_REG);
	val |= mask;
	writel(val, mbox_reg + ADSS_MBOXn_MBOX_INT_ENABLE_REG);
	val = readl(mbox_reg + ADSS_MBOXn_MBOX_INT_ENABLE_REG);
	printk("Enabling INT base:%04x, CHANNEL: %d, val %x", mbox_reg, channel_id, val);

	return 0;
}

static inline int ipq40xx_mbox_interrupt_disable(int channel_id,
							unsigned int mask)
{
	volatile void __iomem *mbox_reg;
	unsigned int val;
	uint32_t index;

	index = ipq40xx_convert_id_to_channel(channel_id);

	if (!mbox_rtime[index])
		return -ENOMEM;

	mbox_reg = mbox_rtime[index]->mbox_reg_base;

	val = readl(mbox_reg + ADSS_MBOXn_MBOX_INT_ENABLE_REG);
	val &= ~mask;
	writel(val, mbox_reg + ADSS_MBOXn_MBOX_INT_ENABLE_REG);

	return 0;
}

static inline int ipq40xx_mbox_interrupt_ack(int channel_id, unsigned int mask)
{
	volatile void __iomem *mbox_reg;
	unsigned int val;
	uint32_t index;

	index = ipq40xx_convert_id_to_channel(channel_id);

	if (!mbox_rtime[index])
		return -ENOMEM;

	mbox_reg = mbox_rtime[index]->mbox_reg_base;

	val = readl(mbox_reg + ADSS_MBOXn_MBOX_INT_STATUS_REG);
	val &= ~mask;
	writel(val, mbox_reg + ADSS_MBOXn_MBOX_INT_STATUS_REG);

	return 0;
}

uint32_t ipq40xx_mbox_get_elapsed_size(uint32_t channel_id);

/* If number of mbox descriptors are less than MBOX_MIN_DESC_NUM
 * there should be duplicate mbox descriptors in order to compliant
 * with the mbox operation logic described at the definitions of
 * macros MBOX_MIN_DESC_NUM and MBOX_DESC_REPEAT_NUM in this file */
static inline int ipq40xx_get_mbox_descs_duplicate(int ndescs)
{
	if (ndescs < MBOX_MIN_DESC_NUM)
		ndescs *= MBOX_DESC_REPEAT_NUM;

	return ndescs;
}

#endif /* _IPQ40XX_MBOX_H_ */