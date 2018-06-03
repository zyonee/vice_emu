/*
 * translate_text.c - Translation texts to be included in translate.c
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

translate_t string_table[] = {

/* c64/c64-cmdline-options.c, scpu64/scpu64-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2,    N_("Specify index of keymap file (0=sym, 1=symDE, 2=pos)")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_DA, "Angiv indeks for tastaturindstillingsfil (0=symbolsk, 1=symbolsk tysk, 2=positionsbestemt)"},
/* de */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_DE, "Index f�r Keymap Datei festlegen (0=symbol, 1=symDE, 2=positional)"},
/* es */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_ES, "Especificar �ndice fichero mapa teclado (0=sim, 1=simAL, 2=pos)"},
/* fr */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_FR, "Sp�cifier l'index du fichier keymap (0=sym, 1=symDE, 2=pos)"},
/* hu */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_HU, "Adja meg a billenty�zet lek�pz�si f�jl t�pus�t (0=szimbolikus, 1=n�met szimbolikus, 2=poz�ci� szerinti)"},
/* it */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_IT, "Specifica l'indice del file della mappa della tastiera (0=sim, 1=simGER, 2=pos)"},
/* ko */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_NL, "Geef de index van het toetstoewijzingsbestand (0=sym, 1=symDE, 2=pos)"},
/* pl */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_PL, "Podaj indeks dla uk�adu klawiatury (0=symbol, 1=symbolDE, 2=pozycja)"},
/* ru */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_RU, "Specify index of keymap file (0=sym, 1=symDE, 2=pos)"},
/* sv */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_SV, "Ange index f�r f�r tangentbordsinst�llningsfil (0=symbolisk, 1=symbolisk tysk, 2=positionsriktig)"},
/* tr */ {IDCLS_SPECIFY_INDEX_KEYMAP_FILE_0_2_TR, "Tu� haritas� dosyas�n�n indeksini belirt (0=sembol, 1=sembol Almanca, 2=konumsal)"},
#endif

