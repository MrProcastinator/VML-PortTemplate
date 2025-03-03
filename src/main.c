#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#include <vitasdk.h>

#include <VML/VML.h>
#include <VML/SDL2/SDL.h>
#include <VML/SDL2/SDL_image.h>
#include <VML/SDL2/SDL_mixer.h>
#include <VML/FAudio.h>
#include <VML/Theorafile.h>
#include <VML/FNA.h>

/* Must increment from vita C runtime */
unsigned int sceLibcHeapSize = 300 * 1024 * 1024;

#define VML_USE_OPT_PARAM
#define ASSEMBLIES_DLL_FILE			"app0:/VML/VMLPortTemplate.dll"

#define LIBFIOS2_PATH				"ur0:/data/VML/libfios2.suprx"
#define LIBC_PATH					"ur0:/data/VML/libc.suprx"
#define SUPRX_MANAGER_PATH			"ur0:/data/VML/SUPRXManager.suprx"
#define MONO_VITA_PATH				"ur0:/data/VML/mono-vita.suprx"
#define PTHREAD_PATH				"ur0:/data/VML/pthread.suprx"

extern void** mono_aot_module_VMLPortTemplate_info;

FILE* mono_log;

int tryLoadModule(const char* module)
{
	int ret = sceKernelLoadStartModule(module, 0, NULL, 0, NULL, 0);

	if (ret <= 0) {
		fprintf(mono_log, "[VMLPortTemplate] sceKernelLoadStartModule() failed for %s with code: %8x\n", module, ret);
	} else {
		fprintf(mono_log, "[VMLPortTemplate] sceKernelLoadStartModule() ran successfully for %s!\n", module);
	}

	return ret;
}

int loadModules()
{
	int ret = 0;
	int cont = 1;

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);	
	if (ret < 0) {
		fprintf(mono_log, "[VMLPortTemplate] sceSysmoduleLoadModule(SCE_SYSMODULE_NET) failed with code: %8x\n", ret);
	} else {
		fprintf(mono_log, "[VMLPortTemplate] sceSysmoduleLoadModule(SCE_SYSMODULE_NET) ran successfully!\n");
	}
	cont &= (ret >= 0);
	
	ret = tryLoadModule(SUPRX_MANAGER_PATH);
	cont &= (ret > 0);
	
	ret = tryLoadModule(PTHREAD_PATH);
	cont &= (ret > 0);

	ret = tryLoadModule(MONO_VITA_PATH);
	cont &= (ret > 0);

#ifdef USE_CUSTOM_LIBC
	ret = tryLoadModule(LIBFIOS2_PATH);
	cont &= (ret > 0);

	ret = tryLoadModule(LIBC_PATH);
	cont &= (ret > 0);
#endif

	return cont;
}

void rootEntry()
{
    VMLSDL2Register();
	VMLSDL2ImageRegister();
    VMLSDL2MixerRegister();
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

FILE* vml_port_template_log_file;
SDL_mutex* log_file_mutex;

/* Uncomment to disable logging */
#define USE_CUSTOM_LOGGING

static void SDL_CustomLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
#ifdef USE_CUSTOM_LOGGING
	if(SDL_mutexP(log_file_mutex) == -1)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't lock log mutex\n");
	if(vml_port_template_log_file)
	{
		fprintf(vml_port_template_log_file, "[%s][%s] %s\n", category >= 0 && category < SDL_LOG_CATEGORY_CUSTOM ? category_strings[category] : "CUSTOM", priority_strings[priority], message);
		fflush(vml_port_template_log_file);
	}
	if(SDL_mutexV(log_file_mutex) == -1)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't unlock log mutex\n");
	fflush(mono_log);
#endif
}

int main(int argc, char* argv[])
{
	int ret = 0;

	SDL_LogInit();
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(SDL_CustomLogFunction, NULL);

	SDL_SetHint("FNA3D_FORCE_DRIVER", "OpenGL");

	if(!(mono_log = fopen("ux0:data/VMLPortTemplate.log", "w")))
	{
		return 1;
	}

	if(!(vml_port_template_log_file = fopen("ux0:data/VMLPortTemplate-SDL.log", "w")))
	{
		return 1;
	}

	log_file_mutex = SDL_CreateMutex();

	if(!log_file_mutex)
		fprintf(mono_log, "[VMLPortTemplate] Couldn't start mutex\n");
	else
		fprintf(mono_log, "[VMLPortTemplate] Mutex started\n");

	ret = loadModules();
	if (!ret)
		return 0;

	fprintf(mono_log, "[VMLPortTemplate] Setting mono paths\n");
	VMLSetPaths("app0:VML", "app0:VML/mono/2.0/machine.config");
	fprintf(mono_log, "[VMLPortTemplate] Initializing VML\n");

	fflush(mono_log);

#ifdef VML_USE_OPT_PARAM
	VMLInitOptParam optParam;
	optParam.stackSize = SCE_KERNEL_1MiB;
	optParam.cpuAffinity = SCE_KERNEL_CPU_MASK_USER_0;
	optParam.priority = SCE_KERNEL_INDIVIDUAL_QUEUE_HIGHEST_PRIORITY;
	optParam.programName = "";
	optParam.monoVerboseDebug = true;

	ret = VMLInitialize(rootEntry, &optParam);
#else
	ret = VMLInitialize(rootEntry, NULL);
#endif

	if (ret < 0)
		fprintf(mono_log, "[VMLPortTemplate] VMLInitialize(): 0x%08X", ret);

	SDL_DestroyMutex(log_file_mutex);
	fclose(vml_port_template_log_file);
	fclose(mono_log);

	return 0;
}