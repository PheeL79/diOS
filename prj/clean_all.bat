@ECHO OFF
FOR /R %%G IN (clean.bat) DO @IF EXIST %%G CALL %%G
