@ECHO OFF
PUSHD
CD /D %~dp0
%1 dios.ewp -build %2 -log warnings
POPD