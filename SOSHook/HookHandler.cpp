#include "stdafx.h"

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
			shellRect.Left = (SHORT)m_shellParams->BufferColumns - columns;
			shellRect.Right = (SHORT)m_shellParams->BufferColumns - 1;
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
}

void HookHandler::SetResetKeyInput(scoped_array<INPUT>& inputs, WORD vKey, short & count)
{
}

void HookHandler::SendMouseEvent(HANDLE stdIn)
{
}

void HookHandler::ScrollShell(HANDLE stdOut, int xDelta, int yDelta)
{
}

void HookHandler::SetShellConfig(DWORD hookThreadId, HANDLE stdOut)
{
}

DWORD HookHandler::StaticMonitorThread(LPVOID lpParam)
{
	return 0;
}

DWORD HookHandler::MonitorThread()
{
	return 0;
}
