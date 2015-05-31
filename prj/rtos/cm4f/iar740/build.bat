@ECHO OFF
PUSHD
CD /D %~dp0
%1 rtos.ewp -build %2 -log warnings
POPD