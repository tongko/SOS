#pragma once

struct ShellParams
{
	ShellParams()
		: ShellMainThreadId(0)
		, ParentProcessId(0)
		, NotificationTimeout(0)
		, RefreshInterval(0)
		, Rows(0)
		, Columns(0)
		, BufferRows(0)
		, BufferColumns(0)
		, MaxRows(0)
		, MaxColumns(0)
		, ShellWindow(NULL)
		, HookThreadId(0)
	{
	}

	//	Copy constructor
	ShellParams(const ShellParams& other)
		: ShellMainThreadId(other.ShellMainThreadId)
		, ParentProcessId(other.ParentProcessId)
		, NotificationTimeout(other.NotificationTimeout)
		, RefreshInterval(other.RefreshInterval)
		, Rows(other.Rows)
		, Columns(other.Columns)
		, BufferRows(other.BufferRows)
		, BufferColumns(other.BufferColumns)
		, MaxRows(other.MaxRows)
		, MaxColumns(other.MaxColumns)
		, ShellWindow(other.ShellWindow)
		, HookThreadId(other.HookThreadId)
	{
	}

	DWORD	ShellMainThreadId;
	DWORD	ParentProcessId;
	DWORD	NotificationTimeout;
	DWORD	RefreshInterval;
	DWORD	Rows;
	DWORD	Columns;
	DWORD	BufferRows;
	DWORD	BufferColumns;

	// stuff set by console hook
	DWORD	MaxRows;
	DWORD	MaxColumns;
	union
	{
		HWND	ShellWindow;
		// padding for 32-bit processes started from 64-bit Console
		__int64	padding;
	};

	DWORD	HookThreadId;
};

struct ShellSize
{
	ShellSize()
		: Rows(0)
		, Columns(0)
		, ResizeWindowEdge(0)
	{
	}

	DWORD	Rows;
	DWORD	Columns;

	//	window edge used for resizing, one of WMSZ_* constants.
	DWORD	ResizeWindowEdge;
};

enum _CopyNewlineChar
{
	NewlineCRLF = 0,
	NewlineLF = 1
};

struct ShellCopy
{
	ShellCopy()
		: Start()
		, End()
		, NoWrap(false)
		, TrimSpaces(false)
		, CopyNewlineChar(_CopyNewlineChar::NewlineCRLF)
	{
	}

	COORD	Start;
	COORD	End;
	bool	NoWrap;
	bool	TrimSpaces;
	_CopyNewlineChar	CopyNewlineChar;
};

struct ShellInfo
{
	ShellInfo()
		: ScreenBufferInfo()
		, TextChanged(false)
	{
	}

	CONSOLE_SCREEN_BUFFER_INFO	ScreenBufferInfo;
	bool						TextChanged;
};

struct CharInfo
{
	CharInfo()
		: Changed(false)
	{
		// This is actually a shortcut for:
		// charInfo.Attributes = 0;
		// charInfo.Char.UnicodeChar = L' ';
		// Is it faster, though?
		*(reinterpret_cast<DWORD*>(&CharacterInfo)) = 0x00000020;
	}

	CHAR_INFO	CharacterInfo;
	bool		Changed;
};

struct TextInfo
{
	TextInfo()
		: Padding(0)
	{
	}

	union
	{
		UINT_PTR	Mem;
		// padding for 32-bit processes started from 64-bit Console
		__int64	Padding;
	};
};