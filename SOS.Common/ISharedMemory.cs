using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace SOS.Common
{
	public interface ISharedMemory<T> : IDisposable
	{
		void Create(string name, SyncObjectTypes syncObjects);
		void Open(string name, SyncObjectTypes syncObjects);
		void Lock();
		void Release();

		void SignalRequest();
		void SignalResponse();

		EventWaitHandle Request { get; }
		EventWaitHandle Response { get; }
	}
}
