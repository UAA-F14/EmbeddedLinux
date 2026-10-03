#include "pwm.h"

void pwm_init(int channel){
    FILE *file = fopen(PWM_PATH "/export", "w");
    if (file == NULL) {
        perror("Error opening export file");
        return;
    }
    fprintf(file, "%d", channel); // Export the specified pin
    fclose(file);
}

void pwm_set_period(int period){
    FILE *file = fopen(PWM_PATH "/period", "w");
    if (file == NULL) {
        perror("Error opening period file");
        return;
    }
    fprintf(file, "%d", period);
    fclose(file);
}
void pwm_set_duty_cycle(int duty_cycle){
    FILE *file = fopen(PWM_PATH "/duty_cycle", "w");
    if (file == NULL) {
        perror("Error opening duty_cycle file");
        return;
    }
    fprintf(file, "%d", duty_cycle);
    fclose(file);
}
void pwm_set_channel(int channel){
    FILE *file = fopen(PWM_PATH "/channel", "w");
    if (file == NULL) {
        perror("Error opening channel file");
        return;
    }
    fprintf(file, "%d", channel);
    fclose(file);
    printf("Channel set to %d on pin %d\n", channel, PWM_PIN[channel]);
}

void pwm_enable(){
    FILE *file = fopen(PWM_PATH "/enable", "w");
    if (file == NULL) {
        perror("Error opening enable file");
        return;
    }
    fprintf(file, "%d", 1);
    fclose(file);
}