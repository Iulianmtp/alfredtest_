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

    char *evento = (mask & IN_ISDIR) ? "DIR" : "FILE";
    char *azione = "EVENTO";
    
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

    // Watch iniziali
    for (int i = 1; i < argc; i++) {
        int wd = inotify_add_watch(fd, argv[i], m);
        if (wd != -1) {
            lista_watch[wd].wd = wd;
            realpath(argv[i], lista_watch[wd].path);
        }
    }

    char buf[BUF_LEN];
    while (1) {
        int n = read(fd, buf, BUF_LEN);
        for (char *p = buf; p < buf + n; ) {
            struct inotify_event *ev = (struct inotify_event *) p;
            
            if (ev->len > 0) {
                scrivi_log(ev->wd, ev->mask, ev->name);

                // Se nasce una cartella, aggiungi watch
                if ((ev->mask & IN_CREATE) && (ev->mask & IN_ISDIR)) {
                    char npath[PATH_MAX];
                    snprintf(npath, sizeof(npath), "%s/%s", lista_watch[ev->wd].path, ev->name);
                    int nwd = inotify_add_watch(fd, npath, m);
                    if (nwd != -1) {
                        lista_watch[nwd].wd = nwd;
                        strcpy(lista_watch[nwd].path, npath);
                    }
                }
            }
            p += sizeof(struct inotify_event) + ev->len;
        }
    }
    return 0;
}
