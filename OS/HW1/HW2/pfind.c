#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/file.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "que.h"

int read_bytes(int fd, void *a, int len);
int write_bytes (int fd, void * a, int len);
void removepipe(char * pipe_name);
void summary(int line_count, int file_count, int dir_count);
void worker(int case_opt, char **keyword);
int is_INT = 1;
int child_num = 0;
void handler(int sig){
    if(sig == SIGINT){
        for(int i = 0; i < child_num; i++){
            printf("cotinue\n");
            kill(getpid(), SIGCONT);
        }
        is_INT = 0;
    }
}

//해야할 것 : 절대경로
int main(int argc, char* argv[]){
    signal(SIGINT, handler);

    char *dir;
    int term = 0;
    int num = 2, case_opt = 0, abs = 0; //옵션 관련 인자
    int opt; //옵션 인자받기
    int file_count = 0, dir_count = 0, line_count = 0;
    extern char *optarg; //옵션 뒤에 오는 인자
    extern int optind; //옵션이 끝나는 index위치
    struct itimerval t;
    char **keyword;


    //option 인자 받기
    while((opt = getopt(argc, argv, "p:ac")) != -1)
    {
        switch(opt)
        {
            case 'p': // child process 갯수 정하기
                num = atoi(optarg);
                if(num < 1 || num >7){
                    perror("the number of worker process greater than 0 and less than 8.\n");
                    exit(1);
                }
                child_num - num;
                break;
            case 'c': // 대소문자 구분
                case_opt = 1;
                break;
            case 'a':
                //절대경로 or 상대경로 default는 상대경로getcwd(char *buf, size_t size)사용
                abs = 1;
                break;
            case '?':
                perror("Unknow argument.\n");
                exit(1);
        }
    }
    //dir 입력했는지 확인
    if((dir = argv[optind]) == NULL)
    {
        printf("There is no directory path\n");
        return 0;
    }
    //abs에 따라서 절대경로 설정하기
    if(abs == 1){
        getcwd(dir, 128);
    }
    else{
        //상대경로 해주기
    }
    //만약 keyword가 없으면 종료
    if(argc - optind == 1)
    {
        printf("There is no keyword.\n");
        return 0;
    }
    //keyword 입력받기
    optind++; //위에 두개 if문 통과. 즉, dir과 keyword 다 존재. index올려주기
    keyword = (char**) malloc(sizeof(char*) * (argc - optind));
    for(int i = 0; i < argc - optind; i++){
        keyword[i] = (char*) malloc(sizeof(char) * strlen(argv[optind + i]));
    }
    for(int i = 0; i < argc - optind; i++){
        keyword[i] = argv[optind + i];
        printf("keyword check(m): %s\n", keyword[i]);
    }

    //checking 
    printf("check precondition.\n");
    printf("directory is : %s (abs = %d)\n", dir, abs);
    printf("check process number %d\n", num);
    printf("---------------------------\n");
    //이미 존재한 네임드 파이프 있으면 삭제
    removepipe("./pipe1");
    removepipe("./pipe2");
    /////////////전처리 끝
    //fifo만들기
    if((mkfifo("pipe1", 0666)) || (mkfifo("pipe2", 0666))){
        if(errno != EEXIST){
            perror("Fail to open fifo.\n");
            exit(1);
        }
    }
    //fork로 worker num개 만들기
    pid_t child[num];
    for(int i = 0; i < num; i++){
        child[i] = fork();
        if(child[i] == 0){
            worker(case_opt, keyword);
        }
    }///////////////요기까지
    int pipe1 = open("/home/s21600635/OS/HW3/pipe1", O_WRONLY | O_SYNC);
    int pipe2 = open("/home/s21600635/OS/HW3/pipe2", O_RDONLY | O_SYNC);
    ///////////////////////여기서 부터 다시 짜기
    ///////////////////////
    /////////////////////// 
    ///////////////////////

    //부모 프로세스
    Queue queue;
    InitQueue(&queue);
    Enqueue(&queue, dir);
    int available_task;//1임
    char new_task[20][128];
    int ind = 0;
    while(is_INT){
        size_t len1 = 0, len2 = 0, bs2 = 0;
        //worker의 갯수 정해주기
        available_task = queue.count;
        if(available_task > num)
            available_task = num;

        //----------pipe1에 쓰기 (일 주기)
        flock(pipe1, LOCK_EX);
        for(int i = 0; i < available_task; i++){
            char task[128];
            len1 = 0;
            sprintf(task, "%s", Dequeue(&queue));
            size_t a = strlen(task);
            task[a] = 0;

            for(int j = 0; j < strlen(task); j++){ len1++; }
            //pipe1에 task주기
            if(write_bytes(pipe1, &len1, sizeof(len1)) != sizeof(len1)){
                perror("Wrong lenth of pipe2\n");
                exit(1);
            }
            if(write_bytes(pipe1, task, len1) != len1){
                perror("Wrong lenth of pipe2\n");
                exit(1);
            }
        }
        //pipe1파일락 풀기
        flock(pipe1, LOCK_UN);
        for(int i = 0; i < available_task; i++)
            kill(child[i], SIGCONT);
        //-----------------------

        //어떻게 잠시 멈출 것인가. worker에서 pipe2를 잠근다? -> 와 그냥 SIGSTOP 하면 끝.
        for(int i = 0; i < available_task; i++)
            kill(getpid(), SIGSTOP);

        //------------manager에서 update해주기
        flock(pipe2, LOCK_EX);
        //pipe2를 읽기
        for(int i = 0; i < available_task; i++){
            while(1){
                char task_temp[128];
                size_t len_temp = 0, bs_temp = 0;
                if(read_bytes(pipe2, &len_temp, sizeof(len_temp)) != sizeof(len_temp)){
                    perror("Error on pipe2 when read from manager.");
                    exit(1);
                }
                bs_temp = read_bytes(pipe2, task_temp, len_temp);
                task_temp[bs_temp] = 0;
                if(task_temp[0] == 'f'){
                    file_count++;
                }
                else if(task_temp[0] == 'l'){
                    line_count++;
                }
                else if(task_temp[0] == 'd'){
                    len_temp = 0;
                    bs_temp = 0;
                    if(read_bytes(pipe2, &len_temp, sizeof(len_temp)) != sizeof(len_temp)){
                        perror("Error on pipe2 when read from manager.");
                        exit(1);
                    }
                    bs_temp = read_bytes(pipe2, new_task[ind], len_temp);
                    new_task[ind][bs_temp] = 0;
                    dir_count++;
                    Enqueue(&queue, new_task[ind]);
                    ind++;
                }
                else if(task_temp[0] == 'n'){
                    break;
                }
            }
        }
        available_task = queue.count;
        //pipe2파일락 풀기
        flock(pipe2, LOCK_UN);
        //-----------------------
        if(IsEmpty(&queue))
            break;
        printf("\n\n");
    }///여기가 제일 큰 무한루프 끝

    //다 죽이기
    for(int i = 0; i < num; i++){
        kill(child[i], SIGKILL);
        printf("killed\n");
        wait(NULL);
    }
    summary(line_count, file_count, dir_count);
    //타이머 재기

    //닫아주기
    close(pipe1);
    close(pipe2);

    //이미 존재한 파일 있으면 삭제
    removepipe("./pipe1");
    removepipe("./pipe2");
    for(int i = 0; i < argc - optind; i++){
        free(keyword[i]);
    }
    free(keyword);
    return 0;
}


