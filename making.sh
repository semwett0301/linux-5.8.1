sudo make menuconfig
sudo make -j12
sudo make modules_install -j12
sudo make install -j12
sudo update-grub
