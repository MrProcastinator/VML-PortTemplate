# VitaMonoLoader FNA port template

This is a template to port a C# FNA game, using [VitaMonoLoader for vitasdk](https://github.com/MrProcastinator/VitaMonoLoader-vitasdk) tools. In order to start porting, you must have:
- The original C# source code of the game to port
- All C# library dependencies source code needed to run the game
- All native libraries source code for library dependencies, or a version compiled for arm-vita-eabi and loadable as a .suprx
- Have met all requirements from [this README.md](https://github.com/MrProcastinator/VitaMonoLoader-vitasdk/blob/master/README.md)
- Have all these mono libraries compiled and installed in your toolchain (in order): [SDL2-CS](https://github.com/MrProcastinator/VML-SDL2), [FNA3D](https://github.com/MrProcastinator/VML-FNA3D), [Theorafile](https://github.com/MrProcastinator/VML-TheoraFile), [FAudio](https://github.com/MrProcastinator/VML-FAudio), [FNA](https://github.com/MrProcastinator/VML-FNA).
- Have all these native vita libraries compiled and installed in your toolchain: SDL2, SDL2_image (and dependencies), SDL2_mixer (and dependencies), SDL2_ttf (and dependencies), SDL2_gfx, vitaGL, vitashark

**Currently, if you don't count with all these requirements, you can't fully port a FNA game.**

## Basic setup guide

1. Put the source code of your XNA game inside the src folder (check the folder where the .sln file resides)
2. Put the source code of any C# library needed as a dependency inside the lib folder (outside of FNA and native Mono libraries).
3. Modify the CMakeLists.txt with the following changes:

   a. Change the name of the project from VMLTemplate-CS to the name of your game. E.g.: AwesomeShmupGame-CS
   
   b. If your project contains external C# libraries, uncomment the line with `add_subdirectory(lib)` to allow library compilation

   c. If any C# library requires a dependency from the mono libraries, add a new CMake target and add a `compile_mono_external_import` declaration for every assembly needed.
   E.g: if your library is XmlSharp.dll and it needs System.Text.dll as a dependency, add the following lines:

    ```csharp
    add_custom_target(ExternalVMLXmlSharp)

    # XmlSharp imports
    compile_mono_external_import(
        ASSEMBLY System.Text.dll
        TARGET ExternalVMLXmlSharp
    )
    ```
4. Modify the src/CMakeLists.txt with the following changes:

   a. Change the name of the project from VMLTemplate to the name of your game. E.g.: AwesomeShmupGame   
   
   b. Add your C# files in the `VML_PORT_TEMPLATE_SOURCES` variable line by line, with full relative path from the src folder

   c. Add your content files in the `VML_PORT_TEMPLATE_CONTENT_FILES`, 

   d. Add any C# compiler custom definition to `VML_PORT_TEMPLATE_DEFINES` if needed

   e. Modify all vpk settings at `VITA_APP_NAME`, `VITA_TITLEID`, `VITA_VPKNAME`, and `VITA_VERSION` as needed

5. If your project contains C# libraries, create a copy of src/CMakelists.txt up to the `add_executable` call, and modify according to your library compilation needs (and change `add_executable` to `add_library`)

6. If you have native library dependencies that are being used in C# (through `[MethodImpl(MethodImplOptions.InternalCall)]`), create the necessary bindings.

NOTE: you may need to do some changes to the original source code, as Mono for vita only supports .NET 2.0 features.

## Folder structure

When implementing an FNA game for PSVita, in order to avoid problems when compiling and working in changes on your libraries and game files, the game should follow this folder structure:
```
app0:
    eboot.bin
    /VML
        /mono
            /2.0
                machine.config
        YourGame.dll
        FAudio-CS.dll
        FNA.dll
        SDL2.dll
        Theorafile.dll
        Mono.Posix.dll
        Mono.Security.dll
        System.Configuration.dll
        System.Core.dll
        System.Security.dll
        System.Xml.dll
        System.dll
        mscorlib.dll
```
Using shared folders in ux0:/data/ is not recommended due to possible problems related with dependency library timestamp errors. 

## Build configuration

This template provides a set of CMake variables to allow build customization according to your development environment.

### VitaMonoLoader specific variables

| **CMake Variable**        | **Description**
|:-------------------|:------------------------------------------
|`SFV_FOLDER`         | Unity Support for Vita installation folder (mandatory)
|`MONO_PATH` | Unity Support for Vita Mono library path (mandatory, should be inside Tools/MonoPSP2)
|`MONO_LIB_DLL_PATH` | Path to the vitasdk include folder containing VML initialization headers (default: lib/mono)
|`MONO_LIB_VML_PATH` | Path to the vitasdk lib folder containing VML binding libraries (default: lib)

### FNA specific variables

| **CMake Variable**        | **Description**
|:-------------------|:------------------------------------------
|`SDL2_LIBRARY`         | Name of SDL2 library (default: SDL2)
|`SDL2_IMAGE_LIBRARY`         | Name of SDL2_image library (default: SDL2_image)
|`SDL2_IMAGE_DEPENDENCIES`         | Dependencies for SDL2_image library (default: png jpeg webp webpdemux)
|`SDL2_MIXER_LIBRARY`         | Name of SDL2 library (default: SDL2_mixer)
|`SDL2_MIXER_DEPENDENCIES`         | Dependencies for SDL2_mixer library (default: FLAC opusfile opus mpg123 mikmod modplug vorbisfile vorbisenc vorbis ogg xmp)
|`SDL2_TTF_LIBRARY`         | Name of SDL2_ttf library (default: SDL2_ttf)
|`SDL2_GFX_LIBRARY`         | Name of SDL2_gfx library (default: SDL2_gfx)
|`USE_PVR`         | Use PVR graphics backend in SDL2 (default: OFF)
|`GL_LIBRARY`         | Name of vitaGL library (default: vitaGL)
|`VITASHARK_LIBRARY`         | Name of vitashark library (default: vitashark)

### Misc variables

| **CMake Variable**        | **Description**
|:-------------------|:------------------------------------------
|`VITA3K_PATH`         | Path to the Vita3K preferred folder path (optional)
|`PSVITA_IP`         | PSVita IPv4 address for FTP access (optional)
|`FOR_VITA3K`         | Indicates whether this build will be executed on Vita3K or a physical PSVita (optional, default: OFF)

## Tips

- If you want to disable logging (e.g. for better perfomance), you can comment the line defining `USE_CUSTOM_LOGGING` in main.c, to disable all FNA (and project debug) logging.
- If you need to recompile to add some changes and test in Vita3K, you can use the make target `send_v3k` to send all project files (without contents) to the installation folder instead of opening the .vpk file

## Known caveats

- Sometimes, depending on the number of dependencies and size of AOT compiled code used, the resulting eboot.bin file may either freeze on Vita3K (at the `[load_module]: Module SceLibFios2 (at "vs0:sys/external/libfios2.suprx") loaded` line) or crash with a core dump on a real PSVita. The only known solution for this problem is to reduce the number of dependencies in order to decrease the final executable size (I recommend disabling SDL2 image and mixer unused features and only focus on FNA compatibility)
- Due to vitaGL not implementing all GLES2 features, the following FNA graphical features are not supported:

    - Occlusion queries
    
    - SpriteBatch blending

    - Multiple render targets

    - Compressed XNB resource textures (if present they must be converted to uncompressed formats)

    Using any of this features by default will display an error on log, but it shouldn't stop operation

- **FNA compatibility is still work in progress, expect some updates for any of the FNA dependencies**

## TODO

- Create a wiki to document everything needed to start porting