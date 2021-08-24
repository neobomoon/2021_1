#Assignments_Homework5

###name : bomoon Jung
###ID : 21600635

###1. Video link
https://youtu.be/XRgSehex7Kw

###2. How to command line
+ first : $make
+ second : $./tfind -t {num} {path} {keywords, keywords,...}
ex) $./tfind -t 4 /home/s21600635/OS/HW5/test god says
+ caution : 'num' have to between 1 and 16

###3. Explain about structure
+ I create the Queue structure
Node : The node structure is the same as a general linked list.
Queue : The Queue structure has a head_lock and a tail_lock. The reason for dividing it into two is because Dequeue and Enqueue can be operated concurrency with each other.
Dequeue : It locks the head_lock after that executes the dequeue, and again unlocks the head_lock.
Enqueue : It locks tail_lock, after that executes enqueue, and then unlocks tail_lock. Through pthread_mutex_lock, the critial section is protected. Therefore, race condition is disappeared.
worker : Next, let's look at the worker function. First, declare the necessary variables. And after dequeueing the first task, And then read the path with the readdir function. The ‘.’ or ‘..’ files are ignored and just filters only regular files and directory files. At that time, if it is a regular file, worker reads the file and finds the line containing the keyword. If it is a directory, insert directory path into queue. After this processing the thread is terminated and the main thread is started again.

###4. Algorithms
 1. After executing the code .
 2. The first directory is put in the queue.
 3. Determine available task number.
 4. Threads are created as many as the input number, and then repeated until there is no task in queue.
 5. The worker function finds the directory and regular file and takes appropriate action.
 6. Ignore ‘.’ or ‘..’ files.
 7. If it is a regular file, finds the line containing the keyword in file.
 8. If it is a directory, insert directory path into queue.
 9. Repeat until queue is empty.


###5. Limitation of my code & difficulty
I can't find the Memeory map error. However, the code is runing well always.
The most difficult job is pointer to me. Because when I enqueue the data and when I dequeue the data, the results are constantly different, so I debugged for a really long time. In the end, I found the problem by Googling and searching for the immaturity of pointer use and was able to solve it.

####If you have any questions, please send an email to 21600635@handong.edu.