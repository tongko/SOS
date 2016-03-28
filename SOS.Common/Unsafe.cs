using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SOS.Common
{
	internal static class Unsafe
	{
		private const uint GENERIC_ALL = 0x10000000;
		private const uint NO_INHERITANCE = 0x00;
		private const uint SECURITY_CREATOR_OWNER_RID = 0x00;
		private const uint ERROR_SUCCESS = 0x00;
		private const uint SECURITY_DESCRIPTOR_REVISION = 0x01;

		public const uint INFINITE = 0xFFFFFFFF;  // Infinite timeout
		public const uint WAIT_ABANDONED = 0x80;
		public const uint WAIT_OBJECT_0 = 0x00;
		public const uint WAIT_TIMEOUT = 0x102;
		public const uint WAIT_FAILED = 0xFFFFFFFF;

		public static readonly IntPtr INVALID_HANDLE_VALUE = new IntPtr(-1);
		private static readonly byte[] SECURITY_CREATOR_SID_AUTHORITY = new byte[] { 0, 0, 0, 0, 0, 3 };

		public enum EventFlags
		{
			PULSE = 1,
			RESET = 2,
			SET = 3
		}

		private enum ACCESS_MODE : uint
		{
			NOT_USED_ACCESS = 0,
			GRANT_ACCESS,
			SET_ACCESS,
			DENY_ACCESS,
			REVOKE_ACCESS,
			SET_AUDIT_SUCCESS,
			SET_AUDIT_FAILURE
		}

		private enum MULTIPLE_TRUSTEE_OPERATION
		{
			NO_MULTIPLE_TRUSTEE,
			TRUSTEE_IS_IMPERSONATE
		}

		private enum TRUSTEE_FORM
		{
			TRUSTEE_IS_SID,
			TRUSTEE_IS_NAME,
			TRUSTEE_BAD_FORM,
			TRUSTEE_IS_OBJECTS_AND_SID,
			TRUSTEE_IS_OBJECTS_AND_NAME
		}

		private enum TRUSTEE_TYPE
		{
			TRUSTEE_IS_UNKNOWN,
			TRUSTEE_IS_USER,
			TRUSTEE_IS_GROUP,
			TRUSTEE_IS_DOMAIN,
			TRUSTEE_IS_ALIAS,
			TRUSTEE_IS_WELL_KNOWN_GROUP,
			TRUSTEE_IS_DELETED,
			TRUSTEE_IS_INVALID,
			TRUSTEE_IS_COMPUTER
		}

		[Flags]
		internal enum FileMapProtection : uint
		{
			PageReadonly = 0x02,
			PageReadWrite = 0x04,
			PageWriteCopy = 0x08,
			PageExecuteRead = 0x20,
			PageExecuteReadWrite = 0x40,
			SectionCommit = 0x8000000,
			SectionImage = 0x1000000,
			SectionNoCache = 0x10000000,
			SectionReserve = 0x4000000,
		}

		//Platform independent (32 & 64 bit) - use Pack = 0 for both platforms. IntPtr works as well.
		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto, Pack = 0)]
		private struct TRUSTEE : IDisposable
		{
			public IntPtr pMultipleTrustee;
			public MULTIPLE_TRUSTEE_OPERATION MultipleTrusteeOperation;
			public TRUSTEE_FORM TrusteeForm;
			public TRUSTEE_TYPE TrusteeType;
			public IntPtr ptstrName;

			void IDisposable.Dispose()
			{
				if (ptstrName != IntPtr.Zero) Marshal.Release(ptstrName);
			}

			public string Name { get { return Marshal.PtrToStringAuto(ptstrName); } }
		}

		private struct SID_IDENTIFIER_AUTHORITY
		{
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
			public byte[] Value;

			public SID_IDENTIFIER_AUTHORITY(byte[] value)
			{
				Value = value;
			}
		}

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
		private struct ACL
		{
			public byte AclRevision;
			public byte Sbz1;
			public ushort AclSize;
			public ushort AceCount;
			public ushort sbz2;
		}

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto, Pack = 0)]
		private struct EXPLICIT_ACCESS
		{
			public UIntPtr grfAccessPermissions;
			public UIntPtr grfAccessMode;
			public UIntPtr grfInheritance;
			public TRUSTEE Trustee;
		}

		[StructLayout(LayoutKind.Sequential)]
		private struct SECURITY_ATTRIBUTES
		{
			public int nLength;
			public IntPtr lpSecurityDescriptor;
			public int bInheritHandle;
		}

		[StructLayoutAttribute(LayoutKind.Sequential)]
		struct SECURITY_DESCRIPTOR
		{
			public byte revision;
			public byte size;
			public short control;
			public IntPtr owner;
			public IntPtr group;
			public IntPtr sacl;
			public IntPtr dacl;

			public IntPtr ToPoiter()
			{
				var pointer = Marshal.AllocHGlobal(Marshal.SizeOf(this));
				Marshal.StructureToPtr(this, pointer, false);
				return pointer;
			}
		}

		[Flags]
		public enum FileMapAccess : uint
		{
			FileMapCopy = 0x0001,
			FileMapWrite = 0x0002,
			FileMapRead = 0x0004,
			FileMapAllAccess = 0x001f,
			FileMapExecute = 0x0020,
		}

		[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
		internal static extern IntPtr OpenFileMapping(uint dwDesiredAccess, bool bInheritHandle, string lpName);

		[DllImport("kernel32.dll", SetLastError = true)]
		internal static extern IntPtr MapViewOfFile(IntPtr hFileMappingObject, FileMapAccess dwDesiredAccess, uint dwFileOffsetHigh, uint dwFileOffsetLow,
			UIntPtr dwNumberOfBytesToMap);

		[DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
		internal static extern IntPtr CreateFileMapping(IntPtr hFile, IntPtr lpFileMappingAttributes, FileMapProtection flProtect, uint dwMaximumSizeHigh,
			uint dwMaximumSizeLow, string lpName);

		[DllImport("advapi32.dll", SetLastError = true)]
		static extern bool AllocateAndInitializeSid(ref SID_IDENTIFIER_AUTHORITY pIdentifierAuthority, byte nSubAuthorityCount,
			uint dwSubAuthority0, uint dwSubAuthority1, uint dwSubAuthority2, uint dwSubAuthority3, uint dwSubAuthority4,
			uint dwSubAuthority5, uint dwSubAuthority6, uint dwSubAuthority7, out IntPtr pSid);

		[DllImport("advapi32.dll", SetLastError = true)]
		private static extern int SetEntriesInAcl(int cCountOfExplicitEntries, ref EXPLICIT_ACCESS[] pListOfExplicitEntries,
			IntPtr OldAcl, out IntPtr NewAcl);

		[DllImport("advapi32.dll", SetLastError = true)]
		private static extern bool InitializeSecurityDescriptor(out SECURITY_DESCRIPTOR SecurityDescriptor, uint dwRevision);

		[DllImport("advapi32.dll", SetLastError = true)]
		private static extern bool SetSecurityDescriptorDacl(ref SECURITY_DESCRIPTOR sd, bool daclPresent, IntPtr dacl, bool daclDefaulted);

		[DllImport("coredll.dll", SetLastError = true)]
		public static extern uint WaitForSingleObject(IntPtr Handle, uint Wait);

		[DllImport("coredll.dll", EntryPoint = "WaitForMultipleObjects", SetLastError = true)]
		public static extern int WaitForMultipleObjects(uint nCount, IntPtr[] lpHandles, bool fWaitAll, uint dwMilliseconds);

		[DllImport("kernel32.dll")]
		public static extern bool ReleaseMutex(IntPtr hMutex);

		[DllImport("kernel32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		public static extern bool SetEvent(IntPtr hEvent);

		[DllImport("kernel32.dll")]
		public static extern IntPtr CreateMutex(IntPtr lpMutexAttributes, bool bInitialOwner, string lpName);

		public static IntPtr CreateSecurityAttribute(string userName)
		{
			if (string.IsNullOrWhiteSpace(userName))
				return IntPtr.Zero;

			var user = userName;
			var ea = new EXPLICIT_ACCESS[2];
			var SIDAuthCreator = new SID_IDENTIFIER_AUTHORITY(SECURITY_CREATOR_SID_AUTHORITY);
			IntPtr tmpSID;

			// initialize an EXPLICIT_ACCESS structure for an ACE
			// the ACE will allow Everyone full access
			ea[0] = new EXPLICIT_ACCESS();
			ea[0].grfAccessPermissions = new UIntPtr(GENERIC_ALL);
			ea[0].grfAccessMode = new UIntPtr((uint)ACCESS_MODE.SET_ACCESS);
			ea[0].grfInheritance = new UIntPtr(NO_INHERITANCE);
			ea[0].Trustee.TrusteeForm = TRUSTEE_FORM.TRUSTEE_IS_NAME;
			ea[0].Trustee.TrusteeType = TRUSTEE_TYPE.TRUSTEE_IS_USER;
			ea[0].Trustee.ptstrName = Marshal.StringToHGlobalAuto(user);

			// create a SID for the BUILTIN\Administrators group
			if (!AllocateAndInitializeSid(ref SIDAuthCreator, 1, SECURITY_CREATOR_OWNER_RID, 0, 0, 0, 0, 0, 0, 0, out tmpSID))
				return IntPtr.Zero;

			// initialize an EXPLICIT_ACCESS structure for an ACE
			// the ACE will allow the Administrators group full access
			var creatorSID = tmpSID;

			ea[1].grfAccessPermissions = new UIntPtr(GENERIC_ALL);
			ea[1].grfAccessMode =  new UIntPtr((uint)ACCESS_MODE.SET_ACCESS);
			ea[1].grfInheritance = new UIntPtr(NO_INHERITANCE);
			ea[1].Trustee.TrusteeForm = TRUSTEE_FORM.TRUSTEE_IS_SID;
			ea[1].Trustee.TrusteeType = TRUSTEE_TYPE.TRUSTEE_IS_WELL_KNOWN_GROUP;
			ea[1].Trustee.ptstrName = creatorSID;

			IntPtr tmpACL;
			if (SetEntriesInAcl(2, ref ea, IntPtr.Zero, out tmpACL) != ERROR_SUCCESS)
				return IntPtr.Zero;

			var acl = tmpACL;

			SECURITY_DESCRIPTOR sd;
			if (!InitializeSecurityDescriptor(out sd, SECURITY_DESCRIPTOR_REVISION))
				return IntPtr.Zero;

			if (!SetSecurityDescriptorDacl(ref sd, true, acl, false))
				return IntPtr.Zero;

			var sa = new SECURITY_ATTRIBUTES();
			sa.nLength = Marshal.SizeOf(typeof(SECURITY_ATTRIBUTES));
			sa.lpSecurityDescriptor = sd.ToPoiter();
			sa.bInheritHandle = 0;

			var pointer = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(SECURITY_ATTRIBUTES)));
			Marshal.StructureToPtr(sa, pointer, false);

			return pointer;
		}
	}
}
