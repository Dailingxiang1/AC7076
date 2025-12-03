@echo off
setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -prj 0x4
copy .\project.bin ..\..\..\..\csc_ui_upgrade\upgrade.sty
copy .\result.bin  ..\..\..\..\csc_ui_upgrade\upgrade.res
copy .\result.str  ..\..\..\..\csc_ui_upgrade\upgrade.str
copy .\version.txt  ..\..\..\..\csc_ui_upgrade\upgrade.json

..\..\..\UITools\redefined -infile ename.h -prefix UPGRADE -outfile rename.h

copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\style_upgrade_new_csc.h

copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_upgrade_new_csc.h
exit

