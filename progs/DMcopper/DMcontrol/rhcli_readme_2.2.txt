Instructions for installing and building Red Hat Linux-based RHCLI:


  1) Install Red Hat Enterprise Linux 4
  2) Install VMETRO Dpio2-drv-src-x86-linux (copy on FTP site)
  3) Follow the VMETRO instructions to build the VMETRO driver and user-mode library
  4) Unpack the provided TAR project archive "rh-test-src-2.2.tar"
  5) type "cd rh-test"
  6) type "make clean; make"
  7) type "./rhload-dpio2" to load the DPIO2 driver
  8) type "./rhcli" to run the command line interface
