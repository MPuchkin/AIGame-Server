using System;
using System.Threading;
using System.Runtime.InteropServices;


namespace GameAlgs
{
    /// <summary>
    /// Вспомогательный класс, для обращения к нативной dll
    /// </summary>
    class Helper
    {
        [DllImport("CppDll.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        //public static extern int solveState(byte[] buff, int buffSize, int depth);
        //extern "C" __declspec(dllexport) int solveState(char* buffer, int* buffSize, int maxBuffSize, int depth, int* score)


        unsafe public static extern int solveState(IntPtr buff, IntPtr buffSize, int maxBuffSize, int depth, IntPtr score);
    }
    /// <summary>
    /// Основной класс - "решатель" задачки и выдача ответа
    /// </summary>
    public class Solver
    {
        /// <summary>
        /// Решение состояния и предложение хода
        /// </summary>
        /// <param name="State">Строка, представляющая игровое поле</param>
        /// <param name="depth">Глубина поиска</param>
        public static string solve(string State, int depth)
        {

            //  Основная проблема - перекинуть строку в кодировку ASCII
            //  Теоретически, строка в кодировке UTF
            byte[] sourceBytes = System.Text.Encoding.UTF8.GetBytes(State);
            byte[] asciiBytes = System.Text.Encoding.Convert(System.Text.Encoding.UTF8, System.Text.Encoding.ASCII, sourceBytes);

            unsafe
            {

                int bufferSize = asciiBytes.Length;
                int maxBufferSize = 2 * bufferSize;
                // Инициализируем массив в неуправляемой памяти - удвоенного размера, через него же результат будет возвращаться
                int size = Marshal.SizeOf(asciiBytes[0]) * maxBufferSize;
                IntPtr ptrToBuffer = Marshal.AllocHGlobal(size);
                Marshal.Copy(asciiBytes, 0, ptrToBuffer, bufferSize);

                //  Выделяем память под актуальный размер буфера и присваиваем текущий размер буфера
                IntPtr ptrToBufferSize = Marshal.AllocHGlobal(Marshal.SizeOf(bufferSize));
                Marshal.WriteIntPtr(ptrToBufferSize, 0, ((IntPtr)(bufferSize)));

                //  Выделяем память под возвращение результата
                //  Эта переменная не имеет начального значения - просто так, функция заполнит
                int Result = 0;
                IntPtr newScore = Marshal.AllocHGlobal(Marshal.SizeOf(Result));
                try
                {
                    //  Собственно, обёртка для вызова функции из нативной dll
                    //unsafe public static extern int SolveState(IntPtr buff, IntPtr buffSize, int maxBuffSize, int depth, IntPtr score);
                    Result = Helper.solveState(ptrToBuffer, ptrToBufferSize, bufferSize * 2, depth, newScore);

                    //  Обратное копирование - теперь в новый буфер, с сохранением
                    //  Сначала узнаём, что там функция насчитала в нашем буфере
                    int bytesReturned = (int)Marshal.PtrToStructure(ptrToBufferSize, typeof(int));
                    byte[] resultBytes = new byte[bytesReturned];
                    Marshal.Copy(ptrToBuffer, resultBytes, 0, bytesReturned);
                    //  Результат сохраняем - вот только куда его девать - непонятно!
                    Result = (int)Marshal.PtrToStructure(newScore, typeof(int));

                    //  Теперь эту пакость надо в строку превратить
                    return System.Text.Encoding.ASCII.GetString(resultBytes);
                }
                finally
                {
                    Marshal.FreeHGlobal(ptrToBuffer);
                    Marshal.FreeHGlobal(ptrToBufferSize);
                    Marshal.FreeHGlobal(newScore);
                }
            }
        }
    }


}
