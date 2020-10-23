// Server.cpp : Defines the entry point for the console application.
// ��� ������ ���������� �������� ws2_32.lib


// ������ �������� TCP ���-�������
#include <stdio.h>
#include <winsock2.h> // Wincosk2.h ������ ���� ������ windows!
#include <windows.h>
#include <locale>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>
#include "Gamer.h"


const u_short PORT_IN = 11001;   // ���� ��� ����������
//const u_short PORT_OUT = 11001;  // ���� ��� "���������"

// �������� �������, ������������� �������������� �������������
DWORD WINAPI WorkWithClient(LPVOID client_socket);
//  DWORD WINAPI waitForClientsMonitors(LPVOID monitor_socket);

// ����������� ������
CRITICAL_SECTION list_ops;
//  ������ �������� �������
std::list<Gamer> gamers;
//  �������� �� ������
bool serverWorking = false;


/// <summary>
/// ��������� ��������� �� �������
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <param name="flags"></param>
/// <param name="bot"></param>
/// <returns>���� false, �� ������� ������� ���������</returns>
bool processClientString(char *buf, int len, int flags, Gamer & bot) {
	//  ��������� ��������� �� �������
	if (len > 0 && buf[0] < 32) return true;
	//  ������������ � string - ��������� �����������!
	if (buf[len - 1] != 0)
		buf[len] = 0;
	std::string message(buf);
	//std::cout << "Message parsing : " + message << std::endl;
	//  �������� ������� �� ������, � ����� �� � ������ �������� ��������
	std::string command, args;
	std::size_t delim = message.find('%');
	if (delim == std::string::npos) {
		command = message;
		args = "";
	}
	else {
		command = message.substr(0, delim);
		args = message.substr(delim + 1, message.size() - delim - 1);
	}
	/*std::cout << "Command : >" + command << "<" << std::endl;
	std::cout << "Args : " + args << std::endl;*/
	//  ��������� ��������� �� ���� � ������� ���������
	bot.logMessage("From client : " + message);

	//  ������������ �������
	if (command == "Login") {
		//std::cout << "Login parsing..." << std::endl;
		//  ����� �� ���������������?
		//  ���������, ���� �� � ����� ������ ��������, ���� �� - ���� � �������� (��������� ������)
		bot.leaveGame();
		
		//  ���������� �������� ������
		//  ������ ��� � ��� ����
		std::size_t delim = args.find('%');
		if (delim == std::string::npos) {
			bot.sendMessage("BadRequest");
			return true;
		}

		bot.botName = args.substr(0, delim);
		bot.gameType = gameNameToEnum(args.substr(delim + 1, message.size() - delim - 1));
		bot.botState = BotState::Idle;
		bot.score = 0;
		bot.opponentIt = gamers.end();
		std::cout << "New bot joined : " + bot.botName + '\n';
		return true;
	}

	//  ������������ �������
	if (command == "Logout" || command == "StopGame") {
		//  ���������� �������
		if (command == "StopGame") {
			bot.sendMessage("Info%Kicked for cheating");
		}
		//EnterCriticalSection(&list_ops);
		bot.leaveGame();
		gamers.remove(bot);
		//LeaveCriticalSection(&list_ops);
		return false;
	}

	if (command == "PassMove" || command == "Move" || command == "RequestField" || command == "Field") {
		if (bot.botState == BotState::GameInProgress) {
			if(bot.opponentIt != gamers.end())
				bot.opponentIt->sendMessage(message);
		}
		else
			bot.sendMessage("LogicError");
		return true;
	}

	if (command == "WinGame") {
		if (bot.botState != BotState::GameInProgress) {
			bot.sendMessage("LogicError");
			return true;
		}
		//EnterCriticalSection(&list_ops);
		if (bot.opponentIt != gamers.end()) {
			bot.opponentIt->sendMessage("FailGame");
			bot.opponentIt->score--;
			bot.opponentIt->sendMessage("Score%" + std::to_string(bot.opponentIt->score));
			bot.opponentIt->botState = BotState::Idle;
			bot.opponentIt->opponentIt = gamers.end();
		}
		bot.score++;
		bot.botState = BotState::Idle;
		bot.opponentIt = gamers.end();
		//LeaveCriticalSection(&list_ops);
		bot.sendMessage("Score%" + std::to_string(bot.score));
	}

	if (command == "FailGame") {
		if (bot.botState != BotState::GameInProgress) {
			bot.sendMessage("LogicError");
			return true;
		}
		//EnterCriticalSection(&list_ops);
		if (bot.opponentIt != gamers.end()) {
			bot.opponentIt->sendMessage("WinGame");
			bot.opponentIt->botState = BotState::Idle;
			bot.opponentIt->score++;
			bot.opponentIt->sendMessage("Score%" + std::to_string(bot.opponentIt->score));
			bot.opponentIt->opponentIt = gamers.end();
		}
		bot.score--;
		bot.botState = BotState::Idle;
		bot.opponentIt = gamers.end();
		//LeaveCriticalSection(&list_ops);
		bot.sendMessage("Score%" + std::to_string(bot.score));
	}

	if (command == "DrawGame") {
		if (bot.botState != BotState::GameInProgress) {
			bot.sendMessage("LogicError");
			return true;
		}
		//EnterCriticalSection(&list_ops);
		if (bot.opponentIt != gamers.end()) {
			bot.opponentIt->sendMessage("DrawGame");
			bot.opponentIt->sendMessage("Score%" + std::to_string(bot.opponentIt->score));
			bot.opponentIt->botState = BotState::Idle;
			bot.opponentIt->opponentIt = gamers.end();
		}
		bot.botState = BotState::Idle;
		bot.sendMessage("Score%" + std::to_string(bot.score));
		bot.opponentIt = gamers.end();
		//LeaveCriticalSection(&list_ops);
	}

	if (command == "Objection") {
		std::cout << bot.botName + " request for appeal:\n";
		bot.leaveGame();
		bot.saveDialogHistory();
		bot.sendMessage("Logout");
		return false;
	}

	bot.sendMessage("BadRequest");
	return true;
}

