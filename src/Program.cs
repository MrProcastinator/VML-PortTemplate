using System;
using System.Collections.Generic;
using System.IO;
using SDL2;

static class Program
{
	[STAThread]
	static int Main()
	{
		try
		{
			using (Program.main = new VMLPortTemplate.Main())
			{
				Program.main.Run();
			}
		}
		catch (Exception ex)
		{
			var message = ex.GetType().FullName + ":" + ex.Message;
			SDL.SDL_ShowSimpleMessageBox(SDL.SDL_MessageBoxFlags.SDL_MESSAGEBOX_ERROR, "Unexpected Mono exception", message, IntPtr.Zero);
			SDL.SDL_LogError((int)SDL.SDL_LogCategory.SDL_LOG_CATEGORY_APPLICATION, message);
			SDL.SDL_LogError((int)SDL.SDL_LogCategory.SDL_LOG_CATEGORY_APPLICATION, ex.StackTrace);
		}
		return 0;
	}

	public static VMLPortTemplate.Main main;
}
