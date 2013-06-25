#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

/* Enable debug options */
#define MKMSVC_DEBUG 1

#define CP_PROJECT_NAMES_STRING       "PROJECTS ="
#define CP_PROJECT_NAMES_SIZE         sizeof(CP_PROJECT_NAMES_STRING) - 1

#define CP_NAME_STRING                "PROJECTNAME = "
#define CP_NAME_SIZE                  sizeof(CP_NAME_STRING) - 1

#define CP_TYPE_STRING                "PROJECTTYPE = "
#define CP_TYPE_SIZE                  sizeof(CP_TYPE_STRING) - 1

#define CP_DEPS_STRING                "PROJECTDEPS ="
#define CP_DEPS_SIZE                  sizeof(CP_DEPS_STRING) - 1

#define CP_LIBS_STRING                "PROJECTLIBS ="
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

#define CP_RES_SOURCE_STRING		"RESSOURCE = "
#define CP_RES_SOURCE_SIZE			sizeof(CP_RES_SOURCE_STRING) - 1

#define CP_RES_DEPS_STRING			"RESDEPS = "
#define CP_RES_DEPS_SIZE			sizeof(CP_RES_DEPS_STRING) - 1

#define CP_RES_OUTPUT_STRING		"RESOUTPUT = "
#define CP_RES_OUTPUT_SIZE			sizeof(CP_RES_OUTPUT_STRING) - 1

#define CP_TYPE_LIBRARY   1
#define CP_TYPE_CONSOLE   2
#define CP_TYPE_GUI       3

#define MAX_DIRS        100
#define MAX_DEP_NAMES   100
#define MAX_NAMES       200
#define MAX_CPU_NAMES   10
#define MAX_LIBS        30

/* Main project file handle */
static FILE *mainfile = NULL;

/* project names */
static char *project_names[MAX_NAMES];

/* current project file parameters */
static char *cp_name = NULL;
static int cp_type = -1;
static char *cp_dep_names[MAX_DEP_NAMES];
static char *cp_include_dirs[MAX_DIRS];
static char *cp_source_names[MAX_NAMES];
static char *cp_cpusource_names[MAX_CPU_NAMES];
static char *cp_libs = NULL;
static char *cp_libs_elements[MAX_LIBS];
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
static char *cp_res_source_name = NULL;
static char *cp_res_deps[MAX_DEP_NAMES];
static char *cp_res_output_name = NULL;

/* read buffer related */
static char *names_buffer = NULL;
static char *read_buffer = NULL;
static int read_buffer_line = 0;
static int read_buffer_pos = 0;
static int read_buffer_len = 0;


/* Solution file information structure */
typedef struct project_info_s {
    char *name;
    char *dep[MAX_DEP_NAMES];
} project_info_t;

static project_info_t project_info[MAX_DEP_NAMES];

/* ---------------------------------------------------------------------- */

void pi_init(void)
{
    int i, j;

    /* init project_info */
    for (i = 0; i < MAX_DEP_NAMES; i++) {
        project_info[i].name = NULL;
        for (j = 0; j < MAX_DEP_NAMES; j++) {
            project_info[i].dep[j] = NULL;
        }
    }
}

static void pi_exit(void)
{
    int i, j;

    /* free all names */
    for (i = 0; project_info[i].name; i++) {
        if (project_info[i].dep[0]) {
            for (j = 0; project_info[i].dep[j]; j++) {
                free(project_info[i].dep[j]);
            }
        }
        free(project_info[i].name);
    }
}

static int pi_get_index_of_name(char *name)
{
    int i;
    int retval = -1;

    for (i = 0; project_info[i].name && retval == -1; i++) {
        if (!strcmp(name, project_info[i].name)) {
            retval = i;
        }
    }
    return retval;
}

static int pi_insert_name(char *name)
{
    int i = 0;

    while (project_info[i].name) {
        i++;
    }
    project_info[i].name = strdup(name);

    return i;
}

static void pi_insert_deps(char **names, int index)
{
    int i;

    for (i = 0; names[i]; i++) {
        project_info[index].dep[i] = strdup(names[i]);
    }
}

/* ---------------------------------------------------------------------- */

static char *msvc_platform[3] = {
    "Win32",
    "Itanium",
    "x64"
};

static char *msvc_type_name[4] = {
    "DX Debug",
    "DX Release",
    "Debug",
    "Release"
};

static char *msvc_type[4] = {
    "DXDebug",
    "DXRelease",
    "Debug",
    "Release"
};

static char *msvc_predefs[4] = {
    "_DEBUG",
    "NDEBUG",
    "NODIRECTX,_DEBUG",
    "NODIRECTX,NDEBUG"
};

static char *msvc8_preprodefs[2] = {
    "_DEBUG",
    "NDEBUG"
};

static char *msvc8_project_start = "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\r\n"
                                   "<VisualStudioProject\r\n"
                                   "\tProjectType=\"Visual C++\"\r\n"
                                   "\tVersion=\"8.00\"\r\n"
                                   "\tName=\"%s\"\r\n"
                                   "\tProjectGUID=\"{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}\"\r\n"
                                   "\t>\r\n"
                                   "\t<Platforms>\r\n"
                                   "\t\t\tName=\"Win32\"\r\n"
                                   "\t\t/>\r\n"
                                   "\t\t<Platform\r\n"
                                   "\t\t\tName=\"Itanium\"\r\n"
                                   "\t\t/>\r\n"
                                   "\t\t<Platform\r\n"
                                   "\t\t\tName=\"x64\"\r\n"
                                   "\t\t/>\r\n"
                                   "\t</Platforms>\r\n"
                                   "\t<ToolFiles>\r\n"
                                   "\t</ToolFiles>\r\n"
                                   "\t<Configurations>\r\n";

static char *msvc8_config_start = "\t\t<Configuration\r\n"
                                  "\t\t\tName=\"%s|%s\"\r\n";

static char *msvc8_outputdir_app = "\t\t\tOutputDirectory=\".\\..\\..\\..\\..\\data\"\r\n";

static char *msvc8_outputdir_lib = "\t\t\tOutputDirectory=\".\\libs\\%s\\%s%s\"\r\n";

static char *msvc8_config_part2 = "\t\t\tIntermediateDirectory=\".\\libs\\%s\\%s%s\"\r\n"
                                  "\t\t\tConfigurationType=\"%d\"\r\n"
                                  "\t\t\tInheritedPropertySheets=\"$(VCInstallDir)VCProjectDefaults\\UpgradeFromVC71.vsprops\"\r\n"
                                  "\t\t\tUseOfMFC=\"0\"\r\n"
                                  "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"false\"\r\n";

static char *msvc8_charset = "\t\t\tCharacterSet=\"2\"\r\n";

static char *msvc8_config_part3 = "\t\t\t>\r\n"
                                  "\t\t\t<Tool\r\n"
                                  "\t\t\t\tName=\"VCPreBuildEventTool\"\r\n"
                                  "\t\t\t/>\r\n";

static char *msvc8_cbt_helper = "\t\t\t<Tool\r\n"
                                "\t\t\t\tName=\"VCCustomBuildTool\"\r\n"
                                "\t\t\t\tCommandLine=\"%s&#x0D;&#x0A;\"\r\n"
                                "\t\t\t\tOutputs=\"%s\"\r\n"
                                "\t\t\t/>\r\n";

static char *msvc8_cbt = "\t\t\t<Tool\r\n"
                         "\t\t\t\tName=\"VCCustomBuildTool\"\r\n"
                         "\t\t\t/>\r\n";

static char *msvc8_config_part4 = "\t\t\t<Tool\r\n"
                                  "\t\t\t\tName=\"VCXMLDataGeneratorTool\"\r\n"
                                  "\t\t\t/>\r\n"
                                  "\t\t\t<Tool\r\n"
                                  "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"\r\n"
                                  "\t\t\t/>\r\n";

static char *msvc8_midl_tool_gui = "\t\t\t<Tool\r\n"
                                   "\t\t\t\tName=\"VCMIDLTool\"\r\n"
                                   "\t\t\t\tPreprocessorDefinitions=\"%s\"\r\n"
                                   "\t\t\t\tMkTypLibCompatible=\"true\"\r\n"
                                   "\t\t\t\tSuppressStartupBanner=\"true\"\r\n"
                                   "\t\t\t\tTargetEnvironment=\"%d\"\r\n"
                                   "\t\t\t\tTypeLibraryName=\".\\..\\..\\..\\..\\data\\%s.tlb"
                                   "\t\t\t/>\r\n";

static char *msvc8_midl_tool_console = "\t\t\t<Tool\r\n"
                                       "\t\t\t\tName=\"VCMIDLTool\"\r\n"
                                       "\t\t\t\tTypeLibraryName=\".\\..\\..\\..\\..\\data\\%s.tlb\"\r\n"
                                       "\t\t\t/>\r\n";

static char *msvc8_midl_tool_lib = "\t\t\t<Tool\r\n"
                                   "\t\t\t\tName=\"VCMIDLTool\"\r\n"
                                   "\t\t\t/>\r\n";

static char *msvc8_compiler_tool_start = "\t\t\t<Tool\r\n"
                                         "\t\t\t\tName=\"VCCLCompilerTool\"\r\n";

static char *msvc8_opt0 = "\t\t\t\tOptimization=\"0\"\r\n";

static char *msvc8_ife = "\t\t\t\tInlineFunctionExpansion=\"1\"\r\n";

static char *msvc8_includes = "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc,..\\,..\\..\\..\\";

static char *msvc8_includes_cc = "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc\"\r\n";

static char *msvc8_includes_cc_mixed = "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc,..\\,..\\..\\..\\,..\\..\\..\\resid\"\r\n";

static char *msvc8_defs = "\t\t\t\tPreprocessorDefinitions=\"WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,%s\"\r\n";

static char *msvc8_defs_cc = "\t\t\t\tPreprocessorDefinitions=\"WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,%s,PACKAGE=\\&quot;%s\\&quot;,VERSION=\\&quot;0.7\\&quot;,SIZEOF_INT=4\"\r\n";

static char *msvc8_string_pooling = "\t\t\t\tStringPooling=\"true\"\r\n";

static char *msvc8_ct_part2 = "\t\t\t\tRuntimeLibrary=\"%d\"\r\n";

static char *msvc8_efll = "\t\t\t\tEnableFunctionLevelLinking=\"true\"\r\n";

static char *msvc8_ct_part3 = "\t\t\t\tUsePrecompiledHeader=\"0\"\r\n"
                              "\t\t\t\tPrecompiledHeaderFile=\".\\libs\\%s\\%s%s\\%s.pch\"\r\n"
                              "\t\t\t\tAssemblerListingLocation=\".\\libs\\%s\%s%s\\\"\r\n"
                              "\t\t\t\tObjectFile=\".\\libs\\%s\\%s%s\\\"\r\n"
                              "\t\t\t\tProgramDataBaseFileName=\".\\libs\\%s\\%s%s\\\"\r\n"
                              "\t\t\t\tWarningLevel=\"3\"\r\n"
                              "\t\t\t\tSuppressStartupBanner=\"true\"\r\n";

static char *msvc8_dif = "\t\t\t\tDebugInformationFormat=\"1\"\r\n";

static char *msvc8_config_part5 = "\t\t\t\tCompileAs=\"0\"\r\n"
                                  "\t\t\t/>\r\n"
                                  "\t\t\t<Tool\r\n"
                                  "\t\t\t\tName=\"VCManagedResourceCompilerTool\"\r\n"
                                  "\t\t\t/>\r\n";



