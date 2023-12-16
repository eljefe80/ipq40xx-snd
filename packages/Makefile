include $(TOPDIR)/rules.mk

PKG_NAME:=ipq40xx-snd
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2

PKG_SOURCE_URL:=https://github.com/eljefe80/ipq40xx-snd.git
PKG_SOURCE_PROTO:=git
#PKG_SOURCE_DATE:=2023-05-26
PKG_SOURCE_VERSION:=general-cleanup
#PKG_MIRROR_HASH:=90e145fbe6057da65b637e3a194317c1e4f529b078c7e4a245a286c37e3d1b76

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/snd-simple-card/description
 Kernel modules for the soc simple-audio-card.
endef

define KernelPackage/snd-simple-card
  SUBMENU:=Sound Support
  TITLE:=SOC Simple Audio Card Support
  DEPENDS:=+kmod-sound-soc-core
  KCONFIG:=CONFIG_SND_SIMPLE_CARD
  FILES:=\
	$(LINUX_DIR)/sound/soc/generic/snd-soc-simple-card.ko \
	$(LINUX_DIR)/sound/soc/generic/snd-soc-simple-card-utils.ko
  AUTOLOAD:=$(call AutoLoad,35,snd-soc-simple-card)
  PROVIDES:=kmod-snd-simple-card
endef

define KernelPackage/ipq40xx-snd
  SUBMENU:=Sound Support
  TITLE:=ipq40xx sound driver
  DEPENDS:=+kmod-snd-simple-card
  FILES:=\
	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-codec-tas5782m.ko \
	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-cpu-dai.ko \
	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-pcm-i2s.ko
  AUTOLOAD:=$(call AutoProbe,snd-soc-ipq40xx-pcm-i2s snd-soc-ipq40xx-cpu-dai snd-soc-ipq40xx-codec-tas5782m)
  PROVIDES:=kmod-ipq40xx-snd
endef
#	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-adss.ko
#	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-stereo.ko \
#	$(PKG_BUILD_DIR)/snd-soc-ipq40xx.ko
#	$(PKG_BUILD_DIR)/snd-soc-ipq40xx-mbox.ko \

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \

define Build/Configure
endef

define Build/Compile/ipq40xx-snd
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Build/Compile/snd-simple-card
endef

define Build/Compile
	$(call Build/Compile/ipq40xx-snd)
	$(call Build/Compile/snd-simple-card)
endef

define KernelPackage/ipq40xx-snd/config
endef

$(eval $(call KernelPackage,ipq40xx-snd))
$(eval $(call KernelPackage,snd-simple-card,+kmod-sound-soc-core))
