#ifndef MANAGED_EL_INTEROP_H
#define MANAGED_EL_INTEROP_H

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <string>

class ManagedELInterop
{
public:
    static void SendCommand(const TCHAR* command, int cmd_handle);   
};

#endif //MANAGED_EL_INTEROP_H