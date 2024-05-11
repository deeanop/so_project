#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
FILE *f=NULL;
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
        strcpy(path, dir_name);
        strcat(path, "/");
        strcat(path, d->d_name);

        if (stat(path, &buff) == -1) {
            perror("Eroare la gasirea starii fisierului");
            exit(EXIT_FAILURE);
        }

        fprintf(f, "Nume: %s\n", d->d_name);
        fprintf(f, "Dimensiune: %ld bytes\n", buff.st_size);
        fprintf(f, "Ultima modificare: %s", ctime(&buff.st_mtime));
        fprintf(f, "Permisiuni: %o\n", buff.st_mode & 0777);
        fprintf(f, "Numar i-node: %ld\n", buff.st_ino);

        if (S_ISDIR(buff.st_mode)) {
            printf("Este director\n");
            dir_parse(path);
        } else {
            printf("Este fisier\n");
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
  f=fopen("snapshot.txt", "w");
  if(f==NULL){
    perror("NoFileOpen");
    exit(-1);
  }
    dir_parse(argv[1]);
    fclose(f);
    return 0;
}
//prima versiune a fisierului va accesa fisiele fara a utiliza apeluri sistem, precum open sau write, ci doar functii de biblioteca
//in versiunile urmatoare, acest neajuns va fi corectat, astfel incat sa se foloseasca apeluri sistem
