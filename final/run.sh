# clean
fusermount -u /tmp/ssd
rm -rf /tmp/ssd
# rebuid
make clean && make
mkdir -p /tmp/ssd

# run
./ssd_fuse -d /tmp/ssd 
#gdb ./ssd_fuse -ex "set args -d /tmp/ssd" -ex "b gc" -ex "r"
