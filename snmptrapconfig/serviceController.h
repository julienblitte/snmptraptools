#include <windows.h>

#ifndef SERVICECONTROLLER_H_INCLUDED
#define SERVICECONTROLLER_H_INCLUDED

#define SERVICE_CONTROL_START 0

BOOL openService();
DWORD getServiceState();
BOOL setServiceState(DWORD state);
void closeService();

#endif // SERVICECONTROLLER_H_INCLUDED
