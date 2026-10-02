// 3GPIOIN: el LED parpadea y dos botones (pull-up, activos en bajo)
// controlan la velocidad: uno acelera, el otro desacelera.
// Compilar: cmake --build build   (o: gcc main.c -o build/3GPIOIN -lgpiod)
#include <errno.h>
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CHIP_PATH         "/dev/gpiochip1"
#define CONSUMER          "3GPIOIN"

#define LED_OFFSET        4
#define BTN_FASTER_OFFSET 6   
#define BTN_SLOWER_OFFSET 5   

#define TICK_US           10000   
#define DEBOUNCE_MS       50      


static const unsigned int half_period_ms[] = {1000, 700, 500, 300, 200, 100, 50};
#define N_LEVELS    ((int)(sizeof(half_period_ms) / sizeof(half_period_ms[0])))
#define START_LEVEL 2

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

struct button {
    unsigned int offset;
    int pressed_prev;         // estado en la muestra anterior
    long long last_change_ms; // último flanco (cualquiera) visto
};

// Devuelve 1 si hubo una pulsación nueva (flanco de bajada ya filtrado),
// 0 si no, -1 si falló la lectura. Ignora rebotes tanto al presionar como al soltar.
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
    struct gpiod_line_settings *led_settings = NULL;
    struct gpiod_line_settings *button_settings = NULL;
    struct gpiod_line_config *line_config = NULL;
    struct gpiod_request_config *req_config = NULL;
    struct gpiod_line_request *request = NULL;
    unsigned int led = LED_OFFSET;
    unsigned int buttons[2] = {BTN_FASTER_OFFSET, BTN_SLOWER_OFFSET};
    int ret = 1;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) {
        fprintf(stderr, "Error al abrir %s: %s\n", CHIP_PATH, strerror(errno));
        return 1;
    }

    // LED: salida, apagado al inicio
    led_settings = gpiod_line_settings_new();
    if (!led_settings) {
        fprintf(stderr, "Error al crear LED settings: %s\n", strerror(errno));
        goto cleanup;
    }
    gpiod_line_settings_set_direction(led_settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(led_settings, GPIOD_LINE_VALUE_INACTIVE);

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
    if (gpiod_line_config_add_line_settings(line_config, &led, 1, led_settings) < 0 ||
        gpiod_line_config_add_line_settings(line_config, buttons, 2, button_settings) < 0) {
        fprintf(stderr, "Error al agregar líneas: %s\n", strerror(errno));
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

    // pressed_prev = 1 al inicio: si un botón arranca presionado no cuenta como evento
    struct button faster = { BTN_FASTER_OFFSET, 1, -DEBOUNCE_MS };
    struct button slower = { BTN_SLOWER_OFFSET, 1, -DEBOUNCE_MS };

    int level = START_LEVEL;
    int led_on = 0;
    long long last_toggle = now_ms();

    printf("Nivel %d/%d (%u ms por semiciclo). Ctrl+C para salir.\n",
           level + 1, N_LEVELS, half_period_ms[level]);
    fflush(stdout);

    while (running) {
        long long now = now_ms();

        int up = button_event(request, &faster, now);
        int down = button_event(request, &slower, now);
        if (up < 0 || down < 0) {
            fprintf(stderr, "Error al leer botones: %s\n", strerror(errno));
            break;
        }

        int old_level = level;
        if (up && level < N_LEVELS - 1)
            level++;
        if (down && level > 0)
            level--;
        if (level != old_level) {
            printf("Nivel %d/%d (%u ms por semiciclo)\n",
                   level + 1, N_LEVELS, half_period_ms[level]);
            fflush(stdout);
        }

        if (now - last_toggle >= (long long)half_period_ms[level]) {
            led_on = !led_on;
            gpiod_line_request_set_value(request, led,
                led_on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
            last_toggle = now;
        }

        usleep(TICK_US);
    }

    gpiod_line_request_set_value(request, led, GPIOD_LINE_VALUE_INACTIVE);
    ret = 0;

cleanup:
    if (request)
        gpiod_line_request_release(request);
    gpiod_request_config_free(req_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(button_settings);
    gpiod_line_settings_free(led_settings);
    gpiod_chip_close(chip);
    return ret;
}