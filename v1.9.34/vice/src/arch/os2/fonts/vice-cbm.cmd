@Echo Off

REM /*                        */
REM /* Set Variables          */
REM /*                        */

call e:\toolkit\tksetenv

REM /*                        */
REM /* Convert Unix Font File */
REM /*                        */
chargen2fnt ..\..\..\..\data\C64\chargen         "C64 Upper Case"        0
chargen2fnt ..\..\..\..\data\C64\chargen         "C64 Lower Case"        1
chargen2fnt ..\..\..\..\data\C128\chargen        "C128 Upper Case"       0
chargen2fnt ..\..\..\..\data\C128\chargen        "C128 Lower Case"       1
chargen2fnt ..\..\..\..\data\PET\chargen         "PET"                   0
chargen2fnt ..\..\..\..\data\PET\chargen.de      "PET German"            0
chargen2fnt ..\..\..\..\data\VIC20\chargen       "VIC20 Upper Case"      0
chargen2fnt ..\..\..\..\data\VIC20\chargen       "VIC20 Lower Case"      1
REM *** identical with C64 *** chargen2fnt ..\..\..\..\data\CBM-II\chargen.500  "CBM-II 500 Upper Case" 0
REM *** identical with C64 *** chargen2fnt ..\..\..\..\data\CBM-II\chargen.500  "CBM-II 500 Lower Case" 1
REM chargen2fnt ..\..\..\..\data\CBM-II\chargen.600  "CBM-II 600 Upper Case" 1
REM chargen2fnt ..\..\..\..\data\CBM-II\chargen.600  "CBM-II 600 Lower Case" 0
REM chargen2fnt ..\..\..\..\data\CBM-II\chargen.700  "CBM-II 700 Upper Case" 0
REM chargen2fnt ..\..\..\..\data\CBM-II\chargen.700  "CBM-II 700 Lower Case" 1

REM /*                        */
REM /* Create font-dll        */
REM /*                        */
alp -Mb vice2.asm
link386 vice2,,,,vice2.def /NOLOGO
rc -n vice2.rc vice2.dll

REM /*                        */
REM /* 'Rename' font-dll      */
REM /*                        */
move vice2.dll vice2.fon > NUL

del chargen*.fnt *.res *.obj *.map > NUL
