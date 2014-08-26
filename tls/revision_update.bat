@ECHO ON
@ECHO Revision update.
@ECHO OFF
SETLOCAL
PUSHD
CD /D %~dp0
CALL set_env.bat
POPD
CD /D %1
FOR /F %%R in ('git rev-list --count HEAD') DO SET REPO_BUILD=%%R
FOR /F %%R in ('git describe --always') DO SET REPO_REVISION=%%R
SET REVISION_H=%1\revision.h
DEL /Q /F %REVISION_H%
ECHO #ifndef _REVISION_H_ >> %REVISION_H%
ECHO #define _REVISION_H_ >> %REVISION_H%
ECHO. >> %REVISION_H%
ECHO #define BUILD %REPO_BUILD% >> %REVISION_H%
ECHO #define REVISION "%REPO_REVISION%" >> %REVISION_H%
ECHO. >> %REVISION_H%
ECHO #endif // _REVISION_H_ >> %REVISION_H%
ENDLOCAL