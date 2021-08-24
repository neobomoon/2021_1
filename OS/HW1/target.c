#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <fcntl.h>

int main(){//How many integers are there 0 ~ your input
    int num;
    int count = 1;
    scanf("%d", &num);
    if(num >= 0){
        while(num != 0){
            count++;
            num--;
        }
    }
    else{
        while(num != 0){
            count--;
            num++;
        }
    }
    printf("there are %d integers\n", count);

    // int num;
    // scanf("%d", &num);
    // printf("answer is %d\n", 100/num);
    
    return 0;
}