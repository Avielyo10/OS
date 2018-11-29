#include "ex12.h"  

int main(int argc, char const *argv[]){
    if(argc == 2){ 
        printf("-----------------------------------------------------------\n");        
        printf("%s is starting at %s\n",argv[0],argv[1]);
        int confFd = open(argv[1], O_RDONLY,0);
        if (confFd < 0){
            perror("Error : can't open conf file");
            exit(1);
        }
        if((confFd = dup2(confFd,0)) < 0){
            perror("Error : can't redirect");
            exit(1);
        }
        //every line in the conf file is smaller than 80 characters
        char locPath[80],inPath[80],outPath[80];
        //parsing the conf file to location, input & output paths
        scanf("%s",locPath);
        scanf("%s",inPath);
        scanf("%s",outPath);
        
        printf("%s is now parsed.\n",argv[1]);
        printf("-----------------------------------------------------------\n");
        
        printf("Location path: %s\n",locPath);
        printf("Input path: %s\n",inPath);
        printf("Output path: %s\n",outPath);
        printf("-----------------------------------------------------------\n");

        if(is_dir(locPath)){
            student* students = getStudentsPathList(locPath);
            findCFiles(students);//find c file for each student,\
             if not found set comment to "NO_C_FILE"
            compile(students);//if student has c file we will compile it
            run(students, inPath);//run those compiled files, use file I/O redirection
            compare(students ,outPath);//compare the results from run method with \
            the output path file
            writeToCsv(students);//write the scores to results.csv
            free(students); //let the students go ..  
        }
        if(close(confFd) < 0){
            perror("Error : File can't close - OS will take care of that");
            exit(1);
        }
    }
    else{
        perror("Error : You should enter a path");
        exit(1);
    }
    return 0;
}

