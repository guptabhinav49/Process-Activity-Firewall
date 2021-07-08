#ifndef PCA_H
#define PCA_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <streambuf>
#include <string.h>
#include <string>
#include <vector>
#include <regex>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
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
int find(std::vector<std::tuple<std::string,int,int> >&, std::string &name);
int match_regex(std::vector<std::tuple<std::string,int,int,int> >&, std::string &name);
int match(char a[], const char sub[]);

// printing funtions
void out(std::vector<std::tuple<std::string,int,int> > &fpaths);
void out(std::vector<std::tuple<std::string,int,int,int> >& fpaths);

#endif