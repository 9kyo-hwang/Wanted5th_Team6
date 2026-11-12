
@echo off
setlocal EnableExtensions

REM ==========================================
REM WTG Team Auto Packaging - UE 5.8
REM ==========================================

REM [1] Project Path
set "PROJECT_DIR=%~dp0"
set "PROJECT_FILE=%PROJECT_DIR%GTW.uproject"

if not exist "%PROJECT_FILE%" (
    echo [ERROR] GTW.uproject not found.
    pause
    exit /b 1
)

REM [2] Engine Path
set "ENGINE_DIR="
set "CONFIG_FILE=%LOCALAPPDATA%\WTG_EnginePath.txt"

REM Load saved engine path
if exist "%CONFIG_FILE%" (
    set /p ENGINE_DIR=<"%CONFIG_FILE%"
)

REM Search Epic Games Launcher installation data
if not exist "%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat" (
    set "ENGINE_DIR="

    for /f "usebackq delims=" %%E in (`powershell -NoProfile -Command "$p=Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'; if(Test-Path $p){$j=Get-Content -Raw $p | ConvertFrom-Json; foreach($x in $j.InstallationList){if($x.AppName -eq 'UE_5.8' -and (Test-Path (Join-Path $x.InstallLocation 'Engine\Build\BatchFiles\RunUAT.bat'))){$x.InstallLocation;break}}}"`) do set "ENGINE_DIR=%%E"
)

REM Fallback: Windows Registry
if not exist "%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat" (
    set "ENGINE_DIR="

    for /f "tokens=2,*" %%A in ('
        reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul
    ') do (
        if /I "%%A"=="REG_SZ" set "ENGINE_DIR=%%B"
    )
)

REM Manual fallback
:CHECK_ENGINE
if not exist "%ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat" (
    echo.
    echo [INFO] UE 5.8 path not found automatically.
    echo Example: D:\UE_5.8
    set "ENGINE_DIR="
    set /p "ENGINE_DIR=Enter Engine Path: "
    set "ENGINE_DIR=%ENGINE_DIR:"=%"
    goto CHECK_ENGINE
)

REM Save engine path locally
>"%CONFIG_FILE%" echo(%ENGINE_DIR%

REM [3] Desktop Path
set "DESKTOP_DIR="

for /f "usebackq delims=" %%D in (`powershell -NoProfile -Command "[Environment]::GetFolderPath('Desktop')"`) do set "DESKTOP_DIR=%%D"

if not defined DESKTOP_DIR (
    echo [ERROR] Desktop not found.
    pause
    exit /b 1
)

REM [4] Output Path
set "PACKAGE_DIR=%DESKTOP_DIR%\WTG_Package"

if not exist "%PACKAGE_DIR%" mkdir "%PACKAGE_DIR%"

REM [5] Automation Tool
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

REM [6] Run Packaging
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

set "BUILD_RESULT=%ERRORLEVEL%"

REM [7] Result
if not "%BUILD_RESULT%"=="0" (
    echo.
    echo [FAILED] Packaging failed.
    pause
    exit /b %BUILD_RESULT%
)

echo.
echo [SUCCESS] WTG Packaging Completed
echo Output: %PACKAGE_DIR%

explorer "%PACKAGE_DIR%"
pause
endlocal
