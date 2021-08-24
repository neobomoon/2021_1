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

int read_bytes(int fd, void *a, int len);
int write_bytes (int fd, void * a, int len);
void removepipe(char * pipe_name);
void summary(int line_count, int file_count, int dir_count);
void worker(int *line_count, int *file_count, int *dir_count, char **keyword, char *dir){


//해야할 것 : 절대경로
int main(int argc, char* argv[]){
    char *dir;
    int term = 0;
    int num = 2, case_opt = 0, abs = 0; //옵션 관련 인자
    int opt; //옵션 인자받기
    int *file_count, *dir_count, *line_count;
    extern char *optarg; //옵션 뒤에 오는 인자
    extern int optind; //옵션이 끝나는 index위치
    struct itimerval t;
    char keyword[128];
    char task[128];

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
    //만약 keyword가 없으면 종료
    if(argc - optind == 1)
    {
        printf("There is no keyword.\n");
        return 0;
    }
    //keyword 입력받기
    optind++; //위에 두개 if문 통과. 즉, dir과 keyword 다 존재. index올려주기
    sprintf(keyword, "%s", argv[optind]);
    for(int i = 1; i < argc - optind; i++){
        sprintf(keyword, "%s %s", keyword, argv[i + optind]);
    }
    //checking 
    printf("check precondition.\n");
    printf("check keywords :%s\n", keyword);
    printf("directory is : %s (abs = %d)\n", dir, abs);
    printf("check process number %d\n", num);

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
            printf("Success to %dth fork\n", i);
            execl("./worker", "worker", keyword, (char*) NULL);
        }
        else if(child[i] < 0){
            perror("Error to fork.\n");
            exit(1);
        }
    }
    int pipe1 = open("/home/s21600635/OS/HW3/pipe1", O_WRONLY | O_SYNC);
    int pipe2 = open("/home/s21600635/OS/HW3/pipe2", O_RDONLY | O_SYNC);


    ///////////////////////여기서 부터 다시 짜기
    //부모 프로세스
    size_t len1 = 0;
    //처음 task 주기
    //pipe1파일락하기
    flock(pipe1, LOCK_EX);
    //task의 길이 구하기
    for(int i = 0; i < strlen(dir); i++){
        len1++;
    }
    printf("\n");
    printf("dir len check : %zd\n", len1);
    //pipe1에 첫번째 task주기
    if(write_bytes(pipe1, &len1, sizeof(len1)) != sizeof(len1)){
        perror("Wrong lenth of pipe2\n");
        exit(1);
    }
    if(write_bytes(pipe1, dir, len1) != len1){
        perror("Wrong lenth of pipe2\n");
        exit(1);
    }
    //pipe1파일락 풀기
    flock(pipe1, LOCK_UN);

    while(1){
        size_t len2 = 0, bs2 = 0;
        len1 = 0;
        //pipe2파일락 하기
        flock(pipe2, LOCK_EX);
        //pipe2를 읽기
        if(read_bytes(pipe2, &len2, sizeof(len2)) != sizeof(len2)){
            flock(pipe2, LOCK_UN);
            perror("Error on pipe2 when read from manager.");
            exit(1);
        }
        //pipe2파일락 풀기
        bs2 = read_bytes(pipe2, task, len2);
        flock(pipe2, LOCK_UN);
        //if(pipe2 내용 없다?){
        if(len2 == 0){
            //kill all child process
            for(int i = 0; i < num; i++){
                kill(child[i], SIGKILL);
            }
            break;
        }
        else{//task 있을 떄
            //pipe1파일락하기
            flock(pipe1, LOCK_EX);
            //task의 길이 구하기
            int i = 0;
            for(i; task[i] != 0; i++){ len1++; }
            //pipe1에 task주기
            if(write_bytes(pipe1, &len1, sizeof(len1)) != sizeof(len1)){
                perror("Wrong lenth of pipe2\n");
                exit(1);
            }
            if(write_bytes(pipe1, task, len1) != len1){
                perror("Wrong lenth of pipe2\n");
                exit(1);
            }
            //pipe1파일락 풀기
            flock(pipe1, LOCK_UN);
        }
    }
    //혹시 모르니까 기다리기
    for(int i = 0; i < num; i++)
        wait(NULL);
    //print summary
    summary(*line_count, *file_count, *dir_count);
    //타이머 재기

    //닫아주기
    close(pipe1);
    close(pipe2);

    //이미 존재한 파일 있으면 삭제
    removepipe("./pipe1");
    removepipe("./pipe2");
}


