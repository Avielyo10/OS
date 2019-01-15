#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
/*Author - Aviel Yosef*/
typedef struct student{
    char path[100];
    char name[20];
    char pathToCFile[100];
    char comment[20];
    int grade;
}student;

int is_dir(const char *path);
const char *get_filename_ext(const char *filename);
student* getStudentsPathList(char* pathToDir);
void findCFiles(student* students);
char* lookForCFile(char* pathToDir);
void compile(student* students);
void run(student* students,char* inputFile);
void compare(student* students, char* outputFile);
void writeToCsv(student* students);

int numOfStudents = 0; 


void writeToCsv(student* students){
    int csvFd = open("results.csv",
    O_APPEND | O_WRONLY | O_CREAT,
     S_IRUSR | S_IRGRP | S_IROTH |
      S_IWUSR | S_IWGRP | S_IWOTH);
    if(csvFd < 0){
        perror("Error : can't open results.csv");
        exit(1);
    }
    for(size_t i = 0; i < numOfStudents; i++){
        char line[50]="";
        char snum[5];
        sprintf(snum,"%d",(students + i)->grade);
        //create the line to push 
        strcat(line,(students + i)->name);
        strcat(line,",");
        strcat(line,snum);
        strcat(line,",");
        strcat(line,(students + i)->comment);
        strcat(line,"\n");

        //push the line
        if(!write(csvFd,line,strlen(line))){
            perror("Error : can't write to results.csv");
            exit(1);
        }
    }
    if(close(csvFd) < 0){
        perror("Error : File can't close - OS will take care of that");
        exit(1);
    }
}

void compare(student* students, char* outputFile){
    for(size_t i = 0; i < numOfStudents; i++){
        //if student has no comment yet
        if(!strcmp((students + i)->comment,"")){
            pid_t child_pid = fork();
            int child_status;
            
            char studentOutput[25] = "";
            strcpy(studentOutput,(students + i)->name);
            strcat(studentOutput,".txt");//studentOutput = <studentName>.txt

            if(!child_pid){//child
                char *args[]={"./bin/comp.out",studentOutput,outputFile,NULL};
                execvp(args[0],args);
            }
            else{
                waitpid(child_pid,&child_status,0);
                if (WIFEXITED(child_status)) {
                    if(WEXITSTATUS(child_status) == 2){
                        strcpy((students + i)->comment,"GREAT_JOB");
                        (students + i)->grade = 100;
                    }
                    else if(WEXITSTATUS(child_status) == 1){
                        strcpy((students + i)->comment,"BAD_OUTPUT");
                    }
                    else{
                        perror("Error: exit status is not 1 or 2");
                        exit(1);
                    }
                    char compProg[25] = "";
                    strcat(compProg,(students + i)->name);
                    strcat(compProg,".out");//compProg = <studentName>.out

                    if(unlink(studentOutput) < 0){
                        perror("Error : File can't be deleted");
                        exit(1);
                    }
                    if(unlink(compProg) < 0){
                        perror("Error : File can't be deleted");
                        exit(1);
                    }
                } 
            }
        }
    }
}

void run(student* students, char* inputFile){
    for(size_t i = 0; i < numOfStudents; i++){
        //if student has no comment yet
        if(!strcmp((students + i)->comment,"")){
            pid_t child_pid = fork();
            int child_status;
            if(!child_pid){//child
                char cmd[25] = "./";
                strcat(cmd,(students + i)->name);
                strcat(cmd,".out");//cmd = ./<studentName>.out --> run it.

                char filename[22] = "";
                strcpy(filename,(students + i)->name);
                strcat(filename,".txt");//filename = <studentName>.txt  
                /*<---I/O redirection--->*/
                int in = open(inputFile,O_RDONLY,0);
                int out = open(filename,
                    O_WRONLY | O_CREAT,
                    S_IRUSR | S_IRGRP | S_IROTH |
                    S_IWUSR | S_IWGRP | S_IWOTH);
      
                if (in < 0){
                    perror("Error : can't open input file");
                    exit(1);
                }
                if (out < 0){
                    perror("Error : can't open output file");
                    exit(1);
                }

                if ((dup2(in,0)) < 0){
                    perror("Error : can't redirect");
                    exit(1);
                }
                if ((dup2(out,1)) < 0){
                    perror("Error : can't redirect");
                    exit(1);
                }

                if(close(in) < 0){
                    perror("Error : File can't close - OS will take care of that");
                    exit(1);
                }
                if(close(out) < 0){
                    perror("Error : File can't close - OS will take care of that");
                    exit(1);
                }

                char *args[]={cmd,NULL};
                execvp(args[0],args);
            }
            else{    
                waitpid(child_pid,&child_status,0);
            }
        }
    }
}

