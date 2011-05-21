#ifndef LOGH_H
#define LOGH_H

#include "bool.h"

#define ERRORMSG_LOGFILE_OPEN "Unable to open log file"
#define M_MSG_LEN    20000
#define M_ERRMSG_LEN 2047

bool InitLogHandler (void);

bool CloseLogHandler (void);

bool LogHandler
(
    char *message	/* I:  message to be written to the log */
);
#endif
