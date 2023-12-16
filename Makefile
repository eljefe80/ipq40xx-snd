#QCA SND CARD
CONFIG_SND_IPQ40XX_SOC_I2S=m
CONFIG_SND_IPQ40XX_SOC_CODEC_TAS5782M=m
CONFIG_SND_IPQ40XX_SOC_CPU=m

snd-soc-ipq40xx-i2s-objs := ipq40xx-pcm-i2s.o ipq40xx-mbox.o ipq40xx-stereo.o
snd-soc-ipq40xx-cpu-objs := ipq40xx-cpu-dai.o ipq40xx-adss.o
tas5782m-objs := tas5782m.o

obj-$(CONFIG_SND_IPQ40XX_SOC_PCM_I2S) += snd-soc-ipq40xx-i2s.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CODEC_TAS5782M) += tas5782m.o
obj-$(CONFIG_SND_IPQ40XX_SOC_CPU_DAI) += snd-soc-ipq40xx-cpu.o


modules:
	@echo Got here
	$(MAKE)ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd) modules
