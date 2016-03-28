using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Sequential)]
	public struct ShellMouseEvent
	{
		public COORD Position;
		public int ButtonState;
		public int ControlKeyState;
		public int EventFlags;
	}
}