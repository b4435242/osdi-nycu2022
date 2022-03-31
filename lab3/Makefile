ARMGNU ?= aarch64-linux-gnu

FLAGS = -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector

BUILD_DIR = build
SRC_DIR = src
KERNEL_DIR = src/kernel
KERNEL_OBJ_DIR = build/kernel
BOOTLOADER_DIR = src/bootloader
BOOTLOADER_OBJ_DIR = build/bootloader
LIB_DIR = src/lib
LIB_OBJ_DIR = build/lib
USER_PROGRAM_DIR = src/user_program
USER_PROGRAM_OBJ_DIR = build/user_program

all: kernel8.img bootloader.img user_program.img

clean:
	rm -rf $(BUILD_DIR)

$(KERNEL_OBJ_DIR)/%.o:	$(KERNEL_DIR)/%.S 
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

$(KERNEL_OBJ_DIR)/%.o:	$(KERNEL_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

$(BOOTLOADER_OBJ_DIR)/%.o:	$(BOOTLOADER_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

$(BOOTLOADER_OBJ_DIR)/%.o:	$(BOOTLOADER_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

$(LIB_OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

$(USER_PROGRAM_OBJ_DIR)/%.o: $(USER_PROGRAM_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc -c $< -o $@ $(FLAGS)

LIB_FILES = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJS = $(LIB_FILES:$(LIB_DIR)/%.c=$(LIB_OBJ_DIR)/%.o)

KERNEL_ASM_FILES = $(wildcard $(KERNEL_DIR)/*.S)
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_OBJS = $(KERNEL_C_FILES:$(KERNEL_DIR)/%.c=$(KERNEL_OBJ_DIR)/%.o)
KERNEL_OBJS += $(KERNEL_ASM_FILES:$(KERNEL_DIR)/%.S=$(KERNEL_OBJ_DIR)/%.o)
KERNEL_OBJS += $(LIB_OBJS)
#$(info $$kernel is [${KERNEL_OBJS}])

BOOTLOADER_ASM_FILES = $(wildcard $(BOOTLOADER_DIR)/*.S)
BOOTLOADER_C_FILES = $(wildcard $(BOOTLOADER_DIR)/*.c)
BOOTLOADER_OBJS = $(BOOTLOADER_C_FILES:$(BOOTLOADER_DIR)/%.c=$(BOOTLOADER_OBJ_DIR)/%.o)
BOOTLOADER_OBJS += $(BOOTLOADER_ASM_FILES:$(BOOTLOADER_DIR)/%.S=$(BOOTLOADER_OBJ_DIR)/%.o)
BOOTLOADER_OBJS += $(LIB_OBJS)
#$(info $$bootloader is [${BOOTLOADER_OBJS}])

USER_PROGRAM_FILES = $(wildcard $(USER_PROGRAM_DIR)/*.S)
USER_PROGRAM_OBJS = $(USER_PROGRAM_FILES:$(USER_PROGRAM_DIR)/%.S=$(USER_PROGRAM_OBJ_DIR)/%.o)


kernel8.img: $(KERNEL_DIR)/linker.ld $(KERNEL_OBJS)
	$(ARMGNU)-ld -T $(KERNEL_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(KERNEL_OBJS)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img

bootloader.img: $(BOOTLOADER_DIR)/linker.ld $(BOOTLOADER_OBJS)
	$(ARMGNU)-ld -T $(BOOTLOADER_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf $(BOOTLOADER_OBJS)
	$(ARMGNU)-objcopy $(BUILD_DIR)/bootloader.elf -O binary $(BUILD_DIR)/bootloader.img

user_program.img: $(USER_PROGRAM_DIR)/linker.ld $(USER_PROGRAM_OBJS)
	$(ARMGNU)-ld -T $(USER_PROGRAM_DIR)/linker.ld -o $(BUILD_DIR)/user_program.elf $(USER_PROGRAM_OBJS)
	$(ARMGNU)-objcopy $(BUILD_DIR)/user_program.elf -O binary $(BUILD_DIR)/user_program.img
	cp $(BUILD_DIR)/user_program.img rootfs/user_program.img

bootloader:
	qemu-system-aarch64 -M raspi3 -kernel build/bootloader.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

run:
	qemu-system-aarch64 -M raspi3 -kernel build/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

debug:
	qemu-system-aarch64 -M raspi3 -kernel build/kernel8.img -display none \
	-serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb \
	-s -S


