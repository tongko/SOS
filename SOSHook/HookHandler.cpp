#include "stdafx.h"

#include "Constants.h"
#include "SharedMemory.h"
#include "HookHandler.h"


HookHandler::HookHandler()
	: m_shellParams()
	, m_shellInfo()
	, m_cursorInfo()
	, m_shellBuffer()
	, m_shellCopyInfo()
	, m_shellTextInfo()
	, m_shellMouseEvent()
	, m_newShellSize()
	, m_newScrollPos()
	, m_monitorThread()
	, m_monitorThreadExit(shared_ptr<void>(CreateEvent(NULL, FALSE, FALSE, NULL), CloseHandle))
	, m_screenBufferSize(0)
{
}


HookHandler::~HookHandler()
{
	StopMonitor();
}

DWORD HookHandler::StartMonitor()
{
	DWORD threadId = 0;
	m_monitorThread = shared_ptr<void>(CreateThread(NULL, 0, StaticMonitorThread, reinterpret_cast<void*>(this), 0, &threadId), CloseHandle);

	return threadId;
}

void HookHandler::StopMonitor()
{
	SetEvent(m_monitorThreadExit.get());
	WaitForSingleObject(m_monitorThread.get(), 10000);
}

bool HookHandler::OpenSharedObjects()
{
	DWORD processId = GetCurrentProcessId();

	m_shellParams.Open((Constants::ShellParamsName % processId).str(), SyncObjectTypes::SyncObjRequest);

	m_shellInfo.Open((Constants::InfoName % processId).str(), SyncObjectTypes::SyncObjRequest);

	m_cursorInfo.Open((Constants::CursorInfoName % processId).str(), SyncObjectTypes::SyncObjRequest);

	m_shellBuffer.Open((Constants::BufferName % processId).str(), SyncObjectTypes::SyncObjRequest);

	m_shellCopyInfo.Open((Constants::CopyInfoName % processId).str(), SyncObjectTypes::SyncObjBoth);

	m_shellTextInfo.Open((Constants::TextInfoName % processId).str(), SyncObjectTypes::SyncObjBoth);

	m_shellMouseEvent.Open((Constants::MouseEventName % processId).str(), SyncObjectTypes::SyncObjBoth);

	m_newShellSize.Open((Constants::NewShellSizeName % processId).str(), SyncObjectTypes::SyncObjRequest);

	m_newScrollPos.Open((Constants::NewScrollPosName % processId).str(), SyncObjectTypes::SyncObjRequest);

	return true;
}

