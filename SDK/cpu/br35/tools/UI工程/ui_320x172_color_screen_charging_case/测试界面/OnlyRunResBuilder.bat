cd project
if not exist ResBuilder.xml copy ..\..\..\UITools\ResBuilder.xml
if exist ..\..\..\UITools\config\ini\option.ini copy ..\..\..\UITools\config\ini\option.ini config\ini\
call ..\..\..\UITools\ResBuilder.exe
call copy_file.bat