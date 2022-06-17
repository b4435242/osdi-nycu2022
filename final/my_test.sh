./ssd_fuse_dut /tmp/ssd/ssd_file l # This show current logical size in ssd_file
./ssd_fuse_dut /tmp/ssd/ssd_file p # This show current physical size in ssd_file
./ssd_fuse_dut /tmp/ssd/ssd_file r 100 0 # This will read 100 Byte at 0 offset
./ssd_fuse_dut /tmp/ssd/ssd_file w 600 10 # This will write 200 Byte at 10 offset
./ssd_fuse_dut /tmp/ssd/ssd_file r 100 450 # This will read 100 Byte at 0 offset
