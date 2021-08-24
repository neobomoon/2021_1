#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#define READ 0
#define WRITE 1

void handler(int sig)
{
    if(sig == SIGALRM) //알람처럼 울림
    {
        perror("timeout\n");
        exit(1);
    }
    else if(sig == SIGCHLD) //자식이 종료되면 켜지는 신호
    {
        pid_t pid;
        int exit_code;
        pid = wait(&exit_code);
        if(WIFEXITED(exit_code))
        {
            return;
        }
        else
        {
            printf("%d\n", exit_code);
            if(exit_code == 136)
            {
                printf("divide by zero. exit code is %d\n", exit_code);
                exit(1);
            }
            else
            {
                printf("abnormal child process end. exit code is %d\n", exit_code);
                exit(1);
            }
        }
    } 
    else if(sig == SIGSEGV) //segmentation fault 신호(메모리 위치 잘못 접근 시)
    {
        perror("Segmentation error.");
        exit(1);
    }
    else if(sig == SIGBUS) //하드웨어 잘못  신호
    {
        perror("Bus error.");
        exit(1);
    }
}

void findmax(double *max, double time)
{     
    if(*max < time)
    {
        *max = time;
    }
}
void findmin(double *min, double time)
{
    if(*min > time)
    {
        *min = time;
    }
}

