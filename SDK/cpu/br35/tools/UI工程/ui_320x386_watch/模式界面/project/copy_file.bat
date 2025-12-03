@echo off

setlocal enabledelayedexpansion

..\..\..\UITools\style_table -stylefile project.bin -tablefile JL.tab -prj 0x0 -spec_page 58/60/61
copy .\project.bin ..\..\..\..\ui_resource\JL.sty
copy .\result.bin  ..\..\..\..\ui_resource\JL.res
copy .\result.str  ..\..\..\..\ui_resource\JL.str
copy .\JL.tab      ..\..\..\..\ui_resource\JL.tab
copy .\ename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\style_JL_new.h
copy .\result_pic_index.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\result_pic_index.h
copy .\result_str_index.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\result_str_index.h

copy .\ename.h     ..\..\..\..\ui_header_file\style_JL_new.h

exit
