@echo off

setlocal enabledelayedexpansion
..\..\..\UITools\style_table -stylefile project.bin -prj 0x1
copy .\project.bin ..\..\..\..\ui_resource\watch.sty
copy .\result.bin  ..\..\..\..\ui_resource\watch.res
copy .\result.str  ..\..\..\..\ui_resource\watch.str
copy .\version.txt  ..\..\..\..\ui_resource\watch.json
copy .\project.tab ..\..\..\..\ui_resource\watch.tab
..\..\..\UITools\redefined -infile ename.h -prefix dial -outfile rename.h


copy .\rename.h     ..\..\..\..\ui_header_file\style_watch.h


exit
