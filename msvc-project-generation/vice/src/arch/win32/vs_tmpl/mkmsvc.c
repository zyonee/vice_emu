#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define CP_NAME_STRING                "PROJECTNAME = "
#define CP_NAME_SIZE                  sizeof(CP_NAME_STRING) - 1

#define CP_TYPE_STRING                "PROJECTTYPE = "
#define CP_TYPE_SIZE                  sizeof(CP_TYPE_STRING) - 1

#define CP_DEPS_STRING                "PROJECTDEPS = "
#define CP_DEPS_SIZE                  sizeof(CP_DEPS_STRING) - 1

#define CP_LIBS_STRING                "PROJECTLIBS = "
#define CP_LIBS_SIZE                  sizeof(CP_LIBS_STRING) - 1

#define CP_INCLUDES_STRING            "INCLUDEDIRS ="
#define CP_INCLUDES_SIZE              sizeof(CP_INCLUDES_STRING) - 1

#define CP_SOURCES_STRING             "SOURCES ="
#define CP_SOURCES_SIZE               sizeof(CP_SOURCES_STRING) - 1

#define CP_CPUSOURCES_STRING          "CPUSOURCES ="
#define CP_CPUSOURCES_SIZE            sizeof(CP_CPUSOURCES_STRING) - 1

#define CP_CUSTOM_MSG_STRING          "CUSTOMMSG = "
#define CP_CUSTOM_MSG_SIZE            sizeof(CP_CUSTOM_MSG_STRING) - 1

#define CP_CUSTOM_SRC_STRING          "CUSTOMSOURCE = "
#define CP_CUSTOM_SRC_SIZE            sizeof(CP_CUSTOM_SRC_STRING) - 1

#define CP_CUSTOM_DEPS_STRING         "CUSTOMDEPS = "
#define CP_CUSTOM_DEPS_SIZE           sizeof(CP_CUSTOM_DEPS_STRING) - 1

#define CP_CUSTOM_OUTPUT_STRING       "CUSTOMOUTPUT = "
#define CP_CUSTOM_OUTPUT_SIZE         sizeof(CP_CUSTOM_OUTPUT_STRING) - 1

#define CP_CUSTOM_COMMAND_STRING      "CUSTOMCOMMAND = "
#define CP_CUSTOM_COMMAND_SIZE        sizeof(CP_CUSTOM_COMMAND_STRING) - 1

#define CP_POST_CUSTOM_MSG_STRING     "POST_CUSTOMMSG = "
#define CP_POST_CUSTOM_MSG_SIZE       sizeof(CP_POST_CUSTOM_MSG_STRING) - 1

#define CP_POST_CUSTOM_SRC_STRING     "POST_CUSTOMSOURCE = "
#define CP_POST_CUSTOM_SRC_SIZE       sizeof(CP_POST_CUSTOM_SRC_STRING) - 1

#define CP_POST_CUSTOM_OUTPUT_STRING  "POST_CUSTOMOUTPUT = "
#define CP_POST_CUSTOM_OUTPUT_SIZE    sizeof(CP_POST_CUSTOM_OUTPUT_STRING) - 1

#define CP_POST_CUSTOM_COMMAND_STRING "POST_CUSTOMCOMMAND = "
#define CP_POST_CUSTOM_COMMAND_SIZE   sizeof(CP_POST_CUSTOM_COMMAND_STRING) - 1

#define CP_CC_SOURCE_PATH_STRING      "CCSOURCEPATH = "
#define CP_CC_SOURCE_PATH_SIZE        sizeof(CP_CC_SOURCE_PATH_STRING) - 1

#define CP_CC_SOURCES_STRING          "CCSOURCES ="
#define CP_CC_SOURCES_SIZE            sizeof(CP_CC_SOURCES_STRING) - 1

#define CP_TYPE_LIBRARY   1
#define CP_TYPE_CONSOLE   2
#define CP_TYPE_GUI       3

#define MAX_DIRS        100
#define MAX_DEP_NAMES   100
#define MAX_NAMES       200
#define MAX_CPU_NAMES   10

/* current project file parameters */
static char *cp_name = NULL;
static int cp_type = -1;
static char *cp_dep_names[MAX_DEP_NAMES];
static char *cp_include_dirs[MAX_DIRS];
static char *cp_source_names[MAX_NAMES];
static char *cp_cpusource_names[MAX_CPU_NAMES];
static char *cp_libs = NULL;
static char *cp_custom_message = NULL;
static char *cp_custom_source = NULL;
static char *cp_custom_deps[MAX_DEP_NAMES];
static char *cp_custom_output = NULL;
static char *cp_custom_command = NULL;
static char *cp_post_custom_message = NULL;
static char *cp_post_custom_source = NULL;
static char *cp_post_custom_output = NULL;
static char *cp_post_custom_command = NULL;
static char *cp_cc_source_path = NULL;
static char *cp_cc_source_names[MAX_NAMES];

