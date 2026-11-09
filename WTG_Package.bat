
@echo off
setlocal EnableExtensions

REM ==========================================
REM WTG Team Packaging - Unreal Engine 5.8
REM ==========================================

REM [1] Find project path automatically
set "PROJECT_DIR=%~dp0"
set "PROJECT_FILE=%PROJECT_DIR%GTW.uproject"

if not exist "%PROJECT_FILE%" (
    echo [ERROR] GTW.uproject not found.
    echo Expected: %PROJECT_FILE%
    pause
    exit /b 1
)

REM [2] Find Unreal Engine installation
set "ENGINE_DIR="
set "CONFIG_FILE=%LOCALAPPDATA%\WTG_EnginePath.txt"

REM Load previously saved path
if exist "%CONFIG_FILE%" (
    set /p ENGINE_DIR=<"%CONFIG_FILE%"
)

REM Search Windows Registry for UE 5.8
if not exist "%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat" (
    set "ENGINE_DIR="

    for /f "tokens=2,*" %%A in ('
        reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul
    ') do (
        if /I "%%A"=="REG_SZ" set "ENGINE_DIR=%%B"
    )
)

REM Manual path fallback
:CHECK_ENGINE
if not exist "%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat" (
    echo.
    echo [INFO] Unreal Engine 5.8 not found.
    echo Example: D:\UE_5.8
    echo.
    set "ENGINE_DIR="
    set /p "ENGINE_DIR=Enter Engine Path: "

    if not defined ENGINE_DIR goto CHECK_ENGINE

    REM Remove quotes if entered
    set "ENGINE_DIR=%ENGINE_DIR:"=%"
    goto CHECK_ENGINE
)

REM Save engine path for next execution
>"%CONFIG_FILE%" echo(%ENGINE_DIR%

REM [3] Find Windows Desktop path
set "DESKTOP_DIR="

for /f "usebackq delims=" %%D in (`powershell -NoProfile -Command "[Environment]::GetFolderPath('Desktop')"`) do set "DESKTOP_DIR=%%D"

if not defined DESKTOP_DIR (
    echo [ERROR] Desktop folder not found.
    pause
    exit /b 1
)

REM [4] Create package output folder
set "PACKAGE_DIR=%DESKTOP_DIR%\WTG_Package"

if not exist "%PACKAGE_DIR%" mkdir "%PACKAGE_DIR%"

REM [5] Unreal Automation Tool
set "UAT=%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat"

echo.
echo ==========================================
echo WTG Packaging Started
echo ==========================================
echo Engine  : %ENGINE_DIR%
echo Project : %PROJECT_FILE%
echo Output  : %PACKAGE_DIR%
echo ==========================================
echo.

REM [6] BuildCookRun
call "%UAT%" ^
 -ScriptsForProject="%PROJECT_FILE%" ^
 -noP4 ^
 -nocompile ^
 -utf8output ^
 BuildCookRun ^
 -project="%PROJECT_FILE%" ^
 -clientconfig=Development ^
 -serverconfig=Development ^
 -nocompileeditor ^
 -installed ^
 -platform=Win64 ^
 -build ^
 -cook ^
 -CookCultures=en ^
 -unversionedcookedcontent ^
 -zenstore ^
 -pak ^
 -stage ^
 -compressed ^
 -archive ^
 -archivedirectory="%PACKAGE_DIR%"

REM [7] Check packaging result
set "BUILD_RESULT=%ERRORLEVEL%"

echo.

if not "%BUILD_RESULT%"=="0" (
    echo ==========================================
    echo [FAILED] WTG Packaging Failed
    echo ==========================================
    echo Check the log above.
    pause
    exit /b %BUILD_RESULT%
)

echo ==========================================
echo [SUCCESS] WTG Packaging Completed
echo ==========================================
echo Output: %PACKAGE_DIR%

REM [8] Open output folder
explorer "%PACKAGE_DIR%"

pause
endlocal