void HookHandler::ReadShellBuffer()
{
	shared_ptr<void> stdOut(CreateFile(L"CONOUT$", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, 0), CloseHandle);
	
	CONSOLE_SCREEN_BUFFER_INFO	bufferInfo;
	GetConsoleScreenBufferInfo(stdOut.get(), &bufferInfo);

	COORD shellSize;
	shellSize.X = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
	shellSize.Y = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

	DWORD screenBufferSize = shellSize.X * shellSize.Y;
	DWORD screenBufferOffset = 0;

	shared_array<CHAR_INFO> screenBuffer(new CHAR_INFO[screenBufferSize]);

	COORD bufferSize;
	bufferSize.Y = 6144 / (bufferSize.X = shellSize.X);

	COORD start = { 0, 0 };
	
	SMALL_RECT bufferRect;
	bufferRect.Top = bufferInfo.srWindow.Top;
	bufferRect.Bottom = bufferInfo.srWindow.Top + bufferSize.Y - 1;
	bufferRect.Left = bufferInfo.srWindow.Left;
	bufferRect.Right = bufferInfo.srWindow.Right;

	SHORT i = 0;
	for (; i < shellSize.Y / bufferSize.Y; ++i)
	{
		ReadConsoleOutput(stdOut.get(), screenBuffer.get() + screenBufferOffset, bufferSize, start, &bufferRect);
		bufferRect.Top = bufferRect.Top + bufferSize.Y;
		bufferRect.Bottom = bufferRect.Bottom + bufferSize.Y;

		screenBufferOffset += bufferSize.X * bufferSize.Y;
	}

	bufferSize.Y = shellSize.Y - i * bufferSize.Y;
	bufferRect.Bottom = bufferInfo.srWindow.Bottom;

	ReadConsoleOutput(stdOut.get(), screenBuffer.get() + screenBufferOffset, bufferSize, start, &bufferRect);

	bool textChanged = (memcmp(m_shellBuffer.Get(), screenBuffer.get(), m_screenBufferSize * sizeof(CHAR_INFO)) != 0);

	if (memcmp(&m_shellInfo->ScreenBufferInfo, &bufferInfo, sizeof(CONSOLE_SCREEN_BUFFER_INFO)) != 0 ||
		m_screenBufferSize != screenBufferSize || textChanged)
	{
		m_screenBufferSize = screenBufferSize;

		CopyMemory(&m_shellInfo->ScreenBufferInfo, &bufferInfo, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
		
		if (textChanged)
			m_shellInfo->TextChanged = true;
		CopyMemory(m_shellBuffer.Get(), screenBuffer.get(), m_screenBufferSize * sizeof(CHAR_INFO));

		m_shellBuffer.SetReqEvent();
	}
}

void HookHandler::ResizeShellWindow(HANDLE stdOut, DWORD & columns, DWORD & rows, DWORD resizeWindowEdge)
{
	CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	GetConsoleScreenBufferInfo(stdOut, &screenBufferInfo);
	TRACE(L"Console size: %i x %i\n", screenBufferInfo.dwSize.X, screenBufferInfo.dwSize.Y);
	TRACE(L"Old win pos: %i x %i - %i x %i\n", screenBufferInfo.srWindow.Left, screenBufferInfo.srWindow.Top, screenBufferInfo.srWindow.Right,
		screenBufferInfo.srWindow.Bottom);

	TRACE(L"Columns: %i\n", columns);
	TRACE(L"Max columns: %i\n", m_shellParams->MaxColumns);
	TRACE(L"Rows: %i\n", rows);
	TRACE(L"Max rows: %i\n", m_shellParams->MaxRows);

	if (columns > m_shellParams->MaxColumns)
		columns = m_shellParams->MaxColumns;
	if (rows > m_shellParams->MaxRows)
		rows = m_shellParams->MaxRows;

	TRACE(L"Screen buffer: %i x %i\n", m_shellParams->BufferRows, m_shellParams->BufferColumns);

	COORD	bufferSize;
	if (m_shellParams->BufferColumns == 0)
		bufferSize.X = (SHORT) columns;
	else
		bufferSize.X = (SHORT) m_shellParams->BufferColumns;

	if (m_shellParams->BufferRows == 0)
		bufferSize.Y = (SHORT)rows;
	else
		bufferSize.Y = (SHORT)m_shellParams->BufferRows;

	SMALL_RECT shellRect, window = screenBufferInfo.srWindow;
	switch (resizeWindowEdge)
	{
	case WMSZ_TOP:
	case WMSZ_TOPLEFT:
	case WMSZ_TOPRIGHT:
	{
		if (window.Top == 0 || window.Bottom - (SHORT)(rows - 1) <= 0)
		{
			shellRect.Top = 0;
			shellRect.Bottom = (SHORT)(rows - 1);
		}
		else
		{
			shellRect.Top = window.Bottom - (SHORT)rows;
			shellRect.Bottom = window.Bottom;
		}

		break;
	}
	case WMSZ_BOTTOM:
	case WMSZ_BOTTOMLEFT:
	case WMSZ_BOTTOMRIGHT:
	{
		if (m_shellParams->BufferRows > 0 && window.Top + (SHORT)rows > (SHORT)m_shellParams->BufferRows)
		{
			shellRect.Top = (SHORT)(m_shellParams->BufferRows - rows);
			shellRect.Bottom = (SHORT)(m_shellParams->BufferRows - 1);
		}
		else
		{
			shellRect.Top = window.Top;
			shellRect.Bottom = window.Top + (SHORT)(rows - 1);
		}

		break;
	}
	default:
		shellRect.Top = window.Top;
		shellRect.Bottom = window.Top + (SHORT)(rows - 1);
		break;
	}

	switch (resizeWindowEdge)
	{
	case WMSZ_LEFT:
	case WMSZ_TOPLEFT:
	case WMSZ_BOTTOMLEFT:
	{
		if (window.Left == 0 || window.Right - (SHORT)(columns - 1) <= 0)
		{
			shellRect.Left = 0;
			shellRect.Right = (SHORT)(columns - 1);
		}
		else
		{
			shellRect.Left = window.Right - (SHORT)columns;
			shellRect.Right = window.Right;
		}

		break;
	}
	case WMSZ_RIGHT:
	case WMSZ_TOPRIGHT:
	case WMSZ_BOTTOMRIGHT:
	{
		if (m_shellParams->BufferColumns != 0 && window.Left + (SHORT)columns > (SHORT)m_shellParams->BufferColumns)
		{
			shellRect.Left = (SHORT)(m_shellParams->BufferColumns - columns);
			shellRect.Right = (SHORT)(m_shellParams->BufferColumns - 1);
		}
		else
		{
			shellRect.Left = window.Left;
			shellRect.Right = window.Left + (SHORT)(columns - 1);
		}
	}
	default: 
	{
		shellRect.Left = window.Left;
		shellRect.Right = window.Left + (SHORT)(columns - 1);
		break; 
	}
	}

	TRACE(L"New win pos: %i x %i - %i x %1", shellRect.Left, shellRect.Top, shellRect.Right, shellRect.Bottom);
	TRACE(L"Buffer size: %i x %i", bufferSize.X, bufferSize.Y);

	COORD finalBufferSize;

	finalBufferSize.X = screenBufferInfo.dwSize.X;
	finalBufferSize.Y = bufferSize.Y;

	SMALL_RECT finalShellRect;
	finalShellRect.Left = window.Left;
	finalShellRect.Right = window.Right;
	finalShellRect.Top = shellRect.Top;
	finalShellRect.Bottom = shellRect.Bottom;

	//	if new buffer size.Y greater than old size.Y, resize the buffer first.
	if (bufferSize.Y > screenBufferInfo.dwSize.Y)
	{
		SetConsoleScreenBufferSize(stdOut, finalBufferSize);
		SetConsoleWindowInfo(stdOut, TRUE, &finalShellRect);
	}
	else
	{
		SetConsoleWindowInfo(stdOut, TRUE, &finalShellRect);
		SetConsoleScreenBufferSize(stdOut, finalBufferSize);
	}

	// then resize columns
	finalBufferSize.X = bufferSize.X;
	finalShellRect.Left = shellRect.Left;
	finalShellRect.Right = shellRect.Right;

	//	if new buffer size.X greater than old size.X, resize the buffer first.
	if (bufferSize.X > screenBufferInfo.dwSize.X)
	{
		SetConsoleScreenBufferSize(stdOut, finalBufferSize);
		SetConsoleWindowInfo(stdOut, TRUE, &finalShellRect);
	}
	else
	{
		SetConsoleWindowInfo(stdOut, TRUE, &finalShellRect);
		SetConsoleScreenBufferSize(stdOut, finalBufferSize);
	}

	TRACE(L"Shell buffer size: %i x %i\n", bufferSize.X, bufferSize.Y);
	TRACE(L"Shell rect: %i x %i\n", finalShellRect.Left, finalShellRect.Top, finalShellRect.Right, finalShellRect.Bottom);

	GetConsoleScreenBufferInfo(stdOut, &screenBufferInfo);
	window = screenBufferInfo.srWindow;
	
	columns = window.Right - window.Left + 1;
	rows = window.Bottom - window.Top + 1;
}

void HookHandler::CopyShellText()
{
	if (!OpenClipboard(NULL))
		return;

	COORD& start = m_shellCopyInfo->Start;
	COORD& end = m_shellCopyInfo->End;

	shared_ptr<void> stdOut(CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, 0), CloseHandle);
	
	CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
	GetConsoleScreenBufferInfo(stdOut.get(), &scrBufferInfo);

	std::wstring text(L"");
	for (SHORT i = start.Y; i <= end.Y; ++i)
	{
		SMALL_RECT buffer;
		buffer.Left = i == start.Y ? start.X : 0;
		buffer.Top = i;
		buffer.Right = i == end.Y ? end.X : m_shellParams->BufferColumns > 0 ? (SHORT)m_shellParams->BufferColumns - 1 : (SHORT)m_shellParams->Columns - 1;
		buffer.Bottom = i;

		COORD bufferSize =
		{
			m_shellParams->BufferColumns > 0 ? (SHORT)m_shellParams->BufferColumns : (SHORT)m_shellParams->Columns,
			1
		};
		shared_array<CHAR_INFO> scrBuffer(new CHAR_INFO[bufferSize.X]);
		COORD from = { 0, 0 };

		ReadConsoleOutput(stdOut.get(), scrBuffer.get(), bufferSize, from, &buffer);

		std::wstring rowText(L"");
		for (SHORT x = 0; x <= buffer.Right - buffer.Left; ++x)
		{
			if (scrBuffer[x].Attributes & COMMON_LVB_TRAILING_BYTE)
				continue;
			rowText += scrBuffer[x].Char.UnicodeChar;
		}

		bool wrapText = true;
		if (i == start.Y)
		{
			if (start.Y == end.Y || (m_shellCopyInfo->NoWrap && start.Y < end.Y && rowText[rowText.length() - 1] != L' '))
				wrapText = false;
		}
		else if (i == end.Y)
		{
			if (rowText.length() < (size_t)bufferSize.X)
				wrapText = false;
		}
		else
		{
			if (m_shellCopyInfo->NoWrap && rowText[rowText.length() - 1] != L' ')
				wrapText = false;
		}

		if (m_shellCopyInfo->TrimSpaces)
			trim_right(rowText);

		if (wrapText)
		{
			switch (m_shellCopyInfo->CopyNewlineChar)
			{
			case _CopyNewlineChar::NewlineLF:
				rowText += std::wstring(L"\n");
				break;
			default:
				rowText += std::wstring(L"\r\n");
				break;
			}
		}

		text += rowText;
	}

	EmptyClipboard();

	HGLOBAL copyText = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));

	if (copyText == NULL)
	{
		CloseClipboard();
		return;
	}

	CopyMemory(static_cast<wchar_t*>(GlobalLock(copyText)), text.c_str(), (text.length() + 1) * sizeof(wchar_t));

	GlobalUnlock(copyText);

	if (SetClipboardData(CF_UNICODETEXT, copyText) == NULL)
		GlobalFree(copyText);

	CloseClipboard();
}

