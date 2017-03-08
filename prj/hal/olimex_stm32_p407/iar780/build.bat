@ECHO OFF
PUSHD
CD /D %~dp0
%1 hal.ewp -build %2 -log warnings
POPD