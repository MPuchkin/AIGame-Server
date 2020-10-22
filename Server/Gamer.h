#pragma once
#include <string>
#include <list>
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
