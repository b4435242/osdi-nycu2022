ARMGNU ?= aarch64-linux-gnu

FLAGS = -g -ffreestanding -nostdinc -nostdlib -nostartfiles

BUILD_DIR = build
SRC_DIR = src
KERNEL_DIR = src/kernel
BOOTLOADER_DIR = src/bootloader

all: kernel8.img bootloader.img

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%_c.o:	$(SRC_DIR)/%.c 
	mkdir -p $(@D) 
	$(ARMGNU)-gcc $(FLAGS) -c $< -o $@ 

$(BUILD_DIR)/%_s.o:	$(KERNEL_DIR)/%.S 
	mkdir -p $(@D) 
	$(ARMGNU)-gcc -c $< -o $@

$(BUILD_DIR)/%_s.o:	$(BOOTLOADER_DIR)/%.S 
	mkdir -p $(@D) 
	$(ARMGNU)-gcc -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
KERNEL_ASM_FILES = $(wildcard $(KERNEL_DIR)/*.S)
BOOTLOADER_ASM_FILES = $(wildcard $(BOOTLOADER_DIR)/*.S)

OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
KERNEL_OBJ_FILES = $(OBJ_FILES)
BOOTLOADER_OBJ_FILES = $(BUILD_DIR)/uart_c.o $(BUILD_DIR)/bootloader_c.o $(BUILD_DIR)/lib_c.o 
KERNEL_OBJ_FILES += $(KERNEL_ASM_FILES:$(KERNEL_DIR)/%.S=$(BUILD_DIR)/%_s.o)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_ASM_FILES:$(BOOTLOADER_DIR)/%.S=$(BUILD_DIR)/%_s.o)

kernel8.img: $(KERNEL_DIR)/linker.ld $(KERNEL_OBJ_FILES)
	$(ARMGNU)-ld -T $(KERNEL_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(KERNEL_OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img 

bootloader.img: $(BOOTLOADER_DIR)/linker.ld $(BOOTLOADER_OBJ_FILES)
	$(ARMGNU)-ld -T $(BOOTLOADER_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf $(BOOTLOADER_OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/bootloader.elf -O binary $(BUILD_DIR)/bootloader.img 

run:
	qemu-system-aarch64 -M raspi3 -kernel build/bootloader.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

cpio:
	qemu-system-aarch64 -M raspi3 -kernel build/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio

dtb:
	qemu-system-aarch64 -M raspi3 -kernel build/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb


