target =gnss_demo
object =gnss_demo.o
CC =i586-poky-linux-gcc -m32 -march=corei7 -mtune=corei7 -mfpmath=sse -msse4.2

CFLAGS = --sysroot=/home/ucpu/ucpu_workspace/sdk/sysroots/corei7-32-poky-linux/
#CFLAGS = --sysroot=TOBY-L4_uCPU/build-ucpu/build/tmp/work/xgold727_ucpu-poky-linux/ucpu-image/1.0-r5.1/rootfs/
CFLAGS += -Wall
CFLAGS += -lgps -lpthread
CFLAGS += -L/home/ucpu/ucpu_workspace/TOBY-L4_uCPU/build-ucpu/build/tmp/work/corei7-32-poky-linux/gpsd/3.17-r0/gpsd-3.17
CFLAGS += -I/home/ucpu/ucpu_workspace/TOBY-L4_uCPU/build-ucpu/build/tmp/work/corei7-32-poky-linux/gpsd/3.17-r0/gpsd-3.17

LDFLAGS = -O2

%.o : %c
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS)

all : $(target)
$(target): $(object)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
clean:
	rm -f $(target) $(object)
