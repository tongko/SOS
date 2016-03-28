using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Explicit)]
	public struct ShellParams
	{
		[FieldOffset(0)]
		public int ShellMainThreadId;
		[FieldOffset(4)]
		public int ParentProcessId;
		[FieldOffset(8)]
		public int NotificationTimeout;
		[FieldOffset(12)]
		public int RefreshInterval;
		[FieldOffset(16)]
		public int Rows;
		[FieldOffset(20)]
		public int Columns;
		[FieldOffset(24)]
		public int BufferRows;
		[FieldOffset(28)]
		public int BufferColumns;
		[FieldOffset(32)]
		public int MaxRows;
		[FieldOffset(36)]
		public int MaxColumns;
		[FieldOffset(40)]
		public IntPtr ShellWindow;
		[FieldOffset(40)]
		internal long Padding;
		[FieldOffset(48)]
		public int HookThreadId;
	}
}
