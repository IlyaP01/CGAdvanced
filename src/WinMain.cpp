#include "App.h"


int WINAPI WinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ LPSTR lpCmdLine,
					_In_ int nShowCmd)
{
	try
	{
		return App{}.Go();
	}
	catch( const BaseException& e )
	{
		MessageBoxA( nullptr, e.what(), e.GetType(),MB_OK | MB_ICONEXCLAMATION );
	}
	catch( const std::exception& e )
	{
		MessageBoxA( nullptr,e.what(),"Standard Exception",MB_OK | MB_ICONEXCLAMATION );
	}
	catch( ... )
	{
		MessageBoxA( nullptr, "No details available", "Unknown Exception",MB_OK | MB_ICONEXCLAMATION );
	}
	return -1;
}