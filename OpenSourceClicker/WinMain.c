#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#define GRAVE_VK VK_OEM_3

HHOOK mouseHook;
HHOOK keyboardHook;

BOOLEAN toggled;
BOOLEAN mouseDown;
BOOLEAN firstClick;

long lastClick;

LRESULT CALLBACK MouseCallBack( int nCode, WPARAM wParam, LPARAM lParam )
{
	PMSLLHOOKSTRUCT pMouse = (PMSLLHOOKSTRUCT)lParam;
	if ( NULL != pMouse )
	{
		if ( WM_MOUSEMOVE != wParam )
		{
			// if this returns 0, we know it's a real click. see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644970(v=vs.85).aspx
			if ( !pMouse->flags )
			{
				switch ( wParam )
				{
				case WM_LBUTTONDOWN:
					mouseDown = TRUE;
					firstClick = TRUE;
					break;
				case WM_LBUTTONUP:
					mouseDown = FALSE;
					break;
				}
			}	
		}
	}
	return CallNextHookEx( mouseHook, nCode, wParam, lParam );
}

LRESULT CALLBACK KeyboardCallBack( int nCode, WPARAM wParam, LPARAM lParam )
{
	PKBDLLHOOKSTRUCT keyStruct = lParam;
	if ( NULL != keyStruct
		&& WM_KEYUP == wParam 
		&& GRAVE_VK == keyStruct->vkCode )
	{
		toggled = !toggled;
	}
	return CallNextHookEx( keyboardHook, nCode, wParam, lParam );
}

int RandomInt( int min, int max )
{
	srand( time( NULL ) );
	return ( ( rand() % (int)( ( (max)+1 ) - ( min ) ) ) + ( min ) );
}

DWORD WINAPI ClickThread( LPVOID lParam )
{
	while ( TRUE )
	{
		Sleep( 1 );
		if ( toggled && mouseDown )
		{
			if ( firstClick )
			{
				Sleep( 30 );
				mouse_event( MOUSEEVENTF_LEFTUP, 0, 0, NULL, NULL );
				firstClick = FALSE;
			}
			else
			{
				// Change this value if you want to change the CPS.
				int randomWaitDelay = RandomInt( 70, 100 );
				if ( ( clock() - lastClick ) > randomWaitDelay )
				{
					mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, NULL, NULL );
					lastClick = clock();

					// Sleep to bypass block interaction check, which measures the time between a left_down and a left_up.
					Sleep( RandomInt( 30, 50 ) );

					mouse_event( MOUSEEVENTF_LEFTUP, 0, 0, NULL, NULL );
				}
			}
		}
	}
}

int wmain()
{
	SetConsoleTitle( L"" );

	mouseHook = SetWindowsHookEx( WH_MOUSE_LL, &MouseCallBack, NULL, NULL );
	keyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, &KeyboardCallBack, NULL, NULL );

	CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&ClickThread, NULL, 0, 0 );

	// Infinite loop basically.
	MSG msg;
	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	UnhookWindowsHookEx( mouseHook );
	UnhookWindowsHookEx( keyboardHook );

	printf( "An error has ocurred while the program was running, as a result the program will be closed.\n" );
	_getch();
	return 1;
}