void worker(int case_opt, char **keyword){
    int pipe1 = open("/home/s21600635/OS/HW3/pipe1", O_RDONLY | O_SYNC);
    int pipe2 = open("/home/s21600635/OS/HW3/pipe2", O_WRONLY | O_SYNC);
    int unpipe[2];
    int count = 0;
    if((mkfifo("pipe1", 0666)) || (mkfifo("pipe2", 0666))){
        if(errno != EEXIST){
            perror("Fail to open fifo.\n");
            exit(1);
        }
    }
    //파이프생성
    if(pipe(unpipe) != 0){
            perror("Error\n");
            exit(1);
    }
 
    int checking = 0;
    while(1){
        kill(getpid(), SIGSTOP);
        checking++;
        char task[128];
        char update[128];
        size_t len1 = 0, len2 = 0, bs1 = 0;
        DIR* dp = NULL;
        struct dirent *file = NULL;
        char* filename;
        pid_t temp;
        //-----------------pipe1 읽기
        flock(pipe1, LOCK_EX);
        if(read_bytes(pipe1, &len1, sizeof(len1)) != sizeof(len1)){
            flock(pipe1, LOCK_UN);
            break;
        }
        bs1 = read_bytes(pipe1, task, len1);
        task[bs1] = 0;
        flock(pipe1, LOCK_UN);
        //--------------------

        char c = task[len1 - 1];
        if(c !=  '/'){
            strcat(task, "/");
            len1++;
        }

        //===========================================update
        //디렉토리 열기
        flock(pipe2, LOCK_EX);
        if((dp = opendir(task)) == NULL){ perror("fail to open dir\n"); exit(1); }
        for(int i = 0; (file = readdir(dp)) != NULL; i++){
            //filename 정하기
            filename = file->d_name;
            //. 이랑 .. 무시하기
            if(!strcmp(filename, "..") || !strcmp(filename, ".")){continue;}
            //printf("filename: %s. %d\n", filename, getpid());
            //fork()하기
            temp = fork();
            if(temp == 0){
                char path[128];
                sprintf(path, "%s%s", task, filename);
                //printf("sdfsadfaschrck : %s\n", path);
                dup2(unpipe[1], 1);
                execl("/usr/bin/file", "file", path, (char*)NULL);
            }
            //else if 부모이면
            else if(temp > 0){
                //unpipe 읽기
                wait(NULL);
                //sleep(1);
                ssize_t s;
                char check[256];
                //unpipe 읽기
                s = read(unpipe[0], check, 256);
                check[s] = 0;
                //-------------------text file일 때
                if(strstr(check, "text")){
                    int count = 1; //몇번째 줄에있는지 
                    int filecheck = 0; //이 파일에 키워드가 포함 돼 있는지
                    //파일 읽기
                    char path[128];
                    char str[128];
                    sprintf(path, "%s%s", task, filename);
                    FILE* fd = fopen(path, "r");
                    //파일 열렸는지 에러체크
                    if(fd == NULL){ perror("Error\n"); exit(1); }
                    while(!feof(fd)){
                        int for_check = 1;
                        char *p = fgets(str, 128, fd);
                        for(int j = 0; keyword[j] != NULL; j++){
                            if(!strstr(str, keyword[j]))
                                for_check = for_check * 0;
                            else
                                filecheck = 1;
                        }
                        if(for_check){
                            printf("%s:%d:%s\n", path, count, p);
                            //줄 찾으면 pipe2에 l 보내기
                            size_t temp_len = 1;
                            //pipe1에 task주기
                            if(write_bytes(pipe2, &temp_len, sizeof(temp_len)) != sizeof(temp_len)){
                                flock(pipe2, LOCK_UN);
                                perror("Wrong lenth of pipe2\n");
                                exit(1);
                            }
                            char option[128] = "l\0";
                            if(write_bytes(pipe2, option, temp_len) != temp_len){
                                flock(pipe2, LOCK_UN);
                                perror("Wrong lenth of pipe2\n");
                                exit(1);
                            }
                        }
                        count++;
                    }
                    //해당 파일에 한 줄이라도 있으면 pipe2에 f보냄
                    if(filecheck == 1){
                        //pipe2에 f보내기
                        size_t temp_len = 1;
                        //pipe1에 task주기
                        if(write_bytes(pipe2, &temp_len, sizeof(temp_len)) != sizeof(temp_len)){
                            flock(pipe2, LOCK_UN);
                            perror("Wrong lenth of pipe2\n");
                            exit(1);
                        }
                        char option[128] = "f";
                        if(write_bytes(pipe2, option, temp_len) != temp_len){
                            flock(pipe2, LOCK_UN);
                            perror("Wrong lenth of pipe2\n");
                            exit(1);
                        }
                        //pipe2파일락 풀기
                    }
                }//---------------------------
                //----------------------directory file일 때
                else if(strstr(check, "directory")){
                    //pipe2에 d 보내기
                    size_t temp_len = 1;
                    //pipe2에 d 보내기
                    if(write_bytes(pipe2, &temp_len, sizeof(temp_len)) != sizeof(temp_len)){
                        flock(pipe2, LOCK_UN);
                        perror("Wrong lenth of pipe2\n");
                        exit(1);
                    }
                    char option[128] = "d";
                    if(write_bytes(pipe2, option, temp_len) != temp_len){
                        flock(pipe2, LOCK_UN);
                        perror("Wrong lenth of pipe2\n");
                        exit(1);
                    }

                    // task update
                    char new_task[128];
                    sprintf(new_task, "%s%s%s", task, filename, "/");

                    len2 = 0;
                    for(int j = 0; new_task[j] != 0; j++){
                        len2++;
                    }
                    if(write_bytes(pipe2, &len2, sizeof(len2)) != sizeof(len2)){
                        flock(pipe2, LOCK_UN);
                        break;
                    }
                    if(write_bytes(pipe2, new_task, len2) != len2){
                        flock(pipe2, LOCK_UN);
                        break;
                    }
                }
                else
                    continue;
            }
        }
        //task에서 directory가 없으면 #보내기
        size_t temp_len = 1;
        //pipe2에 # 보내기
        if(write_bytes(pipe2, &temp_len, sizeof(temp_len)) != sizeof(temp_len)){
            flock(pipe2, LOCK_UN);
            perror("Wrong lenth of pipe2\n");
            exit(1);
        }
        char final[128] = "n";
        if(write_bytes(pipe2, final, temp_len) != temp_len){
            flock(pipe2, LOCK_UN);
            perror("Wrong lenth of pipe2\n");
            exit(1);
        }
        flock(pipe2, LOCK_UN);
        kill(getppid(), SIGCONT);
        //===============================update끝 다시 돌아가서 stop
    }
    close(pipe1);
    close(pipe2);
    return;
}

int read_bytes(int fd, void *a, int len){
    char *s = (char *) a;
    int i;
    for(i = 0; i < len; ){
        int b;
        b = read(fd, s + i, len - i);
        if(b == 0)
            break;
        i += b;
    }
    return i;
}
int write_bytes (int fd, void * a, int len){
	char * s = (char *) a ;

	int i = 0 ; 
	while (i < len) {
		int b ;
		b = write(fd, s + i, len - i) ;
		if (b == 0)
			break ;
		i += b ;
	}
	return i ;	
}

void removepipe(char *pipe_name){
    if(access(pipe_name, F_OK) == 0){
        unlink(pipe_name);
    }
    return;
}
void summary(int line_count, int file_count, int dir_count){
    printf("\n\n\ntotal number of found lines : %d\n", line_count);
    printf("total number of explored files : %d\n", file_count);
    printf("total number of explored directories : %d\n\n\n", dir_count);
    return;
}