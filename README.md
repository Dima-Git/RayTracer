# RayTracer
Simple (or not really) SDL based C++11 ray tracer.

## Linux setup
1. Install libsdl2-dev package provided by your distribution
2. Run make

# Windows setup
1. Download SDL2 development and runtime libraries (check out www.libsdl.org)
2. Make sure to include SDL headers to your INCLUDE_PATH environment variable folder so u can use include like <SDL2/SDL.h>, or configure VS project include directories manually
3. The same with x86 static libs, either add x86 folder to your LIBSDL_PATH environment variable, or configure VS project library directories manually
4. Make sure to place SDL2.dll somewhere, so executable can find it. Either Release folder or system32 folder

For newest SDL builds fo VS check out https://buildbot.libsdl.org/sdl-builds/sdl-visualstudio/
