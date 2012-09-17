#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <libnotify/notify.h>

#define BUFSIZE 512
#define PORT 110

void email_notify(char *username, int mail)
{
	setenv("DISPLAY", ":0.0", 1);
	notify_init("mail daemon");
	NotifyNotification *hi;
	if (mail == 1)
		hi = notify_notification_new(username,
					     "you have a new email :)",
					     "/usr/share/icons/mail/mail.png");
	else {
		char dialog[64];
		sprintf(dialog, "you have %d new emails :)", mail);
		hi = notify_notification_new(username, dialog, NULL);
	}
	notify_notification_show(hi, NULL);
	notify_uninit();
}

char *rec(int sock, char *query)
{
	int r = 0;
	int bytes = 0;
	char *buf = NULL;
	r = send(sock, (char *)query, strlen(query), 0);
	if (r == -1) {
		perror("querry send error");
		return (NULL);
	}
	buf = (char *)malloc(BUFSIZE + 2);
	if (!buf) {
		perror("error  malloc");
		return (NULL);
	}
	recv(sock, buf, BUFSIZE, 0);
	return (buf);
}

int get_mail_num(char *servername, char *username, char *passwd)
{
	int ret = -1;
	int sockfd;
	struct sockaddr_in connection;
	struct hostent *server = gethostbyname(servername);
	if (!server) {
		return -1;
	}
	char popcmd[4][64];
	sprintf(popcmd[0], "USER %s\r\n", username);
	sprintf(popcmd[1], "PASS %s\r\n", passwd);
	strcpy(popcmd[2], "STAT \r\n");
	strcpy(popcmd[3], "QUIT \r\n");

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}
	connection.sin_family = AF_INET;
	connection.sin_port = htons(PORT);
	connection.sin_addr = *((struct in_addr *)server->h_addr);
	if (connect
	    (sockfd, (struct sockaddr *)&connection,
	     sizeof(struct sockaddr)) == -1) {
		return -1;
	}
	char *s = rec(sockfd, popcmd[0]);
	free(s);
	s = rec(sockfd, popcmd[1]);
	free(s);
	s = rec(sockfd, popcmd[2]);
	free(s);
	s = rec(sockfd, popcmd[3]);
	ret = atoi(s + 4);
	free(s);
	close(sockfd);
	return ret;
}

int main(int argc, char **argv)
{
	if (argc < 4) {
		printf("Usage: %s servername user passwd\n", argv[0]);
		exit(1);
	}
	char *passwd = strdup(argv[3]);
	char *username = strndup(argv[2], index(argv[2], '@') - argv[2]);
	while (*argv[3])
		*(argv[3]++) = '*';
	while (1) {
		int n1 = get_mail_num(argv[1], argv[2], passwd);
		sleep(5);
		int n2 = get_mail_num(argv[1], argv[2], passwd);
		if (n2 > n1 && n1 != -1 && n2 != -1)
			email_notify(username, n2 - n1);
	}
	free(passwd);
	free(username);
	return 0;
}
