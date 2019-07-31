@echo off

break>.\src\game_unity.cpp

for %%f in (.\src\game\*.cpp) do (
  REM set /p val=<%%f
  REM echo "name: %%~nf"
  IF "%%~nf"=="game_unity" (
      REM echo "UNITY!"
  ) ELSE (
      echo #include "%%~nf.cpp">>.\src\game_unity.cpp
  )
  
)