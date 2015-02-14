#ifndef WIND32_DEVICECONTEXT
#define WIND32_DEVICECONTEXT
#include <windows.h>

class win32_DeviceContext
{
public:
    win32_DeviceContext(HWND Handle);
    ~win32_DeviceContext(void);
    HDC getCurrentContext();
    void setCurrentContext(HWND Handle);
private:
    HDC deviceContext;
    HWND currentHandle;
};

#endif
