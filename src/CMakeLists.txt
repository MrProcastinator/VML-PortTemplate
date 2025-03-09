enable_language(ASM)

project(VMLPortTemplate CXX)

# SDL2 libraries
set(SDL2_LIBRARY "SDL2" CACHE STRING "Name of SDL2 library")
set(SDL2_IMAGE_LIBRARY "SDL2_image" CACHE STRING "Name of SDL2_image library")
set(SDL2_IMAGE_DEPENDENCIES "png jpeg webp webpdemux" CACHE STRING "Dependencies for SDL2_image library")
set(SDL2_MIXER_LIBRARY "SDL2_mixer" CACHE STRING "Name of SDL2_mixer library")
set(SDL2_MIXER_DEPENDENCIES "FLAC opusfile opus mpg123 mikmod modplug vorbisfile vorbisenc vorbis ogg xmp" CACHE STRING "Dependencies for SDL2_mixer library")
set(SDL2_TTF_LIBRARY "SDL2_ttf" CACHE STRING "Name of SDL2_ttf library")
set(SDL2_GFX_LIBRARY "SDL2_gfx" CACHE STRING "Name of SDL2_gfx library")

# Variations
set(USE_PVR OFF CACHE BOOL "Use PVR graphics backend in SDL2")
set(GL_LIBRARY "vitaGL" CACHE STRING "Name of vitaGL library")
set(VITASHARK_LIBRARY "vitashark" CACHE STRING "Name of vitashark library")

set(MONO_REFERENCES
  System.dll
  System.Core.dll
  System.Xml.dll
  mscorlib.dll
  Mono.Security.dll
  System.Configuration.dll
  System.Security.dll
  Mono.Posix.dll
)

set(VML_PORT_TEMPLATE_REFERENCES
  SDL2.dll
  FAudio-CS.dll
  Theorafile.dll
  FNA.dll
)

if(USE_PVR)
  set(VML_PORT_TEMPLATE_DEFINES VITA USE_PVR)
else()
  set(VML_PORT_TEMPLATE_DEFINES VITA)
endif()

set(VML_FILES "")
get_mono_files(VML_FILES)

set(FNA_FILES
  ${CMAKE_BINARY_DIR}/SDL2.dll VML/SDL2.dll
  ${CMAKE_BINARY_DIR}/FAudio-CS.dll VML/FAudio-CS.dll
  ${CMAKE_BINARY_DIR}/Theorafile.dll VML/Theorafile.dll
  ${CMAKE_BINARY_DIR}/FNA.dll VML/FNA.dll
)

set(VML_PORT_TEMPLATE_CONTENT_FILES
  # ${CMAKE_CURRENT_SOURCE_DIR}/Content/example.xnb Content/example.xnb
)

set(VML_PORT_TEMPLATE_SOURCES
  "Main.cs"
  "Program.cs"
  "Debug.cs" # For internal debugging purposes
)

separate_arguments(SDL2_IMAGE_DEPENDENCIES)
separate_arguments(SDL2_MIXER_DEPENDENCIES)

compile_mono_assembly_aot(
  SOURCES ${VML_PORT_TEMPLATE_SOURCES}
  ASSEMBLY ${PROJECT_NAME}
  REFERENCES ${VML_PORT_TEMPLATE_REFERENCES}
  DEFINES ${VML_PORT_TEMPLATE_DEFINES}
)

add_executable(${PROJECT_NAME}
  main.c
  ${PROJECT_NAME}.dll.s
)

add_dependencies(${PROJECT_NAME} ExternalVMLFNA)

# VML libraries
target_link_libraries(${PROJECT_NAME} VitaMonoLoader VMLCoreAssemblies)
target_link_libraries(${PROJECT_NAME} VMLSDL2 VMLFAudio VMLTheorafile VMLFNA)

# FNA dependencies
target_link_libraries(${PROJECT_NAME} FAudio theorafile FNA3D mojoshader)

# SDL and opengl
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${GL_LIBRARY})

# newlib and utils
target_link_libraries(${PROJECT_NAME} m z supc++)
target_link_libraries(${PROJECT_NAME} mathneon)

