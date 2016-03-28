#pragma once

#include <AccCtrl.h>
#include <AclAPI.h>
#include <string>
#include <boost/smart_ptr.hpp>
using namespace boost;

#include "Constants.h"

enum SyncObjectTypes
{
	SyncObjNone		= 0,
	SyncObjRequest	= 1,
	SyncObjBoth		= 2,
};

template <typename T>
class SharedMemory
{
public:
	SharedMemory();
	SharedMemory(const std::wstring & name, DWORD size, SyncObjectTypes syncObjects, bool create = true);

	~SharedMemory();

	void Create(const std::wstring& name, DWORD size, SyncObjectTypes syncObjects, const std::wstring& userName);
	void Open(const std::wstring& name, SyncObjectTypes syncObjects);

	inline void Lock();
	inline void Release();
	inline void SetReqEvent();
	inline void SetRespEvent();

	inline T* Get() const;
	inline HANDLE GetReqEvent() const;
	inline HANDLE GetRespEvent() const;

	inline T& operator[](size_t index) const;
	inline T* operator->() const;
	inline T& operator*() const;
	inline SharedMemory& operator=(const T& val);

private:
	void CreateSyncObjects(const shared_ptr<SECURITY_ATTRIBUTES>& sa, SyncObjectTypes syncObjects, const std::wstring& name);

private:
	std::wstring				m_name;
	DWORD				m_size;

	shared_ptr<void>	m_sharedMem;
	shared_ptr<T>		m_memView;

	shared_ptr<void>	m_sharedMutex;
	shared_ptr<void>	m_sharedReqEvent;
	shared_ptr<void>	m_sharedRespEvent;

};

class SharedMemoryLock
{
public:
	template <typename T> explicit SharedMemoryLock(SharedMemory<T>& sharedMem)
		: m_lock((sharedMem.Lock(), &sharedMem), boost::mem_fn(&SharedMemory<T>::Release))
	{
	}

private:
	shared_ptr<void>		m_lock;
};

template<typename T>
inline SharedMemory<T>::SharedMemory()
	: m_name(L"")
	, m_size(0)
	, m_sharedMem()
	, m_memView()
	, m_sharedMutex()
	, m_sharedReqEvent()
	, m_sharedRespEvent()
{
}

template<typename T>
inline SharedMemory<T>::SharedMemory(const std::wstring & name, DWORD size, SyncObjectTypes syncObjects, bool create)
	: m_name(name)
	, m_size(size)
	, m_sharedMem()
	, m_memView()
	, m_sharedMutex()
	, m_sharedReqEvent()
	, m_sharedRespEvent()
{
	if (create)
		Create(name, size, syncObjects);
	else
		Open(name, syncObjects);
}

template<typename T>
inline SharedMemory<T>::~SharedMemory()
{
}

template<typename T>
inline void SharedMemory<T>::Create(const std::wstring & name, DWORD size, SyncObjectTypes syncObjects, const std::wstring & userName)
{
	m_name = name;
	m_size = size;

	shared_ptr<SECURITY_ATTRIBUTES> sa;
	EXPLICIT_ACCESS					ea[2];
	SID_IDENTIFIER_AUTHORITY		SidAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;
	PSID							tmpSID = NULL;
	shared_ptr<void>				creatorSID;
	PACL							tmpACL;
	shared_ptr<ACL>					acl;
	shared_ptr<void>				sd;

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS) * 2);

	if (userName.length() > 0)
	{
		// initialize an EXPLICIT_ACCESS structure for an ACE
		// the ACE will allow Everyone full access
		ea[0].grfAccessPermissions = GENERIC_ALL;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
		ea[0].Trustee.ptstrName = (LPTSTR)userName.c_str();

		//	create a SID for the BUILTIN\Administrators group
		if (!AllocateAndInitializeSid(&SidAuthCreator, 1, SECURITY_CREATOR_OWNER_RID, 0, 0, 0, 0, 0, 0, 0, &tmpSID))
			return;

		creatorSID.reset(tmpSID, FreeSid);

		//	initialize an EXPLICIT_ACCESS structure for an ACE, the ACE
		//	will allow the Administrators group full access
		ea[1].grfAccessPermissions = GENERIC_ALL;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[1].Trustee.ptstrName = (LPTSTR)creatorSID.get();

		if (SetEntriesInAcl(2, ea, NULL, &tmpACL) != ERROR_SUCCESS)
			return;

		acl.reset(tmpACL, LocalFree);

		//	init security descriptor
		sd.reset(LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH), LocalFree);
		if (!sd || !InitializeSecurityDescriptor(sd.get(), SECURITY_DESCRIPTOR_REVISION) ||
			!SetSecurityDescriptorDacl(sd.get(), TRUE, acl.get(), FALSE))
			return;

		sa.reset(new SECURITY_ATTRIBUTES);
		sa->nLength = sizeof(SECURITY_ATTRIBUTES);
		sa->lpSecurityDescriptor = sd.get();
		sa->bInheritHandle = FALSE;
	}

	m_sharedMem = shared_ptr<void>(CreateFileMapping(INVALID_HANDLE_VALUE, sa.get(), PAGE_READWRITE, 0, m_size * sizeof(T), m_name.c_str()),
		CloseHandle);
	m_memView = shared_ptr<T>(static_cast<T*>(MapViewOfFile(m_sharedMem.get(), FILE_MAP_ALL_ACCESS, 0, 0, 0)), UnmapViewOfFile);

	ZeroMemory(m_memView.get(), m_size * sizeof(T));

	if (syncObjects > SyncObjectTypes::SyncObjNone)
		CreateSyncObjects(sa, syncObjects, name);
}