/* ### EMPTY
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>

			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\_vice.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\_vice.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### LIBRARY
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXDebug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\libs\c64c128\DXRelease"
			IntermediateDirectory=".\libs\c64c128\DXRelease"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\DXRelease\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\DXRelease\"
				ObjectFile=".\libs\c64c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c64c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXRelease\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\libs\c64c128\Debug"
			IntermediateDirectory=".\libs\c64c128\Debug"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Debug\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Debug\"
				ObjectFile=".\libs\c64c128\Debug\"
				ProgramDataBaseFileName=".\libs\c64c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Debug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\libs\c64c128\Release"
			IntermediateDirectory=".\libs\c64c128\Release"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Release\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Release\"
				ObjectFile=".\libs\c64c128\Release\"
				ProgramDataBaseFileName=".\libs\c64c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Release\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\DXDebug\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\DXDebug\"
				ObjectFile=".\libs\c64c128\DXDebug\"
				ProgramDataBaseFileName=".\libs\c64c128\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXDebug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\DXRelease\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\DXRelease\"
				ObjectFile=".\libs\c64c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c64c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXRelease\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Debug\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Debug\"
				ObjectFile=".\libs\c64c128\Debug\"
				ProgramDataBaseFileName=".\libs\c64c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Debug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Release\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Release\"
				ObjectFile=".\libs\c64c128\Release\"
				ProgramDataBaseFileName=".\libs\c64c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Release\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\DXDebug\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\DXDebug\"
				ObjectFile=".\libs\c64c128\DXDebug\"
				ProgramDataBaseFileName=".\libs\c64c128\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXDebug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\DXRelease\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\DXRelease\"
				ObjectFile=".\libs\c64c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c64c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\DXRelease\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Debug\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Debug\"
				ObjectFile=".\libs\c64c128\Debug\"
				ProgramDataBaseFileName=".\libs\c64c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Debug\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64\cart,..\..\..\drive,..\..\..\drive\iec\c64exp,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vicii"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c64c128\Release\c64c128.pch"
				AssemblerListingLocation=".\libs\c64c128\Release\"
				ObjectFile=".\libs\c64c128\Release\"
				ProgramDataBaseFileName=".\libs\c64c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c64c128\Release\c64c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\c64\c64-snapshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64bus.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64cia2.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64datasette.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64export.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64gluelogic.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64iec.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64io.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64keyboard.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64meminit.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64memrom.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64parallel.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64pla.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64printer.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64rsuser.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\c64sound.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c64\patchrom.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### CPU LIBRARY
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXDebug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\libs\c128\DXRelease"
			IntermediateDirectory=".\libs\c128\DXRelease"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\DXRelease\c128.pch"
				AssemblerListingLocation=".\libs\c128\DXRelease\"
				ObjectFile=".\libs\c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXRelease\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\libs\c128\Debug"
			IntermediateDirectory=".\libs\c128\Debug"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Debug\c128.pch"
				AssemblerListingLocation=".\libs\c128\Debug\"
				ObjectFile=".\libs\c128\Debug\"
				ProgramDataBaseFileName=".\libs\c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Debug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\libs\c128\Release"
			IntermediateDirectory=".\libs\c128\Release"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Release\c128.pch"
				AssemblerListingLocation=".\libs\c128\Release\"
				ObjectFile=".\libs\c128\Release\"
				ProgramDataBaseFileName=".\libs\c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Release\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\DXDebug\c128.pch"
				AssemblerListingLocation=".\libs\c128\DXDebug\"
				ObjectFile=".\libs\c128\DXDebug\"
				ProgramDataBaseFileName=".\libs\c128\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXDebug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\DXRelease\c128.pch"
				AssemblerListingLocation=".\libs\c128\DXRelease\"
				ObjectFile=".\libs\c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXRelease\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Debug\c128.pch"
				AssemblerListingLocation=".\libs\c128\Debug\"
				ObjectFile=".\libs\c128\Debug\"
				ProgramDataBaseFileName=".\libs\c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Debug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Release\c128.pch"
				AssemblerListingLocation=".\libs\c128\Release\"
				ObjectFile=".\libs\c128\Release\"
				ProgramDataBaseFileName=".\libs\c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Release\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\DXDebug\c128.pch"
				AssemblerListingLocation=".\libs\c128\DXDebug\"
				ObjectFile=".\libs\c128\DXDebug\"
				ProgramDataBaseFileName=".\libs\c128\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXDebug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\DXRelease\c128.pch"
				AssemblerListingLocation=".\libs\c128\DXRelease\"
				ObjectFile=".\libs\c128\DXRelease\"
				ProgramDataBaseFileName=".\libs\c128\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\DXRelease\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Debug\c128.pch"
				AssemblerListingLocation=".\libs\c128\Debug\"
				ObjectFile=".\libs\c128\Debug\"
				ProgramDataBaseFileName=".\libs\c128\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Debug\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\c64,..\..\..\c64\cart,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\raster,..\..\..\rtc,..\..\..\sid,..\..\..\tape,..\..\..\userport,..\..\..\vdc,..\..\..\vicii,..\..\..\video"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\c128\Release\c128.pch"
				AssemblerListingLocation=".\libs\c128\Release\"
				ObjectFile=".\libs\c128\Release\"
				ProgramDataBaseFileName=".\libs\c128\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\c128\Release\c128.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\c128\c128-cmdline-options.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128-resources.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128-snapshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128cia1.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128cpu.c"
			>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\c128\c128drive.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128fastiec.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128mem.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128meminit.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128memlimit.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128memrom.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128memsnapshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128mmu.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128model.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128rom.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128romset.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\c128video.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\daa.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\functionrom.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\z80.c"
			>
		</File>
		<File
			RelativePath="..\..\..\c128\z80mem.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### CUSTOM LIBARY
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXDebug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\libs\base\DXRelease"
			IntermediateDirectory=".\libs\base\DXRelease"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\DXRelease\base.pch"
				AssemblerListingLocation=".\libs\base\DXRelease\"
				ObjectFile=".\libs\base\DXRelease\"
				ProgramDataBaseFileName=".\libs\base\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXRelease\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\libs\base\Debug"
			IntermediateDirectory=".\libs\base\Debug"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Debug\base.pch"
				AssemblerListingLocation=".\libs\base\Debug\"
				ObjectFile=".\libs\base\Debug\"
				ProgramDataBaseFileName=".\libs\base\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Debug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\libs\base\Release"
			IntermediateDirectory=".\libs\base\Release"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Release\base.pch"
				AssemblerListingLocation=".\libs\base\Release\"
				ObjectFile=".\libs\base\Release\"
				ProgramDataBaseFileName=".\libs\base\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Release\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\DXDebug\base.pch"
				AssemblerListingLocation=".\libs\base\DXDebug\"
				ObjectFile=".\libs\base\DXDebug\"
				ProgramDataBaseFileName=".\libs\base\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXDebug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\DXRelease\base.pch"
				AssemblerListingLocation=".\libs\base\DXRelease\"
				ObjectFile=".\libs\base\DXRelease\"
				ProgramDataBaseFileName=".\libs\base\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXRelease\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Debug\base.pch"
				AssemblerListingLocation=".\libs\base\Debug\"
				ObjectFile=".\libs\base\Debug\"
				ProgramDataBaseFileName=".\libs\base\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Debug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Release\base.pch"
				AssemblerListingLocation=".\libs\base\Release\"
				ObjectFile=".\libs\base\Release\"
				ProgramDataBaseFileName=".\libs\base\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Release\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\DXDebug\base.pch"
				AssemblerListingLocation=".\libs\base\DXDebug\"
				ObjectFile=".\libs\base\DXDebug\"
				ProgramDataBaseFileName=".\libs\base\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXDebug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\DXRelease\base.pch"
				AssemblerListingLocation=".\libs\base\DXRelease\"
				ObjectFile=".\libs\base\DXRelease\"
				ProgramDataBaseFileName=".\libs\base\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\DXRelease\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Debug\base.pch"
				AssemblerListingLocation=".\libs\base\Debug\"
				ObjectFile=".\libs\base\Debug\"
				ProgramDataBaseFileName=".\libs\base\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Debug\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\imagecontents,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\rs232drv,..\..\..\rtc,..\..\..\userport,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\base\Release\base.pch"
				AssemblerListingLocation=".\libs\base\Release\"
				ObjectFile=".\libs\base\Release\"
				ProgramDataBaseFileName=".\libs\base\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\base\Release\base.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\alarm.c"
			>
		</File>
		<File
			RelativePath="..\..\..\attach.c"
			>
		</File>
		<File
			RelativePath="..\..\..\autostart-prg.c"
			>
		</File>
		<File
			RelativePath="..\..\..\autostart.c"
			>
		</File>
		<File
			RelativePath="..\..\..\cbmdos.c"
			>
		</File>
		<File
			RelativePath="..\..\..\cbmimage.c"
			>
		</File>
		<File
			RelativePath="..\..\..\charset.c"
			>
		</File>
		<File
			RelativePath="..\..\..\clipboard.c"
			>
		</File>
		<File
			RelativePath="..\..\..\clkguard.c"
			>
		</File>
		<File
			RelativePath="..\..\..\cmdline.c"
			>
		</File>
		<File
			RelativePath="..\..\..\color.c"
			>
		</File>
		<File
			RelativePath="..\..\..\crc32.c"
			>
		</File>
		<File
			RelativePath="..\..\..\datasette.c"
			>
		</File>
		<File
			RelativePath="..\..\..\debug.c"
			>
		</File>
		<File
			RelativePath="..\..\..\dma.c"
			>
		</File>
		<File
			RelativePath="..\..\..\event.c"
			>
		</File>
		<File
			RelativePath="..\..\..\findpath.c"
			>
		</File>
		<File
			RelativePath="..\..\..\fliplist.c"
			>
		</File>
		<File
			RelativePath="..\..\..\gcr.c"
			>
		</File>
		<File
			RelativePath="..\..\..\info.c"
			>
		</File>
		<File
			RelativePath="..\..\..\init.c"
			>
		</File>
		<File
			RelativePath="..\..\..\initcmdline.c"
			>
		</File>
		<File
			RelativePath="..\..\..\interrupt.c"
			>
		</File>
		<File
			RelativePath="..\..\..\ioutil.c"
			>
		</File>
		<File
			RelativePath="..\..\..\joystick.c"
			>
		</File>
		<File
			RelativePath="..\..\..\kbdbuf.c"
			>
		</File>
		<File
			RelativePath="..\..\..\keyboard.c"
			>
		</File>
		<File
			RelativePath="..\..\..\lib.c"
			>
		</File>
		<File
			RelativePath="..\..\..\lightpen.c"
			>
		</File>
		<File
			RelativePath="..\..\..\log.c"
			>
		</File>
		<File
			RelativePath="..\..\..\machine-bus.c"
			>
		</File>
		<File
			RelativePath="..\..\..\machine.c"
			>
		</File>
		<File
			RelativePath="..\..\..\midi.c"
			>
		</File>
		<File
			RelativePath="..\..\..\mouse.c"
			>
		</File>
		<File
			RelativePath="..\..\..\network.c"
			>
		</File>
		<File
			RelativePath="..\..\..\opencbmlib.c"
			>
		</File>
		<File
			RelativePath="..\..\..\palette.c"
			>
		</File>
		<File
			RelativePath="..\..\..\ram.c"
			>
		</File>
		<File
			RelativePath="..\..\..\rawfile.c"
			>
		</File>
		<File
			RelativePath="..\..\..\rawnet.c"
			>
		</File>
		<File
			RelativePath="..\..\..\resources.c"
			>
		</File>
		<File
			RelativePath="..\..\..\romset.c"
			>
		</File>
		<File
			RelativePath="..\..\..\screenshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\snapshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\socket.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sound.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sysfile.c"
			>
		</File>
		<File
			RelativePath="..\..\..\translate.c"
			>
		</File>
		<File
			RelativePath="..\..\..\translate.txt"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating translate.h and translate_table.h"
					CommandLine="..\..\..\..\data\gentranslate ..\..\..\translate.txt ..\..\..\translate.h ..\..\..\translate_table.h&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\translate.txt;"
					Outputs="..\..\..\translate_table.h"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\traps.c"
			>
		</File>
		<File
			RelativePath="..\..\..\util.c"
			>
		</File>
		<File
			RelativePath="..\..\..\vsync.c"
			>
		</File>
		<File
			RelativePath=".\winid.bat"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x86.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x86.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x86.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x86.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x86.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x86.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x86.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x86.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_ia64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_ia64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_ia64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_ia64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_ia64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_ia64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_ia64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_ia64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					Description="Generating winid.bat"
					CommandLine="copy winid_x64.bat winid.bat&#x0D;&#x0A;"
					AdditionalDependencies=".\winid_x64.bat;"
					Outputs=".\winid.bat"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\zfile.c"
			>
		</File>
		<File
			RelativePath="..\..\..\zipcode.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### CC LIBRARY
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXDebug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\libs\resid\DXRelease"
			IntermediateDirectory=".\libs\resid\DXRelease"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\DXRelease\resid.pch"
				AssemblerListingLocation=".\libs\resid\DXRelease\"
				ObjectFile=".\libs\resid\DXRelease\"
				ProgramDataBaseFileName=".\libs\resid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXRelease\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\libs\resid\Debug"
			IntermediateDirectory=".\libs\resid\Debug"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Debug\resid.pch"
				AssemblerListingLocation=".\libs\resid\Debug\"
				ObjectFile=".\libs\resid\Debug\"
				ProgramDataBaseFileName=".\libs\resid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Debug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\libs\resid\Release"
			IntermediateDirectory=".\libs\resid\Release"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Release\resid.pch"
				AssemblerListingLocation=".\libs\resid\Release\"
				ObjectFile=".\libs\resid\Release\"
				ProgramDataBaseFileName=".\libs\resid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Release\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\DXDebug\resid.pch"
				AssemblerListingLocation=".\libs\resid\DXDebug\"
				ObjectFile=".\libs\resid\DXDebug\"
				ProgramDataBaseFileName=".\libs\resid\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXDebug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\DXRelease\resid.pch"
				AssemblerListingLocation=".\libs\resid\DXRelease\"
				ObjectFile=".\libs\resid\DXRelease\"
				ProgramDataBaseFileName=".\libs\resid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXRelease\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Debug\resid.pch"
				AssemblerListingLocation=".\libs\resid\Debug\"
				ObjectFile=".\libs\resid\Debug\"
				ProgramDataBaseFileName=".\libs\resid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Debug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Release\resid.pch"
				AssemblerListingLocation=".\libs\resid\Release\"
				ObjectFile=".\libs\resid\Release\"
				ProgramDataBaseFileName=".\libs\resid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Release\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\DXDebug\resid.pch"
				AssemblerListingLocation=".\libs\resid\DXDebug\"
				ObjectFile=".\libs\resid\DXDebug\"
				ProgramDataBaseFileName=".\libs\resid\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXDebug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\DXRelease\resid.pch"
				AssemblerListingLocation=".\libs\resid\DXRelease\"
				ObjectFile=".\libs\resid\DXRelease\"
				ProgramDataBaseFileName=".\libs\resid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\DXRelease\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Debug\resid.pch"
				AssemblerListingLocation=".\libs\resid\Debug\"
				ObjectFile=".\libs\resid\Debug\"
				ProgramDataBaseFileName=".\libs\resid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Debug\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;resid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\resid\Release\resid.pch"
				AssemblerListingLocation=".\libs\resid\Release\"
				ObjectFile=".\libs\resid\Release\"
				ProgramDataBaseFileName=".\libs\resid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\resid\Release\resid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\resid\dac.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\envelope.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\extfilt.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\filter.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\pot.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\sid.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\version.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\voice.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\resid\wave.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXDebug\resid.pch&quot; /Fo&quot;libs\resid\DXDebug/&quot; /Fd&quot;libs\resid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\DXRelease\resid.pch&quot; /Fo&quot;libs\resid\DXRelease/&quot; /Fd&quot;libs\resid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Debug\resid.pch&quot; /Fo&quot;libs\resid\Debug/&quot; /Fd&quot;libs\resid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;resid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\resid\Release\resid.pch&quot; /Fo&quot;libs\resid\Release/&quot; /Fd&quot;libs\resid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\resid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### CC MIXED LIBRARY
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXDebug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\libs\sid\DXRelease"
			IntermediateDirectory=".\libs\sid\DXRelease"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\DXRelease\sid.pch"
				AssemblerListingLocation=".\libs\sid\DXRelease\"
				ObjectFile=".\libs\sid\DXRelease\"
				ProgramDataBaseFileName=".\libs\sid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXRelease\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\libs\sid\Debug"
			IntermediateDirectory=".\libs\sid\Debug"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Debug\sid.pch"
				AssemblerListingLocation=".\libs\sid\Debug\"
				ObjectFile=".\libs\sid\Debug\"
				ProgramDataBaseFileName=".\libs\sid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Debug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\libs\sid\Release"
			IntermediateDirectory=".\libs\sid\Release"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Release\sid.pch"
				AssemblerListingLocation=".\libs\sid\Release\"
				ObjectFile=".\libs\sid\Release\"
				ProgramDataBaseFileName=".\libs\sid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Release\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\DXDebug\sid.pch"
				AssemblerListingLocation=".\libs\sid\DXDebug\"
				ObjectFile=".\libs\sid\DXDebug\"
				ProgramDataBaseFileName=".\libs\sid\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXDebug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\DXRelease\sid.pch"
				AssemblerListingLocation=".\libs\sid\DXRelease\"
				ObjectFile=".\libs\sid\DXRelease\"
				ProgramDataBaseFileName=".\libs\sid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXRelease\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Debug\sid.pch"
				AssemblerListingLocation=".\libs\sid\Debug\"
				ObjectFile=".\libs\sid\Debug\"
				ProgramDataBaseFileName=".\libs\sid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Debug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Release\sid.pch"
				AssemblerListingLocation=".\libs\sid\Release\"
				ObjectFile=".\libs\sid\Release\"
				ProgramDataBaseFileName=".\libs\sid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Release\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\DXDebug\sid.pch"
				AssemblerListingLocation=".\libs\sid\DXDebug\"
				ObjectFile=".\libs\sid\DXDebug\"
				ProgramDataBaseFileName=".\libs\sid\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXDebug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\DXRelease\sid.pch"
				AssemblerListingLocation=".\libs\sid\DXRelease\"
				ObjectFile=".\libs\sid\DXRelease\"
				ProgramDataBaseFileName=".\libs\sid\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\DXRelease\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Debug\sid.pch"
				AssemblerListingLocation=".\libs\sid\Debug\"
				ObjectFile=".\libs\sid\Debug\"
				ProgramDataBaseFileName=".\libs\sid\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Debug\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="4"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\resid"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG,PACKAGE=\&quot;sid\&quot;,VERSION=\&quot;0.7\&quot;,SIZEOF_INT=4"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\sid\Release\sid.pch"
				AssemblerListingLocation=".\libs\sid\Release\"
				ObjectFile=".\libs\sid\Release\"
				ProgramDataBaseFileName=".\libs\sid\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile=".\libs\sid\Release\sid.lib"
				SuppressStartupBanner="true"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\sid\fastsid.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sid\resid-fp.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\sid\resid.cc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXDebug\sid.pch&quot; /Fo&quot;libs\sid\DXDebug/&quot; /Fd&quot;libs\sid\DXDebug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXDebug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\DXRelease\sid.pch&quot; /Fo&quot;libs\sid\DXRelease/&quot; /Fd&quot;libs\sid\DXRelease/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\DXRelease\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MTd /W3 /EHsc /Z7 /Od /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Debug\sid.pch&quot; /Fo&quot;libs\sid\Debug/&quot; /Fd&quot;libs\sid\Debug/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Debug\$(InputName).obj"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="cl /nologo /MT /W3 /EHsc /I &quot;..\msvc&quot; /I &quot;..\..\../&quot; /I &quot;../&quot; /I &quot;..\..\..\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; /D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot; /D PACKAGE=\&quot;sid\&quot; /D VERSION=\&quot;0.7\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\sid\Release\sid.pch&quot; /Fo&quot;libs\sid\Release/&quot; /Fd&quot;libs\sid\Release/&quot; /FD /TP /c &quot;$(InputPath)&quot;&#x0D;&#x0A;"
					Outputs="libs\sid\Release\$(InputName).obj"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\..\..\sid\sid-cmdline-options.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sid\sid-resources.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sid\sid-snapshot.c"
			>
		</File>
		<File
			RelativePath="..\..\..\sid\sid.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### CONSOLE
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\petcat\DXRelease"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\DXRelease\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\DXRelease\"
				ObjectFile=".\libs\petcat\DXRelease\"
				ProgramDataBaseFileName=".\libs\petcat\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\petcat\Debug"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Debug\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Debug\"
				ObjectFile=".\libs\petcat\Debug\"
				ProgramDataBaseFileName=".\libs\petcat\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\petcat\Release"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Release\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Release\"
				ObjectFile=".\libs\petcat\Release\"
				ProgramDataBaseFileName=".\libs\petcat\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\DXDebug\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\DXDebug\"
				ObjectFile=".\libs\petcat\DXDebug\"
				ProgramDataBaseFileName=".\libs\petcat\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\DXRelease\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\DXRelease\"
				ObjectFile=".\libs\petcat\DXRelease\"
				ProgramDataBaseFileName=".\libs\petcat\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Debug\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Debug\"
				ObjectFile=".\libs\petcat\Debug\"
				ProgramDataBaseFileName=".\libs\petcat\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Release\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Release\"
				ObjectFile=".\libs\petcat\Release\"
				ProgramDataBaseFileName=".\libs\petcat\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\DXDebug\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\DXDebug\"
				ObjectFile=".\libs\petcat\DXDebug\"
				ProgramDataBaseFileName=".\libs\petcat\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\DXRelease\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\DXRelease\"
				ObjectFile=".\libs\petcat\DXRelease\"
				ProgramDataBaseFileName=".\libs\petcat\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Debug\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Debug\"
				ObjectFile=".\libs\petcat\Debug\"
				ProgramDataBaseFileName=".\libs\petcat\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\petcat.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\petcat\Release\petcat.pch"
				AssemblerListingLocation=".\libs\petcat\Release\"
				ObjectFile=".\libs\petcat\Release\"
				ProgramDataBaseFileName=".\libs\petcat\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\petcat.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\petcat.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\charset.c"
			>
		</File>
		<File
			RelativePath="..\..\..\findpath.c"
			>
		</File>
		<File
			RelativePath="..\..\..\ioutil.c"
			>
		</File>
		<File
			RelativePath="..\..\..\lib.c"
			>
		</File>
		<File
			RelativePath="..\..\..\log.c"
			>
		</File>
		<File
			RelativePath="..\..\..\petcat.c"
			>
		</File>
		<File
			RelativePath="..\..\..\rawfile.c"
			>
		</File>
		<File
			RelativePath="..\..\..\resources.c"
			>
		</File>
		<File
			RelativePath="..\..\..\util.c"
			>
		</File>
		<File
			RelativePath="..\..\..\zipcode.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### GUI
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\x64\DXRelease"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="1"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\DXRelease\x64.pch"
				AssemblerListingLocation=".\libs\x64\DXRelease\"
				ObjectFile=".\libs\x64\DXRelease\"
				ProgramDataBaseFileName=".\libs\x64\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\x64\Debug"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_DEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="1"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Debug\x64.pch"
				AssemblerListingLocation=".\libs\x64\Debug\"
				ObjectFile=".\libs\x64\Debug\"
				ProgramDataBaseFileName=".\libs\x64\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\x64\Release"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="1"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Release\x64.pch"
				AssemblerListingLocation=".\libs\x64\Release\"
				ObjectFile=".\libs\x64\Release\"
				ProgramDataBaseFileName=".\libs\x64\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_DEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\DXDebug\x64.pch"
				AssemblerListingLocation=".\libs\x64\DXDebug\"
				ObjectFile=".\libs\x64\DXDebug\"
				ProgramDataBaseFileName=".\libs\x64\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\DXRelease\x64.pch"
				AssemblerListingLocation=".\libs\x64\DXRelease\"
				ObjectFile=".\libs\x64\DXRelease\"
				ProgramDataBaseFileName=".\libs\x64\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_DEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Debug\x64.pch"
				AssemblerListingLocation=".\libs\x64\Debug\"
				ObjectFile=".\libs\x64\Debug\"
				ProgramDataBaseFileName=".\libs\x64\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Release\x64.pch"
				AssemblerListingLocation=".\libs\x64\Release\"
				ObjectFile=".\libs\x64\Release\"
				ProgramDataBaseFileName=".\libs\x64\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_DEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\DXDebug\x64.pch"
				AssemblerListingLocation=".\libs\x64\DXDebug\"
				ObjectFile=".\libs\x64\DXDebug\"
				ProgramDataBaseFileName=".\libs\x64\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\DXRelease\x64.pch"
				AssemblerListingLocation=".\libs\x64\DXRelease\"
				ObjectFile=".\libs\x64\DXRelease\"
				ProgramDataBaseFileName=".\libs\x64\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_DEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Debug\x64.pch"
				AssemblerListingLocation=".\libs\x64\Debug\"
				ObjectFile=".\libs\x64\Debug\"
				ProgramDataBaseFileName=".\libs\x64\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="NDEBUG"
				MkTypLibCompatible="true"
				SuppressStartupBanner="true"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\x64.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\,..\..\..\drive,..\..\..\lib\p64,..\..\..\monitor,..\..\..\platform,..\..\..\vdrive"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\x64\Release\x64.pch"
				AssemblerListingLocation=".\libs\x64\Release\"
				ObjectFile=".\libs\x64\Release\"
				ProgramDataBaseFileName=".\libs\x64\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				Culture="1033"
				AdditionalIncludeDirectories="..\msvc;..\;..\..\..\"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="comctl32.lib version.lib winmm.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\x64.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\x64.pdb"
				SubSystem="2"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\main.c"
			>
		</File>
		<File
			RelativePath="..\..\..\maincpu.c"
			>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCLCompilerTool"
					Optimization="4"
					EnableIntrinsicFunctions="true"
					FavorSizeOrSpeed="0"
					OmitFramePointers="true"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath="..\resc64.rc"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				>
				<Tool
					Name="VCCustomBuildTool"
					CommandLine="copy /b ..\resc64.rc + ..\res.rc + ..\resacia.rc + ..\rescia.rc + ..\resdrivec64.rc + ..\reside64.rc + ..\resreu.rc + ..\resgeoram.rc + ..\resramcart.rc + ..\resplus256k.rc + ..\resplus60k.rc + ..\resc64_256k.rc + ..\resmmc64.rc + ..\resmmcreplay.rc + ..\resretroreplay.rc + ..\resrs232user.rc + ..\ressid.rc + ..\resvicii.rc + ..\resdigimax.rc + ..\resmouse.rc + ..\resmidi.rc + ..\resdqbb.rc + ..\resisepic.rc + ..\reslightpen.rc + ..\ressoundexpander.rc + ..\reseasyflash.rc + ..\resautostart.rc + ..\resmagicvoice.rc + ..\resexpert.rc + ..\resds12c887rtc.rc + ..\restfe.rc resc64cat.rc /b&#x0D;&#x0A;"
					AdditionalDependencies="..\..\..\debug.h;..\resc64.rc;..\res.rc;..\resacia.rc;..\rescia.rc;..\resdrivec64.rc;..\reside64.rc;..\resreu.rc;..\resgeoram.rc;..\resramcart.rc;..\resplus256k.rc;..\resplus60k.rc;..\resc64_256k.rc;..\resmmc64.rc;..\resmmcreplay.rc;..\resretroreplay.rc;..\resrs232user.rc;..\ressid.rc;..\resvicii.rc;..\resdigimax.rc;..\resmouse.rc;..\resmidi.rc;..\resdqbb.rc;..\resisepic.rc;..\reslightpen.rc;..\ressoundexpander.rc;..\reseasyflash.rc;..\resautostart.rc;..\resmagicvoice.rc;..\resexpert.rc;..\resds12c887rtc.rc;..\restfe.rc;"
					Outputs="resc64cat.rc"
				/>
			</FileConfiguration>
		</File>
		<File
			RelativePath=".\resc64cat.rc"
			>
		</File>
		<File
			RelativePath="..\vice.manifest"
			>
			<FileConfiguration
				Name="DX Debug|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|Itanium"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|Itanium"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Itanium"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|Itanium"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Debug|x64"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="DX Release|x64"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|x64"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release|x64"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="VCCustomBuildTool"
				/>
			</FileConfiguration>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

/* ### HELPER TOOL
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\geninfocontrib\DXRelease"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\DXRelease\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\DXRelease\"
				ObjectFile=".\libs\geninfocontrib\DXRelease\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\geninfocontrib\Debug"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Debug\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Debug\"
				ObjectFile=".\libs\geninfocontrib\Debug\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\..\..\..\..\data"
			IntermediateDirectory=".\libs\geninfocontrib\Release"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Release\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Release\"
				ObjectFile=".\libs\geninfocontrib\Release\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\DXDebug\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\DXDebug\"
				ObjectFile=".\libs\geninfocontrib\DXDebug\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\DXRelease\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\DXRelease\"
				ObjectFile=".\libs\geninfocontrib\DXRelease\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Debug\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Debug\"
				ObjectFile=".\libs\geninfocontrib\Debug\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|Itanium"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="2"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Release\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Release\"
				ObjectFile=".\libs\geninfocontrib\Release\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="5"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\DXDebug\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\DXDebug\"
				ObjectFile=".\libs\geninfocontrib\DXDebug\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\DXDebug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="DX Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\DXRelease\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\DXRelease\"
				ObjectFile=".\libs\geninfocontrib\DXRelease\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\DXRelease\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Debug|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,_DEBUG"
				RuntimeLibrary="1"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Debug\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Debug\"
				ObjectFile=".\libs\geninfocontrib\Debug\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Debug\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				DebugInformationFormat="1"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="_DEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="2"
				SuppressStartupBanner="true"
				GenerateDebugInformation="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
		<Configuration
			Name="Release|x64"
			OutputDirectory="$(PlatformName)\$(ConfigurationName)"
			IntermediateDirectory="$(PlatformName)\$(ConfigurationName)"
			ConfigurationType="1"
			InheritedPropertySheets="$(VCInstallDir)VCProjectDefaults\UpgradeFromVC71.vsprops"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="false"
			CharacterSet="2"
			>
			<Tool
				Name="VCPreBuildEventTool"
			/>
			<Tool
				Name="VCCustomBuildTool"
				CommandLine="..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed&#x0D;&#x0A;"
				Outputs="..\..\..\infocontrib.h"
			/>
			<Tool
				Name="VCXMLDataGeneratorTool"
			/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
			<Tool
				Name="VCMIDLTool"
				TargetEnvironment="3"
				TypeLibraryName=".\..\..\..\..\data\geninfocontrib.tlb"
			/>
			<Tool
				Name="VCCLCompilerTool"
				InlineFunctionExpansion="1"
				AdditionalIncludeDirectories="..\msvc,..\,..\..\..\"
				PreprocessorDefinitions="WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,NODIRECTX,NDEBUG"
				StringPooling="true"
				RuntimeLibrary="0"
				EnableFunctionLevelLinking="true"
				UsePrecompiledHeader="0"
				PrecompiledHeaderFile=".\libs\geninfocontrib\Release\geninfocontrib.pch"
				AssemblerListingLocation=".\libs\geninfocontrib\Release\"
				ObjectFile=".\libs\geninfocontrib\Release\"
				ProgramDataBaseFileName=".\libs\geninfocontrib\Release\"
				WarningLevel="3"
				SuppressStartupBanner="true"
				CompileAs="0"
			/>
			<Tool
				Name="VCManagedResourceCompilerTool"
			/>
			<Tool
				Name="VCResourceCompilerTool"
				PreprocessorDefinitions="NDEBUG"
				Culture="1033"
			/>
			<Tool
				Name="VCPreLinkEventTool"
			/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions="/MACHINE:I386"
				AdditionalDependencies="version.lib wsock32.lib"
				OutputFile=".\..\..\..\..\data\geninfocontrib.exe"
				LinkIncremental="1"
				SuppressStartupBanner="true"
				ProgramDatabaseFile=".\..\..\..\..\data\geninfocontrib.pdb"
				SubSystem="1"
				TargetMachine="17"
			/>
			<Tool
				Name="VCALinkTool"
			/>
			<Tool
				Name="VCManifestTool"
			/>
			<Tool
				Name="VCXDCMakeTool"
			/>
			<Tool
				Name="VCBscMakeTool"
			/>
			<Tool
				Name="VCFxCopTool"
			/>
			<Tool
				Name="VCAppVerifierTool"
			/>
			<Tool
				Name="VCWebDeploymentTool"
			/>
			<Tool
				Name="VCPostBuildEventTool"
			/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<File
			RelativePath="..\..\..\arch\win32\utils\geninfocontrib.c"
			>
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
*/

