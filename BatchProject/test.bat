:: 用于编译的命令，并自动检查编译结果
@echo on
set uv4=C:\Users\SheldonLiu\AppData\Local\Keil_v5\UV4\UV4.exe
set project=D:\ScholarStation\SelfBoardProject\MDK-ARM\TinyMLPT.uvprojx
set output=D:\ScholarStation\SelfBoardProject\BatchProject\TinyMLPT.axf
set debug=D:\ScholarStation\SelfBoardProject\BatchProject\TinyMLPT.elf
set log=D:\ScholarStation\SelfBoardProject\BatchProject\build.log
%uv4% -j0 -b %project% -l %log%
if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%
    exit /b %errorlevel%
)

echo Build successful

:: 用于Download的命令
::%uv4% j0 -f %project% 
exit /b 0