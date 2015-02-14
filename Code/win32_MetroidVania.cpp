/*****************************************************************************
FileName : win32_MetroidVania.cpp
							Date Created: 2/12/15   Last Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Contains main functionality and interface with windows api for project
*****************************************************************************/

/*****************************************************************************
Update Notes (2/13/15) : Added win32_WindowDimensions and win32_OffscreenBuffer,
 general cleanup of code, extracted ethods uses Buffer struct instead of having
 all the components as individual global variables
*****************************************************************************/

//Includes
#include <windows.h>
#include <stdint.h>

//Typedefs
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_WindowDimensions
{
    int width;
    int height;
};

struct win32_OffscreenBuffer
{
    BITMAPINFO Info;  //Stores info about the bitmap storing graphics
    void *  Memory;		    //Points the bitmap in memory
    int     Width;
    int     Height;
    int     Pitch;
};

//Global variables
static bool running;			//Determines whether the window is currently running
static win32_OffscreenBuffer backBuffer;


/******************************************************************************
FunctionName : WindowDimensions
                            Date Created: 2/13/15   Date Edited: 2/13/15
                            Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Gets the windows width and height
@param : HWND window            - Window to obtain dimensions of
@param : win32_WindowDimensions - Struct containing window width and height
******************************************************************************/
static win32_WindowDimensions WindowDimensions(HWND window)
{
    win32_WindowDimensions Dimensions;
    RECT WindowRect;
    GetClientRect(window,&WindowRect );


    Dimensions.width = WindowRect.right - WindowRect.left;
    Dimensions.height = WindowRect.bottom - WindowRect.top;

    return(Dimensions);
}



/******************************************************************************
FunctionName : RenderGradient
                            Date Created: 2/13/15   Date Edited: 2/13/15
                            Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Renders a blue->green gradient based on window coordinate
@param : win32_OffscreenBuffer Buffer - Backbuffer to update
@param : int blueOffset	              - Offset for the blue part of the gradient
@param : int greenOffset              - Offset for the green part of the gradient
******************************************************************************/
static void RenderGradient(const win32_OffscreenBuffer *Buffer,int blueOffset, int greenOffset)
{
	uint8 * Row = (uint8 * )Buffer->Memory;
	for(int y = 0; y < Buffer->Height; y++)
	{
		uint32 * Pixel = (uint32 * )Row;
		for(int x = 0; x < Buffer->Width; x++)
		{
			uint8 blue = (uint8)(x + blueOffset);
			uint8 green = (uint8)(y + greenOffset);
			uint8 red = 0;
			*Pixel++ =((green << 8) | blue);

		}//end for
		Row += Buffer->Pitch;
	}//end for
}//end RenderGradient



/******************************************************************************
FunctionName : Win32_UpdateGameWindow
							Date Created: 2/13/15   Date Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Updates the game window based on the bitmap in the passed buffer
@param : HDC DeviceContext  - The context of the device to update
@param : win32_OffscreenBuffer Buffer
                            - The buffer information to use when updating
@param : int WindowWidth    - Width of the rectangle to update
@param : int WindowHeight   - Height of the rectangle to update
******************************************************************************/
static void Win32_UpdateGameWindow( HDC DeviceContext,win32_OffscreenBuffer Buffer,int WindowWidth,int WindowHeight)
{
	//Copy bitmap from bitMapMem to current DeviceContext, stretching bitmap to match size of the device's output
	StretchDIBits(DeviceContext,
				 0,0, WindowWidth, WindowHeight,
                 0,0, Buffer.Width, Buffer.Height,
				 Buffer.Memory,
				 &(Buffer.Info),
				 DIB_RGB_COLORS,	//Using full RGB per bit
				 SRCCOPY);		  //Copy from source to destingation
}



/******************************************************************************
FunctionName : ResizeDIBSection   Date Created: 2/13/15   Date Edited: 2/13/15
								Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Sets up buffer to work for new window size after resize
@param : win32_OffscreenBuffer Buffer - The buffer to be resized
@param : int Width                    - The width of the new screensize
@param : int height                   - The height of the new screensize
******************************************************************************/
static void ResizeDIBSection(win32_OffscreenBuffer *Buffer, int Width, int Height)
{
    //If memory has been allocated before
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

    //Set the width and the height of the buffer
	Buffer->Width = Width;
	Buffer->Height = Height;

	//Create BITMAPINFO struct, all values not initialized set to 0
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;


	//Allocate new memory of size(pixelSize * width * height)
    Buffer->Memory = VirtualAlloc(NULL,			  //Don't care about location
							4 * Buffer->Width * Buffer->Height,
												//Want 4 * width * height allocated
							MEM_COMMIT,		 //Want immediate access
							PAGE_READWRITE);	//Only need to read/write
    Buffer->Pitch = Width * 4;
}



