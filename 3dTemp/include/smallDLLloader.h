#ifndef SMALL_DLL_LOADER
#define	SMALL_DLL_LOADER
typedef void(*func_ptr)(void*);

#if defined( _WIN32 )
#include <Windows.h>
typedef HMODULE DLLHandle;
void UnloadDLL(DLLHandle* DLLHANDLE)
{
	FreeLibrary(*DLLHANDLE);
	*DLLHANDLE = 0;
}


int load_DLL(DLLHandle* DLLHANDLE,const char* name)
{
	WIN32_FILE_ATTRIBUTE_DATA unused;

	UnloadDLL(DLLHANDLE);
	CopyFile(name, "temp.dll", 0);
	*DLLHANDLE = LoadLibrary("game/Project1/x64/Debug/Project1.dll");

	if (!(*DLLHANDLE))
	{
		DWORD err = GetLastError();
		return -1;
	}
	return 0;
}

func_ptr load_DLL_function(DLLHandle DLLHANDLE,const char* name)
{
	return (func_ptr)GetProcAddress(DLLHANDLE, name);
}
#endif // !_WIN32


#endif // !SMALL_DLL_LOADER

