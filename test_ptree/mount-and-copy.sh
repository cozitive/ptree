#/bin/bash

BASEDIR=$(dirname "$0")
cd $BASEDIR

mkdir ./mntdir
sudo mount ../tizen-image/rootfs.img ./mntdir
sudo cp test_ptree ./mntdir/root
sudo umount ./mntdir
rmdir ./mntdir