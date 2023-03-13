#/bin/bash

BASEDIR=$(dirname "$0")
cd $BASEDIR

make clean
make test_ptree
mkdir ./mntdir
sudo mount ../tizen-image/rootfs.img ./mntdir
sudo cp test_ptree ./mntdir/root
sudo umount ./mntdir
rmdir ./mntdir