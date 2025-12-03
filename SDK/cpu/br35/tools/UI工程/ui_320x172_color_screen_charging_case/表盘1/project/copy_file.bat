@echo off

setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -tablefile watch1.tab -prj 0x1
copy .\project.bin ..\..\..\..\csc_ui_resource\watch1.sty
copy .\result.bin  ..\..\..\..\csc_ui_resource\watch1.res
copy .\result.str  ..\..\..\..\csc_ui_resource\watch1.str
copy .\version.txt  ..\..\..\..\csc_ui_resource\watch1.json
copy .\watch1.tab ..\..\..\..\csc_ui_resource\watch1.tab
..\..\..\UITools\redefined -infile ename.h -prefix dial -outfile rename.h
copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\style_watch1_csc.h
copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_watch1_csc.h

exit
