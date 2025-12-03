@echo off

setlocal enabledelayedexpansion

..\..\..\UITools\style_table -stylefile project.bin -tablefile sidebar.tab -prj 0x2
copy .\project.bin	..\..\..\..\csc_ui_resource\sidebar.sty
copy .\result.bin  	..\..\..\..\csc_ui_resource\sidebar.res
copy .\result.str  	..\..\..\..\csc_ui_resource\sidebar.str
copy .\sidebar.tab      ..\..\..\..\csc_ui_resource\sidebar.tab

..\..\..\UITools\redefined -infile ename.h -prefix sidebar -outfile rename.h

copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\style_sidebar_csc.h

copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_sidebar_csc.h

exit
