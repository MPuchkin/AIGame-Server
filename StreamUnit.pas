//uses GraphWPF;
unit StreamUnit;
interface

type callBackProcedure = procedure(s : string);

type StreamDirector = class
  private
    callBackProc : callBackProcedure;
    stdErrStream : System.IO.Stream := nil;
    stdInStream  : System.IO.Stream := nil;
    errWriter : System.IO.StreamWriter := nil;
    inReader : System.IO.StreamReader := nil;
    receiveInProgress : boolean := false;
  public
    /// <summary>
    /// Связывание и открытие стандартных потоков
    /// </summary>
    constructor Create(proc : callBackProcedure);
    begin
      //  Стандартный поток ошибок привязываем
      stdErrStream := Console.OpenStandardError;
      errWriter := new System.IO.StreamWriter(stdErrStream);
      
      stdInStream := Console.OpenStandardInput;
      inReader := new System.IO.StreamReader(stdInStream);
      
      receiveInProgress := false;
      callBackProc := proc;
    end;
    
    /// <summary>
    /// Запуск потока обработки сообщений от другого процесса
    /// </summary>
    procedure StartThread;
    begin
      receiveInProgress := true;
      var newThread := new System.Threading.Thread(System.Threading.ThreadStart(DoWork));
      newThread.Start;
    end;

    procedure StopReceiving;
    begin
      receiveInProgress := false;
    end;

    procedure SendMove(s : string);
    begin
      WriteLn('move :' + s);
    end;

  private
    /// <summary>
    /// Тут собственно процесс получения данных из потока - починить!
    /// Самое главное - процедура в отдельном потоке работает, синхронизовать!
    /// </summary>
    procedure DoWork;
    begin
      while receiveInProgress do
      begin
        var line := ReadString;
        if (line.IndexOf('move :') = 1) and (callBackProc <> nil) then
          callBackProc(line[6:])
        else
          Println('Procedure not binded');
      end;
      println('Thread stopped!');
    end;
   
end;

implementation

end.