// ���������� ���������� � ���������� �������� �������������
int nclients = 0;

///  ����� ���������� � ���������� �������������
void printInfo() {
	std::cout << "������������� ������ : " << nclients << std::endl;
}

//---------------------------------------------------------------------------------------------------------------
// ��� ������� �������� � ��������� ������
// � ��������� ��������� ���������� ����� ������� - ����? 
// 
DWORD WINAPI connectOpponents(LPVOID monitor_socket)
{
	std::cout << "Connect opponents started.......\n";
	srand(static_cast<unsigned int>(time(NULL)));
	while (serverWorking) {
		//  �������� �� ������ � ��������� ���������� ��� ����
		std::multimap<GameType, std::list<Gamer>::iterator> readyForGame;
		EnterCriticalSection(&list_ops);

		//  ������ ���������� ��������� ������� �� ����� ���
		for (auto& it = gamers.begin(); it != gamers.end(); ++it)
			if (it->botState == BotState::Idle && it->gameType != GameType::Unknown) {
				readyForGame.insert(make_pair(it->gameType, it));
			}

		//  ������ ������������ �� ���������� ���������
		while (readyForGame.size() >= 2) {
			//  ������������ ������ ����
			std::cout << "Found " << readyForGame.size() << " gamers\n";
			GameType currentType = readyForGame.begin()->first;
			auto range = readyForGame.equal_range(currentType);
			size_t currentCount = std::distance(range.first, range.second);
			while (currentCount >= 2) {
				size_t Gamer1 = rand() % currentCount;
				size_t Gamer2 = rand() % currentCount;
				while (Gamer1 == Gamer2)
					Gamer2 = rand() % currentCount;
				auto it1 = readyForGame.begin();
				std::advance(it1, Gamer1);
				auto it2 = readyForGame.begin();
				std::advance(it2, Gamer2);
				std::cout << "Game started : " << it1->second->botName << " vs. " << it2->second->botName << std::endl;
				//  ������ �� ���� ����������
				it1->second->startGame(it2->second, true);
				it2->second->startGame(it1->second, false);
				//  ������� �� ������
				readyForGame.erase(it1);
				readyForGame.erase(it2);
				currentCount -= 2;
				
			}
			//  ��������� �� ������� �������, ��������� ������ ���� ���� - �� �������, � ��������� ���
			while (readyForGame.size() > 0 && currentCount > 0) {
				readyForGame.erase(readyForGame.begin());
				currentCount--;
			}
		}
		LeaveCriticalSection(&list_ops);
		Sleep(1000);
	}
	return 0;
}//--------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	setlocale(LC_ALL,".1251");
	serverWorking = false;
	char buff[1024]; // ����� ��� ��������� ����
	std::cout << "Game server starting...\n";
	// ��� 1 - ������������� ���������� �������
	// �. �. ������������ �������� ���������� �� ������������
	// �� ���������� ��������� �� ������� �����, ������������� � ���������
	// �� ��������� WSADATA.
	// ����� ����� ��������� ���������� ���� ����������, ������, �����
	// ������ ���� �� ����� ������������ �������� (��������� WSADATA
	// �������� 400 ����)
	if (WSAStartup(0x0202,(WSADATA *) &buff[0])) {
		// ������!
		std::cout << "������ WSAStartup " << WSAGetLastError() << " \n";
		return -1;
	}

	// ��� 2 - �������� ������
	SOCKET mysocket;
	// AF_INET - ����� ���������
	// SOCK_STREAM - ��������� ����� (� ���������� ����������)
	// 0 - �� ��������� ���������� TCP ��������
	if ((mysocket=socket(AF_INET,SOCK_STREAM,0))<0) {
		// ������!
		std::cout << "������ �������� ������ " << WSAGetLastError() << " \n";
		WSACleanup(); // �������������� ���������� Winsock
		return -1;
	}

	// ��� 3 ���������� ������ � ��������� �������
	sockaddr_in local_addr;
	local_addr.sin_family=AF_INET;
	local_addr.sin_port=htons(PORT_IN); // �� �������� � ������� �������!!!
	local_addr.sin_addr.s_addr=0; // ������ ��������� �����������
	// �� ��� ���� IP-������
	// �������� bind ��� ����������
	if (bind(mysocket,(sockaddr *) &local_addr, sizeof(local_addr))) {
		// ������
		std::cout << "������ ������������� ���������� ������ " << WSAGetLastError() << " \n";
		closesocket(mysocket); // ��������� �����!
		WSACleanup();
		return -1;
	}

	// ��� 4 �������� �����������
	// ������ ������� � 0x100
	if (listen(mysocket, 0x100)) {
		// ������
		std:: cout << "������ ��������� ���������� ������ " << WSAGetLastError() << " \n";
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	serverWorking = true;
	std::cout << "�������� �����������...\n";
	// ��� 5 ��������� ��������� �� �������
	SOCKET client_socket; // ����� ��� �������
	sockaddr_in client_addr; // ����� ������� (����������� ��������)
	// ������� accept ���������� �������� ������ ���������
	int client_addr_size=sizeof(client_addr);
	// ���� ���������� �������� �� ����������� �� �������
	InitializeCriticalSection(&list_ops);

	//  ��������� ������� ������� ����������
	DWORD thID;
	CreateThread(NULL,NULL,connectOpponents,&client_socket,NULL,&thID);

	//  ������� �������� ����������
	while((client_socket=accept(mysocket, (sockaddr *) &client_addr, &client_addr_size)))
	{
		nclients++; // ����������� ������� �������������� ��������
		// �������� �������� ��� �����
		//HOSTENT *hst;
		//hst=gethostbyaddr((char *) &client_addr.sin_addr.s_addr,4,AF_INET);
		// ����� �������� � �������
		//printf("+%s [%s] ����� ����������!\n",(hst)?hst->h_name:"",inet_ntoa(client_addr.sin_addr));
		printInfo();
			// ����� ������ ������ ��� ����������� �������
			// ��, ��� ����� ������������� ������������ _beginthreadex
			// ��, ��������� ������� ����� ������� ����������� �� ����������
			// ����� �� ������, ����� �������� � CreateThread
		DWORD thID;
		CreateThread(NULL,NULL,WorkWithClient,&client_socket,NULL,&thID);
	}
	//  ������� ����������� ������
	serverWorking = false;
	DeleteCriticalSection(&list_ops);
	return 0;
}


