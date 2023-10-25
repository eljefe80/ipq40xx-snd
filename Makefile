#QCA SND CARD
CONFIG_SND_IPQ40XX_SOC_PCM_I2S=m
CONFIG_SND_IPQ40XX_SOC_CODEC_TAS5782M=m
CONFIG_SND_IPQ40XX_SOC_CPU_DAI=m
CONFIG_SND_IPQ40XX_SOC_MBOX=m
CONFIG_SND_IPQ40XX_SOC_ADSS=m
CONFIG_SND_IPQ40XX_SOC_STEREO=m

snd-soc-ipq40xx-pcm-i2s-objs := ipq40xx-pcm-i2s.o ipq40xx-mbox.o ipq40xx-stereo.o
snd-soc-ipq40xx-codec-tas5782m-objs := tas5782m.o
snd-soc-ipq40xx-cpu-dai-objs := ipq40xx-cpu-dai.o ipq40xx-adss.o 
#snd-soc-ipq40xx-mbox-objs := ipq40xx-mbox.o
#snd-soc-ipq40xx-adss-objs := ipq40xx-adss.o
#snd-soc-ipq40xx-stereo-objs := ipq40xx-stereo.o

obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_I2S) += snd-soc-ipq40xx-pcm-i2s.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CODEC_TAS5782M) += snd-soc-ipq40xx-codec-tas5782m.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CPU_DAI) += snd-soc-ipq40xx-cpu-dai.o
#obj-$(CONFIG_SND_IPQ40XX_SOC_MBOX) += snd-soc-ipq40xx-mbox.o
#obj-$(CONFIG_SND_IPQ40XX_SOC_ADSS) += snd-soc-ipq40xx-adss.o
#obj-$(CONFIG_SND_IPQ40XX_SOC_STEREO) += snd-soc-ipq40xx-stereo.o


modules:
	@echo Got here
	$(MAKE)ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd) modules
