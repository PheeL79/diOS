@ECHO OFF
PUSHD
CD /D %~dp0
%1 osal.ewp -build %2 -log warnings
POPD