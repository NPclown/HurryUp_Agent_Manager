#include "CCommunication.h"
#include "CService.h"

CCommunication::CCommunication()
{
}

CCommunication::~CCommunication()
{
}

void CCommunication::Send(int protocol, std::tstring data)
{
	ST_PACKET_INFO stPacketInfo(protocol, data);
	std::tstring jsPacketInfo;
	core::WriteJsonToString(&stPacketInfo, jsPacketInfo);

	std::tstring message = TEXT("BOBSTART") + jsPacketInfo + TEXT("BOBEND");
	int result = write(clientSocket, message.c_str(), message.length());

	if (result == -1)
		core::Log_Warn(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Send Error Code"), errno);
	else
	{
		core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Send Complete"), result);
		core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %s"), TEXT("Send Message"), TEXT(message.c_str()));
	}
}

void CCommunication::Recv()
{
	core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("Working Recv In Thread"));

	while (1)
	{
		int clientLength = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, (socklen_t*)&clientLength);
		if (clientSocket < 0)
		{
			printf("Server: accept failed.\n");
			exit(0);
		}

		int messageLength;
		char message[BUFFER_SIZE + 1];

		while (1) {

			messageLength = read(clientSocket, &message, BUFFER_SIZE);
			core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Read Complete"), messageLength);
			core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %s"), TEXT("Recieve Message"), message);

			// 서버가 종료되었을 경우
			if (messageLength == 0)
			{
				//Disconnect();
				break;
			}
			// 수신 받을 읽기에 문제가 생긴 경우, 보통 읽을 메세지가 없는 경우
			else if (messageLength < 0)
			{
				core::Log_Warn(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Read Error Code"), errno);
				core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %s"), TEXT("Remain MessageBuffer"), TEXT(messageBuffers.c_str()));
				break;
			}
			// 메세지를 정상적으로 수신했을 경우
			else
			{
				message[messageLength] = 0;
				messageBuffers += message;

				while (1)
				{
					size_t start_location = messageBuffers.find("BOBSTART");
					size_t end_location = messageBuffers.find("BOBEND");
					core::Log_Debug(TEXT("CCommunication.cpp - [%s] : %d, %d"), TEXT("Find Position(Start, End)"), start_location, end_location);

					if (start_location == size_t(-1) || end_location == size_t(-1))
						break;

					ST_PACKET_INFO stPacketInfo;
					core::ReadJsonFromString(&stPacketInfo, messageBuffers.substr(start_location + 8, end_location - (start_location + 8)));

					Match(stPacketInfo.protocol, stPacketInfo.data);
					messageBuffers = messageBuffers.substr(end_location + 6);
				}
			}
			memset(message, 0, sizeof(message));
		}
	}
}

void CCommunication::Match(int protocol, std::tstring data)
{
	switch (protocol)
	{
	case AGENT_INIT:
		core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("AGENT_INIT"));
		Send(AGENT_INIT, ServiceManager()->AgentEnvironment(data));
		break;
	case AGENT_START:
		core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("AGENT_START"));
		Send(AGENT_START, ServiceManager()->AgentExecute());
		break;
	case AGENT_STOP:
		core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("AGENT_STOP"));
		Send(AGENT_STOP, ServiceManager()->AgentTerminate());
		break;
	case AGENT_UPDATE:
		core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("AGENT_UPDATE"));
		Send(AGENT_UPDATE, ServiceManager()->AgentUpdate(data));
		break;
	case AGENT_STATUS:
		core::Log_Debug(TEXT("CCommunication.cpp - [%s]"), TEXT("AGENT_DELETE"));
		Send(AGENT_STATUS, ServiceManager()->AgentStatus());
		break;
	default:
		Send(ERROR, "Protocol Is Invalid");
		core::Log_Warn(TEXT("CCommunication.cpp - [%s]"), TEXT("Protocol Is Invalid"));
	}
}

CCommunication* CCommunication::GetInstance(void)
{
	static CCommunication instance;
	return &instance;
}

void CCommunication::Init(std::tstring port)
{
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(uint16_t(atoi(port.c_str())));
}

void CCommunication::Start()
{
	core::Log_Info(TEXT("CCommunication.cpp - [%s]"), TEXT("CCommunication Setting Start"));
	int requestCount = 5;
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (serverSocket < 0)
		core::Log_Warn(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Socket Create Fail"), errno);

	while (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		sleep(5);
		core::Log_Warn(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Socket Bind Fail"), errno);
	}

	while (listen(serverSocket, requestCount) < 0)
	{
		sleep(5);
		core::Log_Warn(TEXT("CCommunication.cpp - [%s] : %d"), TEXT("Socket Listen Fail"), errno);
	}

	core::Log_Info(TEXT("CCommunication.cpp - [%s]"), TEXT("CCommunication Setting End"));
}

void CCommunication::End()
{
}
