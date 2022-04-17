#include <stdlib.h>
#include <stdio.h>
#include<fcntl.h>
#include"p1fxns.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#define BUF_SIZE 256
int main (int argc, char *argv[]) {
	int bufsize, fd, wsize, term;
	char buf[100];
	int pid[20];
	char word[30];
	char *programs[30];
	fd = open(argv[1], O_RDONLY);
	for (int i = 0; i < 3; i++) {
		pid[i] = fork();
		if (pid[i] == 0) {
			bufsize = p1getline(fd,buf,30);
			int len = p1strlen(buf);
			//p1strpack("\0",  0 - len, ' ', buf);
			buf[len - 1] = '\0';
			wsize = 0;
			term = 0;
			while (wsize != -1) {
				wsize = p1getword(buf, wsize, word);
				programs[term] = p1strdup(word);
		//		p1strcpy(programs[term], word);
				term++;
			}
			programs[term - 1] = NULL;
			for (int d = 0; d < 4; d++) {
				printf("Programs: %s\n", programs[d]);
				fflush(stdout);
			}

			close(fd);
			execvp(programs[0], programs);

			//execvp(programs,args[0]);
		}
}

      for (int i = 0; i < 3; i++) {
	      wait(NULL);
      }

}


/*
for i in 0 .. numprograms-1
 pid[i] = fork();
 if (pid[i] == 0)
  	prepare argument structure;
  	execvp(program[i], args[i])
for i in 0 .. numprograms-1
     wait(pid[i])

 */
