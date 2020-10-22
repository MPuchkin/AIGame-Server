#pragma once
#include <string>
#include <list>
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
