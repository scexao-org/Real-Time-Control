# Custom compiler script for the DM drivers

mkdir -p tmp
mkdir -p lib
cd src
make
cd ../bin
sudo insmod bmc_mdrv.ko
sudo chmod o+rw /dev/bmc_mdrv*