static void generate_msvc8_sln(void)
{
    int i, j;

    /* TODO */
}

static int open_msvc8_main_project(void)
{
    pi_init();

    mainfile = fopen("../vs8/vice.sln", "wb");

    if (!mainfile) {
        printf("Cannot open 'vice.sln' for output\n");
        return 1;
    }

    /* TODO */

    return 0;
}

static void close_msvc8_main_project(void)
{
    generate_msvc8_sln();
    pi_exit();
    fclose(mainfile);
}

/* ### */
static int output_msvc8_file(char *fname, int filelist)
{
    char *filename;
    int retval = 0;
    FILE *outfile = NULL;
    int i = 0;
    int j, k;

    if (filelist) {
        i = pi_insert_name(cp_name);
        if (cp_dep_names[0]) {
            pi_insert_deps(cp_dep_names, i);
        }
        filename = malloc(strlen(fname) + sizeof("../vs8/.vcproj"));
        sprintf(filename, "../vs8/%s.vcproj", fname);
    } else {
        filename = malloc(strlen(fname) + sizeof(".vcproj"));
        sprintf(filename, "%s.vcproj", fname);
    }

    outfile = fopen(filename, "wb");
    if (!outfile) {
        printf("Cannot open %s for output\n", filename);
        retval = 1;
    }

    if (!retval) {
        fprintf(outfile, msvc8_project_start, cp_name, i);
        for (k = 0; k < 3; k++) {
            for (i = 0; i < 4; i++) {
                fprintf(outfile, msvc8_config_start, msvc_type_name[i], msvc_platform[k]);
                if (cp_type != CP_TYPE_LIBRARY) {
                    fprintf(outfile, msvc8_outputdir_app);
                } else {
                    fprintf(outfile, msvc8_outputdir_lib, cp_name, msvc_platform[k], msvc_type[i]);
                }
                fprintf(outfile, msvc8_config_part2, cp_name, msvc_platform[k], msvc_type[i], (cp_type == CP_TYPE_LIBRARY) ? 4 : 1);
                if (cp_type == CP_TYPE_CONSOLE) {
                    fprintf(outfile, msvc8_charset);
                }
                fprintf(outfile, msvc8_config_part3);
                if (cp_post_custom_message) {
                    fprintf(outfile, msvc8_cbt_helper, cp_post_custom_command, cp_post_custom_output);
                } else {
                    fprintf(outfile, msvc8_cbt);
                }
                fprintf(outfile, msvc8_config_part4);
                switch (cp_type) {
                    default:
                    case CP_TYPE_GUI:
                        fprintf(outfile, msvc8_midl_tool_gui, msvc8_preprodefs[i & 1], k + 1, cp_name);
                        break;
                    case CP_TYPE_CONSOLE:
                        fprintf(outfile, msvc8_midl_tool_console, cp_name);
                        break;
                    case CP_TYPE_LIBRARY:
                        fprintf(outfile, msvc8_midl_tool_lib);
                        break;
                }
                fprintf(outfile, msvc8_compiler_tool_start);
                if (!(i & 1)) {
                    fprintf(outfile, msvc8_opt0);
                } else {
                    fprintf(outfile, msvc8_ife);
                }
                if (cp_cc_source_path) {
                    if (cp_source_names[0]) {
                        fprintf(outfile, msvc8_includes_cc_mixed);
                    } else {
                        fprintf(outfile, msvc8_includes_cc);
                    }
                } else {
                    fprintf(outfile, msvc8_includes);
                    if (cp_include_dirs[0]) {
                        for (j = 0; cp_include_dirs[j]; j++) {
                            fprintf(outfile, ",..\\..\\..\\%s", cp_include_dirs[j]);
                        }
                    }
                }
                fprintf(outfile, "\"\r\n");
                if (!cp_cc_source_path) {
                    fprintf(outfile, msvc8_defs, msvc_predefs[i]);
                } else {
                    fprintf(outfile, msvc8_defs_cc, msvc_predefs[i], cp_name);
                }
                if (i & 1) {
                    fprintf(outfile, msvc8_string_pooling);
                }
                fprintf(outfile, msvc8_ct_part2, (!(i & 1)));
                if (i & 1) {
                    fprintf(outfile, msvc8_efll);
                }
                fprintf(outfile, msvc8_ct_part3, cp_name, msvc_platform[k], msvc_type[i], cp_name, cp_name, msvc_platform[k], msvc_type[i], cp_name, msvc_platform[k], msvc_type[i], cp_name, msvc_platform[k], msvc_type[i]);
                if (!(i & 1)) {
                    fprintf(outfile, msvc8_dif);
                }
                fprintf(outfile, msvc8_config_part5);
            }
        }
    }

    if (outfile) {
        fclose(outfile);
    }

    if (filename) {
        free(filename);
    }

    if (cp_libs) {
        free(cp_libs);
        cp_libs = NULL;
    }

    if (read_buffer) {
        free(read_buffer);
        read_buffer = NULL;
        read_buffer_line = 0;
        read_buffer_pos = 0;
        read_buffer_len = 0;
    }

    return retval;
}

