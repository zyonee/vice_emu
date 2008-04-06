/*
 * dialogs.h - The dialog windows.
 *
 * Written by
 *  Thomas Bretz <tbretz@gsi.de>
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

/*----------------------------------------------------------------- */
/*                           Menubar                                */
/*----------------------------------------------------------------- */

#define FID_STATUS       0x002

#define IDM_FILEOPEN     0x101
#define IDM_FILESAVE     0x102
#define IDM_FILE         0x103
#define IDM_ATTACH       0x104
#define IDM_DETACH       0x105
#define IDM_SNAPSHOT     0x106
#define IDM_RESET        0x107
#define IDM_VIEW         0x108
#define IDM_EMULATOR     0x109
#define IDM_DRIVE        0x10a
#define IDM_JOYSTICK     0x10b
#define IDM_DATASETTE    0x10c
#define IDM_LOGWIN       0x10d
#define IDM_COLOR        0x10e

#define IDM_SETUP        0x10f
#define IDM_SOUND        0x110
#define IDM_SIDCHIP      0x111
#define IDM_OVERSAMPLING 0x112
#define IDM_SAMPLINGRATE 0x113
#define IDM_SOUNDSYNC    0x114
#define IDM_SOUNDDEV     0x115
#define IDM_VDCMEMORY    0x116
#define IDM_VIDEOSTD     0x117
#define IDM_COLLISION    0x118
#define IDM_REFRATE      0x119
#define IDM_BUFFER       0x11a
#define IDM_STRETCH      0x11b
#define IDM_MONITOR      0x11c
#define IDM_HELP         0x11d
#define IDM_CMDLINE      0x11e
#define IDM_ABOUT        0x11f

#define IDM_ATTACHTAPE   0x120
#define IDM_ATTACH8      0x121
#define IDM_ATTACH9      0x122
#define IDM_ATTACH10     0x123
#define IDM_ATTACH11     0x124

#define IDM_DETACHTAPE   0x130
#define IDM_DETACH8      0x138
#define IDM_DETACH9      0x139
#define IDM_DETACH10     0x13a
#define IDM_DETACH11     0x13b
#define IDM_DETACHALL    0x13f

#define IDM_SNAPLOAD     0x125
#define IDM_SNAPSAVE     0x126

#define IDM_CONFIG        0x127
#define IDM_READCONFIG    0x128
#define IDM_WRITECONFIG   0x129
#define IDM_DEFAULTCONFIG 0x12a
#define IDM_PRINTSCRN     0x12b
#define IDM_SOFTRESET     0x12c
#define IDM_HARDRESET     0x12d
#define IDM_EXIT          0x12f

#define IDM_VDC16K       0x140
#define IDM_VDC64K       0x141

#define IDM_FUNCROM      0x142
#define IDM_INTFUNCROM   0x143
#define IDM_EXTFUNCROM   0x144

#define IDM_PAL          0x145
#define IDM_NTSC         0x146
#define IDM_NTSCOLD      0x147

#define IDM_REU          0x148
#define IDM_REUSIZE      0x149
#define IDM_REU128       0x601
#define IDM_REU256       0x602
#define IDM_REU512       0x604
#define IDM_REU1024      0x608
#define IDM_REU2048      0x610
#define IDM_REU4096      0x620
#define IDM_REU8192      0x640
#define IDM_REU16384     0x680

#define IDM_MOUSE        0x150
#define IDM_HIDEMOUSE    0x151
#define IDM_EMUID        0x152
#define IDM_VCACHE       0x153
#define IDM_SBCOLL       0x154
#define IDM_SSCOLL       0x155
#define IDM_VDCVCACHE    0x156
#define IDM_REFRATEAUTO  0x160
#define IDM_REFRATE1     0x161
#define IDM_REFRATE2     0x162
#define IDM_REFRATE3     0x163
#define IDM_REFRATE4     0x164
#define IDM_REFRATE5     0x165
#define IDM_REFRATE6     0x166
#define IDM_REFRATE7     0x167
#define IDM_REFRATE8     0x168
#define IDM_REFRATE9     0x169
#define IDM_REFRATE10    0x16a

