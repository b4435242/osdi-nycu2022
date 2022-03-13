# My OSC 2022

## Author

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
|`310551091`| `b4435242` | `林柏均` | b4435242@gmail.com |

## How to build

**make**


## How to run

* Qemu 

      1. make run
      2. cat /dev/pts/$(n)
      3. python3 write.py /dev/pts/$(n)
      4. echo ls > /dev/pts/$(n)
 
* Rpi3

      1. Make sure SD card contains bootcode.bin, fixup.dat, start.elf, config.txt, bootloader.img, initramfs.cpio, bcm2710-rpi-3-b-plus.dtb
      2. sudo screen /dev/ttyUSB0 115200
      3. sudo python3 write.py /dev/ttyUSB0

