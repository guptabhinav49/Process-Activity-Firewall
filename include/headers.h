#ifndef PCA_H
#define PCA_H

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <regex.h>

#define MAX_PATHLEN 256
#define MAX_BUFFLEN 1024
#define MAX_CONFIGSIZE 2056
#define MAX_SUBSTRLEN_TO_CHECK 128
#define SV_SOCK_PATH "/tmp/server.sock"
#define CONFIGFILE_PATH "../config.json"
#define MAX_FILES_PER_TYPE 64

// utility methods
static int compare(const void *a, const void *b);
void sort(char a[][MAX_PATHLEN], int n);
int find(char arr[][MAX_PATHLEN], const char *name, int n);
int match_regex(regex_t *arr, char *type, const char *s, int n);
int match(char a[], const char sub[]);

#endif