#define IDM_SC6581        0x170
#define IDM_SC8580        0x171
#define IDM_SOUNDON       0x172
#define IDM_SIDFILTER     0x173
#define IDM_RESID         0x174
#define IDM_RESIDMETHOD   0x175
#define IDM_RESIDFAST     0x176
#define IDM_RESIDINTERPOL 0x177
#define IDM_RESIDRESAMPLE 0x178
#define IDM_STEREO        0x179

#define IDM_RESIDBAND     0x17f
#define IDM_RESIDPASS0    0x180
#define IDM_RESIDPASS10   0x181
#define IDM_RESIDPASS20   0x182
#define IDM_RESIDPASS30   0x183
#define IDM_RESIDPASS40   0x184
#define IDM_RESIDPASS50   0x185
#define IDM_RESIDPASS60   0x186
#define IDM_RESIDPASS70   0x187
#define IDM_RESIDPASS80   0x188
#define IDM_RESIDPASS90   0x189

#define IDM_OSOFF        0x190
#define IDM_OS2X         0x191
#define IDM_OS4X         0x192
#define IDM_OS8X         0x193

#define IDM_SR8000       0x194
#define IDM_SR11025      0x195
#define IDM_SR22050      0x196
#define IDM_SR44100      0x197

#define IDM_SYNCFLEX     0x198
#define IDM_SYNCADJUST   0x199
#define IDM_SYNCEXACT    0x19a

#define IDM_BUF010       0x202
#define IDM_BUF025       0x205
#define IDM_BUF040       0x208
#define IDM_BUF055       0x20b
#define IDM_BUF070       0x20e
#define IDM_BUF085       0x211
#define IDM_BUF100       0x214

#define IDM_DEVDART      0x215
#define IDM_DEVSID       0x216
#define IDM_DEVWAV       0x217
#define IDM_DEVRAW       0x218

#define IDM_VOLUME       0x400
#define IDM_VOL100       0x40a
#define IDM_VOL90        0x409
#define IDM_VOL80        0x408
#define IDM_VOL70        0x407
#define IDM_VOL60        0x406
#define IDM_VOL50        0x405
#define IDM_VOL40        0x404
#define IDM_VOL30        0x403
#define IDM_VOL20        0x402
#define IDM_VOL10        0x401

#define IDM_STATUSBAR    0x21f
#define IDM_MENUBAR      0x220

#define IDM_STRETCH1     0x221
#define IDM_STRETCH2     0x222
#define IDM_STRETCH3     0x223

#define IDM_PAUSE        0x224

#define IDM_HLPINDEX     0x225
#define IDM_HLPGENERAL   0x226
#define IDM_HLPUSINGHLP  0x227
#define IDM_HLPKEYBOARD  0x228

#define IDM_IEEE         0x22f

#define IDM_MODEL        0x230

#define IDM_CBM510       0x231
#define IDM_CBM610       0x232
#define IDM_CBM620       0x233
#define IDM_CBM620P      0x234
#define IDM_CBM710       0x235
#define IDM_CBM720       0x236
#define IDM_CBM720P      0x237
      
#define IDM_MODELLINE    0x23f
      
#define IDM_MODEL750     0x240
#define IDM_MODEL660     0x241
#define IDM_MODEL650     0x242
      
#define IDM_RAMSIZE      0x250

#define IDM_RAM64        0x251
#define IDM_RAM128       0x252
#define IDM_RAM256       0x254
#define IDM_RAM512       0x258
#define IDM_RAM1024      0x260
      
#define IDM_RAMMAPPING   0x237
      
#define IDM_RAM08        0x238
#define IDM_RAM10        0x239
#define IDM_RAM20        0x23a
#define IDM_RAM40        0x23b
#define IDM_RAM60        0x23c
#define IDM_RAMC0        0x23d