/* read buffer related */
static char *read_buffer = NULL;
static int read_buffer_line = 0;
static int read_buffer_pos = 0;
static int read_buffer_len = 0;

/* MSVC 6 types */
static char *msvc6_console_type = "Console Application";
static char *msvc6_library_type = "Static Library";
static char *msvc6_gui_type =     "Application";

/* MSVC 6 ids */
static char *msvc6_console_id = "0x0103";
static char *msvc6_library_id = "0x0104";
static char *msvc6_gui_id =     "0x0101";

/* MSVC6 header */
static char *msvc6_header = "# Microsoft Developer Studio Project File - Name=\"%s\" - Package Owner=<4>\r\n"
                           "# Microsoft Developer Studio Generated Build File, Format Version 6.00\r\n"
                           "# ** DO NOT EDIT **\r\n"
                           "\r\n"
                           "# TARGTYPE \"Win32 (x86) %s\" %s\r\n\r\n"
                           "CFG=%s - Win32 Debug\r\n"
                           "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\r\n"
                           "!MESSAGE use the Export Makefile command and run\r\n"
                           "!MESSAGE \r\n"
                           "!MESSAGE NMAKE /f \"%s.mak\".\r\n"
                           "!MESSAGE \r\n"
                           "!MESSAGE You can specify a configuration when running NMAKE\r\n"
                           "!MESSAGE by defining the macro CFG on the command line. For example:\r\n"
                           "!MESSAGE \r\n"
                           "!MESSAGE NMAKE /f \"%s.mak\" CFG=\"%s - Win32 Debug\"\r\n"
                           "!MESSAGE \r\n"
                           "!MESSAGE Possible choices for configuration are:\r\n"
                           "!MESSAGE \r\n"
                           "!MESSAGE \"%s - Win32 Release\" (based on \"Win32 (x86) %s\")\r\n"
                           "!MESSAGE \"%s - Win32 Debug\" (based on \"Win32 (x86) %s\")\r\n"
                           "!MESSAGE \"%s - Win32 DX Release\" (based on \"Win32 (x86) %s\")\r\n"
                           "!MESSAGE \"%s - Win32 DX Debug\" (based on \"Win32 (x86) %s\")\r\n"
                           "!MESSAGE \r\n"
                           "\r\n";

/* MSVC6 begin project section */
static char *msvc6_section1 = "# Begin Project\r\n"
                              "# PROP AllowPerConfigDependencies 0\r\n"
                              "# PROP Scc_ProjName \"\"\r\n"
                              "# PROP Scc_LocalPath \"\"\r\n"
                              "CPP=cl.exe\r\n"
                              "%s"
                              "RSC=rc.exe\r\n"
                              "\r\n";

static char *msvc6_begin_ifs[4] = {
    "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n\r\n",
    "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n\r\n",
    "!ELSEIF  \"$(CFG)\" == \"%s - Win32 DX Release\"\r\n\r\n",
    "!ELSEIF  \"$(CFG)\" == \"%s - Win32 DX Debug\"\r\n\r\n"
};
    
static char *msvc6_releases[4] = {
    "Release",
    "Debug",
    "DXRelease",
    "DXDebug"
};

static int msvc6_enable_debug[4] = { 0, 1, 0, 1 };

/* begin sections */
static char *msvc6_begin_section = "# PROP BASE Use_MFC 0\r\n"
                                   "# PROP BASE Use_Debug_Libraries %d\r\n"
                                   "# PROP BASE Output_Dir \"%s\"\r\n"
                                   "# PROP BASE Intermediate_Dir \"%s\"\r\n"
                                   "# PROP BASE Target_Dir \"\"\r\n"
                                   "# PROP Use_MFC 0\r\n"
                                   "# PROP Use_Debug_Libraries %d\r\n";

static char *msvc6_middle_section_lib = "# PROP Output_Dir \"libs\\%s\\%s\"\r\n";
static char *msvc6_middle_section_app = "# PROP Output_Dir \"..\\..\\..\\..\\data\"\r\n";

static char *msvc6_section2 = "# PROP Intermediate_Dir \"libs\\%s\\%s\"\r\n";

static char *msvc6_app_section1 = "# PROP Ignore_Export_Lib 0\r\n";

static char *msvc6_section3 = "# PROP Target_Dir \"\"\r\n";

static char *msvc6_base_cpp_lib_gui_part1[4] = {
    "/MT /W3 /GX /O2",
    "/MTd /W3 /GX /Z7 /Od",
    "/MT /W3 /GX /O2",
    "/MTd /W3 /GX /Z7 /Od"
};

