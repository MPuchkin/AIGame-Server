/* ��� ������ ��� ������� ������ state, ��� ������ ������� � ������� ����������� ������.
*  �� ����, ��� ������ ��������������� � dll ��� x64, � ���������� ���������� ���� ��������� 
*  ���������� �� C#
*/


#include <string>
#include <sstream>
#include <iterator>


namespace Solver {
	
	/// <summary>
	/// �������� ������� ����������, ������������ �������� ����� ��� �� �����
	/// � ���������� ������� ��� �������� ��������� � ���� ������� ������ (������), ������ � ������� � ���� ������ �� ��������
	/// </summary>
	extern "C" __declspec(dllexport) int solveState(char *buffer, int * buffSize, int maxBuffSize, int depth, int * score)
	{
		std::string Result;
		for (int i = 0; i < *buffSize; ++i)
			Result += char(buffer[i]);

		//  ��� �����-�� ���������, ��� ������ ��������� �����. ������ ������������!
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
