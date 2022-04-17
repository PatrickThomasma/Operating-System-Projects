#include <stdlib.h>
#include <stdio.h>
#include<fcntl.h>
#include"p1fxns.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#define BUF_SIZE 256


void NotEnoughArgumentsError() {
	char msg[] = "Too many or too few arguments when executing (Should be three)\n";
	int fd = open("/dev/tty", O_WRONLY);
	write(fd, msg, sizeof(msg));
	exit(EXIT_FAILURE);
}	


void NotValidFileError(char *file) {
	char msg[] = "Not a valid file.\n";
	int fd = open("/dev/tty", O_WRONLY);
	write(fd, msg, sizeof(msg));
	exit(EXIT_FAILURE);
}



int main (int argc, char *argv[]) {
	char *p;
	int val = -1;
	if ((p = getenv("VARIABLE_NAME")) != NULL)
		val = atoi(p);
	if (argc != 3) {
		NotEnoughArgumentsError();
	}

	int bufsize, fd, wsize, term;
	int place = 0;
	char buf[100];
	int pid[20];
	char word[30];
	char *programs[10][30];
	fd = open(argv[2], O_RDONLY);
	if (fd == -1)
		NotValidFileError(argv[2]);
	while (bufsize != 0) {
		bufsize = p1getline(fd,buf,30);
		int len = p1strlen(buf);
		buf[len - 1] = '\0';
		wsize = 0;
		term = 0;
		while (wsize != -1) {
			wsize = p1getword(buf,wsize,word);
			programs[place][term] = p1strdup(word);
			term++;
		}
		programs[place][term-1] = NULL;
		place++;
	}

	for (int i = 0; i < place; i++) {
		pid[i] = fork();
		if (pid[i] == 0) {
			close(fd);
			execvp(programs[i][0], programs[i]);

			//execvp(programs,args[0]);
		}
}

      for (int i = 0; i < place; i++) {
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