static char *msvc6_base_cpp_lib_gui_part2[4] = {
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"NODIRECTX\" /D \"NDEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"NODIRECTX\" /D \"_DEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\"  /D \"NDEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"_DEBUG\" /YX /FD /c"
};

static char *msvc6_base_cpp_cc[4] = {
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"NODIRECTX\" /D \"NDEBUG\" /D PACKAGE=\\\"%s\\\" /D VERSION=\\\"0.7\\\" /D SIZEOF_INT=4",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"NODIRECTX\" /D \"_DEBUG\" /D PACKAGE=\\\"%s\\\" /D VERSION=\\\"0.7\\\" /D SIZEOF_INT=4",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\"  /D \"NDEBUG\" /D PACKAGE=\\\"%s\\\" /D VERSION=\\\"0.7\\\" /D SIZEOF_INT=4",
    " /D \"WIN32\" /D \"_WINDOWS\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"_DEBUG\" /D PACKAGE=\\\"%s\\\" /D VERSION=\\\"0.7\\\" /D SIZEOF_INT=4"
};

static char *msvc6_base_cpp_cc_end = " /YX /FD /c\r\n";

static char *msvc6_base_cpp_console_part1[4] = {
    "/MT /W3 /GX /O2",
    "/MTd /W3 /Gm /GX /Zi /Od",
    "/MT /W3 /GX /O2",
    "/MTd /W3 /GX /Z7 /Od"
};

static char *msvc6_base_cpp_console_part2[4] = {
    " /D \"WIN32\" /D \"_CONSOLE\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"_MBCS\" /D \"NDEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_CONSOLE\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H \"/D \"_MBCS\" /D \"_DEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_CONSOLE\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"_MBCS\" /D \"NDEBUG\" /YX /FD /c",
    " /D \"WIN32\" /D \"_CONSOLE\" /D \"IDE_COMPILE\" /D \"DONT_USE_UNISTD_H\" /D \"_MBCS\" /D \"_DEBUG\" /YX /FD /c"
};

static char *msvc6_base_mtl[2] = {
    "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n"
    "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n",
    "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n"
    "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32\r\n"
};

static char *msvc6_base_rsc[4] = {
    "# ADD BASE RSC /l 0x409 /d \"NDEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\" /d \"NODIRECTX\"\r\n"
    "# ADD RSC /l 0x409 /i \"..\\msvc\" /i \"..\\\\\" /i \"..\\..\\..\\\\\" /d \"NDEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\" /d \"NODIRECTX\"\r\n",
    "# ADD BASE RSC /l 0x409 /d \"_DEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\" /d \"NODIRECTX\"\r\n"
    "# ADD RSC /l 0x409 /i \"..\\msvc\" /i \"..\\\\\" /i \"..\\..\\..\\\\\" /d \"_DEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\" /d \"NODIRECTX\"\r\n",
    "# ADD BASE RSC /l 0x409 /d \"NDEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\"\r\n"
    "# ADD RSC /l 0x409 /i \"..\\msvc\" /i \"..\\\\\" /i \"..\\..\\..\\\\\" /d \"NDEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\"\r\n",
    "# ADD BASE RSC /l 0x409 /d \"_DEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\"\r\n"
    "# ADD RSC /l 0x409 /i \"..\\msvc\" /i \"..\\\\\" /i \"..\\..\\..\\\\\" /d \"_DEBUG\" /d \"WIN32\" /d \"IDE_COMPILE\"\r\n"
};

static char *msvc6_bsc32 = "BSC32=bscmake.exe\r\n"
                           "# ADD BASE BSC32 /nologo\r\n"
                           "# ADD BSC32 /nologo\r\n";

static char *msvc6_lib32 = "LIB32=link.exe -lib\r\n"
                           "# ADD BASE LIB32 /nologo\r\n"
                           "# ADD LIB32 /nologo\r\n\r\n";

static char *msvc6_link32_console = "LINK32=link.exe\r\n"
                                    "# ADD BASE LINK32 %s /nologo /subsystem:console /machine:I386\r\n"
                                    "# ADD LINK32 %s /nologo /subsystem:console /machine:I386\r\n\r\n";

static char *msvc6_link32_gui = "LINK32=link.exe\r\n"
                                "# ADD BASE LINK32 %s%s /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept\r\n"
                                "# ADD LINK32 %s%s /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept\r\n\r\n";

static char *msvc6_dx_libs[2] = {
    "",
    " dsound.lib dxguid.lib"
};

static char *msvc6_endif = "!ENDIF\r\n"
                           "\r\n";

static char *msvc6_begin_target = "# Begin Target\r\n"
                                  "\r\n"
                                  "# Name \"%s - Win32 Release\"\r\n"
                                  "# Name \"%s - Win32 Debug\"\r\n"
                                  "# Name \"%s - Win32 DX Release\"\r\n"
                                  "# Name \"%s - Win32 DX Debug\"\r\n";

