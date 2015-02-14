/*****************************************************************************
FileName : win32_MetroidVania.cpp
							Date Created: 2/12/15   Last Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Contains main functionality and interface with windows api for project
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


//Global variables
static bool running;			//Determines whether the window is currently running
static BITMAPINFO bitMapInfo;   //Stores info about the bitmap storing graphics
static void * bitMapMem;		//Points the bitmap in memory
static int bitMapWidth;
static int bitMapHeight;


static void RenderGradient(int XOffset, int YOffset)
{
	int pitch = bitMapWidth * 4;
	uint8 * Row = (uint8 * )bitMapMem;
	for(int y = 0; y < bitMapHeight; y++)
	{
		uint32 * Pixel = (uint32 * )Row;
		for(int x = 0; x < bitMapWidth; x++)
		{
			uint8 blue = (uint8)(x + XOffset);
			uint8 green = (uint8)(y + YOffset);
			uint8 red = 0;
			*Pixel++ =((green << 8) | blue);

		}
		Row += pitch;
	}
}



/******************************************************************************
FunctionName : Win32_UpdateGameWindow
							Date Created: 2/13/15   Date Edited: 2/13/15
							Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Updates the game window based on the bitmap in bitMapMem
@param : HDC DeviceContext - The context of the device to update
@param : int x			   - Position of the left part of the screen to update
@param : int y			   - Position of the top part of the screen to update
@param : int width		   - Width of the rectangle to update
@param : int height		   - Height of the rectangle to update
******************************************************************************/
static void Win32_UpdateGameWindow( HDC DeviceContext,const RECT * WindowRect,int x,int y,int width,int height)
{
	int windowWidth = WindowRect->right - WindowRect->left;
	int windowHeight = WindowRect->bottom - WindowRect->top;
	//Copy bitmap from bitMapMem to current DeviceContext, stretching bitmap to match size of the device's output
	StretchDIBits(DeviceContext,
				 /*x,y,width,height,  //Destination rectangle
				 x,y,width,height,  //Target Rectangle
				 */
				 0,0, bitMapWidth, bitMapHeight,
				 0,0, windowWidth, windowHeight,
				 bitMapMem,
				 &bitMapInfo,
				 DIB_RGB_COLORS,	//Using full RGB per bit
				 SRCCOPY);		  //Copy from source to destingation
}



/******************************************************************************
FunctionName : ResizeDIBSection   Date Created: 2/13/15   Date Edited: 2/13/15
								Creator: Tyler Whittin  Edited by: Tyler Whittin
Purpose: Set up bitMapInfo and reallocate bitMapMem when window resized
@param : HWND	Window    - Handle of calling window
@param : UINT	Message   - Message being sent
@param : WPARAM  wParam   - Additional info dependent on Message
@param : LPARAM  lParam   - Additional info dependent on Message
@return: LRESULT CALLBACK - Result of message processing
******************************************************************************/
static void ResizeDIBSection(int Width, int Height)
{

	if(bitMapMem)
	{
		VirtualFree(bitMapMem, 0, MEM_RELEASE);
	}

	bitMapWidth = Width;
	bitMapHeight = Height;

	//Create BITMAPINFO struct, all values not initialized set to 0
	bitMapInfo.bmiHeader.biSize = sizeof(bitMapInfo.bmiHeader);
	bitMapInfo.bmiHeader.biWidth = bitMapWidth;
	bitMapInfo.bmiHeader.biHeight = -bitMapHeight;
	bitMapInfo.bmiHeader.biPlanes = 1;
	bitMapInfo.bmiHeader.biBitCount = 32;
	bitMapInfo.bmiHeader.biCompression = BI_RGB;


	//Allocate new memory of size(pixelSize * width * height)
	bitMapMem = VirtualAlloc(NULL,			  //Don't care about location
							4 * bitMapWidth * bitMapHeight,
												//Want 4 * width * height allocated
							MEM_COMMIT,		 //Want immediate access
							PAGE_READWRITE);	//Only need to read/write

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
			//Get the new RECT to draw
			RECT clientRect;
			GetClientRect(Window,&clientRect );
			int height = clientRect.bottom - clientRect.top;
			int width = clientRect.right - clientRect.left;

			//Resize the DIB section
			ResizeDIBSection(width, height);
			OutputDebugString("WM_SIZE\n");
			break;
		}
		//Window Destroyed by ending process
		case WM_DESTROY:
			//Exit the program
			running = false;
			OutputDebugString("WM_DESTROY\n");
			break;
		//Window sent message to close
		case WM_CLOSE:
			//Exit the program
			running = false;
			OutputDebugString("WM_CLOSE\n");
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

				int x = paint.rcPaint.left;
				int y = paint.rcPaint.top;
				int height = paint.rcPaint.bottom - paint.rcPaint.top;
				int width = paint.rcPaint.right - paint.rcPaint.left;

				//Update the window
				RECT clientRect;
				GetClientRect(Window,&clientRect );

				Win32_UpdateGameWindow(contxt,&clientRect, x,y,width,height);

				EndPaint(Window, &paint);
				OutputDebugString("WM_PAINT\n");
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
  WNDCLASS WindowClass = {0,WindowProc,0,0,Instance,0,0,0,0,"MetroidVaniaWindowClass"};
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
			MSG message;
			while(running)
			{
				while(PeekMessage(&message, 0,0,0,PM_REMOVE))
				{
					if(message.message == WM_QUIT) {running = false;}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}//end if
				RenderGradient(xOffset, yOffset);
				HDC deviceContext = GetDC(WindowHandle);
				RECT WindowRect;
				GetClientRect(WindowHandle,&WindowRect );


				int windowWidth = WindowRect.right - WindowRect.left;
				int windowHeight = WindowRect.bottom - WindowRect.top;

				Win32_UpdateGameWindow(deviceContext,&WindowRect,0,0,windowWidth,windowHeight);

				ReleaseDC(WindowHandle, deviceContext);
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
