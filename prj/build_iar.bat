@ECHO OFF
SETLOCAL
FOR /R %%G IN (%1) DO @IF EXIST %%G CALL %%G %IAR_BUILD_PATH%
ENDLOCAL