/* ---------------------------------------------------------------------- */

static char *msvc7x_main_project_end = "\tEndGlobalSection\r\n"
                                       "\tGlobalSection(ExtensibilityGlobals) = postSolution\r\n"
                                       "\tEndGlobalSection\r\n"
                                       "\tGlobalSection(ExtensibilityAddIns) = postSolution\r\n"
                                       "\tEndGlobalSection\r\n"
                                       "EndGlobal\r\n";

static char *msvc7x_main_project_confs = "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.Debug.ActiveCfg = Debug|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.Debug.Build.0 = Debug|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.DX Debug.ActiveCfg = DX Debug|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.DX Debug.Build.0 = DX Debug|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.DX Release.ActiveCfg = DX Release|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.DX Release.Build.0 = DX Release|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.Release.ActiveCfg = Release|Win32\r\n"
                                         "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.Release.Build.0 = Release|Win32\r\n";

static char *msvc71_xml_header = "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\r\n";

static char *msvc71_xmldgt = "\t\t\t<Tool\r\n"
                             "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\r\n";

static char *msvc71_wrapper_tools = "\t\t\t<Tool\r\n"
                                    "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>\r\n"
                                    "\t\t\t<Tool\r\n"
                                    "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\r\n";

static char *msvc71_configs_files = "\t</Configurations>\r\n"
                                    "\t<References>\r\n"
                                    "\t</References>\r\n"
                                    "\t<Files>\r\n";

