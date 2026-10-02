# 9-15

sudo apt update
sudo apt upgrade
sudo apt install gpiod libgpiod-dev

orangepi@orangepi4pro:~$ sudo gpiodetect
gpiochip0 [2000000.pinctrl] (352 lines)
gpiochip1 [7025000.pinctrl] (64 lines)

gcc main.c -o main -lgpiod


# 9 - 18 Instalar Cmake list

sudo apt install cmake


cmake -S . -B build

cmake --build build

./build/2CMake