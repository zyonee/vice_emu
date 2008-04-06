/*
 * res.h
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef _RES_H
#define _RES_H

#define IDR_MENUC64                     101
#define IDR_MENUC128                    102
#define IDR_MENUVIC                     103
#define IDR_MENUPET                     104
#define IDR_MENUCBM2                    105

#define IDI_ICON1                       106

#define IDD_ABOUT                       109
#define IDC_ABOUT_VERSION               1002
#define IDC_BROWSEDISK                  1006
#define IDC_SELECTDISK                  1007
#define IDC_SELECTNONE                  1008
#define IDC_SELECTDIR                   1009
#define IDC_BROWSEDIR                   1010
#define IDD_DISKDEVICE_DIALOG           1010
#define IDC_AUTOSTART                   1011
#define IDC_DISKIMAGE                   1012
#define IDC_DIR                         1013
#define IDC_TOGGLE_READP00              1014
#define IDC_TOGGLE_WRITEP00             1015
#define IDC_TOGGLE_HIDENONP00           1016
#define IDD_DRIVE_SETTINGS_DIALOG       1020
#define IDC_SELECT_DRIVE_TYPE_1541      1021
#define IDC_SELECT_DRIVE_TYPE_1541II    1022
#define IDC_SELECT_DRIVE_TYPE_1571      1023
#define IDC_SELECT_DRIVE_TYPE_1581      1024
#define IDC_SELECT_DRIVE_TYPE_2031      1025
#define IDC_SELECT_DRIVE_TYPE_NONE      1026
#define IDC_SELECT_DRIVE_TYPE_1001      1027
#define IDC_SELECT_DRIVE_TYPE_8050      1028
#define IDC_SELECT_DRIVE_TYPE_8250      1029
#define IDD_DRIVE_EXTEND_DIALOG         1030
#define IDC_SELECT_DRIVE_EXTEND_NEVER   1031
#define IDC_SELECT_DRIVE_EXTEND_ASK     1032
#define IDC_SELECT_DRIVE_EXTEND_ACCESS  1033
#define IDD_DRIVE_IDLE_METHOD           1040
#define IDC_SELECT_DRIVE_IDLE_NO_IDLE   1041
#define IDC_SELECT_DRIVE_IDLE_TRAP_IDLE 1042
#define IDC_SELECT_DRIVE_IDLE_SKIP_CYCLES 1043
#define IDD_DRIVE_SYNC_FACTOR           1050
#define IDC_SELECT_DRIVE_SYNC_PAL       1051
#define IDC_SELECT_DRIVE_SYNC_NTSC      1052
#define IDC_SELECT_DRIVE_SYNC_NTSCOLD   1053
#define IDC_TOGGLE_DRIVE_PARALLEL_CABLE 1060
#define IDD_SNAPSHOT_SAVE_DIALOG        1070
#define IDC_SNAPSHOT_SAVE_IMAGE         1071
#define IDC_TOGGLE_SNAPSHOT_SAVE_DISKS  1072
#define IDC_TOGGLE_SNAPSHOT_SAVE_ROMS   1073
#define IDD_VICII_PALETTE_DIALOG        1080
#define IDC_SELECT_VICII_DEFAULT        1081
#define IDC_SELECT_VICII_CUSTOM         1082
#define IDC_SELECT_VICII_C64S           1083
#define IDC_SELECT_VICII_CCS64          1084
#define IDC_SELECT_VICII_FRODO          1085
#define IDC_SELECT_VICII_GODOT          1086
#define IDC_SELECT_VICII_PC64           1087
#define IDC_VICII_CUSTOM_NAME           1090
#define IDC_VICII_CUSTOM_BROWSE         1091
#define IDD_VICII_SPRITES_DIALOG        1092
#define IDC_TOGGLE_VICII_SSC            1093
#define IDC_TOGGLE_VICII_SBC            1094
#define IDD_PET_SETTINGS_MODEL_DIALOG   1095
#define IDD_PET_SETTINGS_IO_DIALOG      1096
#define IDD_PET_SETTINGS_SUPER_DIALOG   1097
#define IDD_PET_SETTINGS_8296_DIALOG    1098
#define IDC_SELECT_PET_2001_8N          1100
#define IDC_SELECT_PET_3008             1101
#define IDC_SELECT_PET_3016             1102
#define IDC_SELECT_PET_3032             1103
#define IDC_SELECT_PET_3032B            1104
#define IDC_SELECT_PET_4016             1105
#define IDC_SELECT_PET_4032             1106
#define IDC_SELECT_PET_4032B            1107
#define IDC_SELECT_PET_8032             1108
#define IDC_SELECT_PET_8096             1109
#define IDC_SELECT_PET_8296             1110
#define IDC_SELECT_PET_SUPER            1111
#define IDC_SELECT_PET_MEM4K            1120
#define IDC_SELECT_PET_MEM8K            1121
#define IDC_SELECT_PET_MEM16K           1122
#define IDC_SELECT_PET_MEM32K           1123
#define IDC_SELECT_PET_MEM96K           1124
#define IDC_SELECT_PET_MEM128K          1125
#define IDC_SELECT_PET_IO2K             1128
#define IDC_SELECT_PET_IO256            1129
#define IDC_SELECT_PET_VIDEO_AUTO       1130
#define IDC_SELECT_PET_VIDEO_40         1131
#define IDC_SELECT_PET_VIDEO_80         1132
#define IDC_SELECT_PET_KEYB_GRAPHICS    1135
#define IDC_SELECT_PET_KEYB_BUSINESS    1136
#define IDC_TOGGLE_PET_CRTC             1140
#define IDC_TOGGLE_PET_SUPER_IO_ENABLE  1141
#define IDC_TOGGLE_PET_8296_RAM9        1142
#define IDC_TOGGLE_PET_8296_RAMA        1143
#define IDD_CBMII_SETTINGS_MODEL_DIALOG 1148
#define IDD_CBMII_SETTINGS_IO_DIALOG    1149
#define IDC_SELECT_CBMII_610            1150
#define IDC_SELECT_CBMII_620            1151
#define IDC_SELECT_CBMII_620P           1152
#define IDC_SELECT_CBMII_710            1153
#define IDC_SELECT_CBMII_720            1154
#define IDC_SELECT_CBMII_720P           1155
#define IDC_SELECT_CBMII_MEM_128        1160
#define IDC_SELECT_CBMII_MEM_256        1161
#define IDC_SELECT_CBMII_MEM_512        1162
#define IDC_SELECT_CBMII_MEM_1024       1163
#define IDC_SELECT_CBMII_HW0            1165
#define IDC_SELECT_CBMII_HW1            1166
#define IDC_SELECT_CBMII_HW2            1167
#define IDC_TOGGLE_CBMII_RAM08          1170
#define IDC_TOGGLE_CBMII_RAM1           1171
#define IDC_TOGGLE_CBMII_RAM2           1172
#define IDC_TOGGLE_CBMII_RAM4           1173
#define IDC_TOGGLE_CBMII_RAM6           1174
#define IDC_TOGGLE_CBMII_RAMC           1175
#define IDC_SELECT_CBMII_KEYB_GRAPHICS  1180
#define IDC_SELECT_CBMII_KEYB_BUSINESS  1181
#define IDC_TOGGLE_DRIVE_EXPANSION_2000 1190
#define IDC_TOGGLE_DRIVE_EXPANSION_4000 1191
#define IDC_TOGGLE_DRIVE_EXPANSION_6000 1192
#define IDC_TOGGLE_DRIVE_EXPANSION_8000 1193
#define IDC_TOGGLE_DRIVE_EXPANSION_A000 1194

#define IDD_JOY_SETTINGS_DIALOG         1200
#define IDC_JOY_DEV1                    1201
#define IDC_JOY_DEV2                    1202
#define IDC_JOY_CALIBRATE               1203
#define IDC_JOY_CONFIG_A                1204
#define IDC_JOY_CONFIG_B                1205

#define IDD_CONFIG_KEYSET_DIALOG        1210
#define IDC_KEYSET_SW                   1211
#define IDC_KEYSET_S                    1212
#define IDC_KEYSET_SE                   1213
#define IDC_KEYSET_W                    1214
#define IDC_KEYSET_E                    1215
#define IDC_KEYSET_NW                   1216
#define IDC_KEYSET_N                    1217
#define IDC_KEYSET_NE                   1218
#define IDC_KEYSET_FIRE                 1219
#define IDC_KEY_SW                      1220
#define IDC_KEY_S                       1221
#define IDC_KEY_SE                      1222
#define IDC_KEY_W                       1223
#define IDC_KEY_E                       1224
#define IDC_KEY_NW                      1225
#define IDC_KEY_N                       1226
#define IDC_KEY_NE                      1227
#define IDC_KEY_FIRE                    1228

#define IDD_CONFIG_KEY_DIALOG           1229

#define IDD_SOUND_SETTINGS_DIALOG       1240
#define IDC_SOUND_FREQ                  1241
#define IDC_SOUND_BUFFER                1242
#define IDC_SOUND_OVERSAMPLE            1243
#define IDC_SOUND_SYNCH                 1244

#define IDD_OPEN_TEMPLATE               1250
#define IDD_OPENTAPE_TEMPLATE           1251
#define IDC_PREVIEW                     1252
#define IDC_BLANK_IMAGE                 1253
#define IDC_BLANK_IMAGE_TYPE            1254
#define IDC_BLANK_IMAGE_NAME            1255
#define IDC_BLANK_IMAGE_ID              1256

#define IDD_DATASETTE_SETTINGS_DIALOG   1260
#define IDC_DATASETTE_RESET_WITH_CPU    1261
#define IDC_DATASETTE_SPEED_TUNING      1262
#define IDC_DATASETTE_ZERO_GAP_DELAY    1263

#define IDD_VIC_SETTINGS_DIALOG         1270
#define IDC_VIC_NOEXPANSION             1271
#define IDC_VIC_3KEXPANSION             1272
#define IDC_VIC_8KEXPANSION             1273
#define IDC_VIC_16KEXPANSION            1274
#define IDC_VIC_24KEXPANSION            1275
#define IDC_VIC_FULLEXPANSION           1276
#define IDC_VIC_CUSTOMEXPANSION         1277
#define IDC_VIC_MEMORY_BLOCK0           1278
#define IDC_VIC_MEMORY_BLOCK1           1279
#define IDC_VIC_MEMORY_BLOCK2           1280
#define IDC_VIC_MEMORY_BLOCK3           1281
#define IDC_VIC_MEMORY_BLOCK5           1282

#define IDD_PRINTDEVICE_DIALOG          1290
#define IDC_TOGGLE_PRINTER              1291

#define IDD_DIALOG1                     1301
#define IDD_TEXTDLG                     1301
#define IDC_TEXT                        1302
#define IDC_HEADER                      1303

#define IDD_CONSOLE_SAVE_DIALOG         1310
#define IDC_TOGGLE_CONSOLE_APPEND       1311

#define IDM_FILE_EXIT                   40001
#define IDM_EXIT                        40001
#define IDM_ABOUT                       40002
#define IDM_AUTOSTART                   40004
#define IDM_HARD_RESET                  40005
#define IDM_SOFT_RESET                  40006
#define IDM_ATTACH_8                    40007
#define IDM_ATTACH_9                    40008
#define IDM_ATTACH_10                   40009
#define IDM_ATTACH_11                   40010
#define IDM_ATTACH_TAPE                 40011
#define IDM_DETACH_TAPE                 40012
#define IDM_DATASETTE_CONTROL           40013
#define IDM_DETACH_8                    40014
#define IDM_DETACH_9                    40015
#define IDM_DETACH_10                   40016
#define IDM_DETACH_11                   40017
#define IDM_DETACH_ALL                  40018
#define IDM_TOGGLE_SOUND                40019
#define IDM_TOGGLE_DOUBLESIZE           40020
#define IDM_TOGGLE_DOUBLESCAN           40021
#define IDM_TOGGLE_DRIVE_TRUE_EMULATION 40022
#define IDM_TOGGLE_VIDEOCACHE           40023
#define IDM_TOGGLE_SIDFILTERS           40024
#define IDM_TOGGLE_SOUND_RESID          40025
#define IDM_DRIVE_SETTINGS              40030
#define IDM_CART_ATTACH_CRT             40040
#define IDM_CART_ATTACH_8KB             40041
#define IDM_CART_ATTACH_16KB            40042
#define IDM_CART_ATTACH_AR              40043
#define IDM_CART_ATTACH_AT              40044
#define IDM_CART_ATTACH_EPYX            40045
#define IDM_CART_ATTACH_IEEE488         40046
#define IDM_CART_ATTACH_SS4             40047
#define IDM_CART_ATTACH_SS5             40048
#define IDM_CART_SET_DEFAULT            40058
#define IDM_CART_DETACH                 40059
#define IDM_CART_VIC20_8KB_2000         40060
#define IDM_CART_VIC20_8KB_6000         40061
#define IDM_CART_VIC20_8KB_A000         40062
#define IDM_CART_VIC20_4KB_B000         40063
#define IDM_CART_VIC20_16KB             40064
#define IDM_CART_VIC20_16KB_4000        40065
#define IDM_SNAPSHOT_LOAD               40080
#define IDM_SNAPSHOT_SAVE               40081
#define IDM_SETTINGS_SAVE               40090
#define IDM_SETTINGS_LOAD               40091
#define IDM_SETTINGS_DEFAULT            40092
#define IDM_DEVICEMANAGER               40093
#define IDM_REFRESH_RATE_AUTO           40100
#define IDM_REFRESH_RATE_1              40101
#define IDM_REFRESH_RATE_2              40102
#define IDM_REFRESH_RATE_3              40103
#define IDM_REFRESH_RATE_4              40104
#define IDM_REFRESH_RATE_5              40105
#define IDM_REFRESH_RATE_6              40106
#define IDM_REFRESH_RATE_7              40107
#define IDM_REFRESH_RATE_8              40108
#define IDM_REFRESH_RATE_9              40109
#define IDM_REFRESH_RATE_10             40110
#define IDM_REFRESH_RATE_CUSTOM         40111
#define IDM_MAXIMUM_SPEED_200           40120
#define IDM_MAXIMUM_SPEED_100           40121
#define IDM_MAXIMUM_SPEED_50            40122
#define IDM_MAXIMUM_SPEED_20            40123
#define IDM_MAXIMUM_SPEED_10            40124
#define IDM_MAXIMUM_SPEED_NO_LIMIT      40125
#define IDM_MAXIMUM_SPEED_CUSTOM        40126
#define IDM_TOGGLE_WARP_MODE            40129
#define IDM_VICII_SETTINGS              40130
#define IDM_PET_SETTINGS                40131
#define IDM_CBM2_SETTINGS               40132
#define IDM_SYNC_FACTOR_PAL             40140
#define IDM_SYNC_FACTOR_NTSC            40141
#define IDM_SYNC_FACTOR_NTSCOLD         40142
#define IDM_JOY_SETTINGS                40143
#define IDM_SOUND_SETTINGS              40144
#define IDM_STATUS_WINDOW               40145
#define IDM_MONITOR                     40146
#define IDM_SIDTYPE_6581                40147
#define IDM_SIDTYPE_8580                40148
#define IDM_CART_FREEZE                 40149
#define IDM_SAVEQUICK                   40150
#define IDM_LOADQUICK                   40151
#define IDM_HELP                        40152
#define IDM_DATASETTE_CONTROL_START     40160
#define IDM_DATASETTE_CONTROL_STOP      40161
#define IDM_DATASETTE_CONTROL_FORWARD   40162
#define IDM_DATASETTE_CONTROL_REWIND    40163
#define IDM_DATASETTE_CONTROL_RECORD    40164
#define IDM_DATASETTE_CONTROL_RESET     40165
#define IDM_DATASETTE_RESET_COUNTER     40166
#define IDM_FLIP_ADD                    40170
#define IDM_FLIP_REMOVE                 40171
#define IDM_FLIP_NEXT                   40172
#define IDM_FLIP_PREVIOUS               40173
#define IDM_TOGGLE_REU                  40180
#define IDM_TOGGLE_CRTCDOUBLESIZE       40181
#define IDM_TOGGLE_CRTCDOUBLESCAN       40182
#define IDM_TOGGLE_CRTCVIDEOCACHE       40183
#define IDM_SWAP_JOYSTICK               40184
#define IDM_TOGGLE_EMUID                40185
#define IDM_IEEE488                     40186
#define IDM_MOUSE                       40187
#define IDM_DATASETTE_SETTINGS          40188
#define IDM_TOGGLE_VDC_64KB             40189
#define IDM_TOGGLE_VDC_DOUBLESIZE       40190
#define IDM_TOGGLE_VDC_DOUBLESCAN       40191
#define IDM_VIC_SETTINGS                40192
#define IDM_TOGGLE_VIRTUAL_DEVICES      40193
#define IDM_CONTRIBUTORS                40194
#define IDM_LICENSE                     40195
#define IDM_WARRANTY                    40196
#define IDM_CMDLINE                     40197
#endif

