#ifndef LEAPIMAGE_WINDOWS_WINHELPER_HPP_
#define LEAPIMAGE_WINDOWS_WINHELPER_HPP_

#ifdef _WIN32
#include <Windows.h>	
#include <vector>

struct EnumWindowsCallbackArgs {
	EnumWindowsCallbackArgs( DWORD p ) : pid( p ) { }
	const DWORD pid;
	std::vector<HWND> handles;
};

class WinHelper {

public:
	static BOOL CALLBACK EnumWindowsCallback( HWND hnd, LPARAM lParam )
	{
		EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;

		DWORD windowPID;
		(void)::GetWindowThreadProcessId( hnd, &windowPID );
		if ( windowPID == args->pid ) {
			args->handles.push_back( hnd );
		}

		return TRUE;
	}

	static std::vector<HWND> getToplevelWindows()
	{
		EnumWindowsCallbackArgs args( ::GetCurrentProcessId() );
		if ( ::EnumWindows( &EnumWindowsCallback, (LPARAM) &args ) == FALSE ) {
			// XXX Log error here
			return std::vector<HWND>();
		}
		return args.handles;
	}

};

#endif

#endif