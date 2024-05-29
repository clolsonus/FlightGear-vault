/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
 
#include "manifest.h"
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <setjmp.h>
#include <string.h>

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#ifndef bcopy
#define bcopy(s, d, n)  memcpy ((d), (s), (n))
#endif
#endif

extern char *getenv ();
extern struct servent *getservent();

jmp_buf	dead;

#if defined(SIGVOID) || defined(linux)
void
#endif
done () {
	longjmp (dead, 0);
}

main (argc, argv)
int	argc;
char	*argv[]; {

	char	*display;
	char	*host;
	char	*user;
	char	*plane;
	char	myhost[64];
	char	args[256];
	int	s, n = 1;
#ifdef _POSIX_VERSION
	pid_t	pid;
#else
	int	pid;
#endif
#ifndef linux
	struct passwd	*pwent, *getpwuid();
#else
	struct passwd   *pwent;
#endif
	struct sockaddr_in sin;
	struct hostent	*h = (struct hostent *) NULL;

	(void) gethostname (myhost, sizeof(myhost));

	if ((host = getenv("ACMSERVER")) == NULL)
		if (argc >= 2 && *argv[1] != '-') {
			host = argv[1];
			n = 2;
		}
		else {
			host = malloc (strlen(myhost) + 1);
			(void) strcpy (host, myhost);
			n = 1;
		}

	if ((pwent = getpwuid(getuid())) == NULL) {
		(void) fprintf (stderr, "Yow!\n");
		exit (1);
	}

	if ((user = getenv("ACM_HANDLE")) != NULL) {
		if (strncmp (user, "drone", 5) != 0)
			pwent->pw_name = user;
	}

	plane = getenv ("ACM_PLANE");

	if ((display = getenv("DISPLAY")) == NULL) {
		(void) fprintf (stderr, "Excuse me, but you have no DISPLAY.\n\
How do you use X, anyway?\n");
		exit (1);
	}

/*
 *  Sometimes we end up with a DISPLAY value that won't jive on the
 *  network (e.g. ":0.0") -- fix these cases.  Also, from Michael Pall,
 *  if the acm server is running on the local system, allow "unix:*" and
 *  "local:*" connections.
 */

	if (*display == ':') {
		(void) strcat (myhost, display);
		display = myhost;
	}
	else if (strcmp (myhost, host)) {
		if (strncmp ("unix:", display, 5) == 0) {
			(void) strcat (myhost, display+4);
			display = myhost;
		}
		else if (strncmp ("local:", display, 6) == 0) {
			(void) strcat (myhost, display+5);
			display = myhost;
		}
	}

	if ((sin.sin_addr.s_addr = inet_addr (host)) != -1) {
		sin.sin_family = AF_INET;
	}
	else if ((h = gethostbyname (host)) != 0) {
		sin.sin_family = h->h_addrtype;
		bcopy (h->h_addr, (char *) &sin.sin_addr, h->h_length);
	}
	else {
		(void) fprintf (stderr, "Unknown host \"%s\"\n", host);
		exit (1);
	}

	sin.sin_port = htons(ACM_PORT);

	signal (SIGPIPE, done);

reconnect:
	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("can't get a socket");
		exit (1);
	}

	if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
		(void) close (s);
		pid = fork();
		if (pid == 0) {
			(void) fprintf (stderr, "Automatically starting server ...\n");
			(void) execl ("/bin/sh", "sh", "-c", "exec acms", (char *) 0);
			perror ("exec of acms failed");
			exit (1);
		}
		else if (pid > 0) {
			sleep (3);
			goto reconnect;
		}
		perror ("can't connect to server");
		(void) close (s);
		exit (1);
	}

	if (argv[n] != (char *) NULL) 
		(void) strcpy (args, argv[n++]);
	else
		(void) strcpy (args, "");

	for (; argv[n] != (char *) NULL; ++n) {
		(void) strcat (args, "|");
		(void) strcat (args, argv[n]);
	}

	if (plane) {
		(void) strcat (args, "|-plane|");
		(void) strcat (args, plane);
	}

	if (setjmp (dead) == 0)
		(void) handshake (s, display, pwent->pw_name, args);

	exit (0);
}

handshake (s, dpy, name, args)
int	s;
char	*dpy, *name, *args; {

	char	info[256];
	int	n;

	(void) sprintf (info, "%s %s %s\n", dpy, name, args);
	(void) write (s, info, strlen (info));
	while ((n = read (s, info, sizeof (info))) > 0)
		(void) write (fileno (stdout), info, n);

	(void) shutdown (s, 2);

	return 0;
}
