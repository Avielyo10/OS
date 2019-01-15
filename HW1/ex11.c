#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
/*Author - Aviel Yosef*/
  
int main(int argc, char const *argv[]){
    // opening both files in read only mode 
    if (argc == 3){
        char buf1, buf2;
        int fd1 = open(argv[1],O_RDONLY);
        int fd2 = open(argv[2],O_RDONLY);
  
        if (fd1 < 0 || fd2 < 0) { 
            perror("Error : Files won't open,\
             please check the path that was inserted\n");
            exit(1); 
        } 

        int pos1, pos2; 
        while((pos1 = read(fd1,&buf1,1)) == 1 && (pos2 = read(fd2,&buf2,1)) == 1){
            if (buf1 != buf2){
                perror("Error : Files are not equal!");
                exit(1);
            }
        }

        /*
            Handle the case that both files are the same till
            one file has ended and the second one didn't.
            for example: 
                1. 'aaa' and 'aaaa'
                2. 'aaa' and 'aaac'
            both cases are been handled  
        */
        if((pos1 = read(fd1,&buf1,1)) == 1 || (pos2 = read(fd2,&buf2,1)) == 1){
            if(pos1 != pos2){
                perror("Error : Files are not equal!");
                exit(1);
            }
        } 
        
        // closing both file 
        if (close(fd1) < 0 || close(fd2) < 0) { 
            perror("Error : Files didn't close - Your OS will handle it");
            exit(1); 
        }
        printf("Files Are Equal!\n");
        return 2;  
    }
    else{
        perror("Error : You should enter 2 paths, not more or less");
        return 1;
    }
    //You won't get by here ..
    return 1;
}

 