/******************************************************************************
FunctionName : WindowProc   Date Created: 2/12/15   Date Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Handles windows messages
Update Notes (2/13/15) : Added ability to close window, added calls to ResizeDIBSection and Win32_UpdateGameWindow functions
@param : HWND	Window    - Handle of calling window
@param : UINT	Message   - Message being sent
@param : WPARAM  wParam   - Additional info dependent on Message
@param : LPARAM  lParam   - Additional info dependent on Message
@return: LRESULT CALLBACK - Result of message processing
******************************************************************************/
LRESULT CALLBACK WindowProc(HWND Window,
							UINT Message,
							WPARAM wParam,
							LPARAM lParam)
{
	//Assume message succeeds by default
	LRESULT result = 0;
	switch(Message)
	{
		//Window changes size
		case WM_SIZE:
		{

		}break;
		//Window Destroyed by ending process
		case WM_DESTROY:
			//Exit the program
			running = false;
			break;
		//Window sent message to close
		case WM_CLOSE:
			//Exit the program
			running = false;
			break;
		//Window activated
		case WM_ACTIVATEAPP:
			OutputDebugString("WM_ACTIVEAPP\n");
			break;
		//Window painted
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				//Bind paint to the current window
				HDC contxt = BeginPaint(Window, &paint);

                //Update the window
                win32_WindowDimensions Dimensions = WindowDimensions(Window);

				Win32_UpdateGameWindow(contxt, backBuffer,Dimensions.width, Dimensions.height);

				EndPaint(Window, &paint);
			}break;
		//For all messages not explicitly handled use default windows handlers
		default:
			result = DefWindowProc(Window, Message, wParam, lParam);
			break;

  }//end switch
  return (result);
}



/******************************************************************************
FunctionName : WinMain	  Date Created: 2/12/15   Date Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Main entry point for application
@param : HINSTANCE Instance	    - Handle of current instance of application
@param : HINSTANCE PrevInstance - Handle of previous instance of application
@param : LPSTR	   CmdLine	    - The command line for the application
@param : int	   ShowCode	    - Bitfield determining how window is shown
@return: int CALLBACK		    - Determines what message ended the program,
									returns 0 by default
******************************************************************************/
int CALLBACK
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CmdLine,
		int ShowCode)
{
/*
  WNDPROC   lpfnWndProc = MAINWINDOWCALLBACK;
  HINSTANCE hInstance;
//Todo(Set icon)
  HICON	 hIcon;
  LPCTSTR   lpszClassName;
*/
	//Create the Window Class
    WNDCLASS WindowClass = {CS_HREDRAW|CS_VREDRAW,WindowProc,0,0,Instance,0,0,0,0,"MetroidVaniaWindowClass"};

    //Reserve memory for backBuffer
    ResizeDIBSection(&backBuffer,1200,800);

    //If registration of Window Class is successfull
    if(RegisterClass(&WindowClass))
	{
		//Create the window
		HWND WindowHandle = CreateWindowEx(0,
										WindowClass.lpszClassName,
										"MetroidVania",
										WS_OVERLAPPEDWINDOW|WS_VISIBLE,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										CW_USEDEFAULT,
										0,
										0,
										Instance,
										0);
		//If the Window was created successfully
		if(WindowHandle)
		{
			running = true;


            int xOffset = 0;
			int yOffset = 0;
			//Message loop
			while(running)
			{
                MSG message;

				//Handle all messages in the queue
                while(PeekMessage(&message, 0,0,0,PM_REMOVE))
				{
					if(message.message == WM_QUIT) {running = false;}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}//end if

                //Render the gradient
				RenderGradient(&backBuffer, xOffset, yOffset);
				HDC deviceContext = GetDC(WindowHandle);

                win32_WindowDimensions Dimensions = WindowDimensions(WindowHandle);

                //Update the window
				Win32_UpdateGameWindow(deviceContext,backBuffer,Dimensions.width, Dimensions.height);

				ReleaseDC(WindowHandle, deviceContext);
				//Update xOffset and yOffset
                xOffset++;
				yOffset++;
			}//end Message loop
		}//end if
		else
		{
			//Todo log error
		}//end else
    }//end if
    else
    {
	       //Todo log register failure
    }//end else
  return (0);
}
