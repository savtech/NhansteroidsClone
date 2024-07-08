@echo off
cls

:: Edit this path to point to your vcvars script
set "__local_vcvars_path=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"

if not defined VSCMD_ARG_TGT_ARCH (
    if exist "%__local_vcvars_path%" (
        call "%__local_vcvars_path%"
    ) else (
        echo Unable to locate the specified vcvars script: %__local_vcvars_path%
        exit /b 1
    )
)

setlocal ENABLEDELAYEDEXPANSION

set "executable_name=a.exe"
set "compiler_options=/EHsc /nologo /W4 /external:W0 /MP /MTd /std:c17"
set "debug_options=/Od /Zi"
set "release_options=/O2"

:: You should change these paths to match your raylib directory
set "includes=/external:IC:/Dev/_libraries/raylib/include"
set "raylib_path=C:/Dev/_libraries/raylib/lib/raylib.lib

set build_result=0

echo Building !executable_name!...

set "build_options="
if "%~1"=="-r" (
    set "build_options=!release_options!"
) else (
    set "build_options=!debug_options!"
)


cl !compiler_options! !build_options! !includes! src\main.c /Fe!executable_name! /link /NODEFAULTLIB:LIBCMTD !raylib_path! winmm.lib gdi32.lib User32.lib Shell32.lib msvcrt.lib

if !ERRORLEVEL! neq 0 (
    set build_result=1
)

:end

if !build_result! equ 0 (
    echo Build successful.
) else (
    echo Build failed.
)

endlocal