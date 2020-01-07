setlocal
set z88root=c:\users\charlie\bin\z88dk\

set path=%PATH%;%z88root%bin\
set zcccfg=%z88root%lib\config\
set z80_ozfiles=%z88root%lib\

zcc +zx81 -startup=2 -create-app z8048.c
endlocal
