#QCA SND CARD
snd-soc-ipq40xx-objs := ipq40xx.o
snd-soc-ipq40xx-pcm-i2s-objs := ipq40xx-pcm-i2s.o
snd-soc-ipq40xx-pcm-tdm-objs := ipq40xx-pcm-tdm.o
snd-soc-ipq40xx-pcm-spdif-objs := ipq40xx-pcm-spdif.o
snd-soc-ipq40xx-codec-objs := ipq40xx-codec.o
snd-soc-ipq40xx-codec-alc1312-objs := ipq40xx-codec-alc1312.o
snd-soc-ipq40xx-dac-rt5616-objs := rt5616.o
snd-soc-ipq40xx-cpu-dai-objs := ipq40xx-cpu-dai.o
snd-soc-ipq40xx-mbox-objs := ipq40xx-mbox.o
snd-soc-ipq40xx-pcm-raw-objs := ipq40xx-pcm-raw.o
 #ipq40xx-pcm-raw-lb-test.o

obj-$(CONFIG_SND_QCA_SOC_IPQ40XX) += snd-soc-ipq40xx.o
obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_I2S) += snd-soc-ipq40xx-pcm-i2s.o
obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_TDM) += snd-soc-ipq40xx-pcm-tdm.o
obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_SPDIF) += snd-soc-ipq40xx-pcm-spdif.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CODEC) += snd-soc-ipq40xx-codec.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CODEC_ALC1312) += snd-soc-ipq40xx-codec-alc1312.o
obj-$(CONFIG_SND_IPQ40XX_SOC_DAC_RT5616) += snd-soc-ipq40xx-dac-rt5616.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CPU_DAI) += snd-soc-ipq40xx-cpu-dai.o
obj-$(CONFIG_SND_IPQ40XX_SOC_MBOX) += ipq40xx-mbox.o
obj-$(CONFIG_SND_IPQ40XX_SOC_ADSS) += ipq40xx-adss.o
obj-$(CONFIG_SND_IPQ40XX_SOC_STEREO) += ipq40xx-stereo.o
obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_RAW) += snd-soc-ipq40xx-pcm-raw.o

obj-m += ipq40xx.o
obj-m += ipq40xx-pcm-i2s.o
obj-m += ipq40xx-pcm-tdm.o
obj-m += ipq40xx-pcm-spdif.o
obj-m += ipq40xx-codec.o
obj-m += ipq40xx-codec-alc1312.o
obj-m += rt5616.o
obj-m += ipq40xx-cpu-dai.o
obj-m += ipq40xx-mbox.o
obj-m += ipq40xx-adss.o
obj-m += ipq40xx-stereo.o
obj-m += ipq40xx-pcm-raw.o


modules:
	@echo Got here
	$(MAKE)ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules
