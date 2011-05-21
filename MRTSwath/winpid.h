#ifndef WINPID_H
#define WINPID_H

#ifdef WIN32

typedef unsigned long pid_t;

pid_t getpid(void);

#endif

#endif

