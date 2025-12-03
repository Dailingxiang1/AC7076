@echo off

setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -tablefile watch4.tab -prj 0x1
copy .\project.bin ..\..\..\..\csc_ui_resource\watch4.sty
copy .\result.bin  ..\..\..\..\csc_ui_resource\watch4.res
copy .\result.str  ..\..\..\..\csc_ui_resource\watch4.str
copy .\version.txt  ..\..\..\..\csc_ui_resource\watch4.json
copy .\watch4.tab ..\..\..\..\csc_ui_resource\watch4.tab
..\..\..\UITools\redefined -infile ename.h -prefix dial -outfile rename.h
copy .\rename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\style_watch4_csc.h
copy .\rename.h     ..\..\..\..\csc_ui_header_file\style_watch4_csc.h

exit