void compile(student* students){
    for(size_t i = 0; i < numOfStudents; i++){
        //if student has c file
        if(strcmp((students + i)->pathToCFile,"NO_C_FILE")){
            pid_t child_pid = fork();
            int child_status;
            if(!child_pid){//child

                char name[25] = "";
                strcpy(name,(students + i)->name);
                strcat(name,".out");

                char *args[]={"gcc",(students + i)->pathToCFile,"-o",name,NULL};
                execvp(args[0],args);
            }
            else{
                waitpid(child_pid,&child_status,0);
                if (WIFEXITED(child_status)&& WEXITSTATUS(child_status)) {
                    strcpy((students + i)->comment,"COMPILATION_ERROR");
                } 
            }        
        }
    }
}

char* lookForCFile(char* pathToDir){//using DFS
    if(is_dir(pathToDir)){
        DIR *dir;
        struct dirent *ent;
        strcat(pathToDir,"/");
        int i = 0;
        student* students = (student*)malloc(sizeof(student));
        if ((dir = opendir (pathToDir)) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                //ignore . && .. directories
                if(strcmp(ent->d_name,".") && strcmp(ent->d_name,"..")){
                    char* path = (char*)malloc(100);
                    strcpy(path,pathToDir);
                    strcat(path,ent->d_name);
                    if(is_dir(path)){ 
                        lookForCFile(path);
                    }
                    else{
                        if(!strcmp(get_filename_ext(path),"c")){
                            return path;
                        }
                    } 
                    free(path);
                }
            }
            closedir (dir);
        } 
            else {
            // could not open directory
            perror ("could not open directory");
            exit(1);
        }
    }
    return "NO_C_FILE";
}

void findCFiles(student* students){
     for(size_t i = 0; i < numOfStudents; i++){
        //find c file for each student, if not found set comment to "NO_C_FILE" 
        strcpy((students + i)->pathToCFile,lookForCFile((students + i)->path));
        if(!strcmp((students + i)->pathToCFile,"NO_C_FILE")){
            strcpy((students + i)->comment,"NO_C_FILE");
        } 
    }
}

student* getStudentsPathList(char* pathToDir){
    DIR *dir;
    struct dirent *ent;
    strcat(pathToDir,"/");
    int i = 0;
    student* students = (student*)malloc(sizeof(student));
    if ((dir = opendir (pathToDir)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            //ignore . && .. directories
            if(strcmp(ent->d_name,".") && strcmp(ent->d_name,"..")){
                char* path = (char*)malloc(100);
                strcpy(path,pathToDir);
                strcat(path,ent->d_name);
                if(is_dir(path)){ //if this path is a directory it's a student
                    strcpy((students + i)->name, ent->d_name);
                    strcpy((students + i)->path, path);
                    i++;
                    students = (student*)realloc(students,((i+1)*sizeof(student)));
                }
                free(path);
            }
        }
        closedir (dir);
    } 
    else {
        // could not open directory 
        perror ("could not open directory");
        exit(1);
    }
    numOfStudents = i;
    return students;
}

const char* get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int is_dir(const char *path){
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

