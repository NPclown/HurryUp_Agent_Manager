#include "stdafx.h"
#include "CCommunication.h"
#include "CService.h"

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	SetLogger("Agent-Log", core::LOG_INFO | core::LOG_WARN | core::LOG_ERROR | core::LOG_DEBUG);
	core::Log_Info(TEXT("main.cpp - [%s]"), TEXT("Program is Debug Mode"));
#else
	SetLogger("Agent-Log", core::LOG_INFO | core::LOG_WARN | core::LOG_ERROR);
	core::Log_Info(TEXT("main.cpp - [%s]"), TEXT("Program is Release Mode"));
#endif

	CommunicationManager()->Init("9090");

	CommunicationManager()->Start();
	ServiceManager()->Init("HurryUp_Agent.out");

	CommunicationManager()->Recv();
}