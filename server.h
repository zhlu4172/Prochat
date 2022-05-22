#ifndef SERVER_H
#define SERVER_H
#define _POSIX_SOURCE

#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>


#define PATH ("gevent")
#define DOMAIN_LEN (256)
#define IDENTIFIER_LEN (256)
#define WHOLE_LEN (2048)


void CREATE_WR_PATH(char* dest, char* domain, char* identifier);
void CREATE_RD_PATH(char* dest, char* domain, char* identifier);
void sigusr1_handler(int signal);
void zombie_child_signal(int signal);

