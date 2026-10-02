#ifndef PWM_H
#define PWM_H
#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0"

/*
PWM Bus     Pin     dtbo
SPWM0-2     7       spwm2
PWM0-0      29      pwm0
PWM0-2      33      pwm2
PWM0-3      35      pwm3
PWM0-4      37      pwm4
PWM0-5      32      pwm5
PWM0-6      36      pwm6
PWM0-7      38      pwm7
*/
int PWM_PIN* = {29, 0,33, 35, 37, 32, 36, 38};
void pwm_set_period(int period);
void pwm_set_duty_cycle(int duty_cycle);
void pwm_set_channel(int channel);
#endif 