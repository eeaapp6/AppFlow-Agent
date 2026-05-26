
call "E:\tool\vsIDE\vs2017\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.17763.0

SET "PATH=C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\;E:\tool\vsIDE\vs2017\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64;%PATH%"

qmake -r -tp vc FlowApp.pro

pause
