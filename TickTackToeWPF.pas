﻿uses GraphABC;
  
  const N = 6;          //  размер поля. 
  const cellSize = 150; //  размер клетки 
  const border = 5;     //  величина отступа от края в пикселах
  const winLine = 4;

type GameStateEnum = (Ended, InProgress, Idle);

var field : array [1..N, 1..N] of integer;
    
    gameState : GameStateEnum;
    
    botName : string;
    

//  Рисование нолика в заданной клетке
//  row - строка (нумерация сверху)
//  col - столбец, нумерация слева направо
procedure DrawNolik(row,col : integer);
begin
  Pen.Width := 3;
  Pen.Color := Color.Red;
  var y := (row-1)*cellSize + cellSize div 2;
  var x := (col-1)*cellSize + cellSize div 2;
  Circle(x,y,cellSize div 2 - border);
end;

//  Рисование крестика в заданной клетке
//  row - строка (нумерация сверху)
//  col - столбец, нумерация слева направо
procedure DrawKrestik(row,col : integer);
begin
  Pen.Width := 3;
  Pen.Color := Color.Blue;
  var d := cellSize - 2*border;
  var y := (row-1)*cellSize + border;
  var x := (col-1)*cellSize + border;
  Line(x,y,x+d,y+d);
  Line(x,y+d,x+d,y);
end;

//  Рисование всего поля сразу, с предварительной очисткой
procedure DrawField;
begin
  //  Очищаем окно
  Window.Clear;
  
  //  Рисуем линии сетки
  Pen.Width := 1;
  Pen.Color := Color.Gray;
  for var i:=1 to N-1 do
    begin
      Line(border,cellSize*i,Window.Width-border,cellSize*i);
      Line(cellSize*i,border,cellSize*i,Window.Height-border);
    end;
  //  Рисуем крестики и нолики там, где они есть
  for var i:=1 to N do
    for var j:=1 to N do
      if field[i,j]=1 then DrawKrestik(i,j)
      else if field[i,j]=2 then DrawNolik(i,j);
end;

//  Инициализация игры
procedure InitGame;
begin
  for var r := 1 to N do
    for var c := 1 to N do 
      field[r,c] := 0;
  DrawField;
  gameState := GameStateEnum.InProgress;
end;

function FilledLine(r,c, dx,dy : integer) : integer;
begin
  Result := 0;
  if field[r,c] = 0 then exit;
  
  var (row, col) := (r+dx,c+dy);
  var count := 1;
  while (row>=1) and (col>=1) and (row<=N) and (col<=N) and (field[row,col] = field[r,c]) do
  begin
    count+=1;
    (row, col) := (row+dx,col+dy);
  end;
  
  if count >= winLine then
    Result := field[r,c];
end;


/// <summary>
/// Проверка победителя - собрать winLine одинаковых элементов в ряд (либо меньше, по размеру поля)
/// </summary>
/// <returns></returns>
function Winner : integer;
begin
  Result := 0;
  //  Проверка на выигрыш кого-либо
  //  Так как тут поле может быть произвольного размера, то метод другой 
  //  проходим по всем строкам, и каждую строку надо проверить на то, что
  //  она содержит одинаковые ненулевые элементы
  for var row := 1 to N do
    for var col := 1 to N do
    begin
      var rez := FilledLine(row,col,1,0);
      if rez <> 0 then
      begin
        Result := rez;
        Window.Caption := 'Есть победитель!';
        exit;
      end;
      rez := FilledLine(row,col,1,1);
      if rez <> 0 then
      begin
        Result := rez;
        Window.Caption := 'Есть победитель!';
        exit;
      end;
      rez := FilledLine(row,col,0,1);
      if rez <> 0 then
      begin
        Result := rez;
        Window.Caption := 'Есть победитель!';
        exit;
      end;
      rez := FilledLine(row,col,1,-1);
      if rez <> 0 then
      begin
        Result := rez;
        Window.Caption := 'Есть хитрый победитель!';
        exit;
      end;      
    end;
  
  //  Теперь предполагаем ничью. Если встретим хоть одну клетку свободную,
  //  то ничья отменяется
  Result := 3;
  //  Ищем пустую клетку, иначе ничья
  for var r:=1 to N do
    for var c:=1 to N do
      if field[r,c]=0 then
        Result := 0;
end;

//  Ход компьютера - хитрый! 
procedure ComputerMove;
begin
  //sleep(1000);
  for var i:=1 to N do
    for var j:=1 to N do
      if field[i,j]=0 then
      begin
        field[i,j] := 2;
        if Winner = 2 then 
          begin
            DrawField;
            Window.Caption := 'Ха! Я победил!';
            exit;
          end;
        field[i,j] := 0;
      end;
  for var i:=1 to N do
    for var j:=1 to N do
      if field[i,j]=0 then
      begin
        field[i,j] := 1;
        if Winner = 1 then 
          begin
            DrawField;
            Window.Caption := 'Фиг вам!';
            field[i,j] := 2;
            DrawField;
            exit;
          end;
        field[i,j] := 0;
      end;
  var (r,c) := Random2(1,N);
  while field[r,c] <> 0 do
    (r,c) := Random2(1,N);
  field[r,c] := 2;
  DrawField;
end;

//  Ход игрока - тут только надо поставить крестик, нарисовать и проверить 
//  на наличие победителя
procedure PlayerMove(x,y : integer);
begin
  field[x,y] := 1;
  if Winner = 0 then
    begin
      ComputerMove;
      if Winner <> 0 then
        begin
          gameState := GameStateEnum.Idle;
          Window.Caption := 'Победил игрок';
        end;
    end
  else
    begin
      Window.Caption := 'Победил игрок';
      gameState := GameStateEnum.Idle;
    end;
  DrawField;
end;

//  Реакция на нажатие мышки - координаты и норме кнопки
procedure MouseClick(x,y:integer; mb : integer);
begin
  if gameState <> GameStateEnum.InProgress then exit;
  var col := Floor(x / cellSize) + 1;
  var row := Floor(y / cellSize) + 1;
  if field[row,col] = 0 then
    PlayerMove(row,col);
  DrawField;
end;

begin
  Window.SetSize(N*cellSize,N*cellSize);

  DrawField;
  Window.CenterOnScreen;
  OnMouseDown := MouseClick;
  gameState := GameStateEnum.InProgress;
  
  
  
end.