static char *msvc71_project_deps_start = "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%s\", \"%s.vcproj\", \"{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}\"\r\n"
                                         "\tProjectSection(ProjectDependencies) = postProject\r\n";

static char *msvc71_project_deps = "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X} = {8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}\r\n";

static char *msvc71_project_deps_end = "\tEndProjectSection\r\n"
                                       "EndProject\r\n";

static char *msvc71_project_global_start = "Global\r\n"
                                           "\tGlobalSection(SolutionConfiguration) = preSolution\r\n"
                                           "\t\tDebug = Debug\r\n"
                                           "\t\tDX Debug = DX Debug\r\n"
                                           "\t\tDX Release = DX Release\r\n"
                                           "\t\tRelease = Release\r\n"
                                           "\tEndGlobalSection\r\n"
                                           "\tGlobalSection(ProjectConfiguration) = postSolution\r\n";

static void generate_msvc71_sln(void)
{
    int i, j;
    int index;

    for (i = 0; project_info[i].name; i++) {
        fprintf(mainfile, msvc71_project_deps_start, project_info[i].name, project_info[i].name, i);
        if (project_info[i].dep[0]) {
            for (j = 0; project_info[i].dep[j]; j++) {
                index = pi_get_index_of_name(project_info[i].dep[j]);
                fprintf(mainfile, msvc71_project_deps, index, index);
            }
        }
        fprintf(mainfile, msvc71_project_deps_end);
    }
    fprintf(mainfile, msvc71_project_global_start);
    for (i = 0; project_info[i].name; i++) {
        fprintf(mainfile, msvc7x_main_project_confs, i, i, i, i, i, i, i, i);
    }
    fprintf(mainfile, msvc7x_main_project_end);
}

static int open_msvc71_main_project(void)
{
    pi_init();

    mainfile = fopen("../vs71/vice.sln", "wb");

    if (!mainfile) {
        printf("Cannot open 'vice.sln' for output\n");
        return 1;
    }
    fprintf(mainfile, "Microsoft Visual Studio Solution File, Format Version 7.10\r\n");
    return 0;
}

static void close_msvc71_main_project(void)
{
    generate_msvc71_sln();
    pi_exit();
    fclose(mainfile);
}

/* ---------------------------------------------------------------------- */


static char *msvc70_compiler_tool_type[2] = {
    "Optimization=\"0\"",
    "InlineFunctionExpansion=\"1\""
};

static char *msvc70_cc_extra[4] = {
    "/D &quot;_DEBUG&quot;",
    "/D &quot;NDEBUG&quot;",
    "/D &quot;NODIRECTX&quot; /D &quot;_DEBUG&quot;",
    "/D &quot;NODIRECTX&quot; /D &quot;NDEBUG&quot;"
};

static char *msvc70_flags[2] = {
    "/MTd /W3 /EHsc /Z7 /Od",
    "/MT /W3 /EHsc"
};

static char *msvc70_cc_predefs = ",PACKAGE=\\&quot;%s\\&quot;,VERSION=\\&quot;0.7\\&quot;,SIZEOF_INT=4";

static char *msvc70_console_libs = "version.lib wsock32.lib";

static char *msvc70_gui_libs[2] = {
    "comctl32.lib dsound.lib dxguid.lib winmm.lib version.lib wsock32.lib",
    "comctl32.lib version.lib winmm.lib wsock32.lib"
};

static char *msvc70_xml_header = "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\r\n";

static char *msvc70_project_start = "<VisualStudioProject\r\n"
                                    "\tProjectType=\"Visual C++\"\r\n"
                                    "\tVersion=\"%s\"\r\n"
                                    "\tName=\"%s\"\r\n"
                                    "\tSccProjectName=\"\"\r\n"
                                    "\tSccLocalPath=\"\">\r\n"
                                    "\t<Platforms>\r\n"
                                    "\t\t<Platform\r\n"
                                    "\t\t\tName=\"Win32\"/>\r\n"
                                    "\t</Platforms>\r\n";

static char *msvc70_configurations = "\t<Configurations>\r\n";

static char *msvc70_config_part1 = "\t\t<Configuration\r\n"
                                   "\t\t\tName=\"%s|Win32\"\r\n";

static char *msvc70_config_part2_lib = "\t\t\tOutputDirectory=\".\\libs\\%s\\%s\"\r\n";

static char *msvc70_config_part2_app = "\t\t\tOutputDirectory=\".\\..\\..\\..\\..\\data\"\r\n";

static char *msvc70_config_part3 = "\t\t\tIntermediateDirectory=\".\\libs\\%s\\%s\"\r\n"
                                   "\t\t\tConfigurationType=\"%d\"\r\n"
                                   "\t\t\tUseOfMFC=\"0\"\r\n"
                                   "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"";

static char *msvc70_charset = "\r\n\t\t\tCharacterSet=\"2\"";

static char *msvc70_compiler_tool_part1 = ">\r\n"
                                          "\t\t\t<Tool\r\n"
                                          "\t\t\t\tName=\"VCCLCompilerTool\"\r\n"
                                          "\t\t\t\t%s\r\n";

static char *msvc70_aid = "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc,..\\,..\\..\\..\\";

static char *msvc70_aid_cc = "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc";

static char *msvc70_compiler_tool_part2 = "\t\t\t\tPreprocessorDefinitions=\"WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,";

static char *msvc70_predefs_end = "\"\r\n";

static char *msvc70_string_pooling = "\t\t\t\tStringPooling=\"TRUE\"\r\n";

static char *msvc70_compiler_tool_part3 = "\t\t\t\tRuntimeLibrary=\"%d\"\r\n";

static char *msvc70_fll = "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"\r\n";

static char *msvc70_compiler_tool_part4 = "\t\t\t\tUsePrecompiledHeader=\"2\"\r\n"
                                          "\t\t\t\tPrecompiledHeaderFile=\".\\libs\\%s\\%s\\%s.pch\"\r\n"
                                          "\t\t\t\tAssemblerListingLocation=\".\\libs\\%s\\%s\\\"\r\n"
                                          "\t\t\t\tObjectFile=\".\\libs\\%s\\%s\\\"\r\n"
                                          "\t\t\t\tProgramDataBaseFileName=\".\\libs\\%s\\%s\\\"\r\n"
                                          "\t\t\t\tWarningLevel=\"3\"\r\n"
                                          "\t\t\t\tSuppressStartupBanner=\"TRUE\"\r\n";

static char *msvc70_dif = "\t\t\t\tDebugInformationFormat=\"1\"\r\n";

static char *msvc70_compiler_tool_part5 = "\t\t\t\tCompileAs=\"0\"/>\r\n";

static char *msvc70_cbt_normal = "\t\t\t<Tool\r\n"
                                 "\t\t\t\tName=\"VCCustomBuildTool\"/>\r\n";

static char *msvc70_cbt_custom = "\t\t\t<Tool\r\n"
                                 "\t\t\t\tName=\"VCCustomBuildTool\"\r\n"
                                 "\t\t\t\tCommandLine=\"%s\r\n\"\r\n"
                                 "\t\t\t\tOutputs=\"%s\"/>\r\n";

static char *msvc70_lib_tool = "\t\t\t<Tool\r\n"
                               "\t\t\t\tName=\"VCLibrarianTool\"\r\n"
                               "\t\t\t\tOutputFile=\".\\libs\\%s\\%s\\%s.lib\"\r\n"
                               "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>\r\n";

static char *msvc70_linker_tool_part1 = "\t\t\t<Tool\r\n"
                                        "\t\t\t\tName=\"VCLinkerTool\"\r\n"
                                        "\t\t\t\tAdditionalOptions=\"/MACHINE:I386\"\r\n"
                                        "\t\t\t\tAdditionalDependencies=\"%s\"\r\n"
                                        "\t\t\t\tOutputFile=\".\\..\\..\\..\\..\\data\\%s.exe\"\r\n"
                                        "\t\t\t\tLinkIncremental=\"%d\"\r\n"
                                        "\t\t\t\tSuppressStartupBanner=\"TRUE\"\r\n";

static char *msvc70_gdi = "\t\t\t\tGenerateDebugInformation=\"TRUE\"\r\n";

static char *msvc70_linker_tool_part2 = "\t\t\t\tProgramDatabaseFile=\".\\..\\..\\..\\..\\data\\%s.pdb\"\r\n"
                                        "\t\t\t\tSubSystem=\"%d\"/>\r\n";

static char *msvc70_vcmidl_tool_console = "\t\t\t<Tool\r\n"
                                          "\t\t\t\tName=\"VCMIDLTool\"\r\n"
                                          "\t\t\t\tTypeLibraryName=\".\\..\\..\\..\\..\\data\\%s.tlb\"/>\r\n";

static char *msvc70_vcmidl_tool_gui = "\t\t\t<Tool\r\n"
                                      "\t\t\t\tName=\"VCMIDLTool\"\r\n"
                                      "\t\t\t\tPreprocessorDefinitions=\"%s\"\r\n"
                                      "\t\t\t\tMkTypLibCompatible=\"TRUE\"\r\n"
                                      "\t\t\t\tSuppressStartupBanner=\"TRUE\"\r\n"
                                      "\t\t\t\tTargetEnvironment=\"1\"\r\n"
                                      "\t\t\t\tTypeLibraryName=\".\\..\\..\\..\\..\\data\\%s.tlb\"/>\r\n";


static char *msvc70_vcmidl_tool_lib = "\t\t\t<Tool\r\n"
                                      "\t\t\t\tName=\"VCMIDLTool\"/>\r\n";

static char *msvc70_link_event_tool = "\t\t\t<Tool\r\n"
                                      "\t\t\t\tName=\"VCPostBuildEventTool\"/>\r\n"
                                      "\t\t\t<Tool\r\n"
                                      "\t\t\t\tName=\"VCPreBuildEventTool\"/>\r\n"
                                      "\t\t\t<Tool\r\n"
                                      "\t\t\t\tName=\"VCPreLinkEventTool\"/>\r\n";

static char *msvc70_resource_tool_lib = "\t\t\t<Tool\r\n"
                                        "\t\t\t\tName=\"VCResourceCompilerTool\"\r\n"
                                        "\t\t\t\tCulture=\"1033\"/>\r\n";