static char *msvc6_source = "# Begin Source File\r\n"
                            "\r\n"
                            "SOURCE=\"..\\..\\..\\%s\"\r\n"
                            "# End Source File\r\n";

static char *msvc6_end_target = "# End Target\r\n"
                                "# End Project\r\n";

static char *msvc6_custom_source = "# Begin Source File\r\n"
                                   "\r\n"
                                   "SOURCE=\"..\\..\\..\\%s\"\r\n"
                                   "\r\n";

static char *msvc6_custom_section_part1 = "# PROP Ignore_Default_Tool 1\r\n"
                                          "USERDEP__CUSTOM=";

static char *msvc6_custom_section_part2 = "\r\n"
                                    "# Begin Custom Build - %s\r\n"
                                    "InputDir=.\r\n"
                                    "InputPath=\"%s\"\r\n"
                                    "\r\n"
                                    "\"$(InputDir)\\%s\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n"
                                    "\t%s\r\n"
                                    "\r\n"
                                    "# End Custom Build\r\n"
                                    "\r\n";

static char *msvc6_post_custom_section = "# Begin Custom Build - %s\r\n"
                                         "InputPath=\"%s\"\r\n"
                                         "SOURCE=\"$(InputPath)\"\r\n"
                                         "\r\n"
                                         "\"%s\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n"
                                         "\t%s\r\n"
                                         "\r\n"
                                         "# End Custom Build\r\n\r\n";

static char *msvc6_end_custom_source = "# End Source File\r\n";

static char *msvc6_custom_cpu_source = "# Begin Source File\r\n"
                                       "\r\n"
                                       "SOURCE=\"..\\..\\..\\%s\"\r\n"
                                       "\r\n"
                                       "!IF  \"$(CFG)\" == \"%s - Win32 Release\"\r\n"
                                       "\r\n"
                                       "# ADD CPP /Ot /Oa /Ow /Oi /Op /Oy\r\n"
                                       "# SUBTRACT CPP /Os\r\n"
                                       "\r\n"
                                       "!ELSEIF  \"$(CFG)\" == \"%s - Win32 Debug\"\r\n"
                                       "\r\n"
                                       "!ELSEIF  \"$(CFG)\" == \"%s - Win32 DX Release\"\r\n"
                                       "\r\n"
                                       "# ADD BASE CPP /Ot /Oa /Ow /Oi /Op /Oy\r\n"
                                       "# SUBTRACT BASE CPP /Os\r\n"
                                       "# ADD CPP /Ot /Oa /Ow /Oi /Op /Oy\r\n"
                                       "# SUBTRACT CPP /Os\r\n"
                                       "\r\n"
                                       "!ELSEIF  \"$(CFG)\" == \"%s - Win32 DX Debug\"\r\n"
                                       "\r\n"
                                       "!ENDIF\r\n"
                                       "\r\n"
                                       "# End Source File\r\n";

static char *msvc6_begin_cc_source = "# Begin Source File\r\n"
                                     "\r\n"
                                     "SOURCE=\"..\\..\\..\\%s\\%s.cc\"\r\n"
                                     "\r\n";

static char *msvc6_cc_custom_build_part1 = "# Begin Custom Build\r\n"
                                           "InputPath=\"..\\..\\..\\%s\\%s.cc\"\r\n"
                                           "InputName=%s\r\n"
                                           "\r\n"
                                           "\"libs\\%s\\%s\\$(InputName).obj\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n";

static char *msvc6_cc_custom_build_part2a = "\tcl /nologo %s /EHsc /I \"..\\msvc\" ";

static char *msvc6_cc_custom_build_part2b = "\tcl /nologo %s /EHsc /I \"..\\msvc\" /I \"..\\\\\" /I \"..\\..\\..\\\\\" ";

static char *msvc6_cc_custom_build_part3 = "%s /Fp\"libs\\%s\\%s\\%s.pch\" /Fo\"libs\\%s\\%s\\\\\" /Fd\"libs\\%s\\%s\\\\\" /FD /TP /c \"$(InputPath)\"\r\n"
                                           "\r\n"
                                           "# End Custom Build\r\n"
                                           "\r\n";