template<typename T>
inline void SharedMemory<T>::Open(const std::wstring & name, SyncObjectTypes syncObjects)
{
	m_name = name;
	m_sharedMem = shared_ptr<void>(OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, m_name.c_str()), CloseHandle);
	if (!m_sharedMem || m_sharedMem.get() == INVALID_HANDLE_VALUE)
		OutputDebugString(FormatError(GetLastError(), Constants::ErrOpenSharedMem, m_name));

	m_memView = shared_ptr<T>(static_cast<T*>(MapViewOfFile(m_sharedMem.get(), FILE_MAP_ALL_ACCESS, 0, 0, 0)), UnmapViewOfFile);
	if (!m_memView)
		OutputDebugString(FormatError(GetLastError(), Constants::ErrMapSharedMem, m_name));

	if (syncObjects > SyncObjectTypes::SyncObjNone)
		CreateSyncObjects(shared_ptr<SECURITY_ATTRIBUTES>(), syncObjects, name);
}

template<typename T>
inline void SharedMemory<T>::Lock()
{
	if (!m_sharedMutex)
		return;
	WaitForSingleObject(m_sharedMutex.get(), INFINITE);
}

template<typename T>
inline void SharedMemory<T>::Release()
{
	if (!m_sharedMutex)
		return;
	ReleaseMutex(m_sharedMutex.get());
}

template<typename T>
inline void SharedMemory<T>::SetReqEvent()
{
	if (!m_sharedReqEvent)
	{
		OutputDebugString((Constants::ErrReqEventNull % m_name).str());
		return;
	}

	if (!SetEvent(m_sharedReqEvent.get()))
		OutputDebugString(FormatError(GetLastError(), Constants::ReqSetEventFailed, m_name));
}

template<typename T>
inline void SharedMemory<T>::SetRespEvent()
{
	if (!m_sharedRespEvent)
	{
		OutputDebugString((Constants::ErrRespEventNull % m_name).str());
		return;
	}

	if (!SetEvent(m_sharedRespEvent.get()))
		OutputDebugString(FormatError(GetLastError(), Constants::RespSetEventFailed, m_name));
}

template<typename T>
inline T * SharedMemory<T>::Get() const
{
	return m_memView.get();
}

template<typename T>
inline HANDLE SharedMemory<T>::GetReqEvent() const
{
	return m_sharedReqEvent.get();
}

template<typename T>
inline HANDLE SharedMemory<T>::GetRespEvent() const
{
	return m_sharedRespEvent.get();
}

template<typename T>
inline T & SharedMemory<T>::operator[](size_t index) const
{
	return *(m_memView.get() + index);
}

template<typename T>
inline T * SharedMemory<T>::operator->() const
{
	return m_memView.get();
}

template<typename T>
inline T & SharedMemory<T>::operator*() const
{
	return *m_memView;
}

template<typename T>
inline SharedMemory<T> & SharedMemory<T>::operator=(const T & val)
{
	*m_memView = val;
	return *this;
}

template<typename T>
inline void SharedMemory<T>::CreateSyncObjects(const shared_ptr<SECURITY_ATTRIBUTES>& sa, SyncObjectTypes syncObjects, const std::wstring & name)
{
	if (syncObjects >= SyncObjectTypes::SyncObjRequest)
	{
		m_sharedMutex = shared_ptr<void>(CreateMutex(sa.get(), FALSE, (name + std::wstring(L"_mutex")).c_str()), CloseHandle);
		OutputDebugString((Constants::InfoMutex % name % (DWORD)m_sharedMutex.get()).str());

		m_sharedReqEvent = shared_ptr<void>(CreateEvent(sa.get(), FALSE, FALSE, (name + std::wstring(L"_req_event")).c_str()), CloseHandle);
		OutputDebugString((Constants::InfoReqEvent % name % (DWORD)m_sharedReqEvent.get()).str());
	}

	if (syncObjects >= SyncObjectTypes::SyncObjBoth)
	{
		m_sharedRespEvent = shared_ptr<void>(CreateEvent(sa.get(), FALSE, FALSE, (name + std::wstring(L"_resp_event")).c_str()), CloseHandle);
		OutputDebugString((Constants::InfoRespEvent % name % (DWORD)m_sharedRespEvent.get()).str());
	}
}
