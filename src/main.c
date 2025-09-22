#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#include <vitasdk.h>
#include <vitaGL.h>

#include <VML/VML.h>
#include <VML/SDL2/SDL.h>
#include <VML/FAudio.h>
#include <VML/Theorafile.h>
#include <VML/FNA.h>

#include "dialog.h"

#ifdef VML_DEBUG
#define MONO_VERBOSE_DEBUG true
#else
#define MONO_VERBOSE_DEBUG false
#endif

#define IS_LOADED(x) ((x) > 0 || (x) == SCE_KERNEL_ERROR_MODULEMGR_OLD_LIB || (x) == SCE_KERNEL_ERROR_MODULEMGR_IN_USE)

#define VML_USE_OPT_PARAM
#define ASSEMBLIES_DLL_FILE			"app0:/VML/VMLPortTemplate.dll"

#define LIBFIOS2_PATH				"ur0:/sys/external/libfios2.suprx"
#define LIBC_PATH					"ur0:/sys/external/libc.suprx"
#define SUPRX_MANAGER_PATH			"ur0:/data/VML/SUPRXManager.suprx"
#define MONO_VITA_PATH				"ur0:/data/VML/mono-vita.suprx"
#define PTHREAD_PATH				"ur0:/data/VML/pthread.suprx"

extern void** mono_aot_module_VMLPortTemplate_info;

FILE* mono_log;

int tryLoadCoreModule(const char* module)
{
	int ret = sceKernelLoadStartModule(module, 0, NULL, 0, NULL, 0);

	if (!IS_LOADED(ret)) {
		fatal_error("[VMLPortTemplate] sceKernelLoadStartModule() failed for %s with code: %8x\n", module, ret);
	} 

	return ret;
}

int loadCoreModules()
{
	int ret = 0;
	int cont = 1;

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	if (ret < 0) {
		fatal_error("[VMLPortTemplate] sceSysmoduleLoadModule(SCE_SYSMODULE_NET) failed with code: %8x\n", ret);
	}
	cont &= (ret >= 0);
	
#ifdef USE_CUSTOM_LIBC
	ret = tryLoadCoreModule(LIBFIOS2_PATH);
	cont &= IS_LOADED(ret);

	ret = tryLoadCoreModule(LIBC_PATH);
	cont &= IS_LOADED(ret);
#endif

	return cont;
}

int tryLoadModule(const char* module)
{
	int ret = sceKernelLoadStartModule(module, 0, NULL, 0, NULL, 0);

	if (!IS_LOADED(ret)) {
		fatal_error("[VMLPortTemplate] sceKernelLoadStartModule() failed for %s with code: %8x\n", module, ret);
	} else {
		fprintf(mono_log, "[VMLPortTemplate] sceKernelLoadStartModule() ran successfully for %s!\n", module);
	}

	return ret;
}

int loadModules()
{
	int ret = 0;
	int cont = 1;
	
	ret = tryLoadModule(SUPRX_MANAGER_PATH);
	cont &= IS_LOADED(ret);
	
	ret = tryLoadModule(PTHREAD_PATH);
	cont &= IS_LOADED(ret);

	ret = tryLoadModule(MONO_VITA_PATH);
	cont &= IS_LOADED(ret);

	return cont;
}

void rootEntry()
{
    VMLSDL2Register();
    VMLFNAFAudioRegister();
    VMLFNATheorafileRegister();
    VMLFNARegister();

	int ret = VMLRunMain(ASSEMBLIES_DLL_FILE, mono_aot_module_VMLPortTemplate_info);
	fprintf(mono_log, "[VMLPortTemplate] Program::Main() returned %d\n", ret);
}

static const char* priority_strings[] = {
	"",
	"VERBOSE"
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRITICAL",
	"RESERVED1",
	"RESERVED2",
	"RESERVED3",
	"RESERVED4",
	"RESERVED5",
	"RESERVED6",
	"RESERVED7",
	"RESERVED8",
	"RESERVED9",
	"RESERVED10",
    ""
};

static const char* category_strings[] = {
	"APPLICATION",
    "ERROR",
    "ASSERT",
    "SYSTEM",
    "AUDIO",
    "VIDEO",
    "RENDER",
    "INPUT",
    "TEST",
};

SDL_mutex* log_file_mutex;

/* Comment to disable logging */
#define USE_CUSTOM_LOGGING

static void SDL_CustomLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
#ifdef USE_CUSTOM_LOGGING
	if(SDL_mutexP(log_file_mutex) == -1)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't lock log mutex\n");
	if(mono_log)
	{
		fprintf(mono_log, "[%s][%s] %s\n", category >= 0 && category < SDL_LOG_CATEGORY_CUSTOM ? category_strings[category] : "CUSTOM", priority_strings[priority], message);
		fflush(mono_log);
	}
	if(SDL_mutexV(log_file_mutex) == -1)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't unlock log mutex\n");
	fflush(mono_log);
#endif
}

int main(int argc, char* argv[])
{
	int ret = 0;

	/* Need to load libc first */
	ret = loadCoreModules();
	if (!ret)
		return 0;

	if(!(mono_log = fopen("ux0:data/VMLPortTemplate.log", "w")))
	{
		return 1;
	}

	ret = loadModules();
	if (!ret)
		return 0;
	fflush(mono_log);

	SDL_LogInit();
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(SDL_CustomLogFunction, NULL);

	SDL_SetHint("FNA3D_FORCE_DRIVER", "OpenGL");
	SDL_SetHint("FNA3D_FORCE_MODES", "960x544");

	fprintf(mono_log, "[VMLPortTemplate] Creating log mutex\n");
	fflush(mono_log);
	log_file_mutex = SDL_CreateMutex();

	if(!log_file_mutex)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't start mutex\n");
	else
		fprintf(mono_log, "[VMLPortTemplate] Mutex started\n");
	fflush(mono_log);

	fprintf(mono_log, "[VMLPortTemplate] Setting mono paths\n");
	VMLSetPaths("app0:VML", "app0:VML/mono/2.0/machine.config");
	fprintf(mono_log, "[VMLPortTemplate] Setting command line args\n");
	VMLSetCommandLineArgs(argc, argv);
	fprintf(mono_log, "[VMLPortTemplate] Initializing VML\n");

	fflush(mono_log);

#ifdef VML_USE_OPT_PARAM
	VMLInitOptParam optParam;
	optParam.stackSize = SCE_KERNEL_1MiB;
	optParam.cpuAffinity = SCE_KERNEL_CPU_MASK_USER_0;
	optParam.priority = SCE_KERNEL_INDIVIDUAL_QUEUE_HIGHEST_PRIORITY;
	optParam.programName = "";
	optParam.monoVerboseDebug = MONO_VERBOSE_DEBUG;

	ret = VMLInitialize(rootEntry, &optParam);
#else
	ret = VMLInitialize(rootEntry, NULL);
#endif

	if (ret < 0)
		fprintf(mono_log, "[VMLPortTemplate] VMLInitialize(): 0x%08X", ret);

	SDL_DestroyMutex(log_file_mutex);
	fclose(mono_log);

	return 0;
}