/* c64/c64-cmdline-options.c, scpu64/scpu64-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP,    N_("Specify name of symbolic German keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_DA, "Angiv fil for tysk symbolsk tastaturindstilling"},
/* de */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_DE, "Name von symbolischer Keymap Datei w�hlen"},
/* es */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_ES, "Especificar nombre del fichero teclado simb�lico alem�n"},
/* fr */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_FR, "Sp�cifier le nom du fichier symbolique de mappage clavier"},
/* hu */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_HU, "Adja meg a nev�t a n�met billenty�zet lek�pz�s f�jlnak"},
/* it */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_IT, "Specifica il nome del file della mappa simbolica della tastiera tedesca"},
/* ko */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_NL, "Geef de naam van het symbolische Duitse toetstoewijzingsbestand"},
/* pl */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_PL, "Okre�l nazw� pliku symbolicznego niemieckiego uk�adu klawiatury"},
/* ru */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_RU, "Specify name of symbolic German keymap file"},
/* sv */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_SV, "Ange fil f�r tysk symbolisk tangentbordsemulering"},
/* tr */ {IDCLS_SPECIFY_NAME_SYM_DE_KEYMAP_TR, "Sembolik Almanca tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_TO_USE_VIC_II,    N_("Specify to use VIC-II")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_TO_USE_VIC_II_DA, "Brug VIC-II"},
/* de */ {IDCLS_SPECIFY_TO_USE_VIC_II_DE, "VIC-II Unterst�tzung aktivieren"},
/* es */ {IDCLS_SPECIFY_TO_USE_VIC_II_ES, "Especificar usar CBM-II"},
/* fr */ {IDCLS_SPECIFY_TO_USE_VIC_II_FR, "Sp�cifier l'utilisation de VIC-II"},
/* hu */ {IDCLS_SPECIFY_TO_USE_VIC_II_HU, "VIC-II haszn�lata"},
/* it */ {IDCLS_SPECIFY_TO_USE_VIC_II_IT, "Specifica di utilizzare il VIC-II"},
/* ko */ {IDCLS_SPECIFY_TO_USE_VIC_II_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_TO_USE_VIC_II_NL, "Gebruik de VIC-II"},
/* pl */ {IDCLS_SPECIFY_TO_USE_VIC_II_PL, "Okre�l u�ycie VIC-II"},
/* ru */ {IDCLS_SPECIFY_TO_USE_VIC_II_RU, "Specify to use VIC-II"},
/* sv */ {IDCLS_SPECIFY_TO_USE_VIC_II_SV, "Ange f�r att anv�nda VIC-II"},
/* tr */ {IDCLS_SPECIFY_TO_USE_VIC_II_TR, "VIC-II kullan�m� i�in se�in"},
#endif

/* cbm2/cbm2-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_TO_USE_CRTC,    N_("Specify to use CRTC")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_TO_USE_CRTC_DA, "Brug CRTC"},
/* de */ {IDCLS_SPECIFY_TO_USE_CRTC_DE, "CRTC Unterst�tzung aktivieren"},
/* es */ {IDCLS_SPECIFY_TO_USE_CRTC_ES, "Especificar usar CRT"},
/* fr */ {IDCLS_SPECIFY_TO_USE_CRTC_FR, "Sp�cifier l'utilisation de CRTC"},
/* hu */ {IDCLS_SPECIFY_TO_USE_CRTC_HU, "CRTC haszn�lata"},
/* it */ {IDCLS_SPECIFY_TO_USE_CRTC_IT, "Specifica di utilizzare il CRTC"},
/* ko */ {IDCLS_SPECIFY_TO_USE_CRTC_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_TO_USE_CRTC_NL, "Gebruik de CRTC"},
/* pl */ {IDCLS_SPECIFY_TO_USE_CRTC_PL, "Okre�l u�ycie CRTC"},
/* ru */ {IDCLS_SPECIFY_TO_USE_CRTC_RU, "Specify to use CRTC"},
/* sv */ {IDCLS_SPECIFY_TO_USE_CRTC_SV, "Ange f�r att anv�nda CRTC"},
/* tr */ {IDCLS_SPECIFY_TO_USE_CRTC_TR, "CRTC kullan�m� i�in se�in"},
#endif

/* cbm2/cbm2-cmdline-options.c */
/* en */ {IDCLS_P_LINENUMBER,    N_("<linenumber>")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_P_LINENUMBER_DA, "<linjenummer>"},
/* de */ {IDCLS_P_LINENUMBER_DE, "<Zeilennummer>"},
/* es */ {IDCLS_P_LINENUMBER_ES, "<n�mero linea>"},
/* fr */ {IDCLS_P_LINENUMBER_FR, "<num�rodeligne>"},
/* hu */ {IDCLS_P_LINENUMBER_HU, ""},  /* fuzzy */
/* it */ {IDCLS_P_LINENUMBER_IT, "<numero di linea>"},
/* ko */ {IDCLS_P_LINENUMBER_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_P_LINENUMBER_NL, "<lijnnummer>"},
/* pl */ {IDCLS_P_LINENUMBER_PL, "<linenumber>"},
/* ru */ {IDCLS_P_LINENUMBER_RU, "<linenumber>"},
/* sv */ {IDCLS_P_LINENUMBER_SV, "<linjenummer>"},
/* tr */ {IDCLS_P_LINENUMBER_TR, "<sat�rnumaras�>"},
#endif

/* cbm2/cbm2-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE,    N_("Specify CBM-II model hardware (0=6x0, 1=7x0)")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_DA, "Angiv hardware for CBM-II-model (0=6x0, 1=7x0)"},
/* de */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_DE, "CBM-II Hardware Modell w�hlen (0=6x0, 1=7x0)"},
/* es */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_ES, "Especificar modelo hardware CBM-II (0=6x0, 1=7x0)"},
/* fr */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_FR, "Sp�cifier le mod�le hardware CBM-II (0=6x0, 1=7x0)"},
/* hu */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_HU, "Adja meg a CBM-II hardver modellt (0=6x0, 1=7x0)"},
/* it */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_IT, "Specifica il modello hardware del CBM-II (0=6x0, 1=7x0)"},
/* ko */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_NL, "Geef CBM-II hardwaremodel (0=6x0, 1=7x0)"},
/* pl */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_PL, "Okre�l model sprz�tu CBM-II (0=6x0, 1=7x0)"},
/* ru */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_RU, "Specify CBM-II model hardware (0=6x0, 1=7x0)"},
/* sv */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_SV, "Ange maskinvara f�r CBM-II-modell (0=6x0, 1=7x0)"},
/* tr */ {IDCLS_SPECIFY_CBM2_MODEL_HARDWARE_TR, "CBM-II modeli donan�m�n� belirt (0=6x0, 1=7x0)"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_KEYMAP_INDEX,    N_("Specify index of keymap file (0: UK business symbolic, 1: UK business positional, 2: Graphics symbolic, 3: Graphics positional, 4: German business symbolic, 5: German business positional)")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_KEYMAP_INDEX_DA, ""},  /* fuzzy */
/* de */ {IDCLS_SPECIFY_KEYMAP_INDEX_DE, "Index der Keymap Datei w�hlen (0: UK business symbolic, 1: UK business positional, 2: Graphics symbolic, 3: Graphics positional, 4: German business symbolic, 5: German business positional)"},
/* es */ {IDCLS_SPECIFY_KEYMAP_INDEX_ES, ""},  /* fuzzy */
/* fr */ {IDCLS_SPECIFY_KEYMAP_INDEX_FR, ""},  /* fuzzy */
/* hu */ {IDCLS_SPECIFY_KEYMAP_INDEX_HU, ""},  /* fuzzy */
/* it */ {IDCLS_SPECIFY_KEYMAP_INDEX_IT, ""},  /* fuzzy */
/* ko */ {IDCLS_SPECIFY_KEYMAP_INDEX_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_KEYMAP_INDEX_NL, "Geef index van het toetstoewijzingsbestand (0: UK business symbolisch, 1: UK business positioneel, 2: grafisch symbolisch, 3: grafisch positioneel, 4: Duits business symbolisch, 5: Duits business positioneel)"},
/* pl */ {IDCLS_SPECIFY_KEYMAP_INDEX_PL, "Podaj indeks uk�adu klawiatury (0: UK biznesowa - symbol, 1: UK biznesowa - pozycja, 2: Graficzna - symbol, 3: Graficzna - pozycja, 4: Niemiecka biznesowa - symbol, 5: Niemiecka biznesowa - pozycja)"},
/* ru */ {IDCLS_SPECIFY_KEYMAP_INDEX_RU, ""},  /* fuzzy */
/* sv */ {IDCLS_SPECIFY_KEYMAP_INDEX_SV, ""},  /* fuzzy */
/* tr */ {IDCLS_SPECIFY_KEYMAP_INDEX_TR, ""},  /* fuzzy */
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME,    N_("Specify name of graphics keyboard symbolic keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_DA, "Angiv fil for symbolsk tastaturemulering for \"graphics\"-tastatur"},
/* de */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_DE, "Keymap Datei f�r graphics keyboard symbolic w�hlen"},
/* es */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_ES, "Especificar nombre fichero gr�ficos teclado simb�lico"},
/* fr */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier symbolique"},
/* hu */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_HU, "Adja meg a grafikus �s szimbolikus billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_IT, "Specifica il nome del file della mappa simbolica della tastiera grafica"},
/* ko */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_NL, "Geef de naam van het symbolisch toetstoewijzingsbestand voor het grafische toetsenbord"},
/* pl */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_PL, "Okre�l nazw� pliku symbolicznego uk�adu klawiatury graficznej"},
/* ru */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_RU, "Specify name of graphics keyboard symbolic keymap file"},
/* sv */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_SV, "Ange fil f�r symbolisk tangentbordsemulering f�r \"graphics\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_GFX_SYM_KEYMAP_NAME_TR, "Grafik klavyesi sembolik tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME,    N_("Specify name of graphics keyboard positional keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_DA, "Angiv fil for positionsbestemt tastaturemulering for \"graphics\"-tastatur"},
/* de */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_DE, "Keymap Datei f�r graphics keyboard positional w�hlen"},
/* es */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_ES, "Especificar nombre fichero gr�ficos teclado posicional"},
/* fr */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier positionnel"},
/* hu */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_HU, "Adja meg a grafikus �s poz�ci� szerinti billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_IT, "Specifica il nome del file della mappa posizionale della tastiera grafica"},
/* ko */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_NL, "Geef de naam van het positioneel toetstoewijzingsbestand voor het grafische toetsenbord"},
/* pl */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_PL, "Okre�l nazw� pliku pozycyjnego uk�adu klawiatury graficznej"},
/* ru */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_RU, "Specify name of graphics keyboard positional keymap file"},
/* sv */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_SV, "Ange fil f�r positionsriktig tangentbordsemulering f�r \"graphics\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_GFX_POS_KEYMAP_NAME_TR, "Grafik klavyesi konumsal tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME,    N_("Specify name of UK business keyboard symbolic keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_DA, "Angiv fil for symbolsk tastaturemulering for Britisk \"business\"-tastatur"},
/* de */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_DE, "Keymap Datei f�r UK business keyboard symbolic w�hlen"},
/* es */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_ES, "Especificar nombre fichero teclado UK business simb�lico"},
/* fr */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier symbolique UK"},
/* hu */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_HU, "Adja meg az angol, hivatalos, szimbolikus billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_IT, "Specifica il nome del file della mappa simbolica della tastiera UK business"},
/* ko */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_NL, "Geef de naam van het symbolisch toetstoewijzingsbestand voor het UK zakelijk toetsenbord"},
/* pl */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_PL, "Okre�l nazw� pliku symbolicznego uk�adu angielskiej klawiatury biznesowej"},
/* ru */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_RU, "Specify name of UK business keyboard symbolic keymap file"},
/* sv */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_SV, "Ange fil f�r symbolisk tangentbordsemulering f�r brittiskt \"business\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_BUK_SYM_KEYMAP_NAME_TR, "UK business klavyesi sembolik tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME,    N_("Specify name of UK business keyboard positional keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_DA, "Angiv fil for positionsbestemt tastaturemulering for Britisk \"business\"-tastatur"},
/* de */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_DE, "Keymap Datei f�r UK business keyboard positional w�hlen"},
/* es */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_ES, "Especificar nombre fichero teclado UK business posicional"},
/* fr */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier positionnel UK"},
/* hu */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_HU, "Adja meg az angol, hivatalos, poz�ci� szerinti billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_IT, "Specifica il nome del file della mappa posizionale della tastiera UK business"},
/* ko */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_NL, "Geef de naam van het positioneel toetstoewijzingsbestand voor het UK zakelijk toetsenbord"},
/* pl */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_PL, "Okre�l nazw� pliku pozycyjnego uk�adu angielskiej klawiatury biznesowej"},
/* ru */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_RU, "Specify name of UK business keyboard positional keymap file"},
/* sv */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_SV, "Ange fil f�r positionsriktig tangentbordsemulering f�r brittiskt \"business\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_BUK_POS_KEYMAP_NAME_TR, "UK business klavyesi konumsal tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME,    N_("Specify name of German business keyboard symbolic keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_DA, "Angiv fil for symbolsk tastaturemulering for Tysk \"business\"-tastatur"},
/* de */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_DE, "Keymap Datei f�r German business keyboard symbolic w�hlen"},
/* es */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_ES, "Especificar nombre fichero teclado Alem�n business simb�lico"},
/* fr */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier symbolique allemand"},
/* hu */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_HU, "Adja meg a n�met, hivatalos, szimbolikus billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_IT, "Specifica il nome del file della mappa simbolica della tastiera business tedesca"},
/* ko */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_NL, "Geef de naam van het symbolisch toetstoewijzingsbestand voor het Duitse zakelijk toetsenbord"},
/* pl */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_PL, "Okre�l nazw� pliku symbolicznego uk�adu niemieckiej klawiatury biznesowej"},
/* ru */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_RU, "Specify name of German business keyboard symbolic keymap file"},
/* sv */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_SV, "Ange fil f�r symbolisk tangentbordsemulering f�r tyskt \"business\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_BDE_SYM_KEYMAP_NAME_TR, "Alman business klavyesi sembolik tu� haritas� dosyas�n�n ismini belirt"},
#endif