#define IDM_CHARSET      0x25f
#define IDM_EOI          0x261
#define IDM_ROMPATCH     0x262
#define IDM_DIAGPIN      0x263
#define IDM_SUPERPET     0x264
#define IDM_MAP9         0x245
#define IDM_MAPA         0x266

#define IDM_IOMEM        0x267
#define IDM_IOMEM256     0x268
#define IDM_IOMEM2K      0x269

#define IDM_PETRAM4      0x304
#define IDM_PETRAM8      0x308
#define IDM_PETRAM16     0x310
#define IDM_PETRAM32     0x320
#define IDM_PETRAM96     0x360
#define IDM_PETRAM128    0x380

#define IDM_PETMODEL     0x270

#define IDM_PET2001      0x271
#define IDM_PET3008      0x272
#define IDM_PET3016      0x273
#define IDM_PET3032      0x274
#define IDM_PET3032B     0x275
#define IDM_PET4016      0x276
#define IDM_PET4032      0x277
#define IDM_PET4032B     0x278
#define IDM_PET8032      0x279
#define IDM_PET8096      0x27a
#define IDM_PET8296      0x27b
#define IDM_PETSUPER     0x27c

#define IDM_CRTC         0x27d

#define IDM_VIDEOSIZE    0x27f

#define IDM_VSDETECT     0x280
#define IDM_VS40         0x281
#define IDM_VS80         0x282

#define IDM_TRUEDRIVE    0x283

#define IDM_FLIPLIST     0x284

#define IDM_PLUS         0x285
#define IDM_MINUS        0x286

#define IDM_FLIPPREV8    0x480
#define IDM_FLIPNEXT8    0x481

#define IDM_FLIPPREV9    0x490
#define IDM_FLIPNEXT9    0x491

#define IDM_KERNALREV     0x290
#define IDM_KERNALREV0    0x291
#define IDM_KERNALREV3    0x292
#define IDM_KERNALREVSX   0x293
#define IDM_KERNALREV4064 0x294

#define IDM_CARTRIDGE    0x500
#define IDM_CRTGEN       0x501
#define IDM_CRTGEN8KB    0x502
#define IDM_CRTGEN16KB   0x503
#define IDM_CRTACTREPL   0x504
#define IDM_CRTATOMPOW   0x505
#define IDM_CRTEPYX      0x506
#define IDM_CRTSSSHOT    0x507
#define IDM_CRTSSSHOT5   0x508
#define IDM_CRTWEST      0x509
#define IDM_CRTIEEE      0x50a
#define IDM_CRTEXPERT    0x50b
#define IDM_CRTFREEZE    0x50c
#define IDM_CRTSAVEIMG   0x50d
#define IDM_CARTRIDGEDET 0x50e

#define IDM_PALCONTROL   0x510
#define IDM_INTERNALPAL  0x511
#define IDM_LUMINANCES   0x512

#define IDM_PRINTER      0x6ff

#define IDM_PRINTER4     0x700
#define IDM_PRT4IEC      0x701
#define IDM_PRT4DRV      0x702
#define IDM_PRT4ASCII    0x703
#define IDM_PRT4MPS803   0x704
#define IDM_PRT4OUT      0x705
#define IDM_PRT4TXT      0x706
#define IDM_PRT4GFX      0x707

#define IDM_PRINTER5     0x710
#define IDM_PRT5IEC      0x711
#define IDM_PRT5DRV      0x712
#define IDM_PRT5ASCII    0x713
#define IDM_PRT5MPS803   0x714
#define IDM_PRT5OUT      0x715
#define IDM_PRT5TXT      0x716
#define IDM_PRT5GFX      0x717

#define IDM_PRINTERUP    0x720
#define IDM_PRTUP        0x721
#define IDM_PRTUPDRV     0x722
#define IDM_PRTUPASCII   0x723
#define IDM_PRTUPMPS803  0x724
#define IDM_PRTUPOUT     0x725
#define IDM_PRTUPTXT     0x726
#define IDM_PRTUPGFX     0x727


void menu_action(HWND hwnd, USHORT item); //, MPARAM mp2);
void menu_select(HWND hwnd, USHORT item);

