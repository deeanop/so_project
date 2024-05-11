
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> 
#include <linux/limits.h>
#include <bits/waitflags.h>

#define MAX_FILENAME_LENGTH 20 //dimensiunea maxima a unei denumiri de fisier
#define MAX_LINE_LENGTH 5000  //dimensiunea maxima a unei linii ce va fi transferata catre fisierele de snapshot finale
int fd,fdr;  //variabile pentru a memora descriptoarele de fisier
int k;
char mal_dir[30];
char target_path[1024];
char a[100][2000];
char b[100][2000];
void compar(char a[][2000], char b[][2000], int k) {
    int i=0,j=0;
    if (a[0][0] != '\0') {
        while(i<k && j<k) {
            if (strcmp(a[i], b[j]) != 0 && strcmp(a[i], b[j++]) == 0){
                printf("A fost sters fisierul cu calea %s\n", a[i]);
                i++;
            }
            else {
                if (strcmp(a[i], b[j]) != 0 && strcmp(a[i++], b[j]) == 0){
                    printf("A fost adaugat fisierul cu calea %s\n", b[i]);
                    j++;
                }
                else{
                    if(strcmp(a[i], b[j])!=0 && strcmp(a[i+1], b[j+1])==0)
                        printf("Fisierul cu calea %s a fost redenumit sau inlocuit cu fisierul cu calea %s", a[i], b[i]);
                }
            }
        }
    }
    for(i=0;i<k;i++)
        strcpy(a[i], b[i]);
}

void dir_parse(char *dir_name) {
    int pfd[2];
    pid_t pid_s;
    char msg[10], msgr[10];
    char line[MAX_LINE_LENGTH];
    int value;
    DIR *dir = opendir(dir_name);
    if(pipe(pfd)<0){
        perror("ERoare la crearea pipe-ului");
        exit(EXIT_FAILURE);
    }

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
        sprintf(line, "Calea catre fisier: %s ,Dimensiunea in octeti: %ld, Permisiuni: %o, Nodul index: %ld, Data ultimei actualizari: %s", path, buff.st_size, buff.st_mode, buff.st_ino, ctime(&buff.st_mtime));
        write(fd, line, strlen(line));

        if (S_ISDIR(buff.st_mode)) {
            printf("Fisierul cu calea %s este director\n", path);
            dir_parse(path);
            strcpy(b[k], path);
            k++;
        } else if (S_ISLNK(buff.st_mode)) {
            char target_path[PATH_MAX];
            ssize_t len = readlink(path, target_path, sizeof(target_path) - 1);
            if (len != -1) {
                target_path[len] = '\0';
                printf("Fisierul cu calea %s este legatura simbolica spre fisierul %s\n", path, target_path);
            } else {
                perror("Eroare la citirea legaturii simbolice");
                exit(EXIT_FAILURE);
            }
            strcpy(b[k], path);
            k++;
        } else {
            printf("Fisierul cu calea %s este un fisier obisnuit\n", path);
            strcpy(b[k], path);
            k++;
            if (!(buff.st_mode & S_IRUSR) && !(buff.st_mode & S_IRGRP) && !(buff.st_mode & S_IROTH) && !(buff.st_mode & S_IWUSR) && !(buff.st_mode & S_IWGRP) && !(buff.st_mode & S_IWOTH) && !(buff.st_mode & S_IXUSR) && !(buff.st_mode & S_IXGRP) && !(buff.st_mode & S_IXOTH)) {
                pid_s = fork();
                if (pid_s < 0) {
                    perror("Eroare la crearea procesului pentru verificarea fisierului");
                    exit(EXIT_FAILURE);
                }
                if (pid_s == 0) {
                    close(pfd[0]);
                    printf("Fisierul cu calea %s este potential periculos.\n", path);
                        char command[5000];
                        sprintf(command, "./test.sh ~/%s", path);
                        value=system(command);
                        if(value==1){
                            strcpy(msg, "SAFE");
                            write(pfd[1], msg, strlen(msg));
                            close(pfd[1]);
                        }else{
                            strcpy(msg, d->d_name);
                            write(pfd[1], msg, strlen(msg));
                            close(pfd[1]);
                        }
                        exit(EXIT_SUCCESS);
                    }
                } else {
                    close(pfd[1]); // Închide capătul de scriere al pipe-ului în procesul părinte
                    read(pfd[0], msgr, sizeof(msgr)); // Citirea mesajului din pipe

                    if (strcmp(msgr, "SAFE") == 0) {
                        printf("Fisierul este sigur.\n");
                    } else {
                        printf("Fisierul %s nu este sigur.\n", msgr);
                        if (rename(path, mal_dir) == 0) {
                            printf("Fisierul a fost izolat.\n");
                        } else {
                            printf("Nu se poate izola fisierul.\n");
                        }
                    }

                close(pfd[0]); // Închide capătul de citire al pipe-ului în procesul părinte

                int status;
                waitpid(pid_s, &status, WCONTINUED);
                printf("Procesul fiu de izolare cu pid-ul %d s-a finalizat cu codul %d.\n", pid_s, status);
            }

        }
    }
    k=0;
    closedir(dir);
}


int main(int argc, char **argv){
    if (argc > 13) { //conditia sa fie maxim 12 argumente
        perror("TooManyDirectories");
        exit(EXIT_FAILURE);
    }
    int valid;  //variabila folosita pentru a verifica functionalitatea unor apeluri sistem
    fd=open("snapshot1.txt", O_WRONLY | O_CREAT | O_TRUNC);  //deshidem fisierul pentru snapshot-uri
    if (fd == -1) {
        perror("NoFileOpen");
        exit(EXIT_FAILURE);
    }
    pid_t pid;
    int i;
    for (i = 1; i < argc-2; i++) {
        pid=fork();  //pentru fiecare director dat ca argument cream un nou proces
        if(pid<0){
            perror("Eroare la crearea procesului");
            exit(-1);
        }
        else{
            if(pid==0){  //daca am intrat in proces vom iesi din bucla, pentru ca procesul fiu sa nu creeze un alt proces
                dir_parse(argv[i]);  //daca procesul a fost creat cu succes, intram in directorul dat ca argument pe pozitia i
                break;
            }
    	else{
            int status; //codul cu care se termina procesul
            waitpid(pid,&status,WCONTINUED); //da valoare variabilei status 
            printf("Procesul cu PID-ul %d s-a incheiat cu codul %d.\n", pid, status);  //marcam in terminal finalizarea procesului curent
            //asteptam pana cand toate procesele au fost realizate
        	}
        }
    }
    compar(a,b,k);
    if(strcmp(argv[argc-2],"-s")==0)
        sprintf(mal_dir, "~/%s", argv[argc-1]);
    valid=close(fd); //inchiderea fisierului initial
    if(valid==-1){
        perror("NoFileOpen");
        exit(-1);
    }
    return 0;
}
//versiunea finala
//am modificat numele script-ului ce va fi apelat
