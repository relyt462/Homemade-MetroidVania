#include "win32_DeviceContext.h"

win32_DeviceContext::win32_DeviceContext(HWND Handle)
{
    currentHandle = Handle;
    deviceContext = GetDC(currentHandle);
}

win32_DeviceContext::~win32_DeviceContext(void)
{
    ReleaseDC(currentHandle, deviceContext);
}

HDC win32_DeviceContext::getCurrentContext()
{
    return(deviceContext);
}

void win32_DeviceContext::setCurrentContext(HWND Handle)
{
    if(!deviceContext)
        ReleaseDC(currentHandle,deviceContext);
    currentHandle = Handle;
    deviceContext = GetDC(currentHandle);
}
