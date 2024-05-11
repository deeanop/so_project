#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
# include <fcntl.h>

#define MAX_FILENAME_LENGTH 20
#define MAX_LINE_LENGTH 200

FILE *f = NULL;

void dir_parse(char *dir_name) {
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;
    struct stat buff;
    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir_name, d->d_name);

        if (stat(path, &buff) == -1) {
            perror("Eroare la găsirea stării fișierului");
            exit(EXIT_FAILURE);
        }

        fprintf(f, "Nume: %s\n", d->d_name);
        fprintf(f, "Dimensiune: %ld bytes\n", buff.st_size);
        fprintf(f, "Ultima modificare: %s", ctime(&buff.st_mtime));
        fprintf(f, "Permisiuni: %o\n", buff.st_mode & 0777);
        fprintf(f, "Număr i-node: %ld\n", buff.st_ino);

        if (S_ISDIR(buff.st_mode)) {
            fprintf(f, "Este director\n");
            dir_parse(path);
        } else {
            fprintf(f, "Este fișier\n");
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    FILE *r = NULL;
    if (argc > 11) {
        perror("TooManyDirectories");
        exit(EXIT_FAILURE);
    }
    f = fopen("snapshot1.txt", "w+");
    if (f == NULL) {
        perror("NoFileOpen");
        exit(EXIT_FAILURE);
    }

    fprintf(f, "*\n");
    for (int i = 1; i < argc; i++) {
        dir_parse(argv[i]);
        fprintf(f, "*\n");
    }

    rewind(f); // Resetăm cursorul la începutul fișierului

    int i = 0;
    char line[MAX_LINE_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    while (fgets(line, sizeof(line), f) != NULL) {
        if (line[0] == '*') {
            snprintf(filename, sizeof(filename), "file%d.txt", i);
            r = fopen(filename, "w");
            if (r == NULL) {
                perror("FileNotOpen");
                exit(EXIT_FAILURE);
            }
            i++;
        } else {
            fprintf(r, "%s", line);
        }

        // Verificăm dacă suntem la sfârșitul fișierului
        if (feof(f)) {
            break;
        }
    }
    if (r != NULL) {
        fclose(r);
    }
    fclose(f);

    return 0;
}
//nici aici nu am folosit inca apeluri sistem, dar am extins functionalitatea pentru a analiza mai multe directoare trimise in linia de comanda
