#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

typedef struct _service
{
    const char *name;
    const char *description;
    const char *dependencies;
    void (*onContinue)(void);
    void (*onPause)(void);
    void (*onStart)(void);
    void (*onStop)(void);
    void (*onReload)(void);
} service;

void InitService(service *Service);
void InstallService();
void UninstallService();
void RunService();
void StopService();
void PanicQuitService(DWORD errorCode);

void SetStatus(DWORD State);
DWORD GetStatus();

#endif // SERVICE_H_INCLUDED
