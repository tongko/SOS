using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Explicit)]
	public struct CHAR_INFO
	{
		[FieldOffset(0)]
		public char UnicodeChar;
		[FieldOffset(0)]
		public byte AsciiChar;
		[FieldOffset(2)]
		public short Attributes;
	}
	
	public struct CharInfo
	{
		public CHAR_INFO CharacterInfo;

		public bool Changed;
	}
}