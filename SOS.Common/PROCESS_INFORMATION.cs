using System;
using System.Runtime.InteropServices;

namespace SOS.Common
{
	[StructLayout(LayoutKind.Sequential)]
	public struct PROCESS_INFORMATION
	{
		public SafeHandle ProcessHandle;
		public SafeHandle ThreadHandle;
		public int ProcessId;
		public int ThreadId;
	}
}