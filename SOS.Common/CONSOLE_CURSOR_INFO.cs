using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Sequential)]
	public struct CONSOLE_CURSOR_INFO
	{
		public int Size;
		public bool Visible;
	}
}