void HookHandler::SendShellText(HANDLE stdIn, const shared_ptr<wchar_t>& buffer)
{
	wchar_t* text = buffer.get();
	size_t textLen = wcslen(text);
	size_t partLen = 512;
	size_t parts = textLen / partLen;

	for (size_t part = 0; part < parts + 1; ++part)
	{
		size_t keyEventCount = 0;
		if (part == parts)
			partLen = textLen - parts * partLen;

		scoped_array<INPUT_RECORD> keyEvents(new INPUT_RECORD[partLen]);
		ZeroMemory(keyEvents.get(), sizeof(INPUT_RECORD) * partLen);

		size_t offset = 0;
		for (size_t i = 0; i < partLen && offset < textLen; ++i, ++offset, ++keyEventCount)
		{
			if (text[offset] == L'\r' || text[offset] == L'\n')
			{
				if (text[offset] == L'\r' && text[offset + 1] == L'\n')
					++offset;

				if (keyEventCount > 0)
				{
					DWORD textWritten = 0;
					WriteConsoleInput(stdIn, keyEvents.get(), (DWORD)keyEventCount, &textWritten);
				}

				PostMessage(m_shellParams->ShellWindow, WM_KEYDOWN, VK_RETURN, 0x001C0001);
				PostMessage(m_shellParams->ShellWindow, WM_KEYUP, VK_RETURN, 0xC01C0001);

				keyEventCount = -1;
				partLen -= i;
				i = -1;
			}
			else
			{
				keyEvents[i].EventType = KEY_EVENT;
				keyEvents[i].Event.KeyEvent.bKeyDown = TRUE;
				keyEvents[i].Event.KeyEvent.wRepeatCount = 1;
				keyEvents[i].Event.KeyEvent.wVirtualKeyCode = LOBYTE(VkKeyScan(text[offset]));
				keyEvents[i].Event.KeyEvent.wVirtualScanCode = 0;
				keyEvents[i].Event.KeyEvent.uChar.UnicodeChar = text[offset];
				keyEvents[i].Event.KeyEvent.dwControlKeyState = 0;
			}
		}

		if (keyEventCount > 0)
		{
			DWORD textWritten = 0;
			WriteConsoleInput(stdIn, keyEvents.get(), (DWORD)keyEventCount, &textWritten);
		}
	}
}

