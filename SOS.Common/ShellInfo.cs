using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Sequential)]
	public struct COORD
	{
		public short X;
		public short Y;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct SMALL_RECT
	{
		public short Left;
		public short Top;
		public short Right;
		public short Bottom;

		public short Width { get { return (short)(Right - Left + 1); } }

		public short Height { get { return (short)(Bottom - Top + 1); } }
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct CONSOLE_SCREEN_BUFFER_INFO
	{
		public COORD Size;
		public COORD CursorPosition;
		public short Attributes;
		public SMALL_RECT WindowRect;
		public COORD MaximumWindowSize;
	}

	public struct ShellInfo
	{
		public CONSOLE_SCREEN_BUFFER_INFO Info;
		public bool TextChanged;
	}
}