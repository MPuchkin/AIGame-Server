{$reference 'GameAlgs.dll'}

uses GameAlgs;

begin
  var s := Solver.solve('String',10);
  println(s);
  print(s.Length);
end.