void worker(int *line_count, int *file_count, int *dir_count, char **keyword, char *dir){
    sleep(2);
    int pipe1 = open("/home/s21600635/OS/HW3/pipe1", O_RDONLY | O_SYNC);
    int pipe2 = open("/home/s21600635/OS/HW3/pipe2", O_WRONLY | O_SYNC);
    int unpipe[2];
    int file_count = 0;
    int dir_count = 0;
    int line_count = 0;
    int count = 0;
    char *temp = argv[1];
    char keyword[20][100];
    char *ptr = strtok(temp, " ");
    for(count = 0; ptr != NULL; count++){
        strcpy(keyword[count], ptr);
        ptr = strtok(NULL, "\n");
    }
    //파이프생성
    if(pipe(unpipe) != 0){
            perror("Error\n");
            exit(1);
    }
    for(int i = 0; i < count; i ++){
        printf("in worker argv[%d] : %s\n", i, keyword[i]);
    }

    while(1){
        char task[128];
        char update[128];
        char check[128];
        size_t len1, len2, bs1;
        DIR* dp = NULL;
        struct dirent *file = NULL;
        char* filename;
        pid_t temp;
        //pipe1 읽기
        flock(pipe1, LOCK_EX);
        if(read_bytes(pipe1, &len1, sizeof(len1)) != sizeof(len1)){
            flock(pipe1, LOCK_UN);
            break;
        }
        bs1 = read_bytes(pipe1, task, len1);
        flock(pipe1, LOCK_UN);
        char c = task[len1 - 1];
        if(task[len1 - 1] !=  '/'){
            strcat(task, "/");
        }
        printf("dir check : %s", task);
        //디렉토리 열기
        if((dp = opendir(task)) == NULL){ perror("Error to open dir\n"); exit(1);}
        //for로 파일 열기
        for(int i = 0; (file = readdir(dp)) != NULL; i++){
            //filename 정하기
            filename = file->d_name;
            //. 이랑 .. 무시하기
            if(!strcmp(filename, "..") || !strcmp(filename, ".")){continue;}
            printf("filename: %s\n", filename);
            //fork()하기
            temp = fork();
            //if child이면
            if(temp == 0){
                close(unpipe[0]);
                //dup2로 check파일 stdout으로 정하기
                dup2(unpipe[1], 1);
                //execl("file", filename, (char*)NULL);
                execl("/usr/bin/file", "file", filename, (char*)NULL);
            }
            //else if 부모이면
            else if(temp > 0){
                //wait(NULL);
                close(unpipe[1]);
                wait(NULL);
                //unpipe 읽기
                ssize_t s;
                while((s = read(unpipe[0], check, 128)) > 0 ){
                    check[s + 1] = 0;
                }
                //if(strstr(읽은 것, "text"))
                if(strstr(check, "text")){
                    int count = 0; //몇번째 줄에있는지 
                    int filecheck = 0; //이 파일에 키워드가 포함 돼 있는지
                    //파일 읽기
                    char path[128];
                    char str[128];
                    sprintf(path, "%s%s", task, filename);
                    FILE* fd = fopen(path, "r");
                    //파일 열렸는지 에러체크
                    if(fd == NULL){ perror("Error\n"); exit(1); }
                    while(1){
                        int for_check = 1;
                        char *p = fgets(str, 128, fd);
                        //파일을 다 읽으면 
                        if(p == NULL){ break; }
                        for(int i = 1; i < argc; i++){
                            if(!strstr(str, argv[i]))
                                for_check = for_check * 0;
                            else
                                filecheck = 1;
                        }
                        if(for_check){
                            printf("%s:%d:%s\n", path, count, p);
                            line_count++;
                        }
                        count++;
                    }
                    //해당 파일에 한 줄이라도 있으면 file_count 증가
                    if(filecheck == 1){ file_count++; }
                }
                //else if(strstr(읽은 것, "directory"))
                else if(strstr(check, "directory")){
                    //fd_2로 주기
                    dir_count++;
                    char path[128];
                    sprintf(path, "%s%s%s", task, filename, "/");
                    flock(pipe2, LOCK_EX);
                    int i = 0;
                    for(i; path[i] != 0; i++){
                        len2++;
                    }
                    if(write_bytes(pipe2, &len2, sizeof(len2)) != sizeof(len2))
                        break;
                    if(write_bytes(pipe2, path, len2) != len2)
                        break;
                    flock(pipe2, LOCK_UN);
                }
                else
                    continue;
            }
        }
    }
    close(pipe1);
    close(pipe2);
    return 0;
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
}
void summary(int line_count, int file_count, int dir_count){
    printf("total number of found lines : %d\n", line_count);
    printf("total number of explored files : %d\n", file_count);
    printf("total number of explored directories : %d\n", dir_count);
}