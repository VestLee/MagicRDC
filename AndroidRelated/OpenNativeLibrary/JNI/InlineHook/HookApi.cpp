#include "HookApi.h"

//__arm__			armeabi-v7
//__aarch64__		arm64-v8a
//__i386__			x86
#if defined(__arm__) || defined(__aarch64__)
#include "inlineHook.h"			// armv7 inline hook
#include "And64InlineHook.h"	// arm64 inline hook
#else
#define ELE7EN_OK 1
bool registerInlineHook(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr) { return false; }
int inlineHook(uint32_t target_addr) { return 0; };
int inlineUnHook(uint32_t target_addr) { return 0; };
void A64HookFunction(void * target_addr, void * new_addr, void **proto_addr) {}
#endif


#if   defined(__aarch64__)
bool hook(void * target_addr, void * new_addr, void **proto_addr)
{
	if (target_addr == NULL || new_addr == NULL || proto_addr == NULL) return false;

	LOGD("A64HookFunction 64 %d %d", (uint32_t)(uintptr_t)(target_addr), (uint32_t)(uintptr_t)(new_addr));
	A64HookFunction(target_addr, new_addr, proto_addr);
	LOGD("hook true");
	return true;
}

bool unhook(void * target_addr)
{
	return true;
}
#elif defined(__arm__)
bool hook(void *  target_addr, void *  new_addr, void **proto_addr)
{
	if (target_addr == NULL || new_addr == NULL || proto_addr == NULL) return false;

	LOGD("registerInlineHook 32 %d %d", (uint32_t)target_addr, (uint32_t)new_addr);
	if (registerInlineHook((uint32_t)target_addr, (uint32_t)new_addr, (uint32_t **)proto_addr)) {
		return false;
	}

	LOGD("inlineHook %d %d", (uint32_t)target_addr, (uint32_t)new_addr);
	if (inlineHook((uint32_t)target_addr) != ELE7EN_OK) {
		return false;
	}
	LOGD("hook true");
	return true;
}

bool unhook(void* target_addr)
{
	if (inlineUnHook((uint32_t)target_addr) != ELE7EN_OK) {
		return false;
	}

	return true;
}
#else	// __i386__
bool hook(void *  target_addr, void *  new_addr, void **proto_addr) { return true; }
bool unhook(void* target_addr) { return true; }
#endif
