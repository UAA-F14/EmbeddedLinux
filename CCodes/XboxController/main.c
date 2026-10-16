#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int main(void) {
    int fd = open("/dev/input/event5", O_RDONLY);  // ajusta el número
    if (fd < 0) { perror("open"); return 1; }

    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY)
            printf("Tecla %u = %d\n", ev.code, ev.value);
        else if (ev.type == EV_ABS)
            printf("Eje %u = %d\n", ev.code, ev.value);
    }
    close(fd);
    return 0;
}