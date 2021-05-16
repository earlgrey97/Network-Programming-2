# Assignment 2

- Student ID: 20160233
- Your Name: ParkNaHyeon
- Submission date and time: 10.13 Tuesday around 5pm
Part2: around 18:20 Tuesday, 10.27
Part3: around 22:20 Tuesday, 10.27

## Ethics Oath
I pledge that I have followed all the ethical rules required by this course (e.g., not browsing the code from a disallowed source, not sharing my own code with others, so on) while carrying out this assignment, and that I will not distribute my code to anyone related to this course even after submission of this assignment. I will assume full responsibility if any violation is found later.

Name: ParkNaHyeon 
Date: 20.10.13. 20.10.27

## Performance measurements
Set 8kB-sized value into a specific key. Measure the time for running 1,000 concurrent GET requests on the key using `ab -c 1000 -n 1000`.
- Part 1 -> This, with submitted file, does not work.... I think I made mistake when submitting
  - Completed requests: _____
  - Time taken for test: _____ ms
  - 99%-ile completion time: _____ ms
- Part 2
  - Completed requests: 1000
  - Time taken for test: 0.450s(450ms)
  - 99%-ile completion time: 49ms
- Part 3
  - Completed requests: 1000
  - Time taken for test: 0.213s(213ms)
  - 99%-ile completion time: 30ms

Briefly compare the performance of part 1 through 3 and explain the results.
part 2 is better than part 1. Since my submitted part 1 code timeouts...
part 3 is better than part 2 since event does the work faster by reducing time from getting reply from redis server. It can accept faster.

## Brief description
Briefly describe here how you implemented the project.
Server that I made makes connections to both client and redis server.
Then, it reads the request from the client and makes key buffer and value buffer seperately. When each buffer is complete, it sends the request immediately to the redis server. This is repeated until all the request is read(until the header content-length is all read).
Then, it receives reply from redis server until server makes reply.
Finally, it gives reply back to the client.

part 2 does this with thread.

part 3 does this by invoking event in sockfds. (client sockfd, server sockfd)

## Misc
Describe here whatever help (if any) you received from others while doing the assignment.
No

How difficult was the assignment? (1-5 scale)
4
Sometimes very big size sending fails.

How long did you take completing? (in hours)
part1: about 20-30 hours
part2: about 5 hours
part3: about 10 hours