static int output_msvc6_file(char *fname)
{
    char *filename = malloc(strlen(fname) + sizeof(".txt"));
    int retval = 0;
    FILE *outfile = NULL;
    char *real_type = NULL;
    char *real_id = NULL;
    char *mtl = (cp_type == CP_TYPE_GUI) ? "MTL=midl.exe\r\n" : "";
    int i, j;

    sprintf(filename, "%s.txt", fname);

    outfile = fopen(filename, "wb");
    if (!outfile) {
        printf("Cannot open %s for output\n", filename);
        retval = 1;
    }

    if (!retval) {
        switch(cp_type) {
            default:
            case CP_TYPE_LIBRARY:
                real_type = msvc6_library_type;
                real_id = msvc6_library_id;
                break;
            case CP_TYPE_CONSOLE:
                real_type = msvc6_console_type;
                real_id = msvc6_console_id;
                break;
            case CP_TYPE_GUI:
                real_type = msvc6_gui_type;
                real_id = msvc6_gui_id;
                break;
        }
        fprintf(outfile, msvc6_header, cp_name, real_type, real_id, cp_name, cp_name, cp_name, cp_name, cp_name, real_type, cp_name, real_type, cp_name, real_type, cp_name, real_type);
        fprintf(outfile, msvc6_section1, mtl, cp_name);
        for (i = 0; i < 4; i++) {
            fprintf(outfile, msvc6_begin_ifs[i], cp_name);
            fprintf(outfile, msvc6_begin_section, msvc6_enable_debug[i], msvc6_releases[i], msvc6_releases[i], msvc6_enable_debug[i]);
            if (cp_type != CP_TYPE_LIBRARY) {
                fprintf(outfile, msvc6_middle_section_app);
            } else {
                fprintf(outfile, msvc6_middle_section_lib, cp_name, msvc6_releases[i]);
            }
            fprintf(outfile, msvc6_section2, cp_name, msvc6_releases[i]);
            if (cp_type != CP_TYPE_LIBRARY) {
                fprintf(outfile, msvc6_app_section1);
            }
            fprintf(outfile, msvc6_section3);
            if (cp_type != CP_TYPE_CONSOLE) {
                if (!cp_source_names[0] && cp_cc_source_names[0]) {
                    fprintf(outfile, "# ADD BASE CPP /nologo %s ", msvc6_base_cpp_lib_gui_part1[i]);
                    fprintf(outfile, msvc6_base_cpp_cc[i], cp_name);
                    fprintf(outfile, msvc6_base_cpp_cc_end);
                } else {
                    fprintf(outfile, "# ADD BASE CPP /nologo %s %s\r\n", msvc6_base_cpp_lib_gui_part1[i], msvc6_base_cpp_lib_gui_part2[i]);
                }
                fprintf(outfile, "# ADD CPP /nologo %s", msvc6_base_cpp_lib_gui_part1[i]);
            } else {
                fprintf(outfile, "# ADD BASE CPP /nologo %s %s\r\n", msvc6_base_cpp_console_part1[i], msvc6_base_cpp_console_part2[i]);
                fprintf(outfile, "# ADD CPP /nologo %s", msvc6_base_cpp_console_part1[i]);
            }
            if (!cp_source_names[0] && cp_cc_source_names[0]) {
                fprintf(outfile, " /I \"..\\msvc\"");
            } else {
                fprintf(outfile, " /I \"..\\msvc\" /I \"..\\\\\" /I \"..\\..\\..\\\\\"");
                if (cp_include_dirs[0]) {
                    for (j = 0; cp_include_dirs[j]; j++) {
                        fprintf(outfile, " /I \"..\\..\\..\\%s\"", cp_include_dirs[j]);
                    }
                }
            }
            if (cp_type != CP_TYPE_CONSOLE) {
                if (!cp_source_names[0] && cp_cc_source_names[0]) {
                    fprintf(outfile, msvc6_base_cpp_cc[i], cp_name);
                    fprintf(outfile, msvc6_base_cpp_cc_end);
                } else {
                    fprintf(outfile, " %s\r\n", msvc6_base_cpp_lib_gui_part2[i]);
                }
            } else {
                fprintf(outfile, " %s\r\n", msvc6_base_cpp_console_part2[i]);
            }
            if (cp_type == CP_TYPE_GUI) {
                fprintf(outfile, msvc6_base_mtl[i & 1]);
            }
            fprintf(outfile, msvc6_base_rsc[i]);
            fprintf(outfile, msvc6_bsc32);
            switch (cp_type) {
                default:
                case CP_TYPE_LIBRARY:
                    fprintf(outfile, msvc6_lib32);
                    break;
                case CP_TYPE_CONSOLE:
                    fprintf(outfile, msvc6_link32_console, cp_libs, cp_libs);
                    break;
                case CP_TYPE_GUI:
                    fprintf(outfile, msvc6_link32_gui, cp_libs, msvc6_dx_libs[i >> 1], cp_libs, msvc6_dx_libs[i >> 1]);
                    break;
            }
            if (cp_post_custom_message) {
                fprintf(outfile, msvc6_post_custom_section, cp_post_custom_message, cp_post_custom_source, cp_post_custom_output, cp_post_custom_command);
            }
        }
        fprintf(outfile, msvc6_endif);
        fprintf(outfile, msvc6_begin_target, cp_name, cp_name, cp_name, cp_name);
        if (cp_cc_source_names[0]) {
            for (j = 0; cp_cc_source_names[j]; j++) {
                fprintf(outfile, msvc6_begin_cc_source, cp_cc_source_path, cp_cc_source_names[j]);
                for (i = 0; i < 4; i++) {
                    fprintf(outfile, msvc6_begin_ifs[i], cp_name);
                    fprintf(outfile, msvc6_cc_custom_build_part1, cp_cc_source_path, cp_cc_source_names[j], cp_cc_source_names[j], cp_name, msvc6_releases[i]);
                    if (cp_source_names[0]) {
                        fprintf(outfile, msvc6_cc_custom_build_part2b, msvc6_base_cpp_lib_gui_part1[i]);
                    } else {
                        fprintf(outfile, msvc6_cc_custom_build_part2a, msvc6_base_cpp_lib_gui_part1[i]);
                    }
                    fprintf(outfile, msvc6_cc_custom_build_part3, msvc6_base_cpp_cc[i], cp_name, msvc6_releases[i], cp_name, cp_name, msvc6_releases[i], cp_name, msvc6_releases[i]);
                }
                fprintf(outfile, "%s# End Source File\r\n", msvc6_endif);
            }
        }
        if (cp_source_names[0]) {
            for (j = 0; cp_source_names[j]; j++) {
                fprintf(outfile, msvc6_source, cp_source_names[j]);
            }
        }
        if (cp_cpusource_names[0]) {
            for (j = 0; cp_cpusource_names[j]; j++) {
                fprintf(outfile, msvc6_custom_cpu_source, cp_cpusource_names[j], cp_name, cp_name, cp_name, cp_name);
            }
        }
        if (cp_custom_message) {
            fprintf(outfile, msvc6_custom_source, cp_custom_source);
            for (i = 0; i < 4; i++) {
                fprintf(outfile, msvc6_begin_ifs[i], cp_name);
                fprintf(outfile, msvc6_custom_section_part1);
                for (j = 0; cp_custom_deps[j]; j++) {
                    fprintf(outfile, "\"%s\"\t", cp_custom_deps[j]);
                }
                fprintf(outfile, msvc6_custom_section_part2, cp_custom_message, cp_custom_source, cp_custom_output, cp_custom_command);
            }
            fprintf(outfile, msvc6_endif);
            fprintf(outfile, msvc6_end_custom_source);
        }
        fprintf(outfile, msvc6_end_target);
    }

    if (outfile) {
        fclose(outfile);
    }

    if (filename) {
        free(filename);
    }
    return retval;
}

