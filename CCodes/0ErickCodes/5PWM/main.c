#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define CHIP_PATH "/sys/class/pwm/pwmchip0"
#define PWM_PATH  CHIP_PATH "/pwm0"

static int write_int(const char *path, int value) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "No se pudo abrir %s: %s\n", path, strerror(errno));
        return -1;
    }
    fprintf(f, "%d", value);
    if (fclose(f) != 0) {   /* el error real de sysfs aparece aquí */
        fprintf(stderr, "Error escribiendo %d en %s: %s\n",
                value, path, strerror(errno));
        return -1;
    }
    return 0;
}

int main(void) {
    struct stat st;

    int period = 1000000000;    /* 1 s en ns -> parpadeo de 1 Hz */
    int duty   = period / 2;    /* 50 % */

    /* Export solo si pwm0 no existe todavía */
    if (stat(PWM_PATH, &st) != 0) {
        if (write_int(CHIP_PATH "/export", 0) != 0) return 1;
        usleep(200000);         /* esperar a que aparezca la carpeta */
    }

    /* Si ya estaba activo con otros valores, lo apagamos antes de reconfigurar */
    write_int(PWM_PATH "/enable", 0);

    /* Primero period y luego duty_cycle, porque duty no puede superar a period */
    if (write_int(PWM_PATH "/period", period) != 0) return 1;
    if (write_int(PWM_PATH "/duty_cycle", duty) != 0) return 1;
    if (write_int(PWM_PATH "/enable", 1) != 0) return 1;

    printf("PWM0 activo: period=%d ns, duty=%d ns\n", period, duty);
    return 0;
}
