ARMGNU ?= aarch64-linux-gnu

FLAGS = -g -ffreestanding -nostdinc -nostdlib -nostartfiles

BUILD_DIR = build
SRC_DIR = src
OS_DIR = os
IMG = os.img

all: kernel8.img

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%_c.o:	$(SRC_DIR)/%.c 
	mkdir -p $(@D) 
	$(ARMGNU)-gcc $(FLAGS) -c $< -o $@ 

$(BUILD_DIR)/%_s.o:	$(SRC_DIR)/%.S 
	mkdir -p $(@D) 
	$(ARMGNU)-gcc -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img 

run:
	qemu-system-aarch64 -M raspi3 -kernel build/kernel8.img -display none -serial null -serial stdio

burn:
	cp $(BUILD_DIR)/kernel8.img /dev/sdb1/