static int split_line_names(char *line, char **names, int max_names)
{
    int count = 0;
    int i = 0;

    names[count++] = line;
    while (line[i]) {
        while (!isspace(line[i]) && line[i]) {
            i++;
        }
        if (line[i]) {
            line[i++] = 0;
            names[count++] = line + i;
            if (count > max_names) {
                printf("name list overflow\n");
                return 1;
            }
        }
    }
    names[count] = NULL;

#if 1
    for (i = 0; names[i]; i++) {
        printf("NAME %d: %s\n", i, names[i]);
    }
#endif

    return 0;
}

static void strip_trailing_spaces(char *line)
{
    int pos = strlen(line);

    while (isspace(line[pos - 1]) && pos) {
        line[pos - 1] = 0;
        pos--;
    }
}

static char *get_next_line_from_buffer(void)
{
    char *retval = NULL;

    if (read_buffer_pos == read_buffer_len) {
        return NULL;
    }

    retval = read_buffer + read_buffer_pos;

    while ((read_buffer_pos < read_buffer_len) && (read_buffer[read_buffer_pos] != '\n')) {
        read_buffer_pos++;
    }
    if (read_buffer[read_buffer_pos - 1] == '\r') {
        read_buffer[read_buffer_pos - 1] = 0;
    } else {
        read_buffer[read_buffer_pos] = 0;
    }
    read_buffer_pos++;
    read_buffer_line++;

    if (strlen(retval) && isspace(retval[strlen(retval) - 1])) {
        strip_trailing_spaces(retval);
    }

    return retval;
}

static int fill_line_names(char **names, int max_names)
{
    int count = 0;
    char *line = get_next_line_from_buffer();
#if 1
    int i;
#endif

    while (line && strlen(line)) {
        names[count++] = line + 1;
        if (count > max_names) {
            printf("name list overflow\n");
            return 1;
        }
        line = get_next_line_from_buffer();
    }
    names[count] = NULL;

#if 1
    for (i = 0; names[i]; i++) {
        printf("NAME %d: %s\n", i, names[i]);
    }
#endif

    return 0;
}

