#ifndef _DOWNLOADER_H_
#define _DOWNLOADER_H_

#include <stdbool.h>

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
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

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
void parse_url(const char *url, char *hostname, char *path);

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
              const char *path);

#endif
