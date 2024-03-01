@echo off
::设置工程路径-需要设置
::cd code
::显示当前路径
echo %cd%
::变量延迟的启动语句
setlocal enabledelayedexpansion
::获取当前路径的解决方案
set project=f
for /f %%f in ('dir /a-d /b *.sln *.vcxproj') do (
    set project=%%~ff
)
::判断工程是否存在
if exist %project% (
    echo %project%
) else (
    echo %project%
    exit 2
)
::开始构建
echo "building ...."
::获取vcvarsall.bat-需要设置
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
::编译程序设置构建模式有错误信息输出到文件msbulid.err中
MSBuild %project% /t:Build /p:Configuration=Release /flp2:errorsonly;logfile=msbuild.err
::构建完成
echo build complete
::获取msbuild.err文件大小
for /f %%i in ('dir /a-d /b msbuild.err') do ( set filesize=%%~zi )
::打印当前日期
date /t
::打印当前时间
time /t
echo %cd%\msbuild.err %filesize%
pause
::判断msbuild.err文件大小是否为0
if %filesize%==0 (
    del msbuild.err
    exit 0
)
::判断msbuild.err文件是否有数据
if %filesize% gtr 0 (
    exit 1
)

exit 2
