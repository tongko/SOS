using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SOS.Common
{
	internal enum ConstantsId
	{
		Params,
		Info,
		Cursor,
		Buffer,
		CopyInfo,
		TextInfo,
		Mouse,
		NewShell,
		NewScroll,
		Watchdog,
		ErrorOpen,
		ErrorMap,
		ErrorReq,
		ErrorResp,
		ReqSet,
		RespSet,
		InfoMutex,
		InfoReq,
		InfoResp
	}

	internal static class Constants
	{
		private const string ShellParamsName = "SOS_params_{0}";
		private const string InfoName = "SOS_shellInfo_{0}";
		private const string CursorInfoName = "SOS_cursorInfo_{0}";
		private const string BufferName = "SOS_shellBuffer_{0}";
		private const string CopyInfoName = "SOS_shellCopyInfo_{0}";
		private const string TextInfoName = "SOS_shellTextInfo_{0}";
		private const string MouseEventName = "SOS_shellMouseEvent_{0}";
		private const string NewShellSizeName = "SOS_newShellSize_{0}";
		private const string NewScrollPosName = "SOS_newScrollPos_{0}";
		private const string WatchdogName = "Local\\SOS_parentProcessExit_{0}";

		private const string ErrOpenSharedMem = "Error occurs attempt to open shared memory {0}, error: {1}\n";
		private const string ErrMapSharedMem = "Error occurs attempt to map shared memory {0}, error: {1}\n";
		private const string ErrReqEventNull = "Request event '{0}' is null.";
		private const string ErrRespEventNull = "Response event '%!%' is null.";
		private const string ReqSetEventFailed = "Error occurs attempt to SetEvent for Request {0}, error: {1}\n";
		private const string RespSetEventFailed = "Error occurs attempt to SetEvent for Response {0}, error: {1}\n";

		private const string InfoMutex = "m_sharedMutex {0}: {1}\n";
		private const string InfoReqEvent = "m_sharedReqEvent {0}: {1}\n";
		private const string InfoRespEvent = "m_sharedRespEvent {0}: {1}\n";

		public static string GetFormattedText(ConstantsId id, params object[] args)
		{
			string format;

			switch (id)
			{
				case ConstantsId.Params:
					format = ShellParamsName;
					break;
				case ConstantsId.Info:
					format = InfoName;
					break;
				case ConstantsId.Cursor:
					format = CursorInfoName;
					break;
				case ConstantsId.Buffer:
					format = BufferName;
					break;
				case ConstantsId.CopyInfo:
					format = CopyInfoName;
					break;
				case ConstantsId.TextInfo:
					format = TextInfoName;
					break;
				case ConstantsId.Mouse:
					format = MouseEventName;
					break;
				case ConstantsId.NewShell:
					format = NewShellSizeName;
					break;
				case ConstantsId.NewScroll:
					format = NewScrollPosName;
					break;
				case ConstantsId.Watchdog:
					format = WatchdogName;
					break;
				case ConstantsId.ErrorOpen:
					format = ErrOpenSharedMem;
					break;
				case ConstantsId.ErrorMap:
					format = ErrMapSharedMem;
					break;
				case ConstantsId.ErrorReq:
					format = ErrReqEventNull;
					break;
				case ConstantsId.ErrorResp:
					format = ErrRespEventNull;
					break;
				case ConstantsId.ReqSet:
					format = ReqSetEventFailed;
					break;
				case ConstantsId.RespSet:
					format = RespSetEventFailed;
					break;
				case ConstantsId.InfoMutex:
					format = InfoMutex;
					break;
				case ConstantsId.InfoReq:
					format = InfoReqEvent;
					break;
				case ConstantsId.InfoResp:
					format = InfoRespEvent;
					break;
				default:
					format = string.Empty;
					break;
			}

			return string.Format(format, args);
		}
	}
}
