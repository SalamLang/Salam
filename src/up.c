/**
 *
 * @Author: Max Base
 * @Repository: https://github.com/BaseMax/file-downloader-c
 * @Date: 09/04/2024
 * @Build (Windows): gcc downloader.c -o downloader -g -Wextra -std=c11 -lws2_32
 * @Build (Linux): gcc downloader.c -o downloader -g -Walloca -Wextra -Wall -fsanitize=address,undefined -std=c11
 *
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
// #define _POSIX_C_SOURCE 200112L
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void parse_url(const char *url, char *hostname, char *path)
{
	const char *start;
	const char *end;

	if (strncmp(url, "https://", 8) == 0)
	{
		fprintf(stderr, "Error: HTTPS protocol is not supported.\n");
		exit(1);
	}

	if (strncmp(url, "http://", 7) == 0)
	{
		start = url + 7;
	}
	else
	{
		start = url;
	}

	end = strchr(start, '/');
	if (end != NULL)
	{
		strncpy(hostname, start, end - start);
		hostname[end - start] = '\0';
		strcpy(path, end);
	}
	else
	{
		strcpy(hostname, start);
		path[0] = '/';
		path[1] = '\0';
	}
}

int main(int argc, char *argv[])
{
	const char *port = "80";
	char hostname[256];
	char path[256];
	char request[1024];
	char buffer[BUFFER_SIZE];

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
		return 1;
	}

	parse_url(argv[1], hostname, path);

	snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, hostname);

	printf("Request:\n%s\n", request);
	printf("Hostname: %s\n", hostname);
	printf("Path: %s\n", path);

	int sockfd;
#ifdef _WIN32
	WSADATA wsaData;
#endif
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int bytes_received;
	FILE *fp;

#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup failed.\n");
		return 1;
	}
#endif

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1)
		{
			perror("socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("connect");
#ifdef _WIN32
			closesocket(sockfd);
#else
			close(sockfd);
#endif
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "Failed to connect.\n");
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	freeaddrinfo(servinfo);

	if (send(sockfd, request, strlen(request), 0) == -1)
	{
		perror("send");
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return 1;
	}

	fp = fopen("downloaded_file.log", "wb");
	if (!fp)
	{
		perror("fopen");
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return 1;
	}

	while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0)
	{
		fwrite(buffer, 1, bytes_received, fp);
	}

	if (bytes_received == -1)
	{
		perror("recv");
	}

	fclose(fp);
#ifdef _WIN32
	closesocket(sockfd);
	WSACleanup();
#else
	close(sockfd);
#endif

	printf("Done.\n");

	return 0;
}
