@ECHO OFF
SETLOCAL
SET BUILD_DIR_ARR=(hal, rtos, osal, firmware)
SET BUILD_CFG_ARR=("Debug FLASH", "Debug SRAM", "Debug")
FOR %%D IN %BUILD_DIR_ARR% DO (
    FOR /D /R %%P IN (%%D) DO @IF EXIST %%P (
        PUSHD
        CD /D %%P
        FOR /R %%B IN (build.bat) DO @IF EXIST %%B (
            FOR %%G IN %BUILD_CFG_ARR% DO (
                CALL %%B %1 %%G
            )
        )
        POPD
    )
)
ENDLOCAL