//---------------------------------------------------------------------------------------------------------------
// ��� ������� �������� � ��������� ������
// � ���������� ���������� ��������������� ������� ���������� �� ���������
DWORD WINAPI WorkWithClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock=((SOCKET *) client_socket)[0];

	// ������� �������, ��������� ��� � ������ ��������
	
	EnterCriticalSection(&list_ops);
	gamers.push_back(Gamer(my_sock));
	auto botIter = gamers.end();
	botIter--;
	LeaveCriticalSection(&list_ops);
	char buff[20*1024];

	// ���� ���-�������: ����� ������ �� ������� � ����������� �� �������
	int bytes_recv;
	while((bytes_recv=recv(my_sock,&buff[0],sizeof(buff),0)) && bytes_recv !=SOCKET_ERROR) {
		//  �������� ������ - ���� ������������, ��������� ���� ��������
		EnterCriticalSection(&list_ops);
		if (processClientString(&buff[0], bytes_recv, 0, *botIter) == false) break;
		LeaveCriticalSection(&list_ops);
	}

	// ��������� �����
	botIter->leaveGame();
	closesocket(botIter->clientSocket);
	EnterCriticalSection(&list_ops);

	gamers.erase(botIter);
	LeaveCriticalSection(&list_ops);

	// ���� �� �����, �� ��������� ����� �� ����� �� �������
	// ���������� �������� recv ������ � ���������� � �������� ���������
	nclients--; // ��������� ������� �������� ��������
	printf("-����������\n"); 
	printInfo();

	return 0;
}

