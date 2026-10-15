first enable the i2c with
sudo orangepi-config

ls /dev/i2c-*
sudo apt install i2c-tools libi2c-dev
sudo i2cdetect -l