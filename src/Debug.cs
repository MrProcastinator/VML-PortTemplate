using System;
using SDL2;

namespace VitaSdk.Tools
{
    public static class Debug
    {
        public static void WriteLine(string str)
        {
            SDL.SDL_LogMessage((int)SDL.SDL_LogCategory.SDL_LOG_CATEGORY_APPLICATION, SDL.SDL_LogPriority.SDL_LOG_PRIORITY_DEBUG, str);
        }
    }
}