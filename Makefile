#TOOLCHAIN=~/toolchain/gcc-arm-none-eabi-4_9-2014q4/bin
#PREFIX=$(TOOLCHAIN)/arm-none-eabi-
PREFIX=arm-none-eabi-

ARCHFLAGS=-mthumb -mcpu=cortex-m0plus
COMMONFLAGS=-g3 -O0 -Wall -Werror $(ARCHFLAGS)

CFLAGS=-I./includes $(COMMONFLAGS)
LDFLAGS=$(COMMONFLAGS) -T link.ld -Wl,--gc-sections,-Map,$(TARGET).map
LDLIBS=-lgcc

CC=$(PREFIX)gcc
LD=$(PREFIX)gcc
RM=rm -f

TARGET=main

SRC=$(wildcard *.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

all: elf
elf: $(TARGET).elf

clean:
	$(RM) $(TARGET).srec $(TARGET).elf $(TARGET).bin $(TARGET).map $(OBJ)

$(TARGET).elf: $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

flash: all
	openocd -f openocd.cfg -c "program $(TARGET).elf verify reset exit"
