import sys
import os


def main(fd):
    path = "build/kernel8.img"
    bin_size = os.path.getsize(path)
    bin = open(path, "rb", buffering=0)
    tty = open(fd, "wb", buffering=0) 
    start_code = "hello"
    tty.write(bytearray(start_code.encode("utf-8")))
    tty.write(bin_size.to_bytes(4, "big"))
    for b in bin_size.to_bytes(4, "big"):
        print(b)
    for i in range(0, bin_size):
        tty.write(bin.read())
    
    tty.close()
    bin.close()

if __name__ == '__main__':
    fd = sys.argv[1]
    main(fd)