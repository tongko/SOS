using System;
using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Explicit)]
	public struct TextInfo
	{
		[FieldOffset(0)]
		public UIntPtr Mem;
		[FieldOffset(0)]
		public long Padding;
	}
}