#pragma once
#include <string>
#include <list>
#include <fstream>
#include <winsock2.h>

/// <summary>
/// Тип игры - нужен для поиска соперников из списка
/// </summary>
enum class GameType {
	Unknown,
	Infection,
	FourInRow,
	Reversi,
	Gomoku,
	Corners,
	NineMensMorris,
	WolfsAndSheeps
};

/// <summary>
/// Состояние бота-игрока в текущий момент
/// </summary>
enum class BotState {
	Connecting,     //  В процессе подключения и идентификации
	Idle,           //  Не играет, ждёт соперника
	GameInProgress, //  Играет с кем-то в данный момент
	Blocked,        //  Бан получил за мат в чате
	Error           //  Состояние ошибки
};

/// <summary>
/// Структура, представляющая бота
/// </summary>
struct Gamer {
	
	/// <summary>
	/// Имя бота
	/// </summary>
	std::string botName;
	
	/// <summary>
	/// Во что бот играет
	/// </summary>
	GameType gameType;
	
	/// <summary>
	/// Текущее состояние бота
	/// </summary>
	BotState botState;
	
	/// <summary>
	/// Текущий счёт игрока
	/// </summary>
	int score;
	
	/// <summary>
	/// Итератор на противника
	/// </summary>
	std::list<Gamer>::iterator opponentIt;
	
	/// <summary>
	/// Список ходов для возможных апелляций
	/// </summary>
	std::list<std::string> dialogHistory;
	
	/// <summary>
	/// Сокет, по которому происходит работа с клиентом
	/// </summary>
	SOCKET clientSocket;

	Gamer(SOCKET clientSock) : clientSocket(clientSock), score(0), botState(BotState::Connecting),
		botName("John Doe"), gameType(GameType::Unknown) {}

	bool sendMessage(const std::string& message) {
		logMessage(" To bot : " + message);
		//std::cout << "Last string symbol : " << message.c_str()[message.size() + 1] << std::endl;
		return send(clientSocket, message.c_str(), message.size() + 1, 0) != SOCKET_ERROR;
	}

	bool startGame(std::list<Gamer>::iterator& otherIt, bool firstMove) {
		botState = BotState::GameInProgress;
		opponentIt = otherIt;
		logMessage("Started game with " + otherIt->botName);
		if (firstMove) {
			logMessage("Takes first move");
			sendMessage("StartFirst");
		}
		else {
			logMessage("Takes second move");
			sendMessage("StartSecond");
		}
		return true;
	}

	/// <summary>
	/// Выход из игры с увеличением счёта оппонента
	/// </summary>
	/// <returns></returns>
	bool leaveGame() {
		if (botState == BotState::GameInProgress) {
			opponentIt->score++;
			opponentIt->botState = BotState::Idle;
			opponentIt->sendMessage("StopGame");
			botState = BotState::Idle;
			return true;
		}
		return false;
	}
	
	/// <summary>
	/// Добавить сообщение к истории сообщений
	/// </summary>
	/// <param name="message"></param>
	void logMessage(const std::string& message) {
		dialogHistory.push_back(message);
		while (dialogHistory.size() > 5000)
			dialogHistory.pop_front();
	}
	
	/// <summary>
	/// Запись истории ходов в файл
	/// </summary>
	void saveDialogHistory() {
		std::string filename("log" + botName + ".log");
		std::ofstream out(filename);
		for (const auto& str : dialogHistory)
			out << str << std::endl;
		out.close();
	}

	/// <summary>
	/// Сравнение на равенство двух клиентов
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool operator==(const Gamer& other) const {
		return botName == other.botName && clientSocket == other.clientSocket;
	}
};

/// <summary>
/// Перечисление типа игры на основе строки
/// </summary>
/// <param name="game"></param>
/// <returns></returns>
GameType gameNameToEnum(std::string game) {
	if (game == "Infection") return GameType::Infection;
	if (game == "FourInRow") return GameType::FourInRow;
	if (game == "Reversi") return GameType::Reversi;
	if (game == "Gomoku") return GameType::Gomoku;
	if (game == "Corners") return GameType::Corners;
	if (game == "NineMensMorris") return GameType::NineMensMorris;
	if (game == "WolfsAndSheeps") return GameType::WolfsAndSheeps;
	return GameType::Unknown;
}
