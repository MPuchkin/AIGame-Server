// Server.cpp : Defines the entry point for the console application.
// Для работы необходимо добавить ws2_32.lib


// Пример простого TCP эхо-сервера
#include <stdio.h>
#include <winsock2.h> // Wincosk2.h должен быть раньше windows!
#include <windows.h>
#include <locale>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>
#include "Gamer.h"


const u_short PORT_IN = 11001;   // Порт для соединений
//const u_short PORT_OUT = 11001;  // Порт для "исходящих"

// Прототип функции, обслуживающий подключившихся пользователей
DWORD WINAPI WorkWithClient(LPVOID client_socket);
//  DWORD WINAPI waitForClientsMonitors(LPVOID monitor_socket);

// критическая секция
CRITICAL_SECTION list_ops;
//  список активных игроков
std::list<Gamer> gamers;
//  работает ли сервер
bool serverWorking = false;


/// <summary>
/// Обработка сообщения от клиента
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
/// <param name="flags"></param>
/// <param name="bot"></param>
/// <returns>Если false, то данного клиента отключить</returns>
bool processClientString(char *buf, int len, int flags, Gamer & bot) {
	//  Обработка сообщения от клиента
	if (len > 0 && buf[0] < 32) return true;
	//  Конвертируем в string - проверить обязательно!
	if (buf[len - 1] != 0)
		buf[len] = 0;
	std::string message(buf);
	//std::cout << "Message parsing : " + message << std::endl;
	//  Получаем команду из строки, и потом на её основе выбираем действие
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
	//  Добавляем сообщение от бота в историю сообщений
	bot.logMessage("From client : " + message);

	//  Обрабатываем команду
	if (command == "Login") {
		//std::cout << "Login parsing..." << std::endl;
		//  Вроде бы переподключение?
		//  Проверяем, есть ли у этого игрока оппонент, если да - плюс к рейтингу (противник сбежал)
		bot.leaveGame();
		
		//  Сбрасываем текущего игрока
		//  Парсим имя и тип игры
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

	//  Обрабатываем команду
	if (command == "Logout" || command == "StopGame") {
		//  Отключение клиента
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

// глобальная переменная – количество активных пользователей
int nclients = 0;

///  Вывод информации о количестве пользователей
void printInfo() {
	std::cout << "Пользователей онлайн : " << nclients << std::endl;
}

//---------------------------------------------------------------------------------------------------------------
// Эта функция работает в отдельном потоке
// и постоянно подбирает соперников среди игроков - пару? 
// 
DWORD WINAPI connectOpponents(LPVOID monitor_socket)
{
	std::cout << "Connect opponents started.......\n";
	srand(static_cast<unsigned int>(time(NULL)));
	while (serverWorking) {
		//  Пройтись по списку и подобрать оппонентов для игры
		std::multimap<GameType, std::list<Gamer>::iterator> readyForGame;
		EnterCriticalSection(&list_ops);

		//  Задали количество свободных игроков по типам игр
		for (auto& it = gamers.begin(); it != gamers.end(); ++it)
			if (it->botState == BotState::Idle && it->gameType != GameType::Unknown) {
				readyForGame.insert(make_pair(it->gameType, it));
			}

		//  Теперь обрабатываем по количеству элементов
		while (readyForGame.size() >= 2) {
			//  Подхватываем первый ключ
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
				//  Теперь им игру устраиваем
				it1->second->startGame(it2->second, true);
				it2->second->startGame(it1->second, false);
				//  Удаляем из списка
				readyForGame.erase(it1);
				readyForGame.erase(it2);
				currentCount -= 2;
				
			}
			//  Доудаляем из очереди игроков, ожидающих именно этой игры - не повезло, в следующий раз
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
	char buff[1024]; // Буфер для различных нужд
	std::cout << "Game server starting...\n";
	// Шаг 1 - Инициализация Библиотеки Сокетов
	// Т. к. возвращенная функцией информация не используется
	// ей передается указатель на рабочий буфер, преобразуемый к указателю
	// на структуру WSADATA.
	// Такой прием позволяет сэкономить одну переменную, однако, буфер
	// должен быть не менее полкилобайта размером (структура WSADATA
	// занимает 400 байт)
	if (WSAStartup(0x0202,(WSADATA *) &buff[0])) {
		// Ошибка!
		std::cout << "Ошибка WSAStartup " << WSAGetLastError() << " \n";
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;
	// AF_INET - сокет Интернета
	// SOCK_STREAM - потоковый сокет (с установкой соединения)
	// 0 - по умолчанию выбирается TCP протокол
	if ((mysocket=socket(AF_INET,SOCK_STREAM,0))<0) {
		// Ошибка!
		std::cout << "Ошибка создания сокета " << WSAGetLastError() << " \n";
		WSACleanup(); // Деиницилизация библиотеки Winsock
		return -1;
	}

	// Шаг 3 связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family=AF_INET;
	local_addr.sin_port=htons(PORT_IN); // не забываем о сетевом порядке!!!
	local_addr.sin_addr.s_addr=0; // сервер принимаем подключения
	// на все свои IP-адреса
	// вызываем bind для связывания
	if (bind(mysocket,(sockaddr *) &local_addr, sizeof(local_addr))) {
		// Ошибка
		std::cout << "Ошибка инициализации слушающего сокета " << WSAGetLastError() << " \n";
		closesocket(mysocket); // закрываем сокет!
		WSACleanup();
		return -1;
	}

	// Шаг 4 ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 0x100)) {
		// Ошибка
		std:: cout << "Ошибка включения слушающего сокета " << WSAGetLastError() << " \n";
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}
	serverWorking = true;
	std::cout << "Ожидание подключений...\n";
	// Шаг 5 извлекаем сообщение из очереди
	SOCKET client_socket; // сокет для клиента
	sockaddr_in client_addr; // адрес клиента (заполняется системой)
	// функции accept необходимо передать размер структуры
	int client_addr_size=sizeof(client_addr);
	// цикл извлечения запросов на подключение из очереди
	InitializeCriticalSection(&list_ops);

	//  Запускаем процесс подбора оппонентов
	DWORD thID;
	CreateThread(NULL,NULL,connectOpponents,&client_socket,NULL,&thID);

	//  Ожидаем входящих соединений
	while((client_socket=accept(mysocket, (sockaddr *) &client_addr, &client_addr_size)))
	{
		nclients++; // увеличиваем счетчик подключившихся клиентов
		// пытаемся получить имя хоста
		//HOSTENT *hst;
		//hst=gethostbyaddr((char *) &client_addr.sin_addr.s_addr,4,AF_INET);
		// вывод сведений о клиенте
		//printf("+%s [%s] новое соединение!\n",(hst)?hst->h_name:"",inet_ntoa(client_addr.sin_addr));
		printInfo();
			// Вызов нового потока для обслужвания клиента
			// Да, для этого рекомендуется использовать _beginthreadex
			// но, поскольку никаких вызов функций стандартной Си библиотеки
			// поток не делает, можно обойтись и CreateThread
		DWORD thID;
		CreateThread(NULL,NULL,WorkWithClient,&client_socket,NULL,&thID);
	}
	//  удаляем критическую секцию
	serverWorking = false;
	DeleteCriticalSection(&list_ops);
	return 0;
}


//---------------------------------------------------------------------------------------------------------------
// Эта функция работает в отдельном потоке
// и обсуживает очередного подключившегося клиента независимо от остальных
DWORD WINAPI WorkWithClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock=((SOCKET *) client_socket)[0];

	// Создать клиента, поместить его в список клиентов
	
	EnterCriticalSection(&list_ops);
	gamers.push_back(Gamer(my_sock));
	auto botIter = gamers.end();
	botIter--;
	LeaveCriticalSection(&list_ops);
	char buff[20*1024];

	// цикл эхо-сервера: прием строки от клиента и возвращение ее клиенту
	int bytes_recv;
	while((bytes_recv=recv(my_sock,&buff[0],sizeof(buff),0)) && bytes_recv !=SOCKET_ERROR) {
		//  получены данные - надо обрабатывать, переслать всем клиентам
		EnterCriticalSection(&list_ops);
		if (processClientString(&buff[0], bytes_recv, 0, *botIter) == false) break;
		LeaveCriticalSection(&list_ops);
	}

	// закрываем сокет
	botIter->leaveGame();
	closesocket(botIter->clientSocket);
	EnterCriticalSection(&list_ops);

	gamers.erase(botIter);
	LeaveCriticalSection(&list_ops);

	// если мы здесь, то произошел выход из цикла по причине
	// возращения функцией recv ошибки – соединение с клиентом разорвано
	nclients--; // уменьшаем счетчик активных клиентов
	printf("-отключение\n"); 
	printInfo();

	return 0;
}

