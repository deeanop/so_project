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
int fd,fdr;

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
        int valid;
        valid=write(fd, d->d_name, strlen(d->d_name));
        if(valid!=strlen(d->d_name))
            exit(-1);
        valid=write(fd, &buff.st_size,sizeof(off_t));
        if(valid!=sizeof(off_t))
            exit(-1);
        valid=write(fd, ctime(&buff.st_mtime), strlen(ctime(&buff.st_mtime)));
        if(valid!=strlen(ctime(&buff.st_mtime)))
                exit(-1);
        valid=write(fd, &buff.st_mode, sizeof(mode_t));
        if(valid!=sizeof(mode_t))
            exit(-1);
        valid=write(fd, &buff.st_ino, sizeof(ino_t));
        if(valid!=sizeof(ino_t))
            exit(-1);

        if (S_ISDIR(buff.st_mode)) {
            printf("Este director\n");
            dir_parse(path);
        } else {
            printf("Este fișier\n");
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc > 11) {
        perror("TooManyDirectories");
        exit(EXIT_FAILURE);
    }
    int valid;
    fd=open("snapshot.txt", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        perror("NoFileOpen");
        exit(EXIT_FAILURE);
    }
    char f[5]="*\n";
    valid=write(fd,f, strlen(f));
    if(valid!=strlen(f))
        exit(-1);
    for (int i = 1; i < argc; i++) {
        dir_parse(argv[i]);
        valid=write(fd, f, strlen(f));
        if(valid!=strlen(f))
            exit(-1);
    }

    valid=lseek(fd, SEEK_SET, 0); // Resetăm cursorul la începutul fișierului
    if(valid==-1)
        exit(-1);
    int i = 0;
    char line[MAX_LINE_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    while(valid=read(fd, line, sizeof(line))){
        if(valid!=sizeof(line))
            exit(-1);
        if (line[0] == '*') {
            strcpy(filename, "file%d.txt");
            fdr = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
            if (fdr == -1) {
                perror("FileNotOpen");
                exit(EXIT_FAILURE);
            }
            i++;
        } else {
            valid=write(fdr, line, strlen(line));
            if(valid!=strlen(line))
                exit(-1);
        }
    }
    if (fdr != -1) {
        valid=close(fdr);
        if(valid==-1){
            perror("NoFileClosed");
            exit(-1);
        }
    }
    valid=close(fd);
    if(valid==-1){
        perror("NoFileOpen");
        exit(-1);
    }

    return 0;
}
//aici am folosit apeluri sistem, dar mai este de lucru la modul in care datele sunt scrise in snapshot
