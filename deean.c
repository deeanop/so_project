
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
#define MAX_LINE_LENGTH 500  //dimensiunea maxima a unei linii ce va fi transferata catre fisierele de snapshot finale
int fd,fdr;  //variabile pentru a memora descriptoarele de fisier
char mal_dir[30];
char target_path[1024];

void dir_parse(char *dir_name) {  //functie care parcurge directorul al carui nume este dat ca parametru
    pid_t pid_s;
    DIR *dir = opendir(dir_name);  //deschiderea directorului
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        exit(EXIT_FAILURE);
    }

    struct dirent *d;  //folosita pentru a citi din director
    struct stat buff, mal;
    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) //verificam ca legatura sa nu pointeze spre acelasi director sau spre directorul parinte
            continue;

        char path[PATH_MAX];  //construim calea directorului
        snprintf(path, sizeof(path), "%s/%s", dir_name, d->d_name);

        if (stat(path, &buff) == -1) {  //verificarea starii directorului
            perror("Eroare la găsirea stării fișierului");
            exit(EXIT_FAILURE);
        }
        int valid;  //variabila folosita pentru a verifica functionarea unor functii si apeluri sistem
        write(fd, "Calea catre fisier\n", 30);
        valid=write(fd, path, strlen(path));  //scriem in fisierul initial numele intrarii curente
        if(valid!=strlen(path))
            exit(-1);
        write(fd, "\n", 3);
        write(fd, "Dimensiunea in octeti a fisierului", 40);
        valid=write(fd, &buff.st_size,sizeof(buff.st_size));  //scriem in fisierul initial dimensiunea in octeti a fisierului catre care pointeaza legatura curenta
        if(valid!=sizeof(off_t))
            exit(-1);
        write(fd, "\n", 3);
        write(fd, "Data ultimei actualizari\n", 30);
        valid=write(fd, ctime(&buff.st_mtime), strlen(ctime(&buff.st_mtime))); //scriem in fisierul initial data untimei acrualizari a fisierului catre care pointeaza legatura curenta
        if(valid!=strlen(ctime(&buff.st_mtime)))
                exit(-1);
        write(fd, "\n", 3);
        write(fd, "Tipul fisierului\n", 20);
        valid=write(fd, &buff.st_mode, sizeof(buff.st_mode));  //scriem in fisierul initial tipul fisierului catre care pointeaza legatura curenta
        if(valid!=sizeof(mode_t))
            exit(-1);
        write(fd, "\n", 3);
        write(fd, "Nodul index\n", 15);
        valid=write(fd, &buff.st_ino, sizeof(buff.st_ino));
        if(valid!=sizeof(ino_t))
            exit(-1);
        write(fd, "\n", 3);
        if (S_ISDIR(buff.st_mode)) {
            printf("Fisierul cu calea %s este director\n", path);  //daca fisierul accesat este director, vom specifica acest lucru in STDOUT
            dir_parse(d->d_name);  //apelam recursiv functia pentru parcurgerea directorului
        }else{
            if(S_ISLNK(buff.st_mode)){
                readlink(d->d_name, target_path, sizeof(target_path) - 1);
                printf("FIsierul cu calea %s este legatura simbolica spre fisierul %s\n", path, target_path);
            }
            else {
                printf("Este fișier\n");  //in orice alta situatie, vom specifica faptul ca este un fisier
            
        
        char command[300]; //comanda cu care vom apela shellscript-ul
        char mal_path[PATH_MAX];
        snprintf(mal_path, sizeof(mal_path), "%s/%s", dir_name, d->d_name);
        if (stat(mal_path, &mal) == 0) {
            if(!(mal.st_mode & S_IRUSR) && !(mal.st_mode & S_IRGRP) && !(mal.st_mode & S_IROTH) && !(mal.st_mode & S_IWUSR) && !(mal.st_mode & S_IWGRP) && !(mal.st_mode & S_IWOTH) && !(mal.st_mode & S_IXUSR) && !(mal.st_mode & S_IXGRP) && !(mal.st_mode & S_IXOTH)){ //verificam dreptuile de acces
                pid_s= fork(); //procesul pentru verificarea fisierului
                if(pid_s<0){
                    printf("Eroare la crearea procesului pentru verificarea fisierului");
                }
                if(pid_s==0){
                    printf("Fisierul cu calea %s este potential periculos.\n", path);
                    sprintf(command, "./test.sh %s %s", d->d_name, mal_dir); //cream comanda cu care apelam script-ul
                    system(command); //apelam scipt-ul
                }
                else{
                    kill(pid_s, SIGINT); //intrerupen procesul cu semnalul SIGINT
                }
            }
        }
            }
        }
    }
    closedir(dir);  //inchidem directorul curent
}
int main(int argc, char **argv) {
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
    char f[5]="*\n";  //variabila folosita pentru a marca directorul curent in fisierul de snapshot-uri
    valid=write(fd,f, strlen(f));
    if(valid!=strlen(f))
        exit(-1);
    pid_t pid;
    int i;
    for (i = 1; i < argc-2; i++) {
        pid=fork();  //pentru fiecare director dat ca argument creem un nou proces
        if(pid<0){
            perror("Eroare la crearea procesului");
            exit(-1);
        }
        else{
            if(pid==0){  //daca am intrat in proces vom iesi din bucla, pentru ca procesul fiu sa nu creeze un alt proces
               
                break;
            }
    	else{
        	int status; //codul cu care se termina procesul
        	
        	waitpid(pid,&status,WCONTINUED); //da valoare variabilei status 
        	//se creeaza un vector de pid-uri, uterior se va folosi waitpid pentru fiecare pid, dupa crearea tuturor proceselor
            		printf("Procesul cu PID-ul %d s-a incheiat cu codul %d.\n", pid, status);  //marcam in terminal finalizarea procesului curent
            		  //asteptam pana cand toate procesele au fost realizate
        	}
        }
    }
        
    for(i=1;i<argc-2;i++){
        printf("%s",argv[i]);
        dir_parse(argv[i]);  //daca procesul a fost creat cu succes, intram in directorul dat ca argument pe pozitia i
        valid=write(fd, f, strlen(f));  //marcam in fisierul de snapshot inceputul directorului curent
        if(valid!=strlen(f))
            exit(-1);
    }
    valid=lseek(fd, 0, SEEK_SET); // resetam cursorul la inceputul fisierului pentru a transfera toate snapshot-urile in fisiere individuale
    if(valid==-1)
        exit(-1);
    i = 0;
    char line[MAX_LINE_LENGTH];  //folosita pentru a citi cate o linie din fisierul initial de snapshot
    char filename[MAX_FILENAME_LENGTH];
   /* FILE * fi=NULL;
    fi=fdopen(fd,"r");  //folosita pentru a denumi fisierul de snapshot individual
    while(fgets( line,MAX_LINE_LENGTH ,fi)!=NULL){
        printf("%s",line);
       if (line[0] == '*') {  //daca se deteteaza caracterul '*' se trece intr-un nou fisier snapshot
            snprintf(filename, sizeof(filename), "file%d.txt", i);
            fdr = open(filename, O_WRONLY | O_CREAT | O_TRUNC);  //deschidem noul fisier snapshot
            if (fdr == -1) {
                perror("FileNotOpen");
                exit(EXIT_FAILURE);
            }
            i++;
        } else {
            valid=write(fdr, line, strlen(line)); //scriem in fisierul desnapshot deschis liniile citite din fisierul initial
            if(valid!=strlen(line))
                exit(-1);
        }
    }
    */
    for(i=1;i<argc;i++) {
        if(strcmp(argv[i], "-s") == 0) {
            if(i+1<argc) {
                 strncpy(mal_dir, argv[++i], sizeof(mal_dir));
                mal_dir[sizeof(mal_dir)-1]='\0';
                break;
            } else {
                printf("Argumentul pentru directorul malitios lipsește.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    if (fdr != -1) {
        valid=close(fdr);  //inchiderea fisierului de snapshot actual
        if(valid==-1){
            perror("NoFileClosed");
            exit(-1);
        }
    }
    valid=close(fd); //inchiderea fisierului initial
    if(valid==-1){
        perror("NoFileOpen");
        exit(-1);
    }
    return 0;
}
//aici am adaugat partea cu analiza fisierelor periculoase
//mai este de lucru ma modul in care se scrie in snapshot
