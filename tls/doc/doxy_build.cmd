@echo off
rem ****************************************************
rem * ��������� ��������� doxygen.
rem * ������� ���������:
rem * ���� ������������ doxygen (���������� .doxy)
rem * ��������! ���������� ��������� �����
rem ****************************************************

echo. 
echo [%1] processing start at %time%

rem ----- set parameters -------

setlocal

set config=%1
set name=%~n1
set ext=%~x1

set log_dir=.\log
set out_dir=.\out\%name%

rem ----- check input -------

rem ��� ��������� - ���������� ���������
rem ���� /i - � ����� �������� (� == �)
if /i [%config%] == [] goto usage
if /i [%name%] == [] goto usage
if /i not [%ext%] == [.doxy] goto usage
if /i not exist %config% goto err1

rem ��������� �������. �������� ��������������� ������,
rem ����� ��������� ������� ��� ���������� NUL 
if /i exist %out_dir%\nul (
  echo Remove previous version [%name%]...
  del /s /q %out_dir% >nul
) else mkdir %out_dir%

rem ----- create log file -------

if /i not exist %log_dir%\nul mkdir %log_dir%

rem ���� yyyy_mm_dd
set date_now=%date:~6,4%%date:~3,2%%date:~0,2%

rem ����� hhmmss
set time_now=%time:~0,2%%time:~3,2%%time:~6,2%
rem ��� ������ <10 ����� - ������ ������� �� 0
if "%time_now:~0,1%" == " " set time_now=0%time_now:~1,5%

rem ���� log_yyyymmdd_hhmmss.txt
set log_file=%log_dir%\%name%_%date_now%_%time_now%.txt
echo Create log file [%log_file%]...

rem ----- generation  -----

echo [%name%] Doxygen processing...

rem ����� STDERR(2) ���� ���������������� � log
doxygen.exe %config% >>%log_file% 2>&1
if errorlevel 1 goto err2

copy /y doxygen.sty %out_dir%\latex

echo [%name%] Make processing...
make -C %out_dir%\latex >> %log_file%
rem 2>&1 

rem if errorlevel 1 goto err3  ??? �������� ������

if /i exist %out_dir%\latex\refman.pdf (
  if /i not exist .\..\..\doc\nul mkdir .\..\..\doc
  copy /y %out_dir%\latex\refman.pdf .\..\..\doc\%name%.pdf >nul
)else goto err3

:exit_ok
rem call clean.bat
echo [%name%] generated successfully.
goto end

rem ----- errors ----- 

:usage
echo Usage: doxy_build config_file.doxy
goto exit_err

:err1
echo File [%config%] not found
goto exit_err

:err2
echo Doxygen error
goto exit_err

:err3
echo Make error
goto exit_err

:exit_err
echo [%name%] creation error

:end
echo [%1] processing stop at %time%
exit /b
