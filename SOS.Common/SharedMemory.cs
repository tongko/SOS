using System;
using System.Diagnostics;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;
using System.Threading;

namespace SOS.Common
{
	public sealed class SharedMemory<T> : ISharedMemory<T> where T : 
	{
		#region Fields

		private string _name;
		private bool _disposedValue = false; // To detect redundant calls
		private MemoryMappedFile _hSharedMem;
		private MemoryMappedViewAccessor _pSharedMem;
		private Mutex _hSharedMutex;
		private EventWaitHandle _hSharedReqEvent;
		private EventWaitHandle _hSharedRespEvent;

		#endregion


		#region Constructors

		public SharedMemory()
		{
			_name = string.Empty;
			//Size = 0;
			//Handle = Pointer = Mutex = RequestEvent = ResponseEvent = IntPtr.Zero;
		}

		public SharedMemory(string name, uint size, SyncObjectTypes syncObjects, bool create = true)
		{
			//if (create)
			//	Create(name, size, syncObjects, null);
			//else
			//	Open(name, syncObjects);
		}

		#endregion


		#region Methods

		public void Create(string name, SyncObjectTypes syncObjects)
		{
			if (string.IsNullOrWhiteSpace(name))
				throw new ArgumentNullException("name");
			_hSharedMem = MemoryMappedFile.CreateNew(name, Marshal.SizeOf(typeof(T)));
			_pSharedMem = _hSharedMem.CreateViewAccessor(0, 0, MemoryMappedFileAccess.ReadWrite);
			var empty = default(T);
			_pSharedMem.Write(0, ref empty);

			if (syncObjects > SyncObjectTypes.SyncObjNone)
				CreateSyncObjects(syncObjects, name);
		}

		private void CreateSyncObjects(SyncObjectTypes syncObjects, string name)
		{
			if (syncObjects >= SyncObjectTypes.SyncObjRequest)
			{
				_hSharedMutex = new Mutex(false, name + "_mutex");
				Debug.WriteLine("_hSharedMutex '{0}': {1}", name, _hSharedMutex.SafeWaitHandle.DangerousGetHandle().ToInt64());
				_hSharedReqEvent = new EventWaitHandle(false, EventResetMode.AutoReset, name + "_req_event");
				Debug.WriteLine("_hSharedReqEvent '{0}': {1}", name, _hSharedReqEvent.SafeWaitHandle.DangerousGetHandle().ToInt64());
			}

			if (syncObjects >= SyncObjectTypes.SyncObjBoth)
			{
				_hSharedRespEvent = new EventWaitHandle(false, EventResetMode.AutoReset, name + "_resp_event");
				Debug.WriteLine("_hSharedRespEvent '{0}': {1}", name, _hSharedRespEvent.SafeWaitHandle.DangerousGetHandle().ToInt64());
			}
		}

		private void Dispose(bool disposing)
		{
			if (!_disposedValue)
			{
				if (disposing)
				{
					// TODO: dispose managed state (managed objects).
					if (_hSharedMutex != null)
						_hSharedMutex.Dispose();
					if (_hSharedReqEvent != null)
						_hSharedReqEvent.Dispose();
					if (_hSharedRespEvent != null)
						_hSharedRespEvent.Dispose();
					if (_pSharedMem != null)
						_pSharedMem.Dispose();
					if (_hSharedMem != null)
						_hSharedMem.Dispose();
				}

				// TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
				// TODO: set large fields to null.

				_disposedValue = true;
			}
		}

		// TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
		// ~SharedMemory() {
		//   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
		//   Dispose(false);
		// }

		// This code added to correctly implement the disposable pattern.
		public void Dispose()
		{
			// Do not change this code. Put cleanup code in Dispose(bool disposing) above.
			Dispose(true);
			// TODO: uncomment the following line if the finalizer is overridden above.
			// GC.SuppressFinalize(this);
		}

		public void Lock()
		{
			if (_hSharedMutex == null)
				return;

			_hSharedMutex.WaitOne();
		}

		public void Open(string name, SyncObjectTypes syncObjects)
		{
			if (string.IsNullOrWhiteSpace(name))
				throw new ArgumentNullException("name");

			_name = name;
			_hSharedMem = MemoryMappedFile.OpenExisting(name, MemoryMappedFileRights.FullControl, System.IO.HandleInheritability.None);
			_pSharedMem = _hSharedMem.CreateViewAccessor(0, 0, MemoryMappedFileAccess.ReadWrite);

			if (syncObjects > SyncObjectTypes.SyncObjNone)
				CreateSyncObjects(syncObjects, name);
		}

		public void Release()
		{
			if (_hSharedMutex == null)
				return;

			_hSharedMutex.ReleaseMutex();
		}

		public void SignalRequest()
		{
			if (_hSharedReqEvent == null)
				return;

			_hSharedReqEvent.Set();
		}

		public void SignalResponse()
		{
			_hSharedRespEvent.Set();
		}


		#endregion


		#region Properties

		public EventWaitHandle Request { get { return _hSharedReqEvent; } }

		public EventWaitHandle Response { get { return _hSharedRespEvent; } }


		#endregion
	}
}
