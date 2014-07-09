@ECHO OFF
PUSHD %~dp0
CD /D %~dp0
CALL build_firmware_d.bat %1
CALL build_firmware_r.bat %1
POPD