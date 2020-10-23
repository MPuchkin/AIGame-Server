
unit TCPUnit;

interface

uses System.IO;
uses System.Net;
uses System.Net.Sockets;


type callBackProcedure = procedure(s : string);

type StreamDirector = class
  private
    callBackProc : callBackProcedure;
   
    clientSocket : TcpClient := nil;
    
    bw : BinaryWriter := nil;
    br : BinaryReader := nil;
    listener : System.Threading.Thread := nil;
    connected : boolean := false;
  public
    /// <summary>
    /// Связывание и открытие стандартных потоков
    /// </summary>
    constructor Create(proc : callBackProcedure);
    begin
      clientSocket := new TcpClient();
      clientSocket.Connect('localhost', 11001);
      
      bw := new BinaryWriter(clientSocket.GetStream());
      br := new BinaryReader(clientSocket.GetStream());
      
      //  Стандартный поток ошибок привязываем
      callBackProc := proc;
      listener := new System.Threading.Thread(System.Threading.ThreadStart(DoWork));
      connected := true;
      listener.Start;
    end;

    procedure StopReceiving;
    begin
      if connected then listener.Suspend;
      connected := false;
    end;

    procedure SendMessage(s : string);
    begin
      
      if connected then
        begin
          var buffer := Encoding.ASCII.GetBytes(s);
          bw.Write(buffer);
        end;
    end;

  private
    /// <summary>
    /// Тут собственно процесс получения данных из потока - починить!
    /// Самое главное - процедура в отдельном потоке работает, синхронизовать!
    /// </summary>
    procedure DoWork;
    begin
      var line : string;
      while connected do
      begin
        var ch := br.ReadByte;
        if ch <> 0 then
          line += ChrAnsi(ch)
        else
          begin
            callBackProc(line);
            line := '';
          end;
      end;
      println('Thread stopped!');
    end;
   
end;

implementation

end.