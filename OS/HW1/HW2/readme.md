<Assignments_Homework3>

- name : bomoon jung
- ID : 21600635

- how to command line()
    1. first : $make
    2. second : $./pfind -p 3 /home/s21600635/OS/HW3/dir1 bomoon
                $./pfind -p 3 /home/s21600635/OS/HW3/bible God
    3. When the program is stopped, press ctrl+c, make one more time and run it again, and it will work!

- video memo link : https://youtu.be/tmKSzrDugBo

- explain about structure
    1. In this task, I used two named pipes and one unamed pipe. The first named pipe was used by the manager to give tasks to multiple workers, and the second named pipe was used by the worker to give a new task to the manager and the result of the task from the worker.
    And the unnamed pipe was used as standard output when executing the “file” command. And also it was used to determine the file format after by reading the “file” command result. Through that, it made it possible to find the subdirectory and read the text file.
    The most difficult part of the assignment was that the manager and several workers communicated without block each other. Because It was my first time encountering this problem, so I took a lot of time and effort to find the most efficient way. That is to use the SIGSTOP and SIGCONT signals.

- algorithms
    1. First, the manager creates two named pipes for communication between the manager and the worker. 
    2. Then, it creates as many workers as the number of input from the user. 
    3. At this time, the worker uses the SIGSTOP signal to wait for input from the manager.
    4then, go back to the manager and put the input directory in the queue.
    5. And until there are no idle workers, the queue writse a task to pipe1.
    6. Then, it sends a SIGCONT signal to reactivate the stopped worker process.
    7. At this time, through getpid() and SIGSTOP signal, the manager itself repeatedly stops as many as the number of workers that have a task. The reason is that, to prevent the block states.
    8. And Returning to the worker, the worker reads pipe1 and is assigned a task. And it reads all the files in the task and finds the keyword.
    9. In the process, if it finds a directory, then sends “d” and directory path to pipe2
    if it finds a file, it sends “f”
    if it finds a keyword, it sends “엘”
    If there is no file to read, it sends ‘n'.
    10. And then it sends a SIGCONT signal, so that the manager can move again. 
    11. The manager that receives the SIGCONT signal as many as the number of workers to make working again, after that read pipe2 and receive new information and tasks, and update the states.
    12. And finally, if the queue size is 0, it means that there are no more tasks, so the result is summarized and the program is terminated.

- Limitaion of my code
    1. I have a little problem with using pointers.
    2. Sometimes the program freezes when you type more than one word.
    3. There is a problem with giving interrupted.
    4. I couldn't measure the time.