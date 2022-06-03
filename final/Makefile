
all: ssd ver

ssd:
	gcc ssd_fuse.c -Wall `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse -g

ver:
	gcc ssd_fuse_dut.c -Wall -o ssd_fuse_dut -g

clean:
	rm -f ssd_fuse ssd_fuse_dut