/* cbm2/cbm2-cmdline-options.c, pet/pet-cmdline-options.c */
/* en */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME,    N_("Specify name of German business keyboard positional keymap file")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_DA, "Angiv fil for positionsbestemt tastaturemulering for Tysk \"business\"-tastatur"},
/* de */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_DE, "Keymap Datei f�r German business keyboard positional w�hlen"},
/* es */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_ES, "Especificar nombre fichero teclado Alem�n business posicional"},
/* fr */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_FR, "Sp�cifier le nom du fichier de mappage clavier positionnel allemand"},
/* hu */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_HU, "Adja meg a n�met, hivatalos, poz�ci� szerinti billenty�zet lek�pz�si f�jl nev�t."},
/* it */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_IT, "Specifica il nome del file della mappa posizionale della tastiera business tedesca"},
/* ko */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_NL, "Geef de naam van het positioneel toetstoewijzingsbestand voor het Duitse zakelijk toetsenbord"},
/* pl */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_PL, "Okre�l nazw� pliku pozycyjnego uk�adu niemieckiej klawiatury biznesowej"},
/* ru */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_RU, "Specify name of German business keyboard positional keymap file"},
/* sv */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_SV, "Ange fil f�r positionsriktig tangentbordsemulering f�r tyskt \"business\"-tangentbord"},
/* tr */ {IDCLS_SPECIFY_BDE_POS_KEYMAP_NAME_TR, "Alman business klavyesi konumsal tu� haritas� dosyas�n�n ismini belirt"},
#endif