static char *msvc70_resource_tool_console = "\t\t\t<Tool\r\n"
                                            "\t\t\t\tName=\"VCResourceCompilerTool\"\r\n"
                                            "\t\t\t\tPreprocessorDefinitions=\"%s\"\r\n"
                                            "\t\t\t\tCulture=\"1033\"/>\r\n";

static char *msvc70_resource_tool_gui = "\t\t\t<Tool\r\n"
                                        "\t\t\t\tName=\"VCResourceCompilerTool\"\r\n"
                                        "\t\t\t\tPreprocessorDefinitions=\"WIN32,_WINDOWS,IDE_COMPILE,DONT_USE_UNISTD_H,%s\"\r\n"
                                        "\t\t\t\tCulture=\"1033\"\r\n"
                                        "\t\t\t\tAdditionalIncludeDirectories=\"..\\msvc;..\\;..\\..\\..\\\"/>\r\n";

static char *msvc70_wspgt = "\t\t\t<Tool\r\n"
                            "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>\r\n";

static char *msvc70_wdt = "\t\t\t<Tool\r\n"
                          "\t\t\t\tName=\"VCWebDeploymentTool\"/>\r\n";

static char *msvc70_config_end = "\t\t</Configuration>\r\n";

static char *msvc70_configs_files = "\t</Configurations>\r\n"
                                    "\t<Files>\r\n";

static char *msvc70_file = "\t\t<File\r\n"
                           "\t\t\tRelativePath=\"..\\..\\..\\%s\">\r\n"
                           "\t\t</File>\r\n";

static char *msvc70_file_start = "\t\t<File\r\n"
                                 "\t\t\tRelativePath=\"%s\">\r\n";

static char *msvc70_custom_fc = "\t\t\t<FileConfiguration\r\n"
                                "\t\t\t\tName=\"%s|Win32\">\r\n"
                                "\t\t\t\t<Tool\r\n"
                                "\t\t\t\t\tName=\"VCCustomBuildTool\"\r\n"
                                "\t\t\t\t\tDescription=\"%s\"\r\n"
                                "\t\t\t\t\tCommandLine=\"%s\r\n\"\r\n"
                                "\t\t\t\t\tAdditionalDependencies=\"";

static char *msvc70_custom_output = "\"\r\n"
                                    "\t\t\t\t\tOutputs=\"%s\"/>\r\n"
                                    "\t\t\t</FileConfiguration>\r\n";

static char *msvc70_end_file = "\t\t</File>\r\n";

static char *msvc70_cpu_file = "\t\t<File\r\n"
                               "\t\t\tRelativePath=\"..\\..\\..\\%s\">\r\n"
                               "\t\t\t<FileConfiguration\r\n"
                               "\t\t\t\tName=\"DX Release|Win32\">\r\n"
                               "\t\t\t\t<Tool\r\n"
                               "\t\t\t\t\tName=\"VCCLCompilerTool\"\r\n"
                               "\t\t\t\t\tOptimization=\"4\"\r\n"
                               "\t\t\t\t\tEnableIntrinsicFunctions=\"TRUE\"\r\n"
                               "\t\t\t\t\tImproveFloatingPointConsistency=\"TRUE\"\r\n"
                               "\t\t\t\t\tFavorSizeOrSpeed=\"0\"\r\n"
                               "\t\t\t\t\tOmitFramePointers=\"TRUE\"/>\r\n"
                               "\t\t\t</FileConfiguration>\r\n"
                               "\t\t\t<FileConfiguration\r\n"
                               "\t\t\t\tName=\"Release|Win32\">\r\n"
                               "\t\t\t\t<Tool\r\n"
                               "\t\t\t\t\tName=\"VCCLCompilerTool\"\r\n"
                               "\t\t\t\t\tOptimization=\"4\"\r\n"
                               "\t\t\t\t\tEnableIntrinsicFunctions=\"TRUE\"\r\n"
                               "\t\t\t\t\tImproveFloatingPointConsistency=\"TRUE\"\r\n"
                               "\t\t\t\t\tFavorSizeOrSpeed=\"0\"\r\n"
                               "\t\t\t\t\tOmitFramePointers=\"TRUE\"/>\r\n"
                               "\t\t\t</FileConfiguration>\r\n"
                               "\t\t</File>\r\n";

static char *msvc70_cc_file_start = "\t\t<File\r\n"
                                    "\t\t\tRelativePath=\"..\\..\\..\\%s\\%s.cc\">\r\n";

static char *msvc70_cc_fc = "\t\t\t<FileConfiguration\r\n"
                            "\t\t\t\tName=\"%s|Win32\">\r\n"
                            "\t\t\t\t<Tool\r\n"
                            "\t\t\t\t\tName=\"VCCustomBuildTool\"\r\n";

static char *msvc70_cc_command = "\t\t\t\t\tCommandLine=\"cl /nologo %s /I &quot;..\\msvc&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; %s /D PACKAGE=\\&quot;%s\\&quot; /D VERSION=\\&quot;0.7\\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\\%s\\%s\\%s.pch&quot; /Fo&quot;libs\\%s\\%s/&quot; /Fd&quot;libs\\%s\\%s/&quot; /FD /TP /c &quot;$(InputPath)&quot;\r\n\"\r\n"
                                 "\t\t\t\t\tOutputs=\"libs\\%s\\%s\\$(InputName).obj\"/>\r\n"
                                 "\t\t\t</FileConfiguration>\r\n";

static char *msvc70_cc_command_sid = "\t\t\t\t\tCommandLine=\"cl /nologo %s /I &quot;..\\msvc&quot; /I &quot;..\\..\\../&quot; /I &quot;../&quot; /I &quot;..\\..\\..\\sid&quot; /D &quot;WIN32&quot; /D &quot;_WINDOWS&quot; /D &quot;IDE_COMPILE&quot; /D &quot;DONT_USE_UNISTD_H&quot; %s /D PACKAGE=\\&quot;%s\\&quot; /D VERSION=\\&quot;0.7\\&quot; /D SIZEOF_INT=4 /Fp&quot;libs\\%s\\%s\\%s.pch&quot; /Fo&quot;libs\\%s\\%s/&quot; /Fd&quot;libs\\%s\\%s/&quot; /FD /TP /c &quot;$(InputPath)&quot;\r\n\"\r\n"
                                     "\t\t\t\t\tOutputs=\"libs\\%s\\%s\\$(InputName).obj\"/>\r\n"
                                     "\t\t\t</FileConfiguration>\r\n";

static char *msvc70_res_file = "\t\t<File\r\n"
                               "\t\t\tRelativePath=\"..\\%s\">\r\n";

static char *msvc70_res_fc = "\t\t\t<FileConfiguration\r\n"
                             "\t\t\t\tName=\"%s|Win32\">\r\n"
                             "\t\t\t\t<Tool\r\n"
                             "\t\t\t\t\tName=\"VCCustomBuildTool\"\r\n"
                             "\t\t\t\t\tCommandLine=\"copy /b ";

static char *msvc70_res_deps = "\t\t\t\t\tAdditionalDependencies=\"..\\..\\..\\debug.h;";

static char *msvc70_res_output = "\"\r\n"
                                 "\t\t\t\t\tOutputs=\"%s\"/>\r\n"
                                 "\t\t\t</FileConfiguration>\r\n";

static char *msvc70_res_end = "\t\t</File>\r\n"
                              "\t\t<File\r\n"
                              "\t\t\tRelativePath=\".\\%s\">\r\n"
                              "\t\t</File>\r\n"
                              "\t\t<File\r\n"
                              "\t\t\tRelativePath=\"..\\vice.manifest\">\r\n"
                              "\t\t</File>\r\n";

static char *msvc70_end_project = "\t</Files>\r\n"
                                  "\t<Globals>\r\n"
                                  "\t</Globals>\r\n"
                                  "</VisualStudioProject>\r\n";

static char *msvc70_main_projects = "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%s\", \"%s.vcproj\", \"{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}\"\r\n"
                                    "EndProject\r\n";

static char *msvc70_main_project_global = "Global\r\n"
                                          "\tGlobalSection(SolutionConfiguration) = preSolution\r\n"
                                          "\t\tConfigName.0 = Debug\r\n"
                                          "\t\tConfigName.1 = DX Debug\r\n"
                                          "\t\tConfigName.2 = DX Release\r\n"
                                          "\t\tConfigName.3 = Release\r\n"
                                          "\tEndGlobalSection\r\n"
                                          "\tGlobalSection(ProjectDependencies) = postSolution\r\n";

static char *msvc70_main_project_deps = "\t\t{8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}.%d = {8BC9CEB8-8B4A-11D0-8D12-00A0C91BC9%02X}\r\n";

static char *msvc70_main_project_conf = "\tEndGlobalSection\r\n"
                                        "\tGlobalSection(ProjectConfiguration) = postSolution\r\n";

static void generate_msvc70_sln(void)
{
    int i, j;

    for (i = 0; project_info[i].name; i++) {
        fprintf(mainfile, msvc70_main_projects, project_info[i].name, project_info[i].name, i);
    }
    fprintf(mainfile, msvc70_main_project_global);
    for (i = 0; project_info[i].name; i++) {
        if (project_info[i].dep[0]) {
            for (j = 0; project_info[i].dep[j]; j++) {
                fprintf(mainfile, msvc70_main_project_deps, i, j, pi_get_index_of_name(project_info[i].dep[j]));
            }
        }
    }
    fprintf(mainfile, msvc70_main_project_conf);
    for (i = 0; project_info[i].name; i++) {
        fprintf(mainfile, msvc7x_main_project_confs, i, i, i, i, i, i, i, i);
    }
    fprintf(mainfile, msvc7x_main_project_end);
}

static int open_msvc70_main_project(void)
{
    pi_init();

    mainfile = fopen("../vs70/vice.sln", "wb");

    if (!mainfile) {
        printf("Cannot open 'vice.sln' for output\n");
        return 1;
    }
    fprintf(mainfile, "Microsoft Visual Studio Solution File, Format Version 7.00\r\n");
    return 0;
}

static void close_msvc70_main_project(void)
{
    generate_msvc70_sln();
    pi_exit();
    fclose(mainfile);
}