# stubs
target_link_libraries(${PROJECT_NAME} mono-vita_stub PTHREAD_PRX_stub SUPRXManager_stub_weak)
target_link_libraries(${PROJECT_NAME} SceLibDbg_stub SceAudio_stub SceAudioIn_stub SceCtrl_stub ScePower_stub SceMotion_stub SceHid_stub SceTouch_stub SceGxm_stub SceCommonDialog_stub SceDisplay_stub)
target_link_libraries(${PROJECT_NAME} SceSysmodule_stub SceAppMgr_stub SceAppUtil_stub SceLibKernel_stub SceSysmem_stub SceIofilemgr_stub SceRtc_stub SceAppUtil_stub SceIme_stub SceNet_stub SceKernelThreadMgr_stub SceLibcMonoBridge_stub SceFios2_stub SceLibKernel_stub)

# vita shark and low level stubs
target_link_libraries(${PROJECT_NAME} ${VITASHARK_LIBRARY} SceShaccCgExt)
target_link_libraries(${PROJECT_NAME} taihen_stub SceShaccCg_stub SceKernelDmacMgr_stub SceKernelModulemgr_stub)

set(VITA_APP_NAME "VML Port Template")
set(VITA_TITLEID  "VMLPORTTP")
set(VITA_VPKNAME  "VmlPortTemplate")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE STRIPPED)
vita_create_vpk(${VITA_VPKNAME}.vpk ${VITA_TITLEID} eboot.bin
VERSION ${VITA_VERSION}
NAME ${VITA_APP_NAME}
  FILE 
    # VPK files
    ${CMAKE_SOURCE_DIR}/vpk/sce_sys/icon0.png sce_sys/icon0.png
    ${CMAKE_SOURCE_DIR}/vpk/sce_sys/pic0.png sce_sys/pic0.png
    ${CMAKE_SOURCE_DIR}/vpk/sce_sys/startup.png sce_sys/livearea/contents/startup.png
    ${CMAKE_SOURCE_DIR}/vpk/sce_sys/bg.png sce_sys/livearea/contents/bg.png
    ${CMAKE_SOURCE_DIR}/vpk/sce_sys/template.xml sce_sys/livearea/contents/template.xml
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dll VML/${PROJECT_NAME}.dll
    ${VML_PORT_TEMPLATE_CONTENT_FILES}
    ${FNA_FILES}
    ${VML_FILES}
)

# Optional: used for "send" target
set(PSVITA_IP "" CACHE STRING "PSVita IP (for FTP access)")
set(VITA3K_PATH "" CACHE STRING "Vita3K preferred folder path")

add_custom_target(send
  COMMAND echo destroy | nc ${PSVITA_IP} 1338
  COMMAND curl -T eboot.bin ftp://${PSVITA_IP}:1337/ux0:/app/${VITA_TITLEID}/
  COMMAND echo launch ${VITA_TITLEID} | nc ${PSVITA_IP} 1338
  DEPENDS ${VITA_VPKNAME}.vpk-vpk
)

add_custom_target(send_kvdb
  COMMAND echo destroy | nc ${PSVITA_IP} 1338
  COMMAND curl -T eboot.bin ftp://${PSVITA_IP}:1337/ux0:/app/${VITA_TITLEID}/
  COMMAND echo launch ux0:/app/${VITA_TITLEID}/eboot.bin | nc ${PSVITA_IP} 1338
  DEPENDS ${VITA_VPKNAME}.vpk-vpk
)

add_custom_target(send_v3k
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/Mono.Posix.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/Mono.Posix.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/Mono.Security.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/Mono.Security.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/System.Configuration.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/System.Configuration.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/System.Core.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/System.Core.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/System.Security.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/System.Security.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/System.Xml.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/System.Xml.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/System.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/System.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${SFV_FOLDER}/Tools/MonoPSP2/mscorlib.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/mscorlib.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/SDL2.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/SDL2.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/FAudio-CS.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/FAudio-CS.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/Theorafile.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/Theorafile.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/FNA.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/FNA.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dll
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/VML/${PROJECT_NAME}.dll
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/eboot.bin
    ${VITA3K_PATH}/ux0/app/${VITA_TITLEID}/eboot.bin
  DEPENDS 
    ${VITA_VPKNAME}.vpk-vpk
    ${CMAKE_BINARY_DIR}/SDL2.dll
    ${CMAKE_BINARY_DIR}/FAudio-CS.dll
    ${CMAKE_BINARY_DIR}/Theorafile.dll
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dll
    ${CMAKE_CURRENT_BINARY_DIR}/eboot.bin
)

add_custom_target(reboot
  COMMAND echo reboot | nc ${PSVITA_IP} 1338
)