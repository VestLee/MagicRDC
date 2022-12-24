/*
    CONTRIBUTORS:
        Sean Pesce

*/

#include "d3d11/main.h"


namespace d3d11 {

HMODULE chain = NULL;
FARPROC functions[func_count];

//¹Ø¼ü´úÂë£º
BOOL IsWow64()
{

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process = nullptr;
	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
		}
	}

	return bIsWow64;
}

void hook_exports()
{
	if (IsWow64())
	{
		chain = LoadLibrary("C:\\Windows\\SysWOW64\\d3d11.dll");
	}
	else
	{
		chain = LoadLibrary("C:\\Windows\\System32\\d3d11.dll");
	}

	if (!chain)
	{
		MessageBox(NULL, "Unable to locate original d3d11.dll (or compatible library to chain)", "ERROR: Failed to load original d3d11.dll", NULL);
		exit(0);
	}

	int count = 0;
	for (int i = 0; i < d3d11::func_count; i++)
	{
		FARPROC func = GetProcAddress(chain, func_names[i]);
		if (func)
		{
			count++;
		}
		functions[i] = func;
	}

	std::string rdcpath = "D:\\RenderDoc\\RenderDoc1.14\\renderdoc.dll";
	HMODULE renderdoc = LoadLibrary(rdcpath.c_str());
	if (!renderdoc) {
		MessageBox(NULL, "Unable to locate renderdoc (or compatible library to chain)", "ERROR: Failed to load original d3d11.dll", NULL);
		exit(0);
	}
}

} // namespace d3d11



