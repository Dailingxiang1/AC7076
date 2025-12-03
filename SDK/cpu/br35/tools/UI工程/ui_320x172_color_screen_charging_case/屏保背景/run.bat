@echo off
setlocal enabledelayedexpansion

:: 设置源文件夹和目标文件夹路径
set "source_folder=.\images"
set "target_folder=.\tmp"

:: 确保目标文件夹存在
if not exist "%target_folder%" (
    mkdir "%target_folder%"
)

:: 逐个移动文件
for %%F in ("%source_folder%\*.*") do (
	del /q ".\tmp\*"
    echo %%~nF
    copy "%%F" "%target_folder%"
	.\..\..\UITools\ResBuilder.exe
	REM echo move
	move .\result.bin .\vie_res\%%~nF
)
del /q ".\tmp\*"
echo succ

set "folderA=.\vie_res"
set "folderB=.\..\..\..\csc_ui_resource"

:: 复制文件夹A的所有文件到文件夹B（不包含子目录）
copy "%folderA%\*" "%folderB%\" /y


pause