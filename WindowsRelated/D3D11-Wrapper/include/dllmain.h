/*
    CONTRIBUTORS:
        Sean Pesce

*/
#pragma once

#ifndef SP_DLL_LIFECYCLE_EVENTS_H_
#define SP_DLL_LIFECYCLE_EVENTS_H_


#include "d3d11/main.h"
#include <windows.h>
#include <tchar.h>


namespace dll {
typedef BOOL(entry_point_t)(HMODULE, LPVOID);
}

#ifdef WRAPPER_ON_PROCESS_ATTACH
extern dll::entry_point_t WRAPPER_ON_PROCESS_ATTACH;
#define WRAPPER_ON_PROCESS_ATTACH_GLOBAL_NS :: ## WRAPPER_ON_PROCESS_ATTACH
#else
#define WRAPPER_ON_PROCESS_ATTACH_GLOBAL_NS(h_module, lp_reserved)
#endif // WRAPPER_ON_PROCESS_ATTACH

#ifdef WRAPPER_ON_PROCESS_DETACH
extern dll::entry_point_t WRAPPER_ON_PROCESS_DETACH;
#define WRAPPER_ON_PROCESS_DETACH_GLOBAL_NS :: ## WRAPPER_ON_PROCESS_DETACH
#else
#define WRAPPER_ON_PROCESS_DETACH_GLOBAL_NS(h_module, lp_reserved)
#endif // WRAPPER_ON_PROCESS_DETACH

#ifdef WRAPPER_ON_THREAD_ATTACH
extern dll::entry_point_t WRAPPER_ON_THREAD_ATTACH;
#define WRAPPER_ON_THREAD_ATTACH_GLOBAL_NS :: ## WRAPPER_ON_THREAD_ATTACH
#else
#define WRAPPER_ON_THREAD_ATTACH_GLOBAL_NS(h_module, lp_reserved)
#endif // WRAPPER_ON_THREAD_ATTACH

#ifdef WRAPPER_ON_THREAD_DETACH
extern dll::entry_point_t WRAPPER_ON_THREAD_DETACH;
#define WRAPPER_ON_THREAD_DETACH_GLOBAL_NS :: ## WRAPPER_ON_THREAD_DETACH
#else
#define WRAPPER_ON_THREAD_DETACH_GLOBAL_NS(h_module, lp_reserved)
#endif // WRAPPER_ON_THREAD_DETACH



namespace dll {

constexpr const char* build = __DATE__ "   " __TIME__;


inline BOOL on_process_attach(HMODULE h_module, LPVOID lp_reserved)
{

    d3d11::hook_exports();

    WRAPPER_ON_PROCESS_ATTACH_GLOBAL_NS(h_module, lp_reserved);

    return TRUE;
}


inline BOOL on_process_detach(HMODULE h_module, LPVOID lp_reserved)
{
    WRAPPER_ON_PROCESS_DETACH_GLOBAL_NS(h_module, lp_reserved);

    return BOOL(!!FreeLibrary(d3d11::chain));
}


inline BOOL on_thread_attach(HMODULE h_module, LPVOID lp_reserved)
{
    WRAPPER_ON_THREAD_ATTACH_GLOBAL_NS(h_module, lp_reserved);
    return TRUE;
}


inline BOOL on_thread_detach(HMODULE h_module, LPVOID lp_reserved)
{
    WRAPPER_ON_THREAD_DETACH_GLOBAL_NS(h_module, lp_reserved);
    return TRUE;
}


} // namespace dll


#endif // SP_DLL_LIFECYCLE_EVENTS_H_
