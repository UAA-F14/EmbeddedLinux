#ifndef PWM_H
#define PWM_H

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0"

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
extern const int PWM_PIN[8];

// channel: indice de pwmchipN/pwmX a exportar y usar (0..7, ver tabla arriba).
// El resto de las funciones operan sobre el ultimo canal inicializado.
int  pwm_init(int channel);
int  pwm_set_period(long period_ns);
int  pwm_set_duty_cycle(long duty_ns);
int  pwm_enable(void);
int  pwm_disable(void);
void pwm_cleanup(void);   // disable + unexport

#endif
