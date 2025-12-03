@echo off

setlocal enabledelayedexpansion

..\..\..\UITools\style_table -stylefile project.bin -tablefile JL.tab -prj 0x0
copy .\project.bin ..\..\..\..\csc_ui_resource\JL.sty
copy .\result.bin  ..\..\..\..\csc_ui_resource\JL.res
copy .\result.str  ..\..\..\..\csc_ui_resource\JL.str
copy .\JL.tab      ..\..\..\..\csc_ui_resource\JL.tab
copy .\ename.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\style_JL_new_csc.h
copy .\result_pic_index.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\result_pic_index_csc.h
copy .\result_str_index.h     ..\..\..\..\..\..\..\apps\watch\include\ui\jlui_app\result_str_index_csc.h

copy .\result_pic_index.h		..\..\..\..\csc_ui_header_file\result_pic_index_csc.h
copy .\result_str_index.h   	..\..\..\..\csc_ui_header_file\result_str_index_csc.h
copy .\ename.h     ..\..\..\..\csc_ui_header_file\style_JL_new_csc.h

exit
