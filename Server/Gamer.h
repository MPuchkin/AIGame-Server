#pragma once
#include <string>
#include <list>
#include <fstream>
#include <winsock2.h>

/// <summary>
/// ��� ���� - ����� ��� ������ ���������� �� ������
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
/// ��������� ����-������ � ������� ������
/// </summary>
enum class BotState {
	Connecting,     //  � �������� ����������� � �������������
	Idle,           //  �� ������, ��� ���������
	GameInProgress, //  ������ � ���-�� � ������ ������
	Blocked,        //  ��� ������� �� ��� � ����
	Error           //  ��������� ������
};

/// <summary>
/// ���������, �������������� ����
/// </summary>
struct Gamer {
	
	/// <summary>
	/// ��� ����
	/// </summary>
	std::string botName;
	
	/// <summary>
	/// �� ��� ��� ������
	/// </summary>
	GameType gameType;
	
	/// <summary>
	/// ������� ��������� ����
	/// </summary>
	BotState botState;
	
	/// <summary>
	/// ������� ���� ������
	/// </summary>
	int score;
	
	/// <summary>
	/// �������� �� ����������
	/// </summary>
	std::list<Gamer>::iterator opponentIt;
	
	/// <summary>
	/// ������ ����� ��� ��������� ���������
	/// </summary>
	std::list<std::string> dialogHistory;
	
	/// <summary>
	/// �����, �� �������� ���������� ������ � ��������
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
	/// ����� �� ���� � ����������� ����� ���������
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
	/// �������� ��������� � ������� ���������
	/// </summary>
	/// <param name="message"></param>
	void logMessage(const std::string& message) {
		dialogHistory.push_back(message);
		while (dialogHistory.size() > 5000)
			dialogHistory.pop_front();
	}
	
	/// <summary>
	/// ������ ������� ����� � ����
	/// </summary>
	void saveDialogHistory() {
		std::string filename("log" + botName + ".log");
		std::ofstream out(filename);
		for (const auto& str : dialogHistory)
			out << str << std::endl;
		out.close();
	}

	/// <summary>
	/// ��������� �� ��������� ���� ��������
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool operator==(const Gamer& other) const {
		return botName == other.botName && clientSocket == other.clientSocket;
	}
};

/// <summary>
/// ������������ ���� ���� �� ������ ������
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
