@echo off
chcp 65001>NUL
setlocal

:: setup colors
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set ESC=%%b
)

set "RootDir=%~dp0"
set SrcDir=%RootDir%src\
if "%1" == "debug" (
	set BuildDir=%RootDir%build_debug\
	echo Building debug version...
) else (
	set BuildDir=%RootDir%build_release\
)

set CommonCompilerFlags=-nologo -GR- -Gm- -EHa- -Oi -WX -W4 -wd4100 -wd4201 -wd4189 -wd4701 -std:c++latest -utf-8
if "%1" == "debug" (
	set CommonCompilerFlags=%CommonCompilerFlags% -Od -Zi -D_DEBUG
) else (
	set CommonCompilerFlags=%CommonCompilerFlags% -O2
)

set CommonLinkerFlags=-opt:ref -incremental:no -subsystem:console -nodefaultlib kernel32.lib libucrt.lib libvcruntime.lib libcmt.lib

if exist %BuildDir% (
	rd /s/q %BuildDir% 2> NUL
	if %errorlevel% neq 0 (
		echo %ESC%[91mFailed to delete build directory!%ESC%[0m
		exit /b
	)
)

if not exist %BuildDir% (
	mkdir %BuildDir%
	pushd %BuildDir%

	cl %CommonCompilerFlags% %SrcDir%main.cpp -Fmcalx -Fecalx -link %CommonLinkerFlags%
	call :CheckCompile

	popd
)

exit /b

:CheckCompile
if %errorlevel% neq 0 (
	echo %ESC%[91mFailed to compile! 😢%ESC%[0m
) else (
	echo %ESC%[92mCompiled sucessfully%ESC%[0m
)
