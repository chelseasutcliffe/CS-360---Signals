#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define bufsize 512

int signalReceived = 1;

static void sleephandler(int code) {
	/* sleep handler doesn't need to do anything but unpause the sleep func */
}

void sleepfunc(int seconds) {
	signal(SIGALRM, sleephandler);
	alarm(seconds);
	pause();
	sleep(0);
}

static void handler(int code) {
	/* handler will prompt user with a message */
	write(1, "Interrupt received, enter new message: ", 39);
}

static void childHandler(int code) {
	/* handler will set signal received to 0 allowing a break from the print while loop */
	signalReceived = 0;
}

int main(int argc, char *argv[]) {

	int fd[2], seconds;
	pid_t cpid;
	char buf[bufsize], readbuf[bufsize];;

	if( pipe(fd) == -1 ) {
		fprintf( stderr, "%s: pipe failed -- %s\n", argv[0], strerror(errno) );
		return errno;
	}

	if( ( cpid = fork() ) == -1 ) {
		fprintf( stderr, "%s: fork failed -- %s\n", argv[0], strerror(errno) );
		return errno;
	}

	if (cpid == 0) {		/* child process */
		signal (SIGINT, SIG_IGN);			/* ignore signal interrupt */
		signal (SIGFPE, childHandler);		/* set child handler to SIGFPE */
		
		close(fd[1]);
		
		while(1) {
			seconds = 5;
			read(fd[0], buf, bufsize);

			if (isdigit(buf[0])) {
				sscanf(buf, "%d %[^\n]", &seconds, readbuf);			/* scan until newline is encountered */
				strcat(readbuf, "\n");									/* add newline back for printing */
			} else {
				strcpy(readbuf, buf);
			}

			if (strcmp(readbuf, "exit\n") == 0) {
				printf("Exiting child process\n");
				if (kill (getppid(), SIGTERM) == -1) {				/* tell parent to terminate */
					perror ("Failed to terminate parent");			/* need to error out if cant terminate parent */
				}
	            return 0;
			}

			signalReceived = 1;
			while (signalReceived != 0) { 			/* loop print unitl parent signals child */
				printf("%s", readbuf);
				sleepfunc(seconds);
			}
		}

	} else {				/* parent process */
		close(fd[0]);
		signal(SIGINT, handler);
		for (;;) {
			pause();			/* pause until signal is received */
			if (kill(cpid, SIGFPE) == -1) {         /* signal child of new prompt */
				perror("Failed to signal child");
			}
			if (fgets(buf, bufsize, stdin) == NULL) {
				fprintf( stderr, "%s: imput read error\n", argv[0]);
				return 1;
			}
			write(fd[1], buf, bufsize);
		}
	}
}
