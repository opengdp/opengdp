
/* Simple function to return the process Id in Windows.  Currently only tested
 * on Windows XP. */

#include <windows.h>
#include "winpid.h"

pid_t getpid(void) {

   DWORD cpid = GetCurrentProcessId();

   return cpid;
}

