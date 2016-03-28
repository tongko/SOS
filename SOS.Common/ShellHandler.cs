using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace SOS.Common
{
	public class ShellHandler : IShellHandler
	{
		#region Fields

		private static IntPtr _evironmentBlock;
		private static Mutex _parentProcessWatchdog;

		private ShellDelegates _shellDelegates;
		private SafeHandle _shellProcess;
		private SharedMemory<ShellParams> _shellParams;
		private SharedMemory<ShellInfo> _shellInfo;
		private SharedMemory<CONSOLE_CURSOR_INFO> _cursorInfo;
		private SharedMemory<CHAR_INFO[]> _shellBuffer;
		private SharedMemory<ShellCopy> _shellCopyInfo;
		private SharedMemory<TextInfo> _shellTextInfo;
		private SharedMemory<ShellMouseEvent> _shellMouseEvent;
		private SharedMemory<ShellSize> _newShellSize;
		private SharedMemory<COORD> _newScrollPos;

		private IntPtr _monitorThread;
		private IntPtr _monitorThreadExit;

		#endregion


		#region Properties

		public SharedMemory<CONSOLE_CURSOR_INFO> CursorInfo
		{
			get
			{
				return _cursorInfo;
			}
		}

		public SharedMemory<COORD> NewScrollPos
		{
			get
			{
				return _newScrollPos;
			}
		}

		public SharedMemory<ShellSize> NewShellSize
		{
			get
			{
				return _newShellSize;
			}
		}

		public SharedMemory<CHAR_INFO[]> ShellBuffer
		{
			get
			{
				return _shellBuffer;
			}
		}

		public SharedMemory<ShellCopy> ShellCopy
		{
			get
			{
				return _shellCopyInfo;
			}
		}

		public SafeHandle ShellHandle
		{
			get
			{
				return _shellProcess;
			}
		}

		public SharedMemory<ShellInfo> ShellInfo
		{
			get
			{
				return _shellInfo;
			}
		}

		public SharedMemory<ShellParams> ShellParams
		{
			get
			{
				return _shellParams;
			}
		}

		public SharedMemory<TextInfo> TextInfo
		{
			get
			{
				return _shellTextInfo;
			}
		}

		#endregion


		#region Methods

		private bool CreateSharedObject(int shellProcessId, string userName)
		{
			_shellParams.Create(Constants.GetFormattedText(ConstantsId.Params, shellProcessId), SyncObjectTypes.SyncObjBoth);
			_shellInfo.Create(Constants.GetFormattedText(ConstantsId.Info, shellProcessId), SyncObjectTypes.SyncObjBoth);
			_cursorInfo.Create(Constants.GetFormattedText(ConstantsId.Cursor, shellProcessId), SyncObjectTypes.SyncObjBoth);
			_shellBuffer.Create(Constants.GetFormattedText(ConstantsId.Buffer, shellProcessId), SyncObjectTypes.SyncObjBoth);

			CHAR_INFO ci = new CHAR_INFO { Attributes = 0, UnicodeChar = ' ' };

		}

		private void CreateWatchdog()
		{

		}

		private string GetModulePath(IntPtr module)
		{
			return null;
		}

		private bool InjectHook(ref PROCESS_INFORMATION pi)
		{
			return false;
		}

		private int MonitorThread()
		{
			return 0;
		}

		public void ResumeScrolling()
		{
			throw new NotImplementedException();
		}

		public void SendMouseEvent(ShellMouseEvent mouseEvent)
		{
			throw new NotImplementedException();
		}

		public void SetupDelegates(ShellDelegates delegates)
		{
			throw new NotImplementedException();
		}

		public int StartMonitorThread()
		{
			throw new NotImplementedException();
		}

		public bool StartShellProcess(ShellStartInfo startInfo)
		{
			throw new NotImplementedException();
		}

		private static int StaticMonitorThread(IntPtr lpParam)
		{
			return 0;
		}

		public void StopMonitorThread()
		{
			throw new NotImplementedException();
		}

		public void StopScrolling()
		{
			throw new NotImplementedException();
		}

		public static void UpdateEnvironmentBlock()
		{

		}

		#endregion	
	}
}
