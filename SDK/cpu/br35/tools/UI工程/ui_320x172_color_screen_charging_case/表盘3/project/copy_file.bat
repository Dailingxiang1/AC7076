@echo off

setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -tablefile watch3.tab -prj 0x1
copy .\project.bin ..\..\..\..\csc_ui_resource\watch3.sty
copy .\result.bin  ..\..\..\..\csc_ui_resource\watch3.res
copy .\result.str  ..\..\..\..\csc_ui_resource\watch3.str
copy .\version.txt  ..\..\..\..\csc_ui_resource\watch3.json
copy .\watch3.tab ..\..\..\..\csc_ui_resource\watch3.tab
..\..\..\UITools\redefined -infile ename.h -prefix dial -outfile rename.h
copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\style_watch3_csc.h
copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_watch3_csc.h

exit
