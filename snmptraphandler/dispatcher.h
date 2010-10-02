#include <windows.h>
#include <stdarg.h>

BOOL initDispatcher();
void cleanDispatcher();
DWORD printDispatcherF(char *format, ...);
DWORD printDispatcher(char *data);

