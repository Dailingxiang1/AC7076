@echo off

setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -tablefile watch.tab -prj 0x1
copy .\project.bin ..\..\..\..\csc_ui_resource\watch.sty
copy .\result.bin  ..\..\..\..\csc_ui_resource\watch.res
copy .\result.str  ..\..\..\..\csc_ui_resource\watch.str
copy .\version.txt  ..\..\..\..\csc_ui_resource\watch.json
copy .\watch.tab ..\..\..\..\csc_ui_resource\watch.tab
..\..\..\UITools\redefined -infile ename.h -prefix dial -outfile rename.h

copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\style_watch_csc.h
copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_watch_csc.h


exit
