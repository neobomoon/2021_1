<Assignments_Homework2>

- name : bomoon jung
- ID : 21600635

- how to command line()
    1. first : $make
    2. second : $./pctest -i ./tests/ -t 6 solution.c target.c
    3. to occurs "zero divide error". Change value of test7.txt -> 0
    and change from comments to code and replace existing code with comments.
    4. then. do this $./pctest -i ./tests/ -t 6 solution.c target.c
    5. If you want to see the -t option followed by a value other than 1-10 and the program stops. do this $./pctest -i ./tests/ -t 11 solution.c target.c
    6. To see the timeover, you can see it by changing one of the test.txt files to a very large value. But from some point, the timeout suddenly fails. It's definitely a problem with my code, but it's worked out a while ago.... :(

- video memo link : https://drive.google.com/file/d/1Knr1jn5Mwlr_6qlRzthnVa6hbBEMRrPM/view?usp=sharing

- explain about structure
    1. Then, I will briefly explain the structure of my program. When the parent process does gcc, it runs different child processes at the same time about the solution.c and target.c files, and that child processes is a multi-process, and the parent process waits for each child process to finish before moving on to the next.
    2. if the gcc process is finished, then run tester.Like the gcc process, each child process is created for target and solution execution.For one input text file, I made it possible to run it in pairs.In addition, the input was delivered to the two created child processes through the pipe() function, respectively, and the standard ouput was changed through the dup2() function so that the execution results of the target and solution were saved in the target_result.txt and solution.txt files, respectively.
    3. After that, the failure detector, which is the last step, in this step i'm not create the child process, because I think it doesn't need a child process So, I made the dectector to excute directly in the parent process. To explain this process, each result.txt file created in the process of the tester was read line by line and the results were compared.
    4. entirely, in order to the parent process to run safely, most of the codes are implemented in the child process, so if an error occurs and the child process is stopped, a signal is sent to the parent process to print an error message.

- Limitaion of my code
    1. The limitation of my code is that I created many child processes at once and tried to run many multiprocesses, but there were still zombie processes, so I made them run in pairs.
    2. Also, if an error occurs in the child process while executing the program, it writes in the result file that the error has occurred. However, there is a problem that the error message is written twice in result file. I don't know what the reason is in this process.
    3. So, if you edit a file other than test7.txt, the test results will be pushed one by one and so final results are all different.
    4. And also, if lines 272 and 278 are interchanged, no value is entered in the target_result file, even if an error occurs in target.c.
    5. timeout does not work at randomly. but I don't know what the reason is.