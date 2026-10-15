// P5PWMCar: control de velocidad de un motor por PWM con dos botones.
// Un boton acelera (sube el duty cycle), el otro frena (lo baja).
// PWM: igual que 0ErickCodes/5PWM_Claude (sysfs, export si hace falta).
// Botones: igual que P4LedSpeed (gpiod, pull-up, activos en bajo, debounce).
// Compilar: gcc main.c -o main -lgpiod
#include <errno.h>
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define GPIO_CHIP_PATH    "/dev/gpiochip1"
#define CONSUMER          "P5PWMCar"

#define BTN_FASTER_OFFSET 6   // pin fisico 8  (TXD.7)
#define BTN_SLOWER_OFFSET 5   // pin fisico 16 (PL5)

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0"
#define PWM_PATH      PWM_CHIP_PATH "/pwm0"   //

#define PWM_PERIOD_NS 10000000   // 10 ms -> 100 Hz, tipico para motor DC chico
#define SPEED_STEP    10         // % que sube/baja cada pulsacion
#define SPEED_MAX     100
#define SPEED_MIN     0
#define START_SPEED   0          // arranca detenido

#define TICK_US     10000   // 10 ms entre lecturas de boton
#define DEBOUNCE_MS 50

static volatile sig_atomic_t running = 1;

static void on_signal(int sig)
{
    (void)sig;
    running = 0;
}

static long long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// ---------- PWM (sysfs) ----------

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

static int pwm_init(void)
{
    struct stat st;

    // Export solo si pwm0 no existe todavia
    if (stat(PWM_PATH, &st) != 0) {
        if (write_int(PWM_CHIP_PATH "/export", 0) != 0)
            return -1;
        usleep(200000);   // esperar a que aparezca la carpeta
    }

    // Si ya estaba activo con otros valores, apagar antes de reconfigurar
    write_int(PWM_PATH "/enable", 0);

    // Primero period y luego duty_cycle, porque duty no puede superar a period
    if (write_int(PWM_PATH "/period", PWM_PERIOD_NS) != 0)
        return -1;
    if (write_int(PWM_PATH "/duty_cycle", 0) != 0)
        return -1;
    if (write_int(PWM_PATH "/enable", 1) != 0)
        return -1;

    return 0;
}

static int pwm_set_speed(int percentage)
{
    long duty = (long)PWM_PERIOD_NS * percentage / 100;
    return write_int(PWM_PATH "/duty_cycle", duty);
}

static void pwm_stop(void)
{
    write_int(PWM_PATH "/duty_cycle", 0);
    write_int(PWM_PATH "/enable", 0);
}

// ---------- Botones (gpiod) ----------

struct button {
    unsigned int offset;
    int pressed_prev;          // estado en la muestra anterior
    long long last_change_ms;  // ultimo flanco (cualquiera) visto
};

// Devuelve 1 si hubo una pulsacion nueva (flanco de bajada ya filtrado),
// 0 si no, -1 si fallo la lectura. Ignora rebotes al presionar y al soltar.
static int button_event(struct gpiod_line_request *req, struct button *b, long long now)
{
    enum gpiod_line_value v = gpiod_line_request_get_value(req, b->offset);
    if (v == GPIOD_LINE_VALUE_ERROR)
        return -1;

    int pressed = (v == GPIOD_LINE_VALUE_ACTIVE);
    int event = 0;

    if (pressed != b->pressed_prev) {
        if (pressed && (now - b->last_change_ms) >= DEBOUNCE_MS)
            event = 1;
        b->last_change_ms = now;
        b->pressed_prev = pressed;
    }
    return event;
}

int main(void)
{
    struct gpiod_chip *chip = NULL;
    struct gpiod_line_settings *button_settings = NULL;
    struct gpiod_line_config *line_config = NULL;
    struct gpiod_request_config *req_config = NULL;
    struct gpiod_line_request *request = NULL;
    unsigned int buttons[2] = {BTN_FASTER_OFFSET, BTN_SLOWER_OFFSET};
    int ret = 1;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (pwm_init() != 0) {
        fprintf(stderr, "No se pudo inicializar el PWM\n");
        return 1;
    }

    chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if (!chip) {
        fprintf(stderr, "Error al abrir %s: %s\n", GPIO_CHIP_PATH, strerror(errno));
        goto cleanup;
    }

    // Botones: entrada con pull-up; active_low => presionado = ACTIVE
    button_settings = gpiod_line_settings_new();
    if (!button_settings) {
        fprintf(stderr, "Error al crear BUTTON settings: %s\n", strerror(errno));
        goto cleanup;
    }
    gpiod_line_settings_set_direction(button_settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(button_settings, GPIOD_LINE_BIAS_PULL_UP);
    gpiod_line_settings_set_active_low(button_settings, true);

    line_config = gpiod_line_config_new();
    if (!line_config) {
        fprintf(stderr, "Error al crear line config: %s\n", strerror(errno));
        goto cleanup;
    }
    if (gpiod_line_config_add_line_settings(line_config, buttons, 2, button_settings) < 0) {
        fprintf(stderr, "Error al agregar lineas: %s\n", strerror(errno));
        goto cleanup;
    }

    req_config = gpiod_request_config_new();
    if (!req_config) {
        fprintf(stderr, "Error al crear request config: %s\n", strerror(errno));
        goto cleanup;
    }
    gpiod_request_config_set_consumer(req_config, CONSUMER);

    request = gpiod_chip_request_lines(chip, req_config, line_config);
    if (!request) {
        fprintf(stderr, "Error al solicitar GPIO: %s (errno=%d)\n", strerror(errno), errno);
        goto cleanup;
    }

    // pressed_prev = 1 al inicio: si un boton arranca presionado no cuenta como evento
    struct button faster = { BTN_FASTER_OFFSET, 1, -DEBOUNCE_MS };
    struct button slower = { BTN_SLOWER_OFFSET, 1, -DEBOUNCE_MS };

    int speed = START_SPEED;
    pwm_set_speed(speed);

    printf("Motor al %d%%. Boton 1 (offset %d) acelera, boton 2 (offset %d) frena. Ctrl+C para salir.\n",
           speed, BTN_FASTER_OFFSET, BTN_SLOWER_OFFSET);
    fflush(stdout);

    while (running) {
        long long now = now_ms();

        int up = button_event(request, &faster, now);
        int down = button_event(request, &slower, now);
        if (up < 0 || down < 0) {
            fprintf(stderr, "Error al leer botones: %s\n", strerror(errno));
            break;
        }

        int old_speed = speed;
        if (up && speed < SPEED_MAX)
            speed += SPEED_STEP;
        if (down && speed > SPEED_MIN)
            speed -= SPEED_STEP;
        if (speed > SPEED_MAX) speed = SPEED_MAX;
        if (speed < SPEED_MIN) speed = SPEED_MIN;

        if (speed != old_speed) {
            if (pwm_set_speed(speed) != 0) {
                fprintf(stderr, "Error al actualizar el duty cycle\n");
                break;
            }
            printf("Motor al %d%%\n", speed);
            fflush(stdout);
        }

        usleep(TICK_US);
    }

    ret = 0;

cleanup:
    pwm_stop();
    if (request)
        gpiod_line_request_release(request);
    gpiod_request_config_free(req_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(button_settings);
    if (chip)
        gpiod_chip_close(chip);
    return ret;
}