#if (!defined  __OS2__ && !defined __BEOS__)
#else

/* initcmdline.c */
/* en */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER,    N_("Don't call exception handler")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_DA, "Kald ikke exception-handler"},
/* de */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_DE, "Ausnahmebehandlung vermeiden"},
/* es */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_ES, "No llamar al manipulador de excepciones"},
/* fr */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_FR, "Ne pas utiliser l'assistant d'exception"},
/* hu */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_HU, "Ne h�vja a kiv�tel kezel�t"},
/* it */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_IT, "Richiama il gestore delle eccezioni"},
/* ko */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_NL, "Geen gebruik maken van de exception handler"},
/* pl */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_PL, "Nie zajmuj si� wyj�tkami"},
/* ru */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_RU, "Don't call exception handler"},
/* sv */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_SV, "Anropa inte undantagshanterare"},
/* tr */ {IDCLS_DONT_CALL_EXCEPTION_HANDLER_TR, "Kural d��� i�leyiciyi �a��rma"},
#endif

/* initcmdline.c */
/* en */ {IDCLS_CALL_EXCEPTION_HANDLER,    N_("Call exception handler (default)")},
#ifdef HAS_TRANSLATION
/* da */ {IDCLS_CALL_EXCEPTION_HANDLER_DA, "Kald exception-handler (standard)"},
/* de */ {IDCLS_CALL_EXCEPTION_HANDLER_DE, "Ausnahmebehandlung aktivieren (Default)"},
/* es */ {IDCLS_CALL_EXCEPTION_HANDLER_ES, "Llamar al manipulador de excepciones (por defecto)"},
/* fr */ {IDCLS_CALL_EXCEPTION_HANDLER_FR, "Utiliser l'assistant d'exception (par d�faut)"},
/* hu */ {IDCLS_CALL_EXCEPTION_HANDLER_HU, "Kiv�tel kezel� h�v�sa (alap�rtelmez�s)"},
/* it */ {IDCLS_CALL_EXCEPTION_HANDLER_IT, "Richiama il gestore delle eccezioni (predefinito)"},
/* ko */ {IDCLS_CALL_EXCEPTION_HANDLER_KO, ""},  /* fuzzy */
/* nl */ {IDCLS_CALL_EXCEPTION_HANDLER_NL, "Gebruik maken van de exception handler (standaard)"},
/* pl */ {IDCLS_CALL_EXCEPTION_HANDLER_PL, "Zajmij si� wyj�tkami (domy�lnie)"},
/* ru */ {IDCLS_CALL_EXCEPTION_HANDLER_RU, "Call exception handler (default)"},
/* sv */ {IDCLS_CALL_EXCEPTION_HANDLER_SV, "Anropa undantagshanterare (standard)"},
/* tr */ {IDCLS_CALL_EXCEPTION_HANDLER_TR, "Kural d��� i�leyiciyi �a��r (varsay�lan)"},
#endif
#endif

};