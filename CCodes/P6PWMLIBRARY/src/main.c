#include "gpio.h"
#include <unistd.h>

#define LED 4
#define BUTTON1 5
#define BUTTON2 6
int main(void){
    // GPIO
    gpio_init();
    gpio_configure(LED,GPIO_OUTPUT);
    gpio_configure(BUTTON1,GPIO_INPUT_PULLUP);
    gpio_configure(BUTTON2,GPIO_INPUT_PULLUP);
    gpio_start();
    // PWM
    int period = 10000000; // 100 Hz -> 10 ms -> 10 000 000 ns
    pwm_set_period(period); // 100 Hz -> 10 ms -> 10 000 000 ns
    pwm_set_duty_cycle(period/2); // 50% duty cycle
    pwm_set_channel(0); // Set channel to 0

    while(1){
        if(gpio_get(BUTTON1)==0){
            gpio_set(LED,1);
            usleep(50000);
            gpio_set(LED,0);
            usleep(50000);
        }
        if(gpio_get(BUTTON2)==0){
            gpio_set(LED,1);
            usleep(500000);
            gpio_set(LED,0);
            usleep(500000);
        }
    }
    gpio_cleanup();
    return 0;
}