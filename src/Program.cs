using System;

static class Program
{
	[STAThread]
	static int Main()
	{
		using (Program.main = new VMLPortTemplate.Main())
		{
			Program.main.Run();
			return 0;
		}
	}

	public static VMLPortTemplate.Main main;
}
