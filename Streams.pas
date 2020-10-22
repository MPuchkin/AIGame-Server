uses StreamUnit;


procedure MakeMove;
begin
  
end;




/// <summary>
/// Процедура обработки хода от программы-соперника
/// </summary>
/// <param name="s">Ход противника в виде строки</param>
procedure ProcessInput(s : string);
begin
  ///  Парсинг хода противника
  Println(s);
  ///  Вывод на экран
  
  ///  И обработка собственного хода
  
end;

begin
  Println('Агрументы командной строки -', commandLineArgs.Length,'всего :');
  commandLineArgs.Println;
  
  var streamShit := new StreamDirector(ProcessInput);
  streamShit.StartThread;
  Writeln('Hello, all!');
  sleep(3000);
  streamShit.StopReceiving;
  sleep(10000);
end.