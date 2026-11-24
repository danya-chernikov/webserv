/* g++ -Wall daytimetcpsrv_ipv4.cpp -Wall -O0 -g3 -o daytimetcpsrv_ipv4 */

/* For now we're launching it with sudo! Be careful!!!
 * In order to launch it, previously execute:
 *    sudo mkdir -p /var/log/daytimetcpsrvf_ipv4/
 * And then:
 *    sudo ./daytimetcpsrvf_ipv4
 * */

/* Sockets */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* File I/O and system */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>		// close
#include <linux/fs.h>

/* From C */
#include <ctime>
#include <cstring>		// strlen, strerror
#include <cerrno>		// errno variable
#include <cstdio>		// perror
#include <cstdlib>		// exit

/* STL */
#include <iostream>
#include <fstream>
#include <string>

const int			MAXLINE			= 4096;
const int			LISTENQ			= 1024;
const u_short		DAYTIME_PORT	= 13;
const std::string	LOG_FILE_PATH	= "/var/log/daytimetcpsrc_ipv4/report.log";
// Write permission for the log file must be granted

void handle_error(std::ofstream &log, const std::string &culprit);
void close_log_file(std::ofstream &log);
void close_with_errcheck(std::ofstream &log, const int &fd);

int main()
{
	/* Create/open the log file */
	std::ofstream log(LOG_FILE_PATH);
	if (!log)
	{
		std::cerr << "ERROR: could not open a log file\n";
		exit(EXIT_FAILURE);
	}

	/* Create a new process */
	pid_t pid = fork();
	if (pid < 0)
		handle_error(log, "fork()");
	else if (pid != 0) // If child process was created successfully
		exit(EXIT_SUCCESS); // The parent must be terminated
	log << "INFO: New process for a daemon was created successfully\n";
	// We're in the child now!

	/* Creating a new session and a new process group */
	if (setsid() < 0)
		handle_error(log, "setsid()");

	/* Set up the root directory as the current directory */
	if (chdir("/") < 0)
		handle_error(log, "chdir()");

	/* Create a socket */
	int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenfd < 0)
		handle_error(log, "socket()");
	
	/* Fill in sockaddr_in */
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DAYTIME_PORT);

	/* Bound sockaddr_in to the listen socket */
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		close_with_errcheck(log, listenfd);
		handle_error(log, "bind()");
	}

	/* Convert the socket `listenfd` into a listening socket */
	log << "INFO: Listening for clients...\n";
	if (listen(listenfd, LISTENQ) < 0)
	{
		close_with_errcheck(log, listenfd);
		handle_error(log, "listen()");
	}

	char	buff[MAXLINE];
	time_t	ticks;
	int		connfd;
	for (;;)
	{
		/* Retrieves the next pending connection from the listening
		 * socket's accept queue and returns a new file descriptor
		 * for the connected socket */
		if ( (connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0)
		{
			int err = errno;
			if (err == ECONNABORTED || err == EINTR)
				continue;
			log << "ERROR: accept(): " << std::strerror(err) << '\n';
			break;
		}
		log << "INFO: A new client connected successfully!\n";

		/* We check for `ECONNABORTED` because a connection waiting in the
		 * listening socket's accept queue may be aborted before `accept()`
		 * can retrieve it.
		 * We check for `EINTR` because `accept()` may be interrupted by a
		 * signal before a connection is accepted.
		 * In both cases, it's generally safe to retry `accept()`.
		 * Nothing particularly bad has happened! */

		/* Get current time in appropriate format */
		ticks = std::time(NULL);
		std::snprintf(buff, sizeof(buff), "%.24s\r\n", std::ctime(&ticks));

		/* Send time string to the client */
		size_t buflen = std::strlen(buff);
		int wlen = write(connfd, buff, buflen);
		if (static_cast<size_t>(wlen) != buflen)
			log << "WARNING: write(): not all data has been written into a socket!\n";
		else if (wlen < 0)
		{
			int err = errno;
			log << "ERROR: write(): " << std::strerror(err) << '\n';
			close_with_errcheck(log, connfd);
			break;
		}

		close_with_errcheck(log, connfd);
	}

	close_with_errcheck(log, listenfd);
	close_log_file(log);
	exit(EXIT_SUCCESS);
}

/* Writes a textual description of the error to the file associated with `log`.
 * There is no point in checking for write errors because this program runs
 * as a daemon, and all its standard streams are suppressed. Therefore, we
 * wouldn't see the error message anyway. Moreover, the program terminates
 * immediately after logging the error.
 *     `culprit` — the name of the function that caused the error */
void handle_error(std::ofstream &log, const std::string &culprit)
{
	int err = errno;
	log << "ERROR: " << culprit << " " << std::strerror(err) << '\n';
	close_log_file(log);
	exit(EXIT_FAILURE);
}

void close_log_file(std::ofstream &log)
{
	log.close();
	if (log.fail())
		log << "ERROR: for some reason log file was not closed\n";
}

/* Attempts to close the file descriptor `fd` using close().
 * Logs an error message if the call fails */
void close_with_errcheck(std::ofstream &log, const int &fd)
{
	if (close(fd) == -1)
	{
		int err = errno;
		log << "ERROR: close(): " << std::strerror(err) << '\n';
	}
}
