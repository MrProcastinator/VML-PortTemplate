using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;

namespace VMLPortTemplate
{
	public class Main : Game
	{
		private GraphicsDeviceManager graphics;
		private SpriteBatch spriteBatch;

		public Main()
		{
			this.graphics = new GraphicsDeviceManager(this);
			base.Content.RootDirectory = "Content";
			base.Window.AllowUserResizing = true;
			base.IsMouseVisible = true;
			this.graphics.PreferredBackBufferWidth = 960;
			this.graphics.PreferredBackBufferHeight = 544;
			this.graphics.IsFullScreen = true;
			this.graphics.ApplyChanges();
		}

		protected override void Initialize()
		{
			base.Initialize();
		}

		protected override void LoadContent()
		{
		}

		protected override void UnloadContent()
		{
		}

		protected override void Update(GameTime gameTime)
		{
			base.Update(gameTime);
		}

		protected override void Draw(GameTime gameTime)
		{
			base.Draw(gameTime);
		}

		protected override void OnExiting(object sender, EventArgs args)
		{
			base.Exit();
		}
	}
}
