#include "stdafx.h"

#include "Constants.h"

wformat Constants::ShellParamsName(L"SOS_params_%1%");
wformat Constants::InfoName(L"SOS_shellInfo_%1%");
wformat Constants::CursorInfoName(L"SOS_cursorInfo_%1%");
wformat Constants::BufferName(L"SOS_shellBuffer_%1%");
wformat Constants::CopyInfoName(L"SOS_shellCopyInfo_%1%");
wformat Constants::TextInfoName(L"SOS_shellTextInfo_%1%");
wformat Constants::MouseEventName(L"SOS_shellMouseEvent_%1%");
wformat Constants::NewShellSizeName(L"SOS_newShellSize_%1%");
wformat Constants::NewScrollPosName(L"SOS_newScrollPos_%1%");
wformat Constants::WatchdogName(L"Local\\SOS_parentProcessExit_%1%");

wformat Constants::ErrOpenSharedMem(L"Error occurs attempt to open shared memory %1%, error: %2%\n");
wformat Constants::ErrMapSharedMem(L"Error occurs attempt to map shared memory %1%, error: %2%\n");
wformat Constants::ErrReqEventNull(L"Request event '%1%' is null.");
wformat Constants::ErrRespEventNull(L"Response event '%!%' is null.");
wformat Constants::ReqSetEventFailed(L"Error occurs attempt to SetEvent for Request %1%, error: %2%\n");
wformat Constants::RespSetEventFailed(L"Error occurs attempt to SetEvent for Response %1%, error: %2%\n");

wformat Constants::InfoMutex(L"m_sharedMutex %1%: %2%\n");
wformat Constants::InfoReqEvent(L"m_sharedReqEvent %1%: %2%\n");
wformat Constants::InfoRespEvent(L"m_sharedRespEvent %1%: %2%\n");