static int parse_template(char *filename)
{
    char *line = NULL;
    int parsed;

    /* set current project parameters to 'empty' */
    cp_name = NULL;
    cp_type = -1;
    cp_dep_names[0] = NULL;
    cp_include_dirs[0] = NULL;
    cp_source_names[0] = NULL;
    cp_custom_message = NULL;
    cp_custom_source = NULL;
    cp_custom_deps[0] = NULL;
    cp_custom_output = NULL;
    cp_custom_command = NULL;
    cp_post_custom_message = NULL;
    cp_post_custom_source = NULL;
    cp_post_custom_output = NULL;
    cp_post_custom_command = NULL;
    cp_cc_source_path = NULL;
    cp_cc_source_names[0] = NULL;

    line = get_next_line_from_buffer();
    while (line) {
        parsed = 0;
        if (!strlen(line)) {
#if 1
            printf("Line %d is empty\n", read_buffer_line);
#endif
            parsed = 1;
        }
        if (!parsed && line[0] == '#') {
#if 1
            printf("Line %d is a comment line\n", read_buffer_line);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_NAME_STRING, CP_NAME_SIZE)) {
            cp_name = line + CP_NAME_SIZE;
#if 1
            printf("Line %d is a project name line: %s\n", read_buffer_line, cp_name);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_TYPE_STRING, CP_TYPE_SIZE)) {
#if 1
            printf("Line %d is a project type line: %s\n", read_buffer_line, line + CP_TYPE_SIZE);
#endif
            if (!strcmp(line + CP_TYPE_SIZE, "library")) {
                cp_type = CP_TYPE_LIBRARY;
                parsed = 1;
            }
            if (!parsed && !strcmp(line + CP_TYPE_SIZE, "console")) {
                cp_type = CP_TYPE_CONSOLE;
                parsed = 1;
            }
            if (!parsed && !strcmp(line + CP_TYPE_SIZE, "gui")) {
                cp_type = CP_TYPE_GUI;
                parsed = 1;
            }
            if (!parsed) {
                printf("Unknown project type '%s' in line %d of %s\n", line + CP_TYPE_SIZE, read_buffer_line, filename);
                return 1;
            }
        }
        if (!parsed && !strncmp(line, CP_DEPS_STRING, CP_DEPS_SIZE)) {
#if 1
            printf("Line %d is a project deps line: %s\n", read_buffer_line, line + CP_DEPS_SIZE);
#endif
            if (split_line_names(line + CP_DEPS_SIZE, cp_dep_names, MAX_DEP_NAMES)) {
                printf("Error parsing names in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_INCLUDES_STRING, CP_INCLUDES_SIZE)) {
#if 1
            printf("Line %d is a project include dirs line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_include_dirs, MAX_DIRS)) {
                printf("Error parsing include dir name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_SOURCES_STRING, CP_SOURCES_SIZE)) {
#if 1
            printf("Line %d is a project sources line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_source_names, MAX_NAMES)) {
                printf("Error parsing source name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CPUSOURCES_STRING, CP_CPUSOURCES_SIZE)) {
#if 1
            printf("Line %d is a project cpu sources line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_cpusource_names, MAX_CPU_NAMES)) {
                printf("Error parsing cpu source name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_LIBS_STRING, CP_LIBS_SIZE)) {
            cp_libs = line + CP_LIBS_SIZE;
#if 1
            printf("Line %d is a project link libs line: %s\n", read_buffer_line, cp_libs);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_MSG_STRING, CP_CUSTOM_MSG_SIZE)) {
            cp_custom_message = line + CP_CUSTOM_MSG_SIZE;
#if 1
            printf("Line %d is a project custom message line: %s\n", read_buffer_line, cp_custom_message);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_SRC_STRING, CP_CUSTOM_SRC_SIZE)) {
            cp_custom_source = line + CP_CUSTOM_SRC_SIZE;
#if 1
            printf("Line %d is a project custom source line: %s\n", read_buffer_line, cp_custom_source);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_DEPS_STRING, CP_CUSTOM_DEPS_SIZE)) {
#if 1
            printf("Line %d is a custom deps line: %s\n", read_buffer_line, line + CP_CUSTOM_DEPS_SIZE);
#endif
            if (split_line_names(line + CP_CUSTOM_DEPS_SIZE, cp_custom_deps, MAX_DEP_NAMES)) {
                printf("Error parsing custom deps in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_OUTPUT_STRING, CP_CUSTOM_OUTPUT_SIZE)) {
            cp_custom_output = line + CP_CUSTOM_OUTPUT_SIZE;
#if 1
            printf("Line %d is a project custom output line: %s\n", read_buffer_line, cp_custom_output);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_COMMAND_STRING, CP_CUSTOM_COMMAND_SIZE)) {
            cp_custom_command = line + CP_CUSTOM_COMMAND_SIZE;
#if 1
            printf("Line %d is a project custom command line: %s\n", read_buffer_line, cp_custom_command);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_MSG_STRING, CP_POST_CUSTOM_MSG_SIZE)) {
            cp_post_custom_message = line + CP_POST_CUSTOM_MSG_SIZE;
#if 1
            printf("Line %d is a project post custom message line: %s\n", read_buffer_line, cp_post_custom_message);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_SRC_STRING, CP_POST_CUSTOM_SRC_SIZE)) {
            cp_post_custom_source = line + CP_POST_CUSTOM_SRC_SIZE;
#if 1
            printf("Line %d is a project post custom source line: %s\n", read_buffer_line, cp_post_custom_source);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_OUTPUT_STRING, CP_POST_CUSTOM_OUTPUT_SIZE)) {
            cp_post_custom_output = line + CP_POST_CUSTOM_OUTPUT_SIZE;
#if 1
            printf("Line %d is a project post custom output line: %s\n", read_buffer_line, cp_post_custom_output);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_COMMAND_STRING, CP_POST_CUSTOM_COMMAND_SIZE)) {
            cp_post_custom_command = line + CP_POST_CUSTOM_COMMAND_SIZE;
#if 1
            printf("Line %d is a project post custom command line: %s\n", read_buffer_line, cp_post_custom_command);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CC_SOURCES_STRING, CP_CC_SOURCES_SIZE)) {
#if 1
            printf("Line %d is a project cc sources line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_cc_source_names, MAX_NAMES)) {
                printf("Error parsing cc source name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CC_SOURCE_PATH_STRING, CP_CC_SOURCE_PATH_SIZE)) {
            cp_cc_source_path = line + CP_CC_SOURCE_PATH_SIZE;
#if 1
            printf("Line %d is a project cc source path line: %s\n", read_buffer_line, cp_cc_source_path);
#endif
            parsed = 1;
        }
        if (!parsed) {
#if 1
            printf("Line %d is something else: %s\n", read_buffer_line, line);
#endif
            printf("Unknown command in line %d in %s\n", read_buffer_line, filename);
            return 1;
        }
        line = get_next_line_from_buffer();
    }

    /* Some sanity checks on the custom section */
    if (cp_custom_message || cp_custom_source || cp_custom_deps[0] || cp_custom_output || cp_custom_command) {
        if (!(cp_custom_message && cp_custom_source && cp_custom_deps[0] && cp_custom_output && cp_custom_command)) {
            printf("Missing custom section elements in %s\n", filename);
            return 1;
        }
    }

    /* Name always has to be given */
    if (!cp_name) {
        printf("Missing project name in %s\n", filename);
        return 1;
    }

    /* type always has to be given */
    if (cp_type == -1) {
        printf("Missing project type in %s\n", filename);
        return 1;
    }

    /* for console and gui types libs have to be given */
    if (cp_type != CP_TYPE_LIBRARY) {
        if (!cp_libs) {
            printf("Missing link libs in %s\n", filename);
            return 1;
        }
    }

    return 0;
}

static int read_template_file(char *fname)
{
    char *filename = malloc(strlen(fname) + sizeof(".tmpl"));
    struct stat statbuf;
    FILE *infile = NULL;
    int retval = 0;

    sprintf(filename, "%s.tmpl", fname);

    if (stat(filename, &statbuf) < 0) {
        printf("%s doesn't exist\n", filename);
        retval = 1;
    }

    if (!retval) {
        read_buffer_len = statbuf.st_size;

        if (read_buffer_len == 0) {
            printf("%s has no size\n", filename);
            retval = 1;
        }
    }

    if (!retval) {
        infile = fopen(filename, "rb");
        if (!infile) {
            printf("%s cannot be opened for reading\n", filename);
            retval = 1;
        }
    }

    if (!retval) {
        read_buffer = malloc(read_buffer_len);

        if (fread(read_buffer, 1, read_buffer_len, infile) < read_buffer_len) {
            printf("Cannot read from %s\n", filename);
            retval = 1;
        }
    }

    if (!retval) {
        if (parse_template(filename)) {
            printf("Parse error in %s\n", filename);
            retval = 1;
        }
    }

    if (infile) {
        fclose(infile);
    }

    if (filename) {
        free(filename);
    }

    return retval;
}

static void usage(void)
{
    printf("This program takes one name as a parameter\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage();
    } else {
        if (read_template_file(argv[1])) {
            printf("Generation error.\n");
        } else {
            if (output_msvc6_file(argv[1])) {
                printf("Generation error.\n");
            } else {
                printf("Generation complete.\n");
            }
        }
    }
    if (read_buffer) {
        free(read_buffer);
    }
}
