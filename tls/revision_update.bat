@ECHO OFF
SETLOCAL
SET SVN_REP=%~dp0\..
SET SVN_DIR=D:\Develop\TortoiseSVN\bin
SET SVN_VER=SubWCRev.exe

%SVN_DIR%\%SVN_VER% %SVN_REP% %~dp0\revision.svn %1\revision.h
ENDLOCAL