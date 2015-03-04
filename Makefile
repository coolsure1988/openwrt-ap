SRC := $(shell ls *.c)
OBJ := $(patsubst %.c, %.o, $(SRC))

CC  := /home/sure/lzy/openwrt/bin/ar71xx/OpenWrt-SDK-ar71xx-for-linux-x86_64-gcc-4.8-linaro_uClibc-0.9.33.2/staging_dir/toolchain-mips_74kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc
LD  := $(CC)

PCAPL := /home/sure/lzy/dpi/platform/lib/libpcap.so.1.0 libdpi.so

openwrt-ap : $(OBJ)
	$(LD) -o $@ $^ ${PCAPL} -lpthread

%.o : %.c
	$(CC) -c -o $@ $<

clean:
	rm -f openwrt-ap *.o *~