static int output_msvc7_file(char *fname, int filelist, int version)
{
    char *filename;
    int retval = 0;
    FILE *outfile = NULL;
    int i, j;

    if (filelist) {
        i = pi_insert_name(cp_name);
        if (cp_dep_names[0]) {
            pi_insert_deps(cp_dep_names, i);
        }
        filename = malloc(strlen(fname) + sizeof("../vs70/.vcproj"));
        if (version == 70) {
            sprintf(filename, "../vs70/%s.vcproj", fname);
        } else {
            sprintf(filename, "../vs71/%s.vcproj", fname);
        }
    } else {
        filename = malloc(strlen(fname) + sizeof(".vcproj"));
        sprintf(filename, "%s.vcproj", fname);
    }

    outfile = fopen(filename, "wb");
    if (!outfile) {
        printf("Cannot open %s for output\n", filename);
        retval = 1;
    }

    if (!retval) {
        if (version == 70) {
            fprintf(outfile, msvc70_xml_header);
        } else {
            fprintf(outfile, msvc71_xml_header);
        }
        fprintf(outfile, msvc70_project_start, (version == 70) ? "7.00" : "7.10", cp_name);
        fprintf(outfile, msvc70_configurations);
        for (i = 0; i < 4; i++) {
            fprintf(outfile, msvc70_config_part1, msvc_type_name[i]);
            if (cp_type == CP_TYPE_LIBRARY) {
                fprintf(outfile, msvc70_config_part2_lib, cp_name, msvc_type[i]);
            } else {
                fprintf(outfile, msvc70_config_part2_app);
            }
            fprintf(outfile, msvc70_config_part3, cp_name, msvc_type[i], (cp_type == CP_TYPE_LIBRARY) ? 4 : 1);
            if (cp_type == CP_TYPE_CONSOLE) {
                fprintf(outfile, msvc70_charset);
            }
            fprintf(outfile, msvc70_compiler_tool_part1, msvc70_compiler_tool_type[i & 1]);
            if (cp_cc_source_path && !cp_source_names[0]) {
                fprintf(outfile, msvc70_aid_cc);
            } else {
                fprintf(outfile, msvc70_aid);
            }
            if (cp_include_dirs[0]) {
                for (j = 0; cp_include_dirs[j]; j++) {
                    fprintf(outfile, ",..\\..\\..\\%s", cp_include_dirs[j]);
                }
            }
            fprintf(outfile, "\"\r\n");
            fprintf(outfile, msvc70_compiler_tool_part2);
            fprintf(outfile, msvc_predefs[i]);
            if (cp_cc_source_path) {
                fprintf(outfile, msvc70_cc_predefs, cp_name);
            }
            fprintf(outfile, msvc70_predefs_end);
            if (i & 1) {
                fprintf(outfile, msvc70_string_pooling);
            }
            fprintf(outfile, msvc70_compiler_tool_part3, !(i & 1));
            if (i & 1) {
                fprintf(outfile, msvc70_fll);
            }
            fprintf(outfile, msvc70_compiler_tool_part4, cp_name, msvc_type[i], cp_name, cp_name, msvc_type[i], cp_name, msvc_type[i], cp_name, msvc_type[i]);
            if (!(i & 1)) {
                fprintf(outfile, msvc70_dif);
            }
            fprintf(outfile, msvc70_compiler_tool_part5);
            if (cp_post_custom_command) {
                fprintf(outfile, msvc70_cbt_custom, cp_post_custom_command, cp_post_custom_output);
            } else {
                fprintf(outfile, msvc70_cbt_normal);
            }
            if (cp_type == CP_TYPE_LIBRARY) {
                fprintf(outfile, msvc70_lib_tool, cp_name, msvc_type[i], cp_name);
            } else {
                fprintf(outfile, msvc70_linker_tool_part1, (cp_type == CP_TYPE_CONSOLE) ? msvc70_console_libs : msvc70_gui_libs[i >> 1], cp_name, (!(i & 1)) + 1);
                if (!(i & 1)) {
                    fprintf(outfile, msvc70_gdi);
                }
                fprintf(outfile, msvc70_linker_tool_part2, cp_name, (cp_type == CP_TYPE_GUI) ? 2 : 1);
            }
            switch (cp_type) {
                default:
                case CP_TYPE_CONSOLE:
                    fprintf(outfile, msvc70_vcmidl_tool_console, cp_name);
                    break;
                case CP_TYPE_GUI:
                    fprintf(outfile, msvc70_vcmidl_tool_gui, msvc_predefs[i & 1], cp_name);
                    break;
                case CP_TYPE_LIBRARY:
                    fprintf(outfile, msvc70_vcmidl_tool_lib);
                    break;
            }
            fprintf(outfile, msvc70_link_event_tool);
            switch (cp_type) {
                default:
                case CP_TYPE_LIBRARY:
                    fprintf(outfile, msvc70_resource_tool_lib);
                    break;
                case CP_TYPE_CONSOLE:
                    fprintf(outfile, msvc70_resource_tool_console, msvc_predefs[i & 1]);
                    break;
                case CP_TYPE_GUI:
                    fprintf(outfile, msvc70_resource_tool_gui, msvc_predefs[i]);
                    break;
            }
            fprintf(outfile, msvc70_wspgt);
            if (version == 71) {
                fprintf(outfile, msvc71_xmldgt);
            }
            if (cp_type != CP_TYPE_LIBRARY) {
                fprintf(outfile, msvc70_wdt);
            }
            if (version == 71) {
                fprintf(outfile, msvc71_wrapper_tools);
            }
            fprintf(outfile, msvc70_config_end);
        }
        if (version == 70) {
            fprintf(outfile, msvc70_configs_files);
        } else {
            fprintf(outfile, msvc71_configs_files);
        }
        if (cp_source_names[0]) {
            for (i = 0; cp_source_names[i]; i++) {
                fprintf(outfile, msvc70_file, cp_source_names[i]);
            }
        }
        if (cp_custom_message) {
            fprintf(outfile, msvc70_file_start, cp_custom_source);
            for (i = 0; i < 4; i++) {
                fprintf(outfile, msvc70_custom_fc, msvc_type_name[i], cp_custom_message, cp_custom_command);
                for (j = 0; cp_custom_deps[j]; j++) {
                    fprintf(outfile, "%s;", cp_custom_deps[j]);
                }
                fprintf(outfile, msvc70_custom_output, cp_custom_output);
            }
            fprintf(outfile, msvc70_end_file);
        }

        if (cp_cpusource_names[0]) {
            for (j = 0; cp_cpusource_names[j]; j++) {
                fprintf(outfile, msvc70_cpu_file, cp_cpusource_names[j]);
            }
        }

        if (cp_cc_source_path) {
            for (j = 0; cp_cc_source_names[j]; j++) {
                fprintf(outfile, msvc70_cc_file_start, cp_cc_source_path, cp_cc_source_names[j]);
                for (i = 0; i < 4; i++) {
                    fprintf(outfile, msvc70_cc_fc, msvc_type_name[i]);
                    if (cp_source_names[0]) {
                        fprintf(outfile, msvc70_cc_command_sid, msvc70_flags[i & 1], msvc70_cc_extra[i], cp_name, cp_name, msvc_type[i], cp_name, cp_name, msvc_type[i], cp_name, msvc_type[i], cp_name, msvc_type[i]);
                    } else {
                        fprintf(outfile, msvc70_cc_command, msvc70_flags[i & 1], msvc70_cc_extra[i], cp_name, cp_name, msvc_type[i], cp_name, cp_name, msvc_type[i], cp_name, msvc_type[i], cp_name, msvc_type[i]);
                    }
                }
                fprintf(outfile, msvc70_end_file);
            }
        }

        if (cp_res_source_name) {
            fprintf(outfile, msvc70_res_file, cp_res_source_name);
            for (i = 0; i < 4; i++) {
                fprintf(outfile, msvc70_res_fc, msvc_type_name[i]);
                for (j = 0; cp_res_deps[j]; j++) {
                    fprintf(outfile, "..\\%s ", cp_res_deps[j]);
                    if (cp_res_deps[j + 1]) {
                        fprintf(outfile, "+ ");
                    }
                }
                fprintf(outfile, "%s /b\r\n\"\r\n", cp_res_output_name);
                fprintf(outfile, msvc70_res_deps);
                for (j = 0; cp_res_deps[j]; j++) {
                    fprintf(outfile, "..\\%s;", cp_res_deps[j]);
                }
                fprintf(outfile, msvc70_res_output, cp_res_output_name);
            }
            fprintf(outfile, msvc70_res_end, cp_res_output_name);
        }
        fprintf(outfile, msvc70_end_project);
    }

    if (outfile) {
        fclose(outfile);
    }

    if (filename) {
        free(filename);
    }

    if (cp_libs) {
        free(cp_libs);
        cp_libs = NULL;
    }

    if (read_buffer) {
        free(read_buffer);
        read_buffer = NULL;
        read_buffer_line = 0;
        read_buffer_pos = 0;
        read_buffer_len = 0;
    }

    return retval;
}

/* ---------------------------------------------------------------------- */

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
                                   "SOURCE=\"%s\"\r\n"
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

static char *msvc6_res_source_start = "# Begin Source File\r\n"
                                      "\r\n"
                                      "SOURCE=\"..\\%s\"\r\n"
                                      "\r\n";

static char *msvc6_res_source_part1 = "# PROP Ignore_Default_Tool 1\r\n"
                                      "USERDEP__RESC6=\"..\\..\\..\\debug.h\"";

static char *msvc6_res_source_part2 = "\r\n"
                                      "# Begin Custom Build\r\n"
                                      "InputPath=\"..\\%s\"\r\n"
                                      "\r\n"
                                      "\"%s\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\r\n"
                                      "\tcopy /b";

static char *msvc6_res_source_part3 = "!ENDIF\r\n"
                                      "\r\n"
                                      "# End Source File\r\n"
                                      "# Begin Source File\r\n"
                                      "\r\n"
                                      "SOURCE=\".\\%s\"\r\n"
                                      "# End Source File\r\n"
                                      "# Begin Source File\r\n"
                                      "\r\n"
                                      "SOURCE=\"..\\vice.manifest\"\r\n"
                                      "# End Source File\r\n";

static int open_msvc6_main_project(void)
{
    mainfile = fopen("../vs6/vice.dsw", "wb");

    if (!mainfile) {
        printf("Cannot open 'vice.dsw' for output\n");
        return 1;
    }
    fprintf(mainfile, "Microsoft Developer Studio Workspace File, Format Version 6.00\r\n");
    fprintf(mainfile, "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\r\n\r\n");
    fprintf(mainfile, "###############################################################################\r\n\r\n");
    return 0;
}

static void close_msvc6_main_project(void)
{
    fprintf(mainfile, "Global:\r\n\r\n");
    fprintf(mainfile, "Package=<5>\r\n{{{\r\n}}}\r\n\r\n");
    fprintf(mainfile, "Package=<3>\r\n{{{\r\n}}}\r\n\r\n");
    fprintf(mainfile, "###############################################################################\r\n\r\n");
    fclose(mainfile);
}

static int output_msvc6_file(char *fname, int filelist)
{
    char *filename;
    int retval = 0;
    FILE *outfile = NULL;
    char *real_type = NULL;
    char *real_id = NULL;
    char *mtl = (cp_type == CP_TYPE_GUI) ? "MTL=midl.exe\r\n" : "";
    int i, j;

    if (filelist) {
        filename = malloc(strlen(fname) + sizeof("../vs6/.dsp"));
        sprintf(filename, "../vs6/%s.dsp", fname);
    } else {
        filename = malloc(strlen(fname) + sizeof(".dsp"));
        sprintf(filename, "%s.dsp", fname);
    }

    outfile = fopen(filename, "wb");
    if (!outfile) {
        printf("Cannot open %s for output\n", filename);
        retval = 1;
    }

    if (!retval && filelist) {
        fprintf(mainfile, "Project: \"%s\"=\".\\%s.dsp\" - Package Owner=<4>\r\n\r\n", cp_name, cp_name);
        fprintf(mainfile, "Package=<5>\r\n{{{\r\n}}}\r\n\r\nPackage=<4>\r\n{{{\r\n");
        if (cp_dep_names[0]) {
            for (i = 0; cp_dep_names[i]; i++) {
                fprintf(mainfile, "    Begin Project Dependency\r\n");
                fprintf(mainfile, "    Project_Dep_Name %s\r\n", cp_dep_names[i]);
                fprintf(mainfile, "    End Project Dependency\r\n");
            }
        }
        fprintf(mainfile, "}}}\r\n\r\n###############################################################################\r\n\r\n");
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
        if (cp_type == CP_TYPE_GUI) {
            if (cp_res_source_name) {
                fprintf(outfile, msvc6_res_source_start, cp_res_source_name);
                for (i = 0; i < 4; i++) {
                    fprintf(outfile, msvc6_begin_ifs[i], cp_name);
                    fprintf(outfile, msvc6_res_source_part1);
                    for (j = 0; cp_res_deps[j]; j++) {
                        fprintf(outfile, "\t\"..\\%s\"", cp_res_deps[j]);
                    }
                    fprintf(outfile, msvc6_res_source_part2, cp_res_source_name, cp_res_output_name);
                    for (j = 0; cp_res_deps[j]; j++) {
                        fprintf(outfile, " ..\\%s", cp_res_deps[j]);
                        if (cp_res_deps[j + 1]) {
                            fprintf(outfile, " +");
                        }
                    }
                    fprintf(outfile, " %s /b\r\n\r\n# End Custom Build\r\n\r\n", cp_res_output_name);
                }
                fprintf(outfile, msvc6_res_source_part3, cp_res_output_name);
            }
        }
        fprintf(outfile, msvc6_end_target);
    }

    if (outfile) {
        fclose(outfile);
    }

    if (filename) {
        free(filename);
    }

    if (cp_libs) {
        free(cp_libs);
        cp_libs = NULL;
    }

    if (read_buffer) {
        free(read_buffer);
        read_buffer = NULL;
        read_buffer_line = 0;
        read_buffer_pos = 0;
        read_buffer_len = 0;
    }

    return retval;
}

/* ---------------------------------------------------------------------- */

