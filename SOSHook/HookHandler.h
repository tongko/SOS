#pragma once

class HookHandler
{
public:
	HookHandler();
	~HookHandler();

public:
	DWORD StartMonitor();
	void StopMonitor();

private:
	bool OpenSharedObjects();
	void ReadShellBuffer();
	void ResizeShellWindow(HANDLE stdOut, DWORD& columns, DWORD& rows, DWORD resizeWindowEdge);
	void CopyShellText();
	void SendShellText(HANDLE stdIn, const shared_ptr<wchar_t>& buffer);
	void SetResetKeyInput(scoped_array<INPUT>& inputs, WORD vKey, short& count);
	void SendMouseEvent(HANDLE stdIn);
	void ScrollShell(HANDLE stdOut, int xDelta, int yDelta);
	void SetShellConfig(DWORD hookThreadId, HANDLE stdOut);

private:
	static DWORD WINAPI StaticMonitorThread(LPVOID lpParam);
	DWORD MonitorThread();

private:
	SharedMemory<ShellParams>				m_shellParams;
	SharedMemory<ShellInfo>					m_shellInfo;
	SharedMemory<CONSOLE_CURSOR_INFO>		m_cursorInfo;
	SharedMemory<CHAR_INFO>					m_shellBuffer;
	SharedMemory<ShellCopy>					m_shellCopyInfo;
	SharedMemory<TextInfo>					m_shellTextInfo;
	SharedMemory<MOUSE_EVENT_RECORD>		m_shellMouseEvent;

	SharedMemory<ShellSize>					m_newShellSize;
	SharedMemory<SIZE>						m_newScrollPos;

	shared_ptr<void>						m_monitorThread;
	shared_ptr<void>						m_monitorThreadExit;

	DWORD									m_screenBufferSize;
};