int main(int argc, char* argv[]){
    //1. 변수선언
    char *test_dir, *solution_file, *target_file; //커맨드라인 관련
    int timeout = 0, opt; //타임아웃, 옵션 인자받기
   int exit_code_gcc1, exit_code_gcc2;
    extern char *optarg; //옵션 뒤에 오는 인자
    extern int optind; //옵션이 끝나는 index위치
   pid_t target_gcc, solution_gcc, term_gcc_pid1, term_gcc_pid2; //gcc관련
    struct itimerval t;

    //2. signal 관련
    signal(SIGALRM, handler); //알람
    signal(SIGCHLD, handler); //자식
    signal(SIGSEGV, handler); //세그먼트
    signal(SIGBUS, handler); //하드웨어 잘못
    signal(SIGSYS, handler); //잘못된 시스템호출

    //3. option 인자 받기
    while((opt = getopt(argc, argv, "i:t:")) != -1)
    {
        switch(opt)
        {
            case 'i':
                test_dir = optarg;
                break;
            case 't':
                timeout = atoi(optarg);
                if(timeout < 1 || timeout > 10)
                {
                    perror("-t should be in 1~10\n");
                    exit(1);
                }
                break;
            case '?':
                perror("input argument\n");
                exit(1);
            default:
                perror("error\n");
                exit(1);
        }
    }

    //만약 argument가 안 맞으면 에러
    if(argc - optind != 2)
    {
        perror("input solution file and target file\n");
        exit(1);
    }

    //solution과 target 파일 입력받기
    for(int i = optind; i < argc; i++)
    {
        if(i == optind)
        {
            solution_file = argv[i];
        }
        else
        {
            target_file = argv[i];
        }
    }

    //timeout측정하고 signal 보내기
    t.it_value.tv_sec = timeout;
    t.it_interval = t.it_value;
    setitimer(ITIMER_REAL, &t, NULL);


   //4. "gcc하는 프로세스"//
   target_gcc = fork();
   if(target_gcc == 0)
   {
      execl("/usr/bin/gcc", "gcc", target_file, "-o", "target", (char*) NULL);
   }
   else if(target_gcc > 0)
   {
      solution_gcc = fork();
      //solution_gcc 프로세스 생성 안 되면 에러
      if(solution_gcc < 0)
      {
         perror("solution_gcc process error\n");
         exit(1);
      }
      //solution_gcc 진행
      if(solution_gcc == 0)
      {
            execl("/usr/bin/gcc", "gcc", solution_file, "-o", "solution", (char*) NULL);
      }
        else //부모프로세스
        {
            //gcc 끝나기 기다리기 & 종료 코드 반환
           term_gcc_pid1 = wait(&exit_code_gcc1);
           term_gcc_pid2 = wait(&exit_code_gcc2);
        }
   }
    else
   {
      perror("target_gcc process error\n");
      exit(1);
   }


    //5. tester에 필요한 변수선언 해주기 및 실행
    struct dirent *file = NULL;
    DIR *dir_ptr = NULL;
    char* filename;
    int pipe_tar_input[2];
    int pipe_sol_input[2];
    pid_t tester_tar[10], tester_sol[10];
    double sum_tar = 0, sum_sol = 0, max_tar = 0, max_sol = 0, min_tar = 100, min_sol = 100;
    //디렉토리 열기
    if((dir_ptr = opendir(test_dir)) == NULL)
    {
        perror("fail to opendir() in solution");
        exit(1);
    }
    //이미 존재한 파일 있으면 삭제
    int check_tar, check_sol;
    if((check_tar = open("./target_result.txt", O_WRONLY)) >= 0)
    {
        int rm = remove("./target_result.txt");
        if(rm != 0)
        {
            perror("Error\n");
            exit(1);
        }
    }
    if((check_sol = open("./solution_result.txt", O_WRONLY)) >= 0)
    {
        int rm = remove("./solution_result.txt");
        if(rm != 0)
        {
            perror("Error\n");
            exit(1);
        }
    }
    //파일 하나씩 읽어서 result값 뽑기
    for(int i = 0; (file = readdir(dir_ptr)) != NULL; i++)
    {
        struct timespec begin_tar, end_tar, begin_sol, end_sol;
        filename = file->d_name;
        if(!strcmp(filename, "..") || !strcmp(filename, ".")){continue;}

        printf("filename: %s\n", filename);
        //pipe생성
        if(pipe(pipe_tar_input) != 0) //int pipe_tar[2], pipe_sol[2];
        {
            perror("Error\n");
            exit(1);
        }
        clock_gettime(CLOCK_MONOTONIC, &begin_tar);
        tester_tar[i] = fork();
        if(tester_tar[i] == 0)
        {
            int tar_result = open("./target_result.txt", O_WRONLY | O_APPEND |O_CREAT, 0644);
            dup2(pipe_tar_input[READ], 0); //stdin
            dup2(tar_result, 1); //stdout
            close(tar_result);

            execl("./target", "target", (char*) NULL);
        }
        else if(tester_tar[i] > 0)
        {
            //pipe생성
            if(pipe(pipe_sol_input) != 0)
            {
                perror("Error\n");
                exit(1);
            }
            clock_gettime(CLOCK_MONOTONIC, &begin_sol);
            tester_sol[i] = fork();
            if(tester_sol[i] == 0)
            {
                int sol_result = open("./solution_result.txt", O_WRONLY | O_APPEND |O_CREAT, 0644);
                dup2(pipe_sol_input[READ], 0); //stdin
                dup2(sol_result, 1); //stdout
                close(sol_result);
                execl("./solution", "solution", (char*) NULL);
            }
            else if(tester_sol[i] > 0)
            {
                //부모에서 파일 읽고 파이프로 WRITE해주기
                char *buf = NULL;
                FILE *fp;
                ssize_t s;
                size_t len = 0;
                char path[100] = "./tests/";
                strcat(path, filename);
                fp = fopen(path, "r");
                if (fp == NULL)
                {
                    perror("Error\n");
                    exit(1);
                }
                while((s = getline(&buf, &len, fp)) != -1)
                {   
                    write(pipe_tar_input[WRITE], buf, sizeof(buf));
                    write(pipe_sol_input[WRITE], buf, sizeof(buf));
                }
                
                waitpid(tester_sol[i], NULL, 0);
                clock_gettime(CLOCK_MONOTONIC, &end_tar);
                double time_tar = (end_tar.tv_sec - begin_tar.tv_sec)*(double)1000 + (end_tar.tv_nsec - begin_tar.tv_nsec)/(double)1000000;
                findmax(&max_tar, time_tar);
                findmin(&min_tar, time_tar);

                waitpid(tester_tar[i], NULL, 0);
                clock_gettime(CLOCK_MONOTONIC, &end_sol);
                double time_sol = (end_sol.tv_sec - begin_sol.tv_sec)*(double)1000 + (end_sol.tv_nsec - begin_sol.tv_nsec)/(double)1000000;
                findmax(&max_sol, time_sol);
                findmin(&min_sol, time_sol);
            
                sum_tar += time_tar;
                sum_sol += time_sol;
            }
            else
            {
                perror("Error\n");
                exit(1);
            }
        }
        else
        {
            perror("Error\n");
            exit(1); 
        }
    }
    closedir(dir_ptr);

    //6. "detecter"
    struct dirent *file_result = NULL;
    DIR *dir_result = NULL;
    char *buf_tar_result = NULL, *buf_sol_result = NULL;
    FILE *fp_tar, *fp_sol;
    ssize_t s_tar, s_sol;
    size_t len_tar = 0, len_sol = 0;

    //process 걸린 시간 출력
    printf("max of target process time: %fmilsec\n", max_tar);
    printf("min of target process time: %fmilsec\n", min_tar);
    printf("max of solution process time: %fmilsec\n", max_sol);
    printf("min of solution process time: %fmilsec\n", min_sol);
    printf("sum of target process time: %fmilsec\n", sum_tar);
    printf("sum of solution process time: %fmilsec\n", sum_sol);

    fp_tar = fopen("./target_result.txt", "r");
    fp_sol = fopen("./solution_result.txt", "r");
    if(fp_tar == NULL && fp_sol == NULL)
    {
        perror("Error\n");
        exit(1);
    }
    int idx = 1;
    while((s_sol = getline(&buf_sol_result, &len_sol, fp_sol)) != -1 )
    {
        s_tar = getline(&buf_tar_result, &len_tar, fp_tar);
        if(strcmp(buf_tar_result, buf_sol_result) != 0)
        {
            printf("%dth result is different.\n", idx);
        }
        else
        {
            printf("%dth result is same.\n", idx);
        }
        idx++;
    }
    fclose(fp_tar);
    fclose(fp_sol);
    printf("done\n");
    return 0;
}