static char *make_cp_libs(void)
{
    int i, j;
    char *retval = NULL;
    int length = 0;
    int k = 0;

    for (i = 0; cp_libs_elements[i]; i++) {
        length += (strlen(cp_libs_elements[i]) + 1);
    }

    retval = malloc(length);

    if (retval) {
        for (i = 0; cp_libs_elements[i]; i++) {
            for (j = 0; j < strlen(cp_libs_elements[i]); j++) {
                retval[k++] = cp_libs_elements[i][j];
            }
            if (cp_libs_elements[i + 1]) {
                retval[k++] = ' ';
            } else {
                retval[k] = 0;
            }
        }
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

#if MKMSVC_DEBUG
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
#if MKMSVC_DEBUG
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

#if MKMSVC_DEBUG
    for (i = 0; names[i]; i++) {
        printf("NAME %d: %s\n", i, names[i]);
    }
#endif

    return 0;
}

static void free_buffers(void)
{
    if (names_buffer) {
        free(names_buffer);
    }
    if (read_buffer) {
        free(read_buffer);
    }
    if (cp_libs) {
        free(cp_libs);
    }
}

/* ---------------------------------------------------------------------- */

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
    cp_res_source_name = NULL;
    cp_res_deps[0] = NULL;
    cp_res_output_name = NULL;
    cp_cpusource_names[0] = NULL;

    int is_main_project_template = 0;

    line = get_next_line_from_buffer();
    while (line) {
        parsed = 0;
        if (!strlen(line)) {
#if MKMSVC_DEBUG
            printf("Line %d is empty\n", read_buffer_line);
#endif
            parsed = 1;
        }
        if (!parsed && line[0] == '#') {
#if MKMSVC_DEBUG
            printf("Line %d is a comment line\n", read_buffer_line);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_NAME_STRING, CP_NAME_SIZE)) {
            cp_name = line + CP_NAME_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project name line: %s\n", read_buffer_line, cp_name);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_TYPE_STRING, CP_TYPE_SIZE)) {
#if MKMSVC_DEBUG
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
        if (!parsed && !strncmp(line, CP_PROJECT_NAMES_STRING, CP_PROJECT_NAMES_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project name: %s\n", read_buffer_line, line + CP_PROJECT_NAMES_SIZE);
#endif
            if (fill_line_names(project_names, MAX_NAMES)) {
                printf("Error parsing project names in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            is_main_project_template = 1;
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_DEPS_STRING, CP_DEPS_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project deps line: %s\n", read_buffer_line, line + CP_DEPS_SIZE);
#endif
            if (fill_line_names(cp_dep_names, MAX_DEP_NAMES)) {
                printf("Error parsing names in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_INCLUDES_STRING, CP_INCLUDES_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project include dirs line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_include_dirs, MAX_DIRS)) {
                printf("Error parsing include dir name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_SOURCES_STRING, CP_SOURCES_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project sources line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_source_names, MAX_NAMES)) {
                printf("Error parsing source name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CPUSOURCES_STRING, CP_CPUSOURCES_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project cpu sources line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_cpusource_names, MAX_CPU_NAMES)) {
                printf("Error parsing cpu source name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_LIBS_STRING, CP_LIBS_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a project link libs line\n", read_buffer_line);
#endif
            if (fill_line_names(cp_libs_elements, MAX_LIBS)) {
                printf("Error parsing link library name in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
            cp_libs = make_cp_libs();
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_MSG_STRING, CP_CUSTOM_MSG_SIZE)) {
            cp_custom_message = line + CP_CUSTOM_MSG_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project custom message line: %s\n", read_buffer_line, cp_custom_message);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_SRC_STRING, CP_CUSTOM_SRC_SIZE)) {
            cp_custom_source = line + CP_CUSTOM_SRC_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project custom source line: %s\n", read_buffer_line, cp_custom_source);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_DEPS_STRING, CP_CUSTOM_DEPS_SIZE)) {
#if MKMSVC_DEBUG
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
#if MKMSVC_DEBUG
            printf("Line %d is a project custom output line: %s\n", read_buffer_line, cp_custom_output);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CUSTOM_COMMAND_STRING, CP_CUSTOM_COMMAND_SIZE)) {
            cp_custom_command = line + CP_CUSTOM_COMMAND_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project custom command line: %s\n", read_buffer_line, cp_custom_command);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_MSG_STRING, CP_POST_CUSTOM_MSG_SIZE)) {
            cp_post_custom_message = line + CP_POST_CUSTOM_MSG_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project post custom message line: %s\n", read_buffer_line, cp_post_custom_message);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_SRC_STRING, CP_POST_CUSTOM_SRC_SIZE)) {
            cp_post_custom_source = line + CP_POST_CUSTOM_SRC_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project post custom source line: %s\n", read_buffer_line, cp_post_custom_source);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_OUTPUT_STRING, CP_POST_CUSTOM_OUTPUT_SIZE)) {
            cp_post_custom_output = line + CP_POST_CUSTOM_OUTPUT_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project post custom output line: %s\n", read_buffer_line, cp_post_custom_output);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_POST_CUSTOM_COMMAND_STRING, CP_POST_CUSTOM_COMMAND_SIZE)) {
            cp_post_custom_command = line + CP_POST_CUSTOM_COMMAND_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project post custom command line: %s\n", read_buffer_line, cp_post_custom_command);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_CC_SOURCES_STRING, CP_CC_SOURCES_SIZE)) {
#if MKMSVC_DEBUG
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
#if MKMSVC_DEBUG
            printf("Line %d is a project cc source path line: %s\n", read_buffer_line, cp_cc_source_path);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_RES_SOURCE_STRING, CP_RES_SOURCE_SIZE)) {
            cp_res_source_name = line + CP_RES_SOURCE_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project res source name line: %s\n", read_buffer_line, cp_res_source_name);
#endif
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_RES_DEPS_STRING, CP_RES_DEPS_SIZE)) {
#if MKMSVC_DEBUG
            printf("Line %d is a res deps line: %s\n", read_buffer_line, line + CP_RES_DEPS_SIZE);
#endif
            if (split_line_names(line + CP_RES_DEPS_SIZE, cp_res_deps, MAX_DEP_NAMES)) {
                printf("Error parsing resource deps in line %d of %s\n", read_buffer_line, filename);
                return 1;
            }
            parsed = 1;
        }
        if (!parsed && !strncmp(line, CP_RES_OUTPUT_STRING, CP_RES_OUTPUT_SIZE)) {
            cp_res_output_name = line + CP_RES_OUTPUT_SIZE;
#if MKMSVC_DEBUG
            printf("Line %d is a project res output name line: %s\n", read_buffer_line, cp_res_output_name);
#endif
            parsed = 1;
        }
        if (!parsed) {
#if MKMSVC_DEBUG
            printf("Line %d is something else: %s\n", read_buffer_line, line);
#endif
            printf("Unknown command in line %d in %s\n", read_buffer_line, filename);
            return 1;
        }
        line = get_next_line_from_buffer();
    }

    if (!is_main_project_template) {

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

        /* for console and gui types, libs have to be given */
        if (cp_type != CP_TYPE_LIBRARY) {
            if (!cp_libs) {
                printf("Missing link libs in %s\n", filename);
                return 1;
            }
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

/* ---------------------------------------------------------------------- */

static void usage(void)
{
    printf("Usage: mkmsvc <basename> <generation options>\n\n");
    printf("basename is the name of a template file, without the '.tmpl' extension.\n");
    printf("generation options are:\n");
    printf("-4 = msvc4 mips makefile generation\n");
    printf("-6 = msvc6 (98) project file generation\n");
    printf("-7[0] = msvc7.0 (2002) project file generation\n");
    printf("-71 = msvc7.1 (2003) project file generation\n");
    printf("-8 = msvc8 (2005) project file generation\n");
    printf("-9 = msvc9 (2008) project file generation\n");
    printf("-10 = msvc10 (2010) project file generation\n");
    printf("-11 = msvc11 (2012) project file generation\n\n");
}

int main(int argc, char *argv[])
{
    int i;
    int msvc4 = 0;
    int msvc6 = 0;
    int msvc70 = 0;
    int msvc71 = 0;
    int msvc8 = 0;
    int msvc9 = 0;
    int msvc10 = 0;
    int msvc11 = 0;
    int error = 0;
    char *filename = NULL;

    if (argc == 1) {
        usage();
    } else {
        for (i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
                if (!filename) {
                    filename = argv[i];
                } else {
                    printf("Error: Only one basename can be given.\n\n");
                    usage();
                    exit(1);
                }
            } else {
                if (!strcmp(argv[i], "-4")) {
                    msvc4 = 1;
                }
                if (!strcmp(argv[i], "-6")) {
                    msvc6 = 1;
                }
                if (!strcmp(argv[i], "-7") || !strcmp(argv[i], "-70")) {
                    msvc70 = 1;
                }
                if (!strcmp(argv[i], "-71")) {
                    msvc71 = 1;
                }
                if (!strcmp(argv[i], "-8")) {
                    msvc8 = 1;
                }
                if (!strcmp(argv[i], "-9")) {
                    msvc9 = 1;
                }
                if (!strcmp(argv[i], "-10")) {
                    msvc10 = 1;
                }
                if (!strcmp(argv[i], "-11")) {
                    msvc11 = 1;
                }
            }
        }

        /* No filename, so use the default */
        if (!filename) {
            filename = "vice";
        }

        project_names[0] = NULL;

        if (read_template_file(filename)) {
            printf("Generation error.\n");
        } else {
            names_buffer = read_buffer;
            read_buffer = NULL;
            read_buffer_line = 0;
            read_buffer_pos = 0;
            read_buffer_len = 0;
            if (msvc4) {
                printf("Not yet implemented.\n");
            }
            if (!error && msvc6) {
                if (project_names[0]) {
                    error = open_msvc6_main_project();
                    for (i = 0; project_names[i] && !error; i++) {
                        error = read_template_file(project_names[i]);
#if MKMSVC_DEBUG
                        printf("Parse done\n");
#endif
                        if (!error) {
                            error = output_msvc6_file(project_names[i], 1);
#if MKMSVC_DEBUG
                            printf("Output done\n");
#endif
                        }
                    }
                    close_msvc6_main_project();
                } else {
                    error = output_msvc6_file(filename, 0);
                }
                if (!error) {
                    free_buffers();
                }
            }
            if (!error && msvc70) {
                if (project_names[0]) {
                    error = open_msvc70_main_project();
                    for (i = 0; project_names[i] && !error; i++) {
                        error = read_template_file(project_names[i]);
#if MKMSVC_DEBUG
                        printf("Parse done\n");
#endif
                        if (!error) {
                            error = output_msvc7_file(project_names[i], 1, 70);
#if MKMSVC_DEBUG
                            printf("Output done\n");
#endif
                        }
                    }
                    close_msvc70_main_project();
                } else {
                    error = output_msvc7_file(filename, 0, 70);
                }
                if (!error) {
                    free_buffers();
                }
            }
            if (!error && msvc71) {
                if (project_names[0]) {
                    error = open_msvc71_main_project();
                    for (i = 0; project_names[i] && !error; i++) {
                        error = read_template_file(project_names[i]);
#if MKMSVC_DEBUG
                        printf("Parse done\n");
#endif
                        if (!error) {
                            error = output_msvc7_file(project_names[i], 1, 71);
#if MKMSVC_DEBUG
                            printf("Output done\n");
#endif
                        }
                    }
                    close_msvc71_main_project();
                } else {
                    error = output_msvc7_file(filename, 0, 71);
                }
                if (!error) {
                    free_buffers();
                }
            }
            if (!error && msvc8) {
                if (project_names[0]) {
                    error = open_msvc8_main_project();
                    for (i = 0; project_names[i] && !error; i++) {
                        error = read_template_file(project_names[i]);
#if MKMSVC_DEBUG
                        printf("Parse done\n");
#endif
                        if (!error) {
                            error = output_msvc8_file(project_names[i], 1);
#if MKMSVC_DEBUG
                            printf("Output done\n");
#endif
                        }
                    }
                    close_msvc8_main_project();
                } else {
                    error = output_msvc8_file(filename, 0);
                }
                if (!error) {
                    free_buffers();
                }
            }
            if (!error && msvc9) {
                printf("Not yet implemented.\n");
            }
            if (!error && msvc10) {
                printf("Not yet implemented.\n");
            }
            if (!error && msvc11) {
                printf("Not yet implemented.\n");
            }
            if (error) {
                printf("Generation error.\n");
            } else {
                printf("Generation complete.\n");
            }
        }
    }
}
