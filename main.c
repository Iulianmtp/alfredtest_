#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

struct Watch {
    int wd;
    char path[PATH_MAX];
};

struct Watch *lista_watch;
int max_w = 100;

// Funzione per scrivere il log su file e a video
void scrivi_log(int wd, uint32_t mask, char *name) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char ts[20];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
    
    if (mask & IN_CREATE) azione = "CREATO";
    else if (mask & IN_DELETE) azione = "ELIMINATO";
    else if (mask & IN_MOVED_FROM) azione = "SPOST_DA";
    else if (mask & IN_MOVED_TO) azione = "SPOST_A";

    FILE *f = fopen("report.txt", "a");
    if (f) {
        fprintf(f, "[%s] %s %s: %s/%s\n", ts, azione, evento, lista_watch[wd].path, name);
        fclose(f);    
    }
    printf("[%s] %s rilevato su %s/%s\n", ts, azione, lista_watch[wd].path, name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) return printf("Uso: %s <dir>\n", argv[0]), 1;

    int fd = inotify_init();
    lista_watch = calloc(max_w, sizeof(struct Watch));
    uint32_t m = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;
    }
    return 0;
}
