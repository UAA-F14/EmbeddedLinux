#include <stdio.h>
#include <stdlib.h>

#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0"
int main() {
    FILE *file;
    // Export PWM channel 0
    file = fopen(PWM_PATH "/period", "w");
    if (file == NULL) {
        perror("Failed to open period file");
        return 1;
    }
    fprintf(file, "100000"); // Set period to 10000000ns (50Hz)
    fclose(file);
    //configure duty cycle
    file = fopen(PWM_PATH "/duty_cycle", "w");
    if (file == NULL) {
        perror("Failed to open duty_cycle file");
        return 1;
    }
    fprintf(file, "10000"); // Set duty cycle to 5000000ns (50%)
    fclose(file);
    //enable PWM channel 0
    file = fopen(PWM_PATH "/enable", "w");
    if (file == NULL) {
        perror("Failed to open enable file");
        return 1;
    }
    fprintf(file, "1"); // Enable PWM channel 0
    fclose(file);
    printf("PWM channel 0 enabled successfully.\n");
    return 0;
}