void HookHandler::SetResetKeyInput(scoped_array<INPUT>& inputs, WORD vKey, short & count)
{
	if ((GetAsyncKeyState(vKey) & 0x8000) == 0)
		return;

	inputs[count].type = INPUT_KEYBOARD;
	inputs[count].ki.wVk = vKey;
	inputs[count].ki.dwFlags = KEYEVENTF_KEYUP;
	++count;
}

void HookHandler::SendMouseEvent(HANDLE stdIn)
{
	INPUT_RECORD mouseEvent;
	ZeroMemory(&mouseEvent, sizeof(INPUT_RECORD));

	mouseEvent.EventType = MOUSE_EVENT;

	CopyMemory(&mouseEvent.Event.MouseEvent, m_shellMouseEvent.Get(), sizeof(MOUSE_EVENT_RECORD));

	DWORD events = 0;
	WriteConsoleInput(stdIn, &mouseEvent, 1, &events);
}

void HookHandler::ScrollShell(HANDLE stdOut, int xDelta, int yDelta)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(stdOut, &info);

	int currentXPos = info.srWindow.Right - m_shellParams->Columns + 1;
	int currentYPos = info.srWindow.Bottom - m_shellParams->Rows + 1;

	xDelta = max(-currentXPos, min(xDelta, (int)(m_shellParams->BufferColumns - m_shellParams->Columns) - currentXPos));
	yDelta = max(-currentYPos, min(yDelta, (int)(m_shellParams->BufferRows - m_shellParams->Rows) - currentYPos));

	SMALL_RECT rect;
	rect.Top = (SHORT)yDelta;
	rect.Bottom = (SHORT)yDelta;
	rect.Left = (SHORT)xDelta;
	rect.Right = (SHORT)xDelta;

	SetConsoleWindowInfo(stdOut, FALSE, &rect);
}

