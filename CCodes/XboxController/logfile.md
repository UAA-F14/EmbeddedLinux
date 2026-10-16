lsusb | grep -i microsoft      # ID 045e:02ea (One S) o 045e:02dd (original)
dmesg | tail                   # debe mencionar "Xbox One" / xpad
ls /dev/input/                 # eventX y jsX
zcat /proc/config.gz | grep XPAD   # CONFIG_JOYSTICK_XPAD=y o =m