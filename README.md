This is a driver for the Atheros IPQ40xx onboard snd card

To add the driver to your openwrt build,
copy
patches/851-add-ipq4019-ad-clock-controller.patch to 
<openwrt-buildroot>/target/linux/ipq40xx/patches-5.15/
and 
package/Makefile to
<openwrt-buildroot>/package/kernel/ipq40xx-snd

then enable `ipq40xx sound driver` under Kernel->Sound Support

