using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SOS.Common
{
	public delegate void ShellChangeDelegate(bool resize);

	public delegate void ShellCloseDelegate();

	public struct ShellDelegates
	{
		public ShellChangeDelegate ShellChangedHandler;

		public ShellCloseDelegate ShellClosedHandler;
	}

	public struct ShellStartInfo
	{
		public string InitialDirectory;
		public string UserName;
		public string Password;
		public string InitialCommand;
		public string ShellTitle;
		public int StartupRows;
		public int StartupColumns;
	}

	interface IShellHandler
	{
		void SetupDelegates(ShellDelegates delegates);

		bool StartShellProcess(ShellStartInfo startInfo);

		int StartMonitorThread();

		void StopMonitorThread();

		void SendMouseEvent(ShellMouseEvent mouseEvent);

		void StopScrolling();

		void ResumeScrolling();

		SafeHandle ShellHandle { get; }

		SharedMemory<ShellParams> ShellParams { get; }

		SharedMemory<ShellInfo> ShellInfo { get; }

		SharedMemory<CONSOLE_CURSOR_INFO> CursorInfo { get; }

		SharedMemory<CHAR_INFO[]> ShellBuffer { get; }

		SharedMemory<ShellCopy> ShellCopy { get; }

		SharedMemory<TextInfo> TextInfo { get; }

		SharedMemory<ShellSize> NewShellSize { get; }

		SharedMemory<COORD> NewScrollPos { get; }
	}
}
