# ==============================================================
# Práctica Make: led_blinky e hello_world para a FRDM-KL46Z
# MCU: MKL46Z256 (Cortex-M0+ @ 48 MHz, 256 KB flash)
# Toolchain GNU: arm-none-eabi-gcc + OpenOCD + GDB
#
# Uso:
#   make                  -> compila led_blinky e hello_world
#   make led_blinky       -> só led_blinky
#   make hello_world      -> só hello_world
#   make flash_led        -> compila e grava led_blinky na flash
#   make flash_hello      -> compila e grava hello_world na flash
#   make BUILD=release    -> versión optimizada (-Os) en vez de debug
#   make clean
# ==============================================================

# ---------- Rutas ----------
# Este Makefile vai no directorio que contén a carpeta do SDK
SDK       = SDK
BOARD_DIR = $(SDK)/boards/frdmkl46z
APPS_DIR  = $(BOARD_DIR)/demo_apps
DEV       = $(SDK)/devices/MKL46Z4

HELLO_DIR = $(APPS_DIR)/hello_world
LED_DIR   = $(APPS_DIR)/led_blinky

# ---------- Tipo de build: debug (por defecto) ou release ----------
BUILD = debug
OUT   = build/$(BUILD)

# ---------- Toolchain ----------
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE    = $(PREFIX)size
GDB = $(PREFIX)gdb
OPENOCD = openocd

# ---------- Flags ----------
ARCH = -mcpu=cortex-m0plus -mthumb

cflags.debug   = -g -O0 -DDEBUG
cflags.release = -Os -DNDEBUG

INCLUDES = -I$(SDK)/CMSIS/Include \
           -I$(SDK)/devices \
           -I$(DEV) \
           -I$(DEV)/drivers \
           -I$(DEV)/utilities \
           -I$(DEV)/utilities/io \
           -I$(DEV)/utilities/log \
           -I$(DEV)/utilities/str \
           -I$(BOARD_DIR)

CPPFLAGS = -DCPU_MKL46Z256VLL4 -DFRDM_KL46Z -DFREEDOM $(INCLUDES)
CFLAGS   = $(ARCH) -std=gnu99 -Wall -fno-common -ffunction-sections \
           -fdata-sections -ffreestanding -fno-builtin $(cflags.$(BUILD))
ASFLAGS  = $(ARCH) -g -D__STARTUP_CLEAR_BSS

LDFLAGS  = $(ARCH) --specs=nano.specs --specs=nosys.specs \
           -Wl,--gc-sections -static -Wl,-z,muldefs
LDLIBS   = -Wl,--start-group -lm -lc -lgcc -lnosys -Wl,--end-group

# ---------- OpenOCD: OpenSDA (firmware CMSIS-DAP) por SWD ----------
OCD_FLAGS = -f interface/cmsis-dap.cfg -c "transport select swd" -f target/klx.cfg

# ---------- Fontes ----------
# Cada demo ten os seus propios board.c, clock_config.c e pin_mux.c
APP_SRCS = board clock_config pin_mux

# Fontes do SDK comúns aos dous exemplos (sacadas dos CMakeLists.txt)
SDK_C = system_MKL46Z4 \
        fsl_clock fsl_common fsl_gpio fsl_lpsci fsl_uart fsl_smc \
        fsl_ftfx_controller fsl_ftfx_flash fsl_ftfx_cache fsl_ftfx_flexnvm \
        fsl_io fsl_log fsl_str fsl_debug_console fsl_assert
SDK_S = startup_MKL46Z4

# Make busca nestes directorios as fontes do SDK que non están na demo
vpath %.c $(DEV) $(DEV)/drivers $(DEV)/utilities \
          $(DEV)/utilities/io $(DEV)/utilities/log $(DEV)/utilities/str
vpath %.S $(DEV)/gcc

# ---------- Obxectos e binarios de cada aplicación ----------
HELLO_OBJS = $(addprefix $(OUT)/hello_world/, \
             $(addsuffix .o, hello_world $(APP_SRCS) $(SDK_C) $(SDK_S)))
LED_OBJS   = $(addprefix $(OUT)/led_blinky/, \
             $(addsuffix .o, led_blinky $(APP_SRCS) $(SDK_C) $(SDK_S)))

HELLO_ELF = $(OUT)/hello_world.elf
LED_ELF   = $(OUT)/led_blinky.elf

HELLO_LD  = $(HELLO_DIR)/armgcc/MKL46Z256xxx4_flash.ld
LED_LD    = $(LED_DIR)/armgcc/MKL46Z256xxx4_flash.ld

# Target-specific variables: só afectan a ese binario e aos seus .o
$(HELLO_ELF): CPPFLAGS += -I$(HELLO_DIR) \
              -DPRINTF_FLOAT_ENABLE=0 -DSCANF_FLOAT_ENABLE=0 \
              -DPRINTF_ADVANCED_ENABLE=0 -DSCANF_ADVANCED_ENABLE=0
$(LED_ELF):   CPPFLAGS += -I$(LED_DIR)

# ==============================================================
# Regras principais
# ==============================================================
.PHONY: all led_blinky hello_world flash_led flash_hello clean cleanall

all: led_blinky hello_world

led_blinky:  $(LED_ELF)   $(LED_ELF:.elf=.bin)
hello_world: $(HELLO_ELF) $(HELLO_ELF:.elf=.bin)

$(HELLO_ELF): $(HELLO_OBJS) $(HELLO_LD)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -T$(HELLO_LD) -Wl,-Map=$(@:.elf=.map) $(HELLO_OBJS) $(LDLIBS) -o $@
	$(SIZE) $@

$(LED_ELF): $(LED_OBJS) $(LED_LD)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -T$(LED_LD) -Wl,-Map=$(@:.elf=.map) $(LED_OBJS) $(LDLIBS) -o $@
	$(SIZE) $@

# .elf -> .bin
$(OUT)/%.bin: $(OUT)/%.elf
	$(OBJCOPY) -O binary $< $@

# ==============================================================
# Regras implícitas propias (compilación)
# Primeiro tenta a fonte da propia demo; se non existe, a do SDK (vpath)
# ==============================================================
$(OUT)/hello_world/%.o: $(HELLO_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT)/hello_world/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT)/hello_world/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

$(OUT)/led_blinky/%.o: $(LED_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT)/led_blinky/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT)/led_blinky/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

# ==============================================================
# Gravación na flash vía OpenOCD
# ==============================================================
flash_led: $(LED_ELF)
	$(OPENOCD) $(OCD_FLAGS) -c "program $< verify reset exit"

flash_hello: $(HELLO_ELF)
	$(OPENOCD) $(OCD_FLAGS) -c "program $< verify reset exit"

# ==============================================================
# Limpeza
# ==============================================================
clean:
	$(RM) -r $(OUT)

cleanall:
	$(RM) -r build

