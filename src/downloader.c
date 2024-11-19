/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 *
 * Welcome to the Salam Programming Language! Salam is a modern, efficient,
 * and developer-friendly language created to inspire and empower programmers
 * of all ages. With its unique features and Persian roots, Salam aims to make
 * programming more accessible and enjoyable for everyone.
 *
 * This file is part of the core implementation of Salam, including its runtime
 * and compiler components. By contributing to or using this codebase, you are
 * part of a growing community dedicated to innovation and inclusivity in
 * programming.
 *
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 *
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#include "downloader.h"

/**
 *
 * @function parse_url
 * @brief Parse URL
 * @params {const char*} url
 * @params {char*} hostname
 * @params {char*} path
 * @returns {void}
 *
 */
void parse_url(const char *url, char *hostname, char *path) {
    const char *start;
    const char *end;

    if (strncmp(url, "https://", 8) == 0) {
        fprintf(stderr, "Error: HTTPS protocol is not supported.\n");
        exit(1);
    }

    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
    } else {
        start = url;
    }

    end = strchr(start, '/');
    if (end != NULL) {
        strncpy(hostname, start, end - start);
        hostname[end - start] = '\0';
        strcpy(path, end);
    } else {
        strcpy(hostname, start);
        path[0] = '/';
        path[1] = '\0';
    }
}

/**
 *
 * @function download
 * @brief Download a file from a HTTP URL
 * @params {FILE*} fp - File pointer
 * @params {const char*} port - Port
 * @params {const char*} hostname - Hostname
 * @params {const char*} path - Path
 * @returns {bool} - True if the download was successful, false otherwise
 *
 */
bool download(FILE *fp, const char *port, const char *hostname,
              const char *path) {
    char request[1024];
    char buffer[BUFFER_SIZE];

    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
             path, hostname);

    int sockfd;
#ifdef _WIN32
    WSADATA wsaData;
#endif
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int bytes_received;

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");

        return false;
    }
#endif

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
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

    if (p == NULL) {
        fprintf(stderr, "Failed to connect.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    freeaddrinfo(servinfo);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("send");
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
#endif
        return false;
    }

    if (!fp) {
        perror("fopen");
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
#endif
        return false;
    }

    int header_found = 0;
    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        if (!header_found) {
            char *header_end = strstr(buffer, "\r\n\r\n");
            if (header_end) {
                header_found = 1;
                size_t header_length = header_end - buffer + 4;
                fwrite(header_end + 4, 1, bytes_received - header_length, fp);
            }
        } else {
            fwrite(buffer, 1, bytes_received, fp);
        }
    }

    if (bytes_received == -1) {
        perror("recv");
        return false;
    }

    fclose(fp);
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return true;
}