void HookHandler::SetShellParams(DWORD hookThreadId, HANDLE stdOut)
{
	COORD maxSize = GetLargestConsoleWindowSize(stdOut);

	m_shellParams->MaxRows = maxSize.Y;
	m_shellParams->MaxColumns = maxSize.X;

	if (m_shellParams->Rows > (DWORD)maxSize.Y)
		m_shellParams->Rows = maxSize.Y;
	if (m_shellParams->Columns > (DWORD)maxSize.X)
		m_shellParams->Columns = maxSize.X;

	if (m_shellParams->BufferRows != 0 && m_shellParams->MaxRows > m_shellParams->BufferRows)
		m_shellParams->MaxRows = m_shellParams->BufferRows;
	if (m_shellParams->BufferColumns != 0 && m_shellParams->MaxColumns > m_shellParams->BufferColumns)
		m_shellParams->MaxColumns = m_shellParams->BufferColumns;

	m_shellParams->ShellWindow = GetConsoleWindow();
	m_shellParams->HookThreadId = hookThreadId;

	TRACE(L"Max columns: %i, max rows: %i\n", m_shellParams->MaxColumns, m_shellParams->MaxRows);

	GetConsoleScreenBufferInfo(stdOut, &m_shellInfo->ScreenBufferInfo);
	GetConsoleCursorInfo(stdOut, m_cursorInfo.Get());

	m_shellParams.SetReqEvent();
}

