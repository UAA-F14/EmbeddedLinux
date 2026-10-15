#include "pwm.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const int PWM_PIN[8] = {29, 0, 33, 35, 37, 32, 36, 38};

// Ruta base del ultimo canal inicializado (pwmchipN/pwmX), la llenan
// pwm_init() y el resto de las funciones la reutilizan. Antes la libreria
// tenia PWM_PATH fijo a pwm0 sin importar el canal pedido; con esto cada
// canal escribe en su propia carpeta.
static char current_chan_path[64];
static int  current_channel = -1;
static int  initialized = 0;

static int write_int(const char *path, long value)
{
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "No se pudo abrir %s: %s\n", path, strerror(errno));
        return -1;
    }
    fprintf(f, "%ld", value);
    if (fclose(f) != 0) {   // el error real de sysfs aparece aqui
        fprintf(stderr, "Error escribiendo %ld en %s: %s\n", value, path, strerror(errno));
        return -1;
    }
    return 0;
}

int pwm_init(int channel)
{
    struct stat st;
    char export_path[64];

    snprintf(export_path, sizeof(export_path), "%s/export", PWM_CHIP_PATH);
    snprintf(current_chan_path, sizeof(current_chan_path), "%s/pwm%d", PWM_CHIP_PATH, channel);

    // Export solo si el canal no existe todavia (re-exportar un canal ya
    // activo regresa EBUSY)
    if (stat(current_chan_path, &st) != 0) {
        if (write_int(export_path, channel) != 0)
            return -1;
        usleep(200000);   // esperar a que el kernel cree la carpeta pwmN
    }

    current_channel = channel;
    initialized = 1;

    // Si ya estaba activo con otros valores, apagar antes de reconfigurar
    pwm_disable();
    return 0;
}

int pwm_set_period(long period_ns)
{
    char path[80];
    if (!initialized) {
        fprintf(stderr, "pwm_set_period: llama pwm_init() primero\n");
        return -1;
    }
    snprintf(path, sizeof(path), "%s/period", current_chan_path);
    return write_int(path, period_ns);
}

int pwm_set_duty_cycle(long duty_ns)
{
    char path[80];
    if (!initialized) {
        fprintf(stderr, "pwm_set_duty_cycle: llama pwm_init() primero\n");
        return -1;
    }
    snprintf(path, sizeof(path), "%s/duty_cycle", current_chan_path);
    return write_int(path, duty_ns);
}

int pwm_enable(void)
{
    char path[80];
    if (!initialized) {
        fprintf(stderr, "pwm_enable: llama pwm_init() primero\n");
        return -1;
    }
    snprintf(path, sizeof(path), "%s/enable", current_chan_path);
    return write_int(path, 1);
}

int pwm_disable(void)
{
    char path[80];
    if (!initialized)
        return 0;
    snprintf(path, sizeof(path), "%s/enable", current_chan_path);
    return write_int(path, 0);
}

void pwm_cleanup(void)
{
    char unexport_path[64];
    if (!initialized)
        return;

    pwm_disable();

    snprintf(unexport_path, sizeof(unexport_path), "%s/unexport", PWM_CHIP_PATH);
    write_int(unexport_path, current_channel);

    current_channel = -1;
    initialized = 0;
}
