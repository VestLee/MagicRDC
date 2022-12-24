#include "InlineHook/HookApi.h"
#include "ndklog.h"

#include <jni.h>
#include <dlfcn.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <mutex>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>
#include <android/log.h>
//#include <dlfcn_compat.h>
#include <sys/system_properties.h>

#define FIND_MONO_METHOD(mono_native_method) mono_native_method = (mono_native_method##Func)FindFunc(monoLibraryHandle, #mono_native_method);if (mono_native_method == NULL){ LOGD("mono method not found : %s", #mono_native_method); }
#define REGISTER_MONO_METHOD(native_method) script_##native_method##_ptr = mono_##native_method

#if	defined(__aarch64__)
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym  Elf64_Sym
#elif defined(__arm__)
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym  Elf32_Sym
#endif

struct ctx {
	void *load_addr;
	void *dynstr;
	void *dynsym;
	int nsyms;
	off_t bias;
};

extern "C"
{
	void* LibraryHandle;
	typedef void* (*OpenNativeLibraryFunc)(JNIEnv* env, int32_t target_sdk_version, const char* path, jobject class_loader, jstring library_path, bool* needs_native_bridge, std::string* error_msg);
	void* (*old_OpenNativeLibrary)(JNIEnv* env, int32_t target_sdk_version, const char* path, jobject class_loader, jstring library_path, bool* needs_native_bridge, std::string* error_msg);
	OpenNativeLibraryFunc OpenNativeLibrary;
	bool is_loaded = false;
	bool IsLoadRenderdoc() {
		if (is_loaded) {
			return false;
		}
		const char* app = getprogname();
		LOGD("app name is %s", app);
		if (!app) {
			LOGD("app name is empty");
			return false;
		}
		const char* renderdoc_cfg = "/data/RDC/renderdoc.cfg";
		FILE* fp = fopen(renderdoc_cfg, "r");
		if (!fp) {
			LOGD("can't open %s", renderdoc_cfg);
			return false;
		}
		bool res = false;
		while (true) {
			char line[1024] = { 0 };
			if (fgets(line, sizeof(line) - 1, fp) == NULL) {
				break;
			}
			int line_len = strlen(line);
			for (int i = 0; i < line_len; i++) {
				if ((line[i] == '\r') || (line[i] == '\n')) {
					line[i] = 0;
				}
			}
			if (strcmp(line, app) == 0) {
				res = true;
				break;
			}
		}
		fclose(fp);
		LOGD("is_loaded:%d , IsLoadRenderdoc:%d", is_loaded, res);
		return res;
	}

	void* new_OpenNativeLibrary(JNIEnv* env, int32_t target_sdk_version, const char* path, jobject class_loader, jstring library_path, bool* needs_native_bridge, std::string* error_msg) {
		if (!is_loaded && IsLoadRenderdoc()) {
#if	defined(__aarch64__)
			const char* dst = "/data/RDC/lib64/libvkEGL.so";
#else
			const char* dst = "/data/RDC/lib/libvkEGL.so";
#endif
			LOGD("---- start load %s", dst);
			LOGD("---- path:%s pid:%u %s", path, getpid(), getprogname());
			void* handle = NULL;
			handle = old_OpenNativeLibrary(env, target_sdk_version, dst, class_loader, library_path, needs_native_bridge, error_msg);
			LOGD("---- open handle:%p %s", handle, dst);
			LOGD("---- end load %s", dst);
			is_loaded = true;
		}
		else {
			LOGD("is loaded is %s", is_loaded ? "true" : "false");
		}
		return old_OpenNativeLibrary(env, target_sdk_version, path, class_loader, library_path, needs_native_bridge, error_msg);
	}

	int fake_dlclose(void *handle) {
		if (handle) {
			struct ctx *ctx = (struct ctx *) handle;
			if (ctx->dynsym) free(ctx->dynsym);    /* we're saving dynsym and dynstr */
			if (ctx->dynstr) free(ctx->dynstr);    /* from library file just in case */
			free(ctx);
		}
		return 0;
	}
	/* flags are ignored */
	void *fake_dlopen_with_path(const char *libpath, int flags) {
		FILE *maps;
		char buff[256];
		struct ctx *ctx = 0;
		off_t load_addr, size;
		int k, fd = -1, found = 0;
		char *shoff;
		Elf_Ehdr *elf = (Elf_Ehdr *)MAP_FAILED;

		maps = fopen("/proc/self/maps", "r");
		if (!maps) LOGD("failed to open maps");

		while (!found && fgets(buff, sizeof(buff), maps)) {
			if ((strstr(buff, "r-xp") || strstr(buff, "r--p")) && strstr(buff, libpath)) {
				found = 1;
				break;
			}
		}

		fclose(maps);

		if (!found) LOGD("%s not found in my userspace", libpath);

		if (sscanf(buff, "%lx", &load_addr) != 1)
			LOGD("failed to read load address for %s", libpath);

		LOGD("%s loaded in Android at 0x%08lx", libpath, load_addr);

		/* Now, mmap the same library once again */

		fd = open(libpath, O_RDONLY);
		if (fd < 0) LOGD("failed to open %s", libpath);

		size = lseek(fd, 0, SEEK_END);
		if (size <= 0) LOGD("lseek() failed for %s", libpath);

		elf = (Elf_Ehdr *)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		fd = -1;

		if (elf == MAP_FAILED) LOGD("mmap() failed for %s", libpath);

		ctx = (struct ctx *) calloc(1, sizeof(struct ctx));
		if (!ctx) LOGD("no memory for %s", libpath);

		ctx->load_addr = (void *)load_addr;
		shoff = ((char *)elf) + elf->e_shoff;

		for (k = 0; k < elf->e_shnum; k++, shoff += elf->e_shentsize) {

			Elf_Shdr *sh = (Elf_Shdr *)shoff;
			LOGD("%s: k=%d shdr=%p type=%x", __func__, k, sh, sh->sh_type);

			switch (sh->sh_type) {

			case SHT_DYNSYM:
				if (ctx->dynsym) LOGD("%s: duplicate DYNSYM sections", libpath); /* .dynsym */
				ctx->dynsym = malloc(sh->sh_size);
				if (!ctx->dynsym) LOGD("%s: no memory for .dynsym", libpath);
				memcpy(ctx->dynsym, ((char *)elf) + sh->sh_offset, sh->sh_size);
				ctx->nsyms = (sh->sh_size / sizeof(Elf_Sym));
				break;

			case SHT_STRTAB:
				if (ctx->dynstr) break;    /* .dynstr is guaranteed to be the first STRTAB */
				ctx->dynstr = malloc(sh->sh_size);
				if (!ctx->dynstr) LOGD("%s: no memory for .dynstr", libpath);
				memcpy(ctx->dynstr, ((char *)elf) + sh->sh_offset, sh->sh_size);
				break;

			case SHT_PROGBITS:
				if (!ctx->dynstr || !ctx->dynsym) break;
				/* won't even bother checking against the section name */
				ctx->bias = (off_t)sh->sh_addr - (off_t)sh->sh_offset;
				k = elf->e_shnum;  /* exit for */
				break;
			}
		}

		munmap(elf, size);
		elf = 0;

		if (!ctx->dynstr || !ctx->dynsym) LOGD("dynamic sections not found in %s", libpath);

		LOGD("%s: ok, dynsym = %p, dynstr = %p", libpath, ctx->dynsym, ctx->dynstr);

		return ctx;

	err_exit:
		if (fd >= 0) close(fd);
		if (elf != MAP_FAILED) munmap(elf, size);
		fake_dlclose(ctx);
		return 0;
	}


#if	defined(__aarch64__)
	static const char *const kSystemLibDir = "/system/lib64/";
	static const char *const kOdmLibDir = "/odm/lib64/";
	static const char *const kVendorLibDir = "/vendor/lib64/";
#elif defined(__arm__)
	static const char* const kSystemLibDir = "/system/lib/";
	static const char* const kOdmLibDir = "/odm/lib/";
	static const char* const kVendorLibDir = "/vendor/lib/";
#endif

	void *fake_dlopen(const char *filename, int flags) {
		if (strlen(filename) > 0 && filename[0] == '/') {
			return fake_dlopen_with_path(filename, flags);
		}
		else {
			char buf[512] = { 0 };
			void *handle = NULL;
			//sysmtem
			strcpy(buf, kSystemLibDir);
			strcat(buf, filename);
			handle = fake_dlopen_with_path(buf, flags);
			if (handle) {
				return handle;
			}

			//odm
			memset(buf, 0, sizeof(buf));
			strcpy(buf, kOdmLibDir);
			strcat(buf, filename);
			handle = fake_dlopen_with_path(buf, flags);
			if (handle) {
				return handle;
			}

			//vendor
			memset(buf, 0, sizeof(buf));
			strcpy(buf, kVendorLibDir);
			strcat(buf, filename);
			handle = fake_dlopen_with_path(buf, flags);
			if (handle) {
				return handle;
			}

			return fake_dlopen_with_path(filename, flags);
		}
	}

	void *fake_dlsym(void *handle, const char *name) {
		int k;
		struct ctx *ctx = (struct ctx *) handle;
		Elf_Sym *sym = (Elf_Sym *)ctx->dynsym;
		char *strings = (char *)ctx->dynstr;

		for (k = 0; k < ctx->nsyms; k++, sym++)
			if (strcmp(strings + sym->st_name, name) == 0) {
				/*  NB: sym->st_value is an offset into the section for relocatables,
				but a VMA for shared libs or exe files, so we have to subtract the bias */
				void *ret = (char *)ctx->load_addr + sym->st_value - ctx->bias;
				LOGD("%s found at %p", name, ret);
				return ret;
			}
		return 0;
	}


	int HookOpenNativeLibrary()
	{
		LOGD("Hook OpenNativeLibrary");
		void* HelperLibraryHandle = NULL;
		if ((LibraryHandle = fake_dlopen("libnativeloader.so", RTLD_NOW)) == NULL) {
			LOGD("LibraryHandle is null : %s", dlerror());
		}
		//mi 5x
		OpenNativeLibrary = (OpenNativeLibraryFunc)fake_dlsym(LibraryHandle, "_ZN7android17OpenNativeLibraryEP7_JNIEnviPKcP8_jobjectP8_jstring");
		//mi 8
		if (OpenNativeLibrary == NULL)
		{
			OpenNativeLibrary = (OpenNativeLibraryFunc)fake_dlsym(LibraryHandle, "_ZN7android17OpenNativeLibraryEP7_JNIEnviPKcP8_jobjectP8_jstringPbPNSt3__112basic_stringIcNS9_11char_traitsIcEENS9_9allocatorIcEEEE");

		}
		LOGD("OpenNativeLibrary is null : %d", OpenNativeLibrary == NULL);
		int result = hook((void*)OpenNativeLibrary, (void*)&new_OpenNativeLibrary, (void **)(&old_OpenNativeLibrary));
		return result;
	}
}

int hooked = 0;
void __attribute__((constructor)) OnLoad() {
	LOGD("OnLoad Hook");
	if (hooked) return;
	hooked = 1;
	int result = HookOpenNativeLibrary();
	LOGD("result %d", result);
}