DWORD HookHandler::StaticMonitorThread(LPVOID lpParam)
{
	HookHandler* hookHandler = reinterpret_cast<HookHandler*>(lpParam);
	return hookHandler->MonitorThread();
}

DWORD HookHandler::MonitorThread()
{
	TRACE(L"Hook start.\n");

	if (!OpenSharedObjects())
		return 0;

	HANDLE stdOut = CreateFile(L"CONOUT$", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, 0);
	SetShellParams(GetCurrentThreadId(), stdOut);
	if (WaitForSingleObject(m_shellParams.GetRespEvent(), 10000) == WAIT_TIMEOUT)
		return 0;

	shared_ptr<void> parentProcWatchdog(OpenMutex(SYNCHRONIZE, FALSE, (LPCTSTR)(Constants::WatchdogName % m_shellParams->ParentProcessId).str().c_str()), CloseHandle);
	TRACE(L"Watchdog handle: 0x%08X\n", parentProcWatchdog.get());

	HANDLE waitHandles[] =
	{
		m_monitorThreadExit.get(),
		m_shellCopyInfo.GetReqEvent(),
		m_shellTextInfo.GetReqEvent(),
		m_newScrollPos.GetReqEvent(),
		m_shellMouseEvent.GetReqEvent(),
		m_newShellSize.GetReqEvent(),
		stdOut,
	};

	DWORD waitRes = 0;
	HANDLE stdIn = CreateFile(L"CONIN$", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, 0);
	while ((waitRes = WaitForMultipleObjects(sizeof(waitHandles) / sizeof(waitHandles[0]), 
												waitHandles, FALSE, m_shellParams->RefreshInterval))
		!= WAIT_OBJECT_0)
	{
		if (parentProcWatchdog.get() != NULL && WaitForSingleObject(parentProcWatchdog.get(), 0) == WAIT_ABANDONED)
		{
			TRACE(L"Watchdog 0x%08X is dead. Time ot exit.\n", parentProcWatchdog.get());
			SendMessage(m_shellParams->ShellWindow, WM_CLOSE, 0, 0);
			break;
		}

		switch (waitRes)
		{
		case WAIT_OBJECT_0 + 1:
		{
			SharedMemoryLock memLock(m_shellCopyInfo);

			CopyShellText();

			m_shellCopyInfo.SetRespEvent();
			break;
		}
		case WAIT_OBJECT_0 + 2:
		{
			SharedMemoryLock memLock(m_shellTextInfo);

			shared_ptr<wchar_t> buffer;
			if (m_shellTextInfo->Mem != NULL)
				buffer.reset(reinterpret_cast<wchar_t*>(m_shellTextInfo->Mem),
					bind<BOOL>(VirtualFreeEx, GetCurrentProcess(), _1, NULL, MEM_RELEASE));

			SendShellText(stdIn, buffer);
			m_shellTextInfo.SetRespEvent();
			break;
		}
		case WAIT_OBJECT_0 + 3:
		{
			SharedMemoryLock memLock(m_newScrollPos);

			ScrollShell(stdOut, m_newScrollPos->cx, m_newScrollPos->cy);
			ReadShellBuffer();
			break;
		}
		case WAIT_OBJECT_0 + 4:
		{
			SharedMemoryLock memLock(m_shellMouseEvent);

			SendMouseEvent(stdIn);
			m_shellMouseEvent.SetRespEvent();
			break;
		}
		case WAIT_OBJECT_0 + 5:
		{
			SharedMemoryLock memLock(m_newShellSize);
			
			ResizeShellWindow(stdOut, m_newShellSize->Columns, m_newShellSize->Rows, m_newShellSize->ResizeWindowEdge);
			ReadShellBuffer();
			break;
		}
		case WAIT_OBJECT_0 + 6:
			Sleep(m_shellParams->NotificationTimeout);
		default:
		{
			ReadShellBuffer();
			break; 
		}
		}
	}

	return 0;
}
