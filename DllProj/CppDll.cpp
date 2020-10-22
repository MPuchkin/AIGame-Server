/* Ёто обЄртка дл€ методов модул€ state, тут запуск решени€ и возврат результатов наружу.
*  ѕо идее, это должно компилироватьс€ в dll дл€ x64, и полученную библиотеку надо подсунуть 
*  приложению на C#
*/


#include <string>
#include <sstream>
#include <iterator>


namespace Solver {
	
	/// <summary>
	/// ќсновна€ функци€ библиотеки, возвращаемое значение через тот же буфер
	/// ¬ нормальной системе тут получить состо€ние в виде массива байтов (строка), решить и обратно в этот массив всЄ записать
	/// </summary>
	extern "C" __declspec(dllexport) int solveState(char *buffer, int * buffSize, int maxBuffSize, int depth, int * score)
	{
		std::string Result;
		for (int i = 0; i < *buffSize; ++i)
			Result += char(buffer[i]);

		//  “ут кака€-то обработка, вот только непон€тно кака€. —троки однобайтовые!
		std::string seconPart(Result);
		std::reverse(seconPart.begin(), seconPart.end());
		Result += seconPart;
		for (size_t i = 0; i < Result.size(); ++i)
			if (isupper(Result[i]))
				Result[i] = tolower(Result[i]);
			else
				Result[i] = toupper(Result[i]);
		
		*buffSize = std::min(maxBuffSize, int(Result.size()));
		for (int i = 0; i < *buffSize; ++i)
			buffer[i] = Result.c_str()[i];

		return *buffSize;
	}
}
