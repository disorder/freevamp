/*
 * freevamp.c
 *
 * by Gary Wong <gtw@gnu.org>, 2002.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#define G_DISABLE_DEPRECATED 1
#define GDK_DISABLE_DEPRECATED 1
#define GDK_PIXBUF_DISABLE_DEPRECATED 1
#define GTK_DISABLE_DEPRECATED 1

#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fvicon.h"
#include "getopt.h"

extern char szCopying[], szWarranty[];

#define RCFILE ".freevamprc"

#define CTRL_WAH_PEDAL 1
#define CTRL_VOLUME_PEDAL 7
#define CTRL_AMP_GAIN 12
#define CTRL_AMP_TREBLE 13
#define CTRL_AMP_MID 14
#define CTRL_AMP_BASS 15
#define CTRL_AMP_VOL 16
#define CTRL_AMP_PRESENCE 17
#define CTRL_REVERB_MIX 18
#define CTRL_AMP_TYPE_DEFAULT 19
#define CTRL_FX_TYPE_DEFAULT 20
#define CTRL_FX 21
#define CTRL_REVERB 22
#define CTRL_CABINET_TYPE 23
#define CTRL_REVERB_TYPE 24
#define CTRL_NOISE_GATE 25
#define CTRL_DRIVE 26
#define CTRL_WAH 27
#define CTRL_PRE_FX_TYPE 44
#define CTRL_PRE_FX_1 45
#define CTRL_PRE_FX_2 46
#define CTRL_PRE_FX_3 47
#define CTRL_PRE_FX_4 48
#define CTRL_DELAY_TYPE 49
#define CTRL_DELAY_TIME_HI 50
#define CTRL_DELAY_TIME_LO 51
#define CTRL_DELAY_SPREAD 52
#define CTRL_DELAY_FEEDBACK 53
#define CTRL_DELAY_MIX 54
#define CTRL_POST_FX_MODE 55
#define CTRL_POST_FX_1 56
#define CTRL_POST_FX_2 57
#define CTRL_POST_FX_3 58
#define CTRL_POST_FX_MIX 59
#define CTRL_FX_ASSIGN 60
#define CTRL_AMP_TYPE 61
#define CTRL_TAP 64
#define CTRL_REQUEST 80
#define CTRL_NAME 81
#define CTRL_TUNER_VOLUME 82
#define CTRL_TUNER_FREQ 83
#define CTRL_CONFIG 84
#define CTRL_LIVE_EQ_TREBLE 85
#define CTRL_LIVE_EQ_MID 86
#define CTRL_LIVE_EQ_BASS 87

#define PARM_AMP_GAIN 0
#define PARM_AMP_TREBLE 1
#define PARM_AMP_MID 2
#define PARM_AMP_BASS 3
#define PARM_AMP_VOL 4
#define PARM_AMP_PRESENCE 5
#define PARM_REVERB_MIX 6
#define PARM_AMP_TYPE 7
#define PARM_FX_MIX_ASSIGN 8
#define PARM_FX 9
#define PARM_FX_REVERB 10
#define PARM_CABINET_TYPE 11
#define PARM_REVERB_TYPE 12
#define PARM_NOISE_GATE 13
#define PARM_DRIVE 14
#define PARM_WAH 15
#define PARM_PRE_FX_TYPE 16
#define PARM_PRE_FX_1 17
#define PARM_PRE_FX_2 18
#define PARM_PRE_FX_3 19
#define PARM_PRE_FX_4 20
#define PARM_DELAY_TYPE 21
#define PARM_DELAY_TIME_HI 22
#define PARM_DELAY_TIME_LO 23
#define PARM_DELAY_SPREAD 24
#define PARM_DELAY_FEEDBACK 25
#define PARM_DELAY_MIX 26
#define PARM_POST_FX_MODE 27
#define PARM_POST_FX_1 28
#define PARM_POST_FX_2 29
#define PARM_POST_FX_3 30
#define PARM_POST_FX 31
#define PARM_PRESET_NAME 32
#define PARM_PRESET_LEN 16
#define NUM_PARMS 48

#define PRE_FX_NONE 0
#define PRE_FX_COMPRESSOR 1
#define PRE_FX_AUTO_WAH 2

#define POST_FX_ROTARY 0
#define POST_FX_PHASER 1
#define POST_FX_TREMOLO 2
#define POST_FX_MONO_CHORUS 3
#define POST_FX_STEREO_CHORUS 4
#define POST_FX_MONO_FLANGER 5
#define POST_FX_STEREO_FLANGER 6

#define DELAY_SIMPLE 0
#define DELAY_ECHO 1
#define DELAY_PING_PONG 2

#define NUM_BANKS 25
#define PRESETS_PER_BANK 5
#define NUM_PRESETS (NUM_BANKS * PRESETS_PER_BANK)
#define PRESET_CURRENT 0x7F
#define PRESET_TUNER 0x7F

#define CMD_IDENTIFY_DEV 0x01
#define CMD_IDENTIFY_RESPONSE 0x02
#define CMD_WRITE_PRESET 0x20
#define CMD_WRITE_ALL_PRESETS 0x21
#define CMD_SET_CHANNEL 0x22
#define CMD_SET_TUNER_VOL 0x23
#define CMD_SET_TUNER_FREQ 0x24
#define CMD_REQUEST_PRESET 0x60
#define CMD_REQUEST_ALL_PRESETS 0x61

#define MIDI_DATA 0x7F
#define MIDI_STATUS 0x80
#define MIDI_OPERATION 0xF0
#define MIDI_CHANNEL 0x0F
#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON 0x90
#define MIDI_AFTERTOUCH 0xA0
#define MIDI_CTRLCHANGE 0xB0
#define MIDI_PROGCHANGE 0xC0
#define MIDI_CHANPRESSURE 0xD0
#define MIDI_PITCHWHEEL 0xE0
#define MIDI_SYSTEM 0xF0
#define MIDI_SYSEX 0xF0
#define MIDI_MTCQUARTERFRAME 0xF1
#define MIDI_SONGPOS 0xF2
#define MIDI_SONGSEL 0xF3
#define MIDI_TUNEREQ 0xF6
#define MIDI_SYSEX_END 0xF7
#define MIDI_SYSREALTIME 0xF8
#define MIDI_CLOCK 0xF8
#define MIDI_TICK 0xF9
#define MIDI_START 0xFA
#define MIDI_CONTINUE 0xFB
#define MIDI_STOP 0xFC
#define MIDI_ACTIVESENSE 0xFE
#define MIDI_RESET 0xFF

#define PROG_TUNER 0x7F

#define ID_BEHRINGER1 0x00
#define ID_BEHRINGER2 0x20
#define ID_BEHRINGER3 0x32

#define DEV_BROADCAST 0x7F
#define MOD_BROADCAST 0x7F

#define POINTS_INCH 72
#define POINTS_MM (POINTS_INCH/25.4)

#define PAPER_ISO_A4_X (210*POINTS_MM)
#define PAPER_ISO_A4_Y (297*POINTS_MM)

#define PAPER_US_LETTER_X (8.5*POINTS_INCH)
#define PAPER_US_LETTER_Y (11*POINTS_INCH)

#define PRINT_AREA_X 451
#define PRINT_AREA_Y 648

#define VLB_HEADER_SIZE 16
#define VLB_PATCH_SIZE 276
#define VLB_PATCH_OFFSET 220

typedef struct _vamp {
    int h; /* MIDI device file descriptor */
    GIOChannel *pioc;
    char *szMidiDevice, *szDevice, *szFileName;
    unsigned char nDeviceID, nModelID, iChannel, iProgram, nRunningStatus;
    char achPreset[ PRESET_CURRENT + 1 ][ NUM_PARMS ];
    char achClipboard[ NUM_PARMS ];
    int fClipboard, fSysex;
    char achCommand[ 3 ];
    int cchCommand;
    GtkWidget *pwEditor, *pwList, *pwLog, *pwLogList;
    GtkListStore *plsLog;
    GtkAccelGroup *pag;
    struct _listwindow *plw;
    struct _editorwindow *pew;
    void *pvSysex;
    void (*control)( struct _vamp *pv, char iController, char nValue );
    void (*program)( struct _vamp *pv, char iProgram );
    void (*sysex)( struct _vamp *pv, unsigned char n, void *p,
		   int cRemaining );
} vamp;

typedef struct _preferences {
    int fLog, fLogScroll, cLogMax, cxPaper, cyPaper;
    char *szPrint;
} preferences;

static char *aszPreEffectsName[] = { "None", "Compressor", "Auto wah", NULL };
static char *aszModulationName[] = { "Rotary", "Phaser", "Tremolo",
				     "Mono chorus", "Stereo chorus",
				     "Mono flanger", "Stereo flanger", NULL };
static char *aszDelayName[] = { "Delay", "Echo", "Ping pong", NULL };
static char *aszReverbName[] = { "Tiny room", "Small room", "Medium room",
				 "Large room", "Ultra room", "Small spring",
				 "Medium spring", "Short ambience",
				 "Long ambience", NULL };
static char *aszAmpName[] = { "None", "American Blues", "And Deluxe",
			      "Modern Class A", "Custom Class A",
			      "Tweed Combo", "Small Combo",
			      "Classic Clean", "Black Twin",
			      "British Blues", "And Custom",
			      "British Class A", "Non Top Boost",
			      "British Classic", "Classic 50W",
			      "British Hi Gain", "British Class A 15W",
			      "Rectified Hi Gain", "Rectified Head",
			      "Modern Hi Gain", "Savage Beast",
			      "Fuzz Box", "Custom Hi Gain",
			      "Ultimate V-AMP", "Ultimate Plus",
			      "Drive V-AMP", "California Drive",
			      "Crunch V-AMP", "Custom Drive",
			      "Clean V-AMP", "California Clean",
			      "Tube Pre-amp", "Custom Clean", NULL };
static char *aszCabinetName[] = { "None", "1 x 8\" Vintage Tweed",
				  "4 x 10\" Vintage Bass",
				  "4 x 10\" V-AMP Custom",
				  "1 x 12\" Mid Combo",
				  "1 x 12\" Blackface",
				  "1 x 12\" Brit '60",
				  "1 x 12\" Deluxe '52",
				  "2 x 12\" Twin Combo",
				  "2 x 12\" U.S. Class A",
				  "2 x 12\" V-AMP Custom",
				  "2 x 12\" Brit '67",
				  "4 x 12\" Vintage 30",
				  "4 x 12\" Standard '78",
				  "4 x 12\" Off Axis",
				  "4 x 12\" V-AMP Custom", NULL };
static char *aszEffectsAssignName[] = { "Echo", "Delay", "Ping pong",
					"Phaser/Delay", "Flanger/Delay 1",
					"Flanger/Delay 2", "Chorus/Delay 1",
					"Chorus/Delay 2", "Chorus/Compressor",
					"Compressor", "Auto wah", "Phaser",
					"Chorus", "Flanger", "Tremolo",
					"Rotary", NULL };
static char *aszAutoWahLabel[ 4 ] = { "Spd", "Dpt", "Off", "Frq" };
static char *aszModulationLabel[ 4 ] = { "Spd", "Dpt", "Fdb", "Mix" };
static char *aszDelayLabel[ 4 ] = { "Tim", "Spr", "Fdb", "Mix" };
static char *aszAmpLabel[ 6 ] = { "Gan", "Bas", "Mid", "Tre", "Pre", "Vol" };
static char szIcon[] = "freevamp-icon";

static char achDefaultParm[ NUM_PARMS ] = {
    64, 64, 64, 64, 64, 64, 64, 32, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    'U','n','n','a','m','e','d',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

static preferences pref = { FALSE, TRUE, 100, PAPER_ISO_A4_X, PAPER_ISO_A4_Y,
			    NULL };

static char *PreEffectsName( int i ) {

    return ( i < 0 || i > 2 ) ? NULL : aszPreEffectsName[ i ];
}

static char *ModulationName( int i ) {

    return ( i < 0 || i > 6 ) ? NULL : aszModulationName[ i ];
}

static char *DelayName( int i ) {

    return ( i < 0 || i > 2 ) ? NULL : aszDelayName[ i ];
}

static char *ReverbName( int i ) {

    return ( i < 0 || i > 8 ) ? NULL : aszReverbName[ i ];
}

static char *AmpName( int i ) {

    if( i < 0x10 )
	return aszAmpName[ ( i << 1 ) + 1 ];
    else if( i < 0x20 )
	return aszAmpName[ ( ( i & 0x0F ) << 1 ) + 2 ];
    else
	return aszAmpName[ 0 ];
}

static char *CabinetName( int i ) {

    return ( i < 0 || i > 15 ) ? NULL : aszCabinetName[ i ];
}

static char *EffectsAssignName( int i ) {

    return ( i < 0 || i > 15 ) ? NULL : aszEffectsAssignName[ i ];
}

static int AutoWahSpeed( int n ) {
    
    static int anAutoWahSpeed[ 10 ] = { 10, 20, 50, 100, 200, 300, 400, 500,
					750, 1000 };

    return ( n < 0 || n > 129 ) ? 0 : anAutoWahSpeed[ n / 13 ];
}

static int Message( GtkWindow *pwParent, GtkMessageType mt, GtkButtonsType bt,
		    char *szFormat, ... ) G_GNUC_PRINTF( 4, 5 );

static int Message( GtkWindow *pwParent, GtkMessageType mt, GtkButtonsType bt,
		    char *szFormat, ... ) {
    
    va_list val;
    char *pch;
    GtkWidget *pw;
    int f;
    
    va_start( val, szFormat );
    pch = g_strdup_vprintf( szFormat, val );
    va_end( val );

    pw = gtk_message_dialog_new( pwParent, GTK_DIALOG_DESTROY_WITH_PARENT,
				 mt, bt, "%s", pch );
    g_free( pch );
    f = gtk_dialog_run( GTK_DIALOG( pw ) );
    gtk_widget_destroy( pw );

    return f;
}

static void LogTrimScroll( vamp *pva ) {

    int c;
    GtkAdjustment *padj;
    GtkTreeIter ti;
    
    if( !pva->plsLog )
	return;
    
    for( c = gtk_tree_model_iter_n_children( GTK_TREE_MODEL( pva->plsLog ),
					     NULL ) - pref.cLogMax;
	 c > 0; c-- ) {
	gtk_tree_model_get_iter_first( GTK_TREE_MODEL( pva->plsLog ),
				       &ti );
	gtk_list_store_remove( pva->plsLog, &ti );
    }

    if( pref.fLogScroll ) {
	padj = gtk_tree_view_get_vadjustment( GTK_TREE_VIEW(
	    pva->pwLogList ) );
	gtk_adjustment_set_value( padj, G_MAXDOUBLE );
    }
}

static void Log( vamp *pva, unsigned char chStatus, char *szFormat, ... ) {

    char sz[ 1024 ], szStatus[ 3 ], szChannel[ 4 ];
    va_list val;
    GtkTreeIter ti;

    if( !pva->plsLog )
	return;
    
    va_start( val, szFormat );
    vsnprintf( sz, sizeof sz, szFormat, val );
    va_end( val );

    if( ( chStatus & MIDI_OPERATION ) == MIDI_SYSTEM )
	strcpy( szChannel, "N/A" );
    else
	sprintf( szChannel, "%d", ( chStatus & MIDI_CHANNEL ) + 1 );

    sprintf( szStatus, "%02X", chStatus );
    
    gtk_list_store_append( pva->plsLog, &ti );
    gtk_list_store_set( pva->plsLog, &ti, 0, pva->szMidiDevice, 1, szChannel,
			2, szStatus, 3, sz, -1 );

    LogTrimScroll( pva );
}

static char *ControllerName( unsigned char i ) {

    switch( i ) {
    case CTRL_WAH_PEDAL:
	return "Wah Pedal";
    case CTRL_VOLUME_PEDAL:
	return "Volume Pedal";
    case CTRL_AMP_GAIN:
	return "Amp Gain";
    case CTRL_AMP_TREBLE:
	return "Amp Treble";
    case CTRL_AMP_MID:
	return "Amp Mid";
    case CTRL_AMP_BASS:
	return "Amp Bass";
    case CTRL_AMP_VOL:
	return "Amp Vol";
    case CTRL_AMP_PRESENCE:
	return "Presence";
    case CTRL_REVERB_MIX:
	return "Reverb Mix";
    case CTRL_AMP_TYPE_DEFAULT:
	return "Amp Type with default cabinet";
    case CTRL_FX_TYPE_DEFAULT:
	return "Fx Type with defaults";
    case CTRL_FX:
	return "Fx Enable";
    case CTRL_REVERB:
	return "Reverb Enable";
    case CTRL_CABINET_TYPE:
	return "Cabinet Type";
    case CTRL_REVERB_TYPE:
	return "Reverb Type";
    case CTRL_NOISE_GATE:
	return "Noise Gate Level";
    case CTRL_DRIVE:
	return "Drive";
    case CTRL_WAH:
	return "Wah position";
    case CTRL_PRE_FX_TYPE:
	return "pre Effect Type";
    case CTRL_PRE_FX_1:
	return "pre Effect 1";
    case CTRL_PRE_FX_2:
	return "pre Effect 2";
    case CTRL_PRE_FX_3:
	return "pre Effect 3";
    case CTRL_PRE_FX_4:
	return "pre Effect 4";
    case CTRL_DELAY_TYPE:
	return "Delay Type";
    case CTRL_DELAY_TIME_HI:
	return "Delay Time hi";
    case CTRL_DELAY_TIME_LO:
	return "Delay Time lo";
    case CTRL_DELAY_SPREAD:
	return "Delay Spread";
    case CTRL_DELAY_FEEDBACK:
	return "Delay Feedback";
    case CTRL_DELAY_MIX:
	return "Delay Mix";
    case CTRL_POST_FX_MODE:
	return "post Fx Mode";
    case CTRL_POST_FX_1:
	return "post Fx 1";
    case CTRL_POST_FX_2:
	return "post Fx 2";
    case CTRL_POST_FX_3:
	return "post Fx 3";
    case CTRL_POST_FX_MIX:
	return "post Fx Mix";
    case CTRL_FX_ASSIGN:
	return "Assign Effects";
    case CTRL_AMP_TYPE:
	return "Amp Type w/o cabinet change";
    case CTRL_TAP:
	return "Tap";
    case CTRL_REQUEST:
	return "Request Controls";
    case CTRL_NAME:
	return "Preset Name";
    case CTRL_TUNER_VOLUME:
	return "Tuner Bypass Volume";
    case CTRL_TUNER_FREQ:
	return "Tuner Centre Frequency";
    case CTRL_CONFIG:
	return "Configuration";
    case CTRL_LIVE_EQ_TREBLE:
	return "Live EQ Treble";
    case CTRL_LIVE_EQ_MID:
	return "Live EQ Mid";
    case CTRL_LIVE_EQ_BASS:
	return "Live EQ Bass";
    default:
	return "unknown";
    }
}

static char *PresetName( char *achPreset ) {

    static char ach[ 17 ];
    char *pch;
    
    memcpy( ach, &achPreset[ PARM_PRESET_NAME ], PARM_PRESET_LEN );
    ach[ 16 ] = 0;
    for( pch = ach + 15; *pch == ' '; pch-- )
	*pch = 0;

    return ach;
}

static int ReadMIDI( vamp *pva ) {

    unsigned char ach[ 1024 ], *pch;
    int n;
    static char *aszNote[ 12 ] = {
	"C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B"
    };
    
    if( ( n = read( pva->h, ach, sizeof ach ) ) <= 0 )
	return -1;

    for( pch = ach; n; pch++, n-- ) {
	if( pva->fSysex && pva->sysex )
	    pva->sysex( pva, *pch, pva->pvSysex, n - 1 );

	if( *pch & MIDI_STATUS ) {
	    if( ( *pch & MIDI_SYSREALTIME ) == MIDI_SYSREALTIME )
		switch( *pch ) {
		case MIDI_CLOCK:
		    Log( pva, *pch, "System Clock" );
		    break;
		case MIDI_TICK:
		    Log( pva, *pch, "System Tick" );
		    break;
		case MIDI_START:
		    Log( pva, *pch, "System Start" );
		    break;
		case MIDI_CONTINUE:
		    Log( pva, *pch, "System Continue" );
		    break;
		case MIDI_STOP:
		    Log( pva, *pch, "System Stop" );
		    break;
		case MIDI_ACTIVESENSE:
		    Log( pva, *pch, "System Active Sense" );
		    break;
		case MIDI_RESET:
		    Log( pva, *pch, "System Clock" );
		    break;
		}
	    else if( *pch == MIDI_TUNEREQ ) {
		Log( pva, *pch, "System Tune Request" );
		pva->nRunningStatus = 0;
		pva->cchCommand = 0;
		pva->fSysex = FALSE;
		break;
	    } else if( pva->nRunningStatus == MIDI_SYSEX &&
		     *pch == MIDI_SYSEX_END ) {
		Log( pva, pva->nRunningStatus,
		     "System Exclusive: %d bytes", pva->cchCommand );;
		pva->nRunningStatus = 0;
		pva->cchCommand = 0;
		pva->fSysex = FALSE;
	    } else {
		pva->nRunningStatus = *pch;
		pva->cchCommand = 0;
		pva->fSysex = *pch == MIDI_SYSEX;
	    }
	} else {
	    if( pva->cchCommand < 3 )
		pva->achCommand[ pva->cchCommand ] = *pch;

	    pva->cchCommand++;
	    
	    switch( pva->nRunningStatus & MIDI_OPERATION ) {
	    case MIDI_NOTEOFF:
	    case MIDI_NOTEON:
		if( pva->cchCommand >= 2 ) {
		    Log( pva, pva->nRunningStatus, "Note %d (%s%d) %s "
			 "(velocity %d)", pva->achCommand[ 0 ],
			 aszNote[ pva->achCommand[ 0 ] % 12 ],
			 pva->achCommand[ 0 ] / 12,
			 ( ( pva->nRunningStatus & MIDI_OPERATION ) ==
			   MIDI_NOTEOFF ) || !pva->achCommand[ 1 ] ? "off" :
			 "on", pva->achCommand[ 1 ] );
		    pva->cchCommand = 0;
		}
		break;

	    case MIDI_AFTERTOUCH:
		if( pva->cchCommand >= 2 ) {
		    Log( pva, pva->nRunningStatus, "Note %d (%s%d) aftertouch "
			 "(pressure %d)", pva->achCommand[ 0 ],
			 aszNote[ pva->achCommand[ 0 ] % 12 ],
			 pva->achCommand[ 0 ] / 12,
			 pva->achCommand[ 1 ] );
		    pva->cchCommand = 0;
		}
		break;
		
	    case MIDI_CTRLCHANGE:
		if( pva->cchCommand >= 2 ) {
		    if( pva->achCommand[ 0 ] == CTRL_NAME ) {
			if( pva->achCommand[ 1 ] < ' ' )
			    Log( pva, pva->nRunningStatus,
				 "Controller %d (%s): position %d",
				 pva->achCommand[ 0 ],
				 ControllerName( pva->achCommand[ 0 ] ),
				 pva->achCommand[ 1 ] );
			else
			    Log( pva, pva->nRunningStatus,
				 "Controller %d (%s): character '%c'",
				 pva->achCommand[ 0 ],
				 ControllerName( pva->achCommand[ 0 ] ),
				 pva->achCommand[ 1 ] );
		    } else
			Log( pva, pva->nRunningStatus,
			     "Controller %d (%s): %d", pva->achCommand[ 0 ],
			     ControllerName( pva->achCommand[ 0 ] ),
			     pva->achCommand[ 1 ] );
		    pva->control( pva, pva->achCommand[ 0 ],
				  pva->achCommand[ 1 ] );
		    pva->cchCommand = 0;
		}
		break;
		
	    case MIDI_PROGCHANGE:
		if( pva->achCommand[ 0 ] < NUM_PRESETS )
		    Log( pva, pva->nRunningStatus,
			 "Program %d (%d%c, %s)", pva->achCommand[ 0 ],
			 pva->achCommand[ 0 ] / PRESETS_PER_BANK + 1,
			 'A' + ( pva->achCommand[ 0 ] % PRESETS_PER_BANK ),
			 PresetName( pva->achPreset[
			     (int) pva->achCommand[ 0 ] ] ) );
		else if( pva->achCommand[ 0 ] == PRESET_TUNER )
		    Log( pva, pva->nRunningStatus,
			 "Program %d (Tuner)", pva->achCommand[ 0 ] );
		else
		    Log( pva, pva->nRunningStatus,
			 "Program %d (unknown)", pva->achCommand[ 0 ] );
		    
		pva->program( pva, pva->achCommand[ 0 ] );
		pva->cchCommand = 0;
		break;

	    case MIDI_CHANPRESSURE:
		Log( pva, pva->nRunningStatus, "Channel pressure %d\n",
		     pva->achCommand[ 0 ] );
		pva->cchCommand = 0;
		break;

	    case MIDI_PITCHWHEEL:
		if( pva->cchCommand >= 2 ) {
		    Log( pva, pva->nRunningStatus, "Pitch bend %d (%dc)",
			 pva->achCommand[ 0 ] | ( pva->achCommand[ 1 ] << 7 ),
			 ( ( pva->achCommand[ 0 ] |
			     ( pva->achCommand[ 1 ] << 7 ) ) - 0x2000 )
			 * 100 / 0x1000 );
		    pva->cchCommand = 0;
		}
		break;

	    case MIDI_MTCQUARTERFRAME:
		Log( pva, pva->nRunningStatus, "System MTC quarter frame %d\n",
		     pva->achCommand[ 0 ] );
		pva->cchCommand = 0;
		pva->nRunningStatus = 0;
		break;
		
	    case MIDI_SONGPOS:
		if( pva->cchCommand >= 2 ) {
		    Log( pva, pva->nRunningStatus, "System Song position %d",
			 pva->achCommand[ 0 ] |
			 ( pva->achCommand[ 1 ] << 7 ) );
		    pva->cchCommand = 0;
		    pva->nRunningStatus = 0;
		}
		break;
		
	    case MIDI_SONGSEL:
		Log( pva, pva->nRunningStatus, "System Song select %d\n",
		     pva->achCommand[ 0 ] );
		pva->cchCommand = 0;
		pva->nRunningStatus = 0;
		break;
		
	    default:
		/* ignore */
	    }
	}
    }

    return 0;
}

typedef struct _sysexstate {
    int cch, cchAlloc, cchExpected, cchOld, fFailed;
    char *pch;
    GtkWidget *pwDialog, *pwProgress;
} sysexstate;

static void HandleSysex( vamp *pva, unsigned char ch, void *p,
			 int cRemaining ) {

    sysexstate *pses = p;

    if( !cRemaining ) {
	if( pses->cchExpected )
	    gtk_progress_bar_set_fraction(
		GTK_PROGRESS_BAR( pses->pwProgress ),
		(double) pses->cch / pses->cchExpected );
	else
	    gtk_progress_bar_pulse( GTK_PROGRESS_BAR( pses->pwProgress ) );

	while( gtk_events_pending() )
	  gtk_main_iteration();
    }

    /* we need to recheck pses->fFailed, since it may have been modified
       in gtk_main_iteration() above */
    if( pses->fFailed )
	return;

    if( ch == MIDI_SYSEX_END ) {
	/* success */
	pva->sysex = NULL; /* don't call us again */
	gtk_dialog_response( GTK_DIALOG( pses->pwDialog ), GTK_RESPONSE_OK );
	return;
    } else if( !( ch & MIDI_STATUS ) ) {
	pses->pch[ pses->cch++ ] = ch;

	if( pses->cch < pses->cchAlloc )
	    return;

	if( ( pses->pch = realloc( pses->pch, pses->cchAlloc <<= 1 ) ) )
	    return;
    }

    pses->fFailed = TRUE;
    gtk_dialog_response( GTK_DIALOG( pses->pwDialog ), GTK_RESPONSE_REJECT );
}

static unsigned char *ReadSysex( vamp *pva, GtkWindow *pwParent, int *cb,
				 int cbExpected ) {

    sysexstate ses;
    GtkWidget *pw;
    int n;
    
    if( pva->h < 0 ) {
	errno = EBADF;
	return NULL;
    }
	
    ses.cch = ses.cchOld = 0;
    ses.cchExpected = cbExpected;
    ses.fFailed = FALSE;
    pw = gtk_dialog_new_with_buttons( "Free V-AMP - Progress", pwParent,
				      GTK_DIALOG_MODAL, GTK_STOCK_CANCEL,
				      GTK_RESPONSE_REJECT, NULL );
    ses.pwDialog = pw;
    ses.pwProgress = gtk_progress_bar_new();
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       ses.pwProgress );
    gtk_widget_show_all( pw );
    if( ( ses.pch = malloc( ses.cchAlloc = 1024 ) ) ) {
	pva->pvSysex = &ses;
	pva->sysex = HandleSysex;

	n = gtk_dialog_run( GTK_DIALOG( pw ) );
	gtk_widget_destroy( pw );
    
	pva->pvSysex = NULL;
	pva->sysex = NULL;
    }

    if( n != GTK_RESPONSE_OK ) {
	free( ses.pch );
	ses.pch = NULL;
	ses.cch = 0;
    }
    
    if( cb )
	*cb = ses.cch;

    return realloc( ses.pch, ses.cch );
}

static int ControlChange( vamp *pva, char iController, char nValue ) {

    unsigned char ach[ 3 ] = { MIDI_CTRLCHANGE | pva->iChannel,
			       iController & MIDI_DATA, nValue & MIDI_DATA };

    write( pva->h, ach, 3 );

    pva->control( pva, iController, nValue );
    
    return 0;
}

static int RequestAllControllers( vamp *pva ) {

    return ControlChange( pva, CTRL_REQUEST, CTRL_REQUEST );
}

static int ProgramChange( vamp *pva, char iProgram ) {

    unsigned char ach[ 2 ] = { MIDI_PROGCHANGE | pva->iChannel,
			       iProgram & MIDI_DATA };

    if( write( pva->h, ach, 2 ) < 0 )
	return -1;

    return 0;
}

static int StartSysEx( vamp *pva, char idCommand ) {

    unsigned char ach[ 7 ] = { MIDI_SYSEX, ID_BEHRINGER1, ID_BEHRINGER2,
			       ID_BEHRINGER3, pva->nDeviceID, pva->nModelID,
			       idCommand };

    if( write( pva->h, ach, 7 ) < 0 )
	return -1;

    return 0;
}

static int EndSysEx( vamp *pva ) {

    static unsigned char ch = MIDI_SYSEX_END;

    if( write( pva->h, &ch, 1 ) < 0 )
	return -1;

    return 0;
}

static int IdentifyDevice( vamp *pva, GtkWindow *pw, char ach[ 256 ] ) {

    int cb;
    unsigned char *pch;
    
    if( StartSysEx( pva, CMD_IDENTIFY_DEV ) < 0 )
	return -1;
    
    if( EndSysEx( pva ) < 0 )
	return -1;
    
    if( !( pch = ReadSysex( pva, pw, &cb, 0 ) ) )
	return -1;

    if( pch[ 0 ] != ID_BEHRINGER1 || pch[ 1 ] != ID_BEHRINGER2 ||
	pch[ 2 ] != ID_BEHRINGER3 || pch[ 5 ] != CMD_IDENTIFY_RESPONSE ) {
	free( pch );
	return -1;
    }
    
    pva->nDeviceID = pch[ 3 ];
    pva->nModelID = pch[ 4 ];

    if( ( cb -= 6 ) > 255 )
	cb = 255;

    memcpy( ach, pch + 6, cb );
    ach[ cb ] = 0;

    free( pch );

    return 0;
}

static int WritePreset( vamp *pva, char iPreset, char ach[ NUM_PARMS ] ) {

    char achCommand[ NUM_PARMS + 2 ];
    int i;
    
    achCommand[ 0 ] = iPreset;
    achCommand[ 1 ] = NUM_PARMS;
    for( i = 0; i < NUM_PARMS; i++ )
	achCommand[ i + 2 ] = ach[ i ] & MIDI_DATA;
    
    if( StartSysEx( pva, CMD_WRITE_PRESET ) < 0 )
	return -1;
    
    if( write( pva->h, achCommand, NUM_PARMS + 2 ) < 0 )
	return -1;

    return EndSysEx( pva );
}

static int WriteAllPresets( vamp *pva, char ach[ NUM_PRESETS ][ NUM_PARMS ] ) {

    static char achCommand[ 2 ] = { NUM_PRESETS, NUM_PARMS };
    int i;
    
    if( StartSysEx( pva, CMD_WRITE_ALL_PRESETS ) < 0 )
	return -1;
    
    if( write( pva->h, achCommand, 2 ) < 0 )
	return -1;

    for( i = 0; i < NUM_PRESETS * NUM_PARMS; i++ )
	ach[ 0 ][ i ] &= MIDI_DATA;
    
    if( write( pva->h, ach, NUM_PRESETS * NUM_PARMS ) < 0 )
	return -1;

    return EndSysEx( pva );
}

static int SetChannel( vamp *pva, char i ) {

    if( StartSysEx( pva, CMD_SET_CHANNEL ) < 0 )
	return -1;
    
    if( write( pva->h, &i, 1 ) < 0 )
	return -1;

    if( EndSysEx( pva ) < 0 )
	return -1;

    pva->iChannel = i;

    return 0;
}

static int SetTunerVol( vamp *pva, char n ) {

    if( StartSysEx( pva, CMD_SET_TUNER_VOL ) < 0 )
	return -1;
    
    if( write( pva->h, &n, 1 ) < 0 )
	return -1;

    return EndSysEx( pva );    
}

static int SetTunerFreq( vamp *pva, char n ) {

    if( StartSysEx( pva, CMD_SET_TUNER_FREQ ) < 0 )
	return -1;
    
    if( write( pva->h, &n, 1 ) < 0 )
	return -1;

    return EndSysEx( pva );    
}

static int RequestPreset( vamp *pva, GtkWindow *pw, char i,
			  char ach[ NUM_PARMS ] ) {

    unsigned char *pch;
    int cb;
    
    if( StartSysEx( pva, CMD_REQUEST_PRESET ) < 0 )
	return -1;
    
    if( write( pva->h, &i, 1 ) < 0 )
	return -1;

    if( EndSysEx( pva ) < 0 )
	return -1;

    if( !( pch = ReadSysex( pva, pw, &cb, 8 + NUM_PARMS ) ) )
	return -1;

    if( pch[ 0 ] != ID_BEHRINGER1 || pch[ 1 ] != ID_BEHRINGER2 ||
	pch[ 2 ] != ID_BEHRINGER3 || pch[ 3 ] != pva->nDeviceID ||
	pch[ 4 ] != pva->nModelID || pch[ 5 ] != CMD_WRITE_PRESET ||
	pch[ 6 ] != i || pch[ 7 ] != NUM_PARMS ) {
	/* can't interpret response */
	free( pch );
	return -1;
    }
	
    memcpy( ach, pch + 8, NUM_PARMS );

    free( pch );

    return 0;
}

static int RequestAllPresets( vamp *pva, GtkWindow *pw ) {

    unsigned char *pch;
    int cb;
    
    if( StartSysEx( pva, CMD_REQUEST_ALL_PRESETS ) < 0 )
	return -1;
    
    if( EndSysEx( pva ) < 0 )
	return -1;

    if( !( pch = ReadSysex( pva, pw, &cb, 8 + NUM_PRESETS * NUM_PARMS ) ) )
	return -1;

    if( pch[ 0 ] != ID_BEHRINGER1 || pch[ 1 ] != ID_BEHRINGER2 ||
	pch[ 2 ] != ID_BEHRINGER3 || pch[ 3 ] != pva->nDeviceID ||
	pch[ 4 ] != pva->nModelID || pch[ 5 ] != CMD_WRITE_ALL_PRESETS ||
	pch[ 6 ] != NUM_PRESETS || pch[ 7 ] != NUM_PARMS ) {
	/* can't interpret response */
	free( pch );
	return -1;
    }
	
    memcpy( pva->achPreset, pch + 8, NUM_PRESETS * NUM_PARMS );

    free( pch );

    return 0;
}

typedef struct _editorwindow {
    vamp *pva;
    GtkWidget *pwNoiseGate, *pwWah,
	*pwPreEffectsType, *apwPreEffects[ 4 ], *apwPreEffectsLabel[ 4 ],
	*pwModulationType, *apwModulation[ 4 ],
	*pwDelayType, *apwDelay[ 4 ],
	*pwReverbType, *pwReverb,
	*pwAmpType, *pwDrive, *apwAmp[ 6 ], *pwCabinetType,
	*pwEffectsAssign, *pwName;
    int fRunning;
} editorwindow;

typedef struct _listwindow {
    vamp *pva;
    GtkWidget *apw[ NUM_PRESETS ], *pwPaste;
} listwindow;

static void UpdateEditorWindow( GtkWidget *pw, char achPreset[ NUM_PARMS ],
				vamp *pva ) {

    char *pch;
    int i;
    static int aiAmpParm[ 6 ] = {
	PARM_AMP_GAIN, PARM_AMP_BASS, PARM_AMP_MID,
	PARM_AMP_TREBLE, PARM_AMP_PRESENCE, PARM_AMP_VOL
    };

    pva->pew->fRunning = FALSE;
    
    /* Noise gate */
    gtk_range_set_value( GTK_RANGE( pva->pew->pwNoiseGate ),
			 achPreset[ PARM_NOISE_GATE ] );

    /* Wah */
    gtk_range_set_value( GTK_RANGE( pva->pew->pwWah ),
			 achPreset[ PARM_WAH ] );

    /* Pre effects */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwPreEffectsType ),
				 achPreset[ PARM_PRE_FX_TYPE ] );
    switch( achPreset[ PARM_PRE_FX_TYPE ] ) {
    case PRE_FX_COMPRESSOR:
	gtk_widget_set_sensitive( pva->pew->apwPreEffects[ 0 ], TRUE );
	gtk_label_set_text( GTK_LABEL( pva->pew->apwPreEffectsLabel[ 0 ] ),
			    "Rat" );
	for( i = 1; i < 4; i++ ) {
	    gtk_widget_set_sensitive( pva->pew->apwPreEffects[ i ], FALSE );
	    gtk_label_set_text( GTK_LABEL( pva->pew->apwPreEffectsLabel[ i ] ),
				NULL );
	}
	break;
	
    case PRE_FX_AUTO_WAH:
	for( i = 0; i < 4; i++ ) {
	    gtk_widget_set_sensitive( pva->pew->apwPreEffects[ i ], TRUE );
	    gtk_label_set_text( GTK_LABEL( pva->pew->apwPreEffectsLabel[ i ] ),
				aszAutoWahLabel[ i ] );
	}
	break;
	
    default:
	for( i = 0; i < 4; i++ ) {
	    gtk_widget_set_sensitive( pva->pew->apwPreEffects[ i ], FALSE );
	    gtk_label_set_text( GTK_LABEL( pva->pew->apwPreEffectsLabel[ i ] ),
				NULL );
	}
    }
    for( i = 0; i < 4; i++ )
	gtk_range_set_value( GTK_RANGE( pva->pew->apwPreEffects[ i ] ),
			     achPreset[ PARM_PRE_FX_1 + i ] );
    
    /* Modulation */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwModulationType ),
				 achPreset[ PARM_POST_FX_MODE ] );
    gtk_widget_set_sensitive( pva->pew->apwModulation[ 1 ],
			      achPreset[ PARM_POST_FX_MODE ] !=
			      POST_FX_TREMOLO );
    gtk_widget_set_sensitive( pva->pew->apwModulation[ 2 ],
			      ( achPreset[ PARM_POST_FX_MODE ] ==
				POST_FX_PHASER ) ||
			      ( achPreset[ PARM_POST_FX_MODE ] ==
				POST_FX_MONO_FLANGER ) ||
			      ( achPreset[ PARM_POST_FX_MODE ] ==
				POST_FX_STEREO_FLANGER ) );
    for( i = 0; i < 4; i++ )
	gtk_range_set_value( GTK_RANGE( pva->pew->apwModulation[ i ] ),
			     achPreset[ PARM_POST_FX_1 + i ] );
    
    /* Delay */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwDelayType ),
				 achPreset[ PARM_DELAY_TYPE ] );
    gtk_widget_set_sensitive( pva->pew->apwDelay[ 1 ],
			      achPreset[ PARM_DELAY_TYPE ] ==
			      DELAY_SIMPLE );
    gtk_range_set_value( GTK_RANGE( pva->pew->apwDelay[ 0 ] ),
			 ( achPreset[ PARM_DELAY_TIME_HI ] << 7 ) |
			 achPreset[ PARM_DELAY_TIME_LO ] );
    for( i = 0; i < 3; i++ )
	gtk_range_set_value( GTK_RANGE( pva->pew->apwDelay[ i + 1 ] ),
			     achPreset[ PARM_DELAY_SPREAD + i ] );
    
    /* Reverb */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwReverbType ),
				 achPreset[ PARM_REVERB_TYPE ] );
    gtk_range_set_value( GTK_RANGE( pva->pew->pwReverb ),
			 achPreset[ PARM_REVERB_MIX ] );
    
    /* Amp */
    if( achPreset[ PARM_AMP_TYPE ] < 0x10 )
	gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwAmpType ),
				     ( achPreset[ PARM_AMP_TYPE ] << 1 ) + 1 );
    else if( achPreset[ PARM_AMP_TYPE ] < 0x20 )
	gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwAmpType ),
				     ( ( achPreset[ PARM_AMP_TYPE ] & 0x0F )
				       << 1 ) + 2 );
    else
	gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwAmpType ),
				     0 );
	
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pva->pew->pwDrive ),
				  achPreset[ PARM_DRIVE ] );
    for( i = 0; i < 6; i++ )
	gtk_range_set_value( GTK_RANGE( pva->pew->apwAmp[ i ] ),
			     achPreset[ aiAmpParm[ i ] ] );
    
    /* Cabinet */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwCabinetType ),
				 achPreset[ PARM_CABINET_TYPE ] );
    
    /* Effects assign */
    gtk_option_menu_set_history( GTK_OPTION_MENU( pva->pew->pwEffectsAssign ),
				 achPreset[ PARM_FX_MIX_ASSIGN ] );
    
    /* Name */
    pch = g_strdup_printf( "Free V-AMP - %s", PresetName( achPreset ) );
    gtk_window_set_title( GTK_WINDOW( pw ), pch );
    g_free( pch );

    gtk_entry_set_text( GTK_ENTRY( pva->pew->pwName ),
			PresetName( achPreset ) );
    
    pva->pew->fRunning = TRUE;
}

static void EditorChange( GtkWidget *pw, editorwindow *pew ) {
    
    int n;
    char iController;

    if( !pew->fRunning )
	return;

    n = gtk_range_get_value( GTK_RANGE( pw ) );
    
    if( pw == pew->apwDelay[ 0 ] ) {
	ControlChange( pew->pva, CTRL_DELAY_TIME_LO, n & 0x7F );
	ControlChange( pew->pva, CTRL_DELAY_TIME_HI, n >> 7 );

	return;
    }
    
    if( pw == pew->pwNoiseGate )
	iController = CTRL_NOISE_GATE;
    else if( pw == pew->pwWah )
	iController = CTRL_WAH;
    else if( pw == pew->apwPreEffects[ 0 ] )
	iController = CTRL_PRE_FX_1;
    else if( pw == pew->apwPreEffects[ 1 ] )
	iController = CTRL_PRE_FX_2;
    else if( pw == pew->apwPreEffects[ 2 ] )
	iController = CTRL_PRE_FX_3;
    else if( pw == pew->apwPreEffects[ 3 ] )
	iController = CTRL_PRE_FX_4;
    else if( pw == pew->apwModulation[ 0 ] )
	iController = CTRL_POST_FX_1;
    else if( pw == pew->apwModulation[ 1 ] )
	iController = CTRL_POST_FX_2;
    else if( pw == pew->apwModulation[ 2 ] )
	iController = CTRL_POST_FX_3;
    else if( pw == pew->apwModulation[ 3 ] )
	iController = CTRL_POST_FX_MIX;
    else if( pw == pew->apwDelay[ 1 ] )
	iController = CTRL_DELAY_SPREAD;
    else if( pw == pew->apwDelay[ 2 ] )
	iController = CTRL_DELAY_FEEDBACK;
    else if( pw == pew->apwDelay[ 3 ] )
	iController = CTRL_DELAY_MIX;
    else if( pw == pew->pwReverb )
	iController = CTRL_REVERB_MIX;
    else if( pw == pew->apwAmp[ 0 ] )
	iController = CTRL_AMP_GAIN;
    else if( pw == pew->apwAmp[ 1 ] )
	iController = CTRL_AMP_BASS;
    else if( pw == pew->apwAmp[ 2 ] )
	iController = CTRL_AMP_MID;
    else if( pw == pew->apwAmp[ 3 ] )
	iController = CTRL_AMP_TREBLE;
    else if( pw == pew->apwAmp[ 4 ] )
	iController = CTRL_AMP_PRESENCE;
    else if( pw == pew->apwAmp[ 5 ] )
	iController = CTRL_AMP_VOL;

    ControlChange( pew->pva, iController, n );
}

static void DriveChange( GtkWidget *pw, editorwindow *pew ) {

    ControlChange( pew->pva, CTRL_DRIVE, gtk_toggle_button_get_active(
		       GTK_TOGGLE_BUTTON( pw ) ) ? 0x7F : 0 );
}

static gchar *FormatValue( GtkWidget *pw, gdouble f, editorwindow *pew ) {

    int n;
    char *pchPreset = pew->pva->achPreset[ PRESET_CURRENT ];
    static char *aszCompressorRatio[ 8 ] = { "1.2", "1.4", "2", "2.5",
					     "3", "4.5", "6", "\xE2\x88\x9E" };
	
    n = gtk_range_get_value( GTK_RANGE( pw ) );
    
    if( pw == pew->apwDelay[ 0 ] )
	return g_strdup_printf( "%d", (int) ( f * 0.128 + 0.5 ) );
    else if( pw == pew->pwNoiseGate ) {
	if( !f )
	    return g_strdup( "-\xE2\x88\x9E" /* UTF-8 infinity */ );
	else
	    return g_strdup_printf( "-%d", 93 - 3 * (int) f );
    } else if( pw == pew->apwPreEffects[ 0 ] )
	switch( pchPreset[ PARM_PRE_FX_TYPE ] ) {
	case PRE_FX_COMPRESSOR:
	    return g_strdup( aszCompressorRatio[ (int) f >> 4 ] );
	case PRE_FX_AUTO_WAH:
	    return g_strdup_printf( "%d", AutoWahSpeed( f ) );
	default:
	    return NULL;
	}
    else if( pw == pew->apwModulation[ 0 ] )
	switch( pchPreset[ PARM_POST_FX_MODE ] ) {
	case POST_FX_ROTARY:
	case POST_FX_PHASER:
	    return g_strdup_printf( "%d", (int) ( 0.59 * f * f + 0.5 ) );
	case POST_FX_TREMOLO:
	    return g_strdup_printf( "%d", (int) ( 0.62 * f * f + 0.5 ) );
	default:
	    return g_strdup_printf( "%d", (int) ( 0.31 * f * f + 0.5 ) );
	}
    else
	return g_strdup_printf( "%d", (int) f );
}

static void EditorSelect( GtkWidget *pw, gpointer *p ) {

    int n = GPOINTER_TO_INT( p ), i = n & 0xFF;
    char iController = n >> 8;
    vamp *pva = g_object_get_data(
	G_OBJECT( gtk_widget_get_toplevel( gtk_menu_get_attach_widget(
	    GTK_MENU( gtk_widget_get_parent( pw ) ) ) ) ), "vamp" );

    if( !pva->pew->fRunning )
	return;
    
    if( iController == CTRL_AMP_TYPE ) {
	if( !i )
	    i = 0x20;
	else if( i & 1 )
	    i = i >> 1;
	else
	    i = ( i >> 1 ) + 0x0F;
    }
    
    ControlChange( pva, iController, i );
}

static void NameChange( GtkWidget *pw, editorwindow *pew ) {

    int i;
    char *pch, *pchPreset = pew->pva->achPreset[ PRESET_CURRENT ];

    if( !pew->fRunning )
	return;
    
    pch = gtk_editable_get_chars( GTK_EDITABLE( pw ), 0, -1 );

    for( i = 0; i < PARM_PRESET_LEN && pch[ i ]; i++ ) {
	ControlChange( pew->pva, CTRL_NAME, i );
	ControlChange( pew->pva, CTRL_NAME,
		       pchPreset[ PARM_PRESET_NAME + i ] = pch[ i ] );
    }

    for( ; i < PARM_PRESET_LEN; i++ ) {
	ControlChange( pew->pva, CTRL_NAME, i );
	ControlChange( pew->pva, CTRL_NAME,
		       pchPreset[ PARM_PRESET_NAME + i ] = ' ' );
    }

    g_free( pch );
}

static void PopulateMenu( GtkWidget *pw, editorwindow *pew, char **ppch,
			  char iController ) {

    GtkWidget *pwMenu = gtk_menu_new(), *pwItem;
    int i = 0;
    
    for( i = 0; *ppch; i++ ) {
	pwItem = gtk_menu_item_new_with_label( *ppch++ );
	g_signal_connect( pwItem, "activate", G_CALLBACK( EditorSelect ),
			  GINT_TO_POINTER( ( iController << 8 ) | i ) );
	gtk_menu_shell_append( GTK_MENU_SHELL( pwMenu ), pwItem );
    }
    gtk_widget_show_all( pwMenu );
    gtk_option_menu_set_menu( GTK_OPTION_MENU( pw ), pwMenu );
}

static void SelectPatch( GtkWidget *ptv, GtkTreePath *ptp, GtkTreeViewColumn
			 *ptvc, GtkWidget *pw ) {

    gtk_dialog_response( GTK_DIALOG( pw ), GTK_RESPONSE_OK );
}

static int ImportPatch( vamp *pva, char achPreset[ NUM_PARMS ] ) {
    
    GtkWidget *pw, *pwList, *pwScrolled;
    GtkListStore *pls;
    GtkTreeIter ti;
    GtkTreePath *ptp;
    const char *pch;
    int i, c, h;
    struct stat st;
    char *pchFile;
    
    pw = gtk_file_selection_new( "Free V-AMP - Open" );
    
    gtk_window_set_transient_for( GTK_WINDOW( pw ),
				  GTK_WINDOW( pva->pwEditor ) );
    gtk_widget_show_all( pw );
    
    if( gtk_dialog_run( GTK_DIALOG( pw ) ) != GTK_RESPONSE_OK ) {
	gtk_widget_destroy( pw );
	return -1;
    }
    
    pch = gtk_file_selection_get_filename( GTK_FILE_SELECTION( pw ) );
    
    gtk_widget_destroy( pw );

    if( ( h = open( pch, O_RDONLY ) ) < 0 ) {
	Message( GTK_WINDOW( pva->pwEditor ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pch, g_strerror( errno ) );
	return -1;
    }

    if( fstat( h, &st ) < 0 ) {
	Message( GTK_WINDOW( pva->pwEditor ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pch, g_strerror( errno ) );
	return -1;
    }

    if( st.st_size == NUM_PRESETS * NUM_PARMS ) {
	/* Raw V-AMP format */
	c = NUM_PRESETS;
	
	pchFile = g_alloca( NUM_PRESETS * NUM_PARMS );
	if( read( h, pchFile, NUM_PRESETS * NUM_PARMS ) !=
	    NUM_PRESETS * NUM_PARMS ) {
	    Message( GTK_WINDOW( pva->pwEditor ), GTK_MESSAGE_ERROR,
		     GTK_BUTTONS_CLOSE, "%s: %s", pch, g_strerror( errno ) );
	    return -1;
	}
    } else if( !( ( st.st_size - VLB_HEADER_SIZE ) % VLB_PATCH_SIZE ) ) {
	/* VALB format */
	if( !( c = ( st.st_size - VLB_HEADER_SIZE ) / VLB_PATCH_SIZE ) )
	    goto bad_format;

	pchFile = g_alloca( st.st_size );
	if( read( h, pchFile, st.st_size ) != st.st_size ) {
	    Message( GTK_WINDOW( pva->pwEditor ), GTK_MESSAGE_ERROR,
		     GTK_BUTTONS_CLOSE, "%s: %s", pch, g_strerror( errno ) );
	    return -1;
	}

	if( memcmp( pchFile, "VALB", 4 ) )
	    goto bad_format;

	for( i = 0; i < c; i++ )
	    memcpy( pchFile + i * NUM_PARMS, pchFile + VLB_HEADER_SIZE +
		    i * VLB_PATCH_SIZE + VLB_PATCH_OFFSET, NUM_PARMS );
    } else {
    bad_format:
	Message( GTK_WINDOW( pva->pwEditor ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: not a valid V-AMP presets file",
		 pch );
	return -1;
    }

    pw = gtk_dialog_new_with_buttons( "Free V-AMP - Open", /* FIXME filename */
				      NULL, 0,
				      GTK_STOCK_OK, GTK_RESPONSE_OK,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      NULL );

    pls = gtk_list_store_new( 1, G_TYPE_STRING );
    pwScrolled = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pwScrolled ),
				    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ), pwScrolled );
    
    pwList = gtk_tree_view_new_with_model( GTK_TREE_MODEL( pls ) );
    g_object_unref( pls );
    gtk_container_add( GTK_CONTAINER( pwScrolled ), pwList );

    gtk_tree_selection_set_mode(
	gtk_tree_view_get_selection( GTK_TREE_VIEW( pwList ) ),
	GTK_SELECTION_BROWSE );
    gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( pwList ), FALSE );
    gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( pwList ),
						 -1, NULL,
						 gtk_cell_renderer_text_new(),
						 "text", 0, NULL );
    
    for( i = 0; i < c; i++ ) {
	gtk_list_store_append( GTK_LIST_STORE( pls ), &ti );
	gtk_list_store_set( GTK_LIST_STORE( pls ), &ti, 0,
			    PresetName( pchFile + i * NUM_PARMS ), -1 );
    }

    gtk_window_set_default_size( GTK_WINDOW( pw ), -1, 300 );
    gtk_widget_show_all( pw );

    g_signal_connect( pwList, "row-activated", G_CALLBACK( SelectPatch ),
		      pw );
    
    if( gtk_dialog_run( GTK_DIALOG( pw ) ) != GTK_RESPONSE_OK ) {
	gtk_widget_destroy( pw );
	return -1;
    }
    
    gtk_tree_selection_get_selected(
	gtk_tree_view_get_selection( GTK_TREE_VIEW( pwList ) ),
	NULL, &ti );

    ptp = gtk_tree_model_get_path( GTK_TREE_MODEL( pls ), &ti );
    memcpy( achPreset, pchFile + gtk_tree_path_get_indices( ptp )[ 0 ] *
	    NUM_PARMS, NUM_PARMS );

    gtk_widget_destroy( pw );

    return 0;
}

#define EDITOR_RESPONSE_OPEN 1

static void EditorResponse( GtkWidget *pw, gint nResponse, vamp *pva ) {

    char *pchPreset = pva->achPreset[ (int) pva->iProgram ];
    
    switch( nResponse ) {
    case EDITOR_RESPONSE_OPEN:
	if( !ImportPatch( pva, pva->achPreset[ PRESET_CURRENT ] ) ) {
	    WritePreset( pva, PRESET_CURRENT,
			 pva->achPreset[ PRESET_CURRENT ] );
	    RequestPreset( pva, GTK_WINDOW( pva->pwList ), PRESET_CURRENT,
			   pva->achPreset[ PRESET_CURRENT ] );
			   
	    if( pva->pwEditor )
		UpdateEditorWindow( pva->pwEditor,
				    pva->achPreset[ PRESET_CURRENT ], pva );
	}
	
	break;

    case GTK_RESPONSE_APPLY:
	if( pva->h >= 0 ) {
	    WritePreset( pva, pva->iProgram,
			 pva->achPreset[ PRESET_CURRENT ] );
	    
	    RequestPreset( pva, GTK_WINDOW( pva->pwList ), pva->iProgram,
			   pchPreset );
	} else
	    memcpy( pchPreset, pva->achPreset[ PRESET_CURRENT ], NUM_PARMS );
	    
	gtk_label_set_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN(
	    pva->plw->apw[ (int) pva->iProgram ] ) ) ),
			    PresetName( pchPreset ) );
	/* fall through */
    default:
	gtk_widget_destroy( pw );
    }
}

static GtkWidget *CreateEditorWindow( vamp *pva,
				      char achPreset[ NUM_PARMS ] ) {
    
    GtkWidget *pwWindow, *pwVbox, *pwHbox, *pwFrame, *pwTable, *pwAlign, *pw;
    editorwindow *pew;
    int i;
    
    if( !( pva->pew = pew = malloc( sizeof( *pew ) ) ) )
	return NULL;
	
    pwWindow = gtk_dialog_new_with_buttons( PresetName( achPreset ), NULL, 0,
					    GTK_STOCK_OPEN,
					    EDITOR_RESPONSE_OPEN,
					    GTK_STOCK_OK, GTK_RESPONSE_APPLY,
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_REJECT, NULL );
    gtk_dialog_set_default_response( GTK_DIALOG( pwWindow ),
				     GTK_RESPONSE_APPLY );
    pew->pva = pva;
    pew->fRunning = FALSE;
    pva->pwEditor = pwWindow;
    g_object_add_weak_pointer( G_OBJECT( pwWindow ),
			       (gpointer *) &pva->pwEditor );
    g_object_add_weak_pointer( G_OBJECT( pwWindow ),
			       (gpointer *) &pva->pew );
    g_object_set_data( G_OBJECT( pwWindow ), "vamp", pva );
    gtk_window_add_accel_group( GTK_WINDOW( pwWindow ), pva->pag );
    g_signal_connect( G_OBJECT( pwWindow ), "response",
		       G_CALLBACK( EditorResponse ), pva );
    g_signal_connect_swapped( pwWindow, "destroy", G_CALLBACK( free ), pew );
    
    pwHbox = gtk_hbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pwWindow )->vbox ), pwHbox );

    /* Noise gate */
    pwFrame = gtk_frame_new( "Noise gate" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );

    gtk_container_add( GTK_CONTAINER( pwFrame ),
		       pw = gtk_vscale_new_with_range( 0, 15, 1 ) );
    g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ), pew );
    g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
    gtk_widget_set_size_request( pw, -1, 80 );
    gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
    pew->pwNoiseGate = pw;
    
    /* Wah */
    pwFrame = gtk_frame_new( "Wah" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    
    gtk_container_add( GTK_CONTAINER( pwFrame ),
		       pw = gtk_vscale_new_with_range( 0, 127, 1 ) );
    g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ), pew );
    g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
    gtk_widget_set_size_request( pw, -1, 80 );
    gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
    pew->pwWah = pw;

    /* Pre effects */
    pwFrame = gtk_frame_new( "Pre effects" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    pwTable = gtk_table_new( 3, 4, FALSE );
    gtk_table_set_col_spacings( GTK_TABLE( pwTable ), 4 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwTable );

    gtk_table_attach( GTK_TABLE( pwTable ), pw = gtk_option_menu_new(),
		      0, 4, 0, 1, 0, 0, 0, 0 );
    PopulateMenu( pw, pew, aszPreEffectsName, CTRL_PRE_FX_TYPE );
    pew->pwPreEffectsType = pw;

    for( i = 0; i < 4; i++ ) {
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   pw = gtk_vscale_new_with_range( 0, 127, 1 ),
				   i, i + 1, 1, 2 );
	g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ),
			  pew );
	g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
	gtk_widget_set_size_request( pw, -1, 80 );
	gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
	pew->apwPreEffects[ i ] = pw;
	gtk_table_attach( GTK_TABLE( pwTable ),
			  pew->apwPreEffectsLabel[ i ] = gtk_label_new( NULL ),
			  i, i + 1, 2, 3, 0, 0, 0, 0 );
    }

    /* Modulation */
    pwFrame = gtk_frame_new( "Modulation" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    pwTable = gtk_table_new( 3, 4, FALSE );
    gtk_table_set_col_spacings( GTK_TABLE( pwTable ), 4 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwTable );

    gtk_table_attach( GTK_TABLE( pwTable ), pw = gtk_option_menu_new(),
		      0, 4, 0, 1, 0, 0, 0, 0 );
    PopulateMenu( pw, pew, aszModulationName, CTRL_POST_FX_MODE );
    pew->pwModulationType = pw;

    for( i = 0; i < 4; i++ ) {
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   pw = gtk_vscale_new_with_range( 0, 127, 1 ),
				   i, i + 1, 1, 2 );
	g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ),
			  pew );
	g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
	gtk_widget_set_size_request( pw, -1, 80 );
	gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
	pew->apwModulation[ i ] = pw;
	gtk_table_attach( GTK_TABLE( pwTable ),
			  gtk_label_new( aszModulationLabel[ i ] ),
			  i, i + 1, 2, 3, 0, 0, 0, 0 );
    }    

    /* Delay */
    pwFrame = gtk_frame_new( "Delay" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    pwTable = gtk_table_new( 3, 4, FALSE );
    gtk_table_set_col_spacings( GTK_TABLE( pwTable ), 4 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwTable );

    gtk_table_attach( GTK_TABLE( pwTable ), pw = gtk_option_menu_new(),
		      0, 4, 0, 1, 0, 0, 0, 0 );
    PopulateMenu( pw, pew, aszDelayName, CTRL_DELAY_TYPE );
    pew->pwDelayType = pw;

    for( i = 0; i < 4; i++ ) {
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   pw = gtk_vscale_new_with_range(
				       0, i ? 127 : 117 * 127, i ? 100 : 1 ),
				   i, i + 1, 1, 2 );
	g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ),
			  pew );
	g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
	gtk_widget_set_size_request( pw, -1, 80 );
	gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
	pew->apwDelay[ i ] = pw;
	gtk_table_attach( GTK_TABLE( pwTable ),
			  gtk_label_new( aszDelayLabel[ i ] ),
			  i, i + 1, 2, 3, 0, 0, 0, 0 );
    }
    
    pwHbox = gtk_hbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pwWindow )->vbox ), pwHbox );

    /* Reverb */
    pwFrame = gtk_frame_new( "Reverb" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    pwTable = gtk_table_new( 3, 1, FALSE );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwTable );

    gtk_table_attach( GTK_TABLE( pwTable ), pw = gtk_option_menu_new(),
		      0, 1, 0, 1, 0, 0, 0, 0 );
    PopulateMenu( pw, pew, aszReverbName, CTRL_REVERB_TYPE );
    pew->pwReverbType = pw;
    
    gtk_table_attach_defaults( GTK_TABLE( pwTable ),
			       pw = gtk_vscale_new_with_range( 0, 127, 1 ),
			       0, 1, 1, 2 );
    g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ), pew );
    g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
    gtk_widget_set_size_request( pw, -1, 80 );
    gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
    pew->pwReverb = pw;

    /* Amp */
    pwFrame = gtk_frame_new( "Amp" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwFrame );
    pwTable = gtk_table_new( 3, 7, FALSE );
    gtk_table_set_col_spacings( GTK_TABLE( pwTable ), 4 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwTable );

    gtk_table_attach( GTK_TABLE( pwTable ), pw = gtk_option_menu_new(),
		      0, 7, 0, 1, 0, 0, 0, 0 );
    PopulateMenu( pw, pew, aszAmpName, CTRL_AMP_TYPE );
    pew->pwAmpType = pw;

    gtk_table_attach( GTK_TABLE( pwTable ),
		      pw = gtk_check_button_new_with_label( "Drive" ),
		      0, 1, 1, 2, 0, 0, 0, 0 );
    g_signal_connect( pw, "toggled", G_CALLBACK( DriveChange ), pew );
    pew->pwDrive = pw;

    for( i = 0; i < 6; i++ ) {
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   pw = gtk_vscale_new_with_range( 0, 127, 1 ),
				   i + 1, i + 2, 1, 2 );
	g_signal_connect( pw, "value-changed", G_CALLBACK( EditorChange ),
			  pew );
	g_signal_connect( pw, "format-value", G_CALLBACK( FormatValue ), pew );
	gtk_widget_set_size_request( pw, -1, 80 );
	gtk_range_set_inverted( GTK_RANGE( pw ), TRUE );
	pew->apwAmp[ i ] = pw;
	gtk_table_attach( GTK_TABLE( pwTable ),
			  gtk_label_new( aszAmpLabel[ i ] ),
			  i + 1, i + 2, 2, 3, 0, 0, 0, 0 );
    }

    pwVbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwVbox );

    /* Cabinet */
    pwFrame = gtk_frame_new( "Cabinet" );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwFrame );
    gtk_container_add( GTK_CONTAINER( pwFrame ),
		       pwAlign = gtk_alignment_new( 0.5, 0, 0, 0 ) );
    gtk_container_add( GTK_CONTAINER( pwAlign ), pw = gtk_option_menu_new() );
    PopulateMenu( pw, pew, aszCabinetName, CTRL_CABINET_TYPE );
    pew->pwCabinetType = pw;

    /* Effects assign */
    pwFrame = gtk_frame_new( "Effects assign" );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwFrame );
    gtk_container_add( GTK_CONTAINER( pwFrame ),
		       pwAlign = gtk_alignment_new( 0.5, 0, 0, 0 ) );
    gtk_container_add( GTK_CONTAINER( pwAlign ), pw = gtk_option_menu_new() );
    PopulateMenu( pw, pew, aszEffectsAssignName, CTRL_FX_ASSIGN );
    pew->pwEffectsAssign = pw;

    /* Name */
    pwFrame = gtk_frame_new( "Name" );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwFrame );
    gtk_container_add( GTK_CONTAINER( pwFrame ),
		       pw = gtk_entry_new() );
    gtk_entry_set_max_length( GTK_ENTRY( pw ), PARM_PRESET_LEN );
    g_signal_connect( pw, "changed", G_CALLBACK( NameChange ), pew );
    pew->pwName = pw;

    UpdateEditorWindow( pwWindow, achPreset, pva );

    gtk_widget_show_all( pwWindow );
    
    return pwWindow;
}

static GtkWidget *CreateLogWindow( vamp *pva ) {
    
    GtkWidget *pwWindow, *pw, *pwScrolled;
    char *pch;
    
    pwWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    pva->pwLog = pwWindow;
    g_object_add_weak_pointer( G_OBJECT( pwWindow ),
			       (gpointer *) &pva->pwLog );
    
    pch = g_strdup_printf( "Free V-AMP - Log (%s)", pva->szDevice );
    gtk_window_set_title( GTK_WINDOW( pwWindow ), pch );
    g_free( pch );

    gtk_window_set_default_size( GTK_WINDOW( pwWindow ), 400, 300 );
    gtk_window_add_accel_group( GTK_WINDOW( pwWindow ), pva->pag );

    pva->plsLog = gtk_list_store_new( 4, G_TYPE_STRING, G_TYPE_STRING,
				      G_TYPE_STRING, G_TYPE_STRING );
    g_object_add_weak_pointer( G_OBJECT( pva->plsLog ),
			       (gpointer *) &pva->plsLog );
			       
    pwScrolled = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pwScrolled ),
				    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_container_add( GTK_CONTAINER( pwWindow ), pwScrolled );
    
    pva->pwLogList = pw =
	gtk_tree_view_new_with_model( GTK_TREE_MODEL( pva->plsLog ) );
    g_object_unref( pva->plsLog );

    gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( pw ),
						 -1, "Device",
						 gtk_cell_renderer_text_new(),
						 "text", 0, NULL );
    gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( pw ),
						 -1, "Channel",
						 gtk_cell_renderer_text_new(),
						 "text", 1, NULL );
    gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( pw ),
						 -1, "Status",
						 gtk_cell_renderer_text_new(),
						 "text", 2, NULL );
    gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( pw ),
						 -1, "Description",
						 gtk_cell_renderer_text_new(),
						 "text", 3, NULL );
    
    gtk_container_add( GTK_CONTAINER( pwScrolled ), pw );
    
    gtk_widget_show_all( pwWindow );

    return pwWindow;
}

static void UpdateListWindow( vamp *pva ) {

    int i;

    for( i = 0; i < NUM_PRESETS; i++ )
	gtk_label_set_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN(
	    pva->plw->apw[ i ] ) ) ), PresetName( pva->achPreset[ i ] ) );
}

static void UpdateListWindowTitle( vamp *pva ) {

    char *pch, *pchBase;

    if( pva->szFileName )
	pchBase = g_path_get_basename( pva->szFileName );
    else
	pchBase = "No file";
    pch = g_strdup_printf( "Free V-AMP - %s (%s)", pchBase, pva->szDevice );
    
    gtk_window_set_title( GTK_WINDOW( pva->pwList ), pch );
    
    if( pva->szFileName )
	g_free( pchBase );
    g_free( pch );
}

static void SetFileName( vamp *pva, const char *sz ) {

    if( pva->szFileName )
	g_free( pva->szFileName );

    pva->szFileName = g_strdup( sz );

    if( pva->pwList )
	UpdateListWindowTitle( pva );
}

static void ActivatePreset( GtkWidget *pw, vamp *pva ) {

    char *pch = g_object_get_data( G_OBJECT( pw ), "preset" );
    int iProgram, iProgramOld;
    
    if( !pch )
	return;

    iProgram = (char (*)[ NUM_PARMS ]) pch - pva->achPreset;
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) ) ) {
	/* open editor window */
	memcpy( pva->achPreset[ PRESET_CURRENT ], pch, NUM_PARMS );
	if( pva->pwEditor ) {
	    gtk_window_present( GTK_WINDOW( pva->pwEditor ) );
	    UpdateEditorWindow( pva->pwEditor, pch, pva );
	} else {
	    pva->pwEditor = CreateEditorWindow( pva, pch );
	    g_object_add_weak_pointer( G_OBJECT( pva->pwEditor ),
				       (gpointer *) &pva->pwEditor );
	}
    }
    
    if( pva->iProgram != iProgram &&
	gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) ) ) {
	/* changing to a new program */
	iProgramOld = pva->iProgram;
	pva->iProgram = iProgram;
	if( iProgramOld != PRESET_CURRENT )
	    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
		pva->plw->apw[ iProgramOld ] ), FALSE );

	ProgramChange( pva, iProgram );
    } else if( pva->iProgram == iProgram &&
	       !gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pw ) ) )
	/* attempting to deselect current preset; refuse */
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pw ), TRUE );
}

static int ReadPresetsFile( vamp *pva, GtkWindow *pw, const char *pch ) {

    int h, c;
    char achPreset[ NUM_PRESETS ][ NUM_PARMS ];
    struct stat st;

    if( ( h = open( pch, O_RDONLY ) ) < 0 ) {
	Message( pw, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s", pch,
		 g_strerror( errno ) );
	return -1;
    }

    if( fstat( h, &st ) < 0 ) {
	Message( pw, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s", pch,
		 g_strerror( errno ) );
	return -1;
    }

    if( st.st_size != sizeof achPreset ) {
	Message( pw, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
		 "%s: not a valid V-AMP presets file", pch );
	return -1;
    }
    
    if( ( c = read( h, achPreset, sizeof achPreset ) ) != sizeof achPreset ) {
	Message( pw, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s", pch,
		 g_strerror( errno ) );
	return -1;
    }

    if( pva->h < 0 ||
	Message( pw, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		 "Are you sure you want to overwrite all device presets "
		 "with the contents of file %s?\n(Please note that you "
		 "must enter MIDI mode on the V-AMP before continuing.)",
		 pch ) == GTK_RESPONSE_YES ) {
	memcpy( pva->achPreset, achPreset, sizeof achPreset );

	WriteAllPresets( pva, achPreset );

	SetFileName( pva, pch );
	
	return 0;
    }

    return -1;
}

static void Refresh( vamp *pva, guint n, GtkWidget *pwItem );

typedef enum _aboutresponse {
    RESPONSE_COPYING = 1, RESPONSE_WARRANTY
} aboutresponse;

static void AboutConditions( char *szTitle, char *sz ) {

    GtkWidget *pw, *pwText, *pwScrolled;
    GtkTextBuffer *ptb;
    GtkTextIter i;
    PangoFontDescription *pfd;
    
    pw = gtk_dialog_new_with_buttons( szTitle, NULL, 0, GTK_STOCK_CLOSE,
				      GTK_RESPONSE_ACCEPT, NULL );
    gtk_dialog_set_default_response( GTK_DIALOG( pw ), GTK_RESPONSE_ACCEPT );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       pwScrolled = gtk_scrolled_window_new( NULL, NULL ) );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pwScrolled ),
				    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
    gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( pwScrolled ),
					 GTK_SHADOW_IN );
    gtk_container_add( GTK_CONTAINER( pwScrolled ),
		       pwText = gtk_text_view_new() );
    ptb = gtk_text_view_get_buffer( GTK_TEXT_VIEW( pwText ) );
    gtk_text_view_set_editable( GTK_TEXT_VIEW( pwText ), FALSE );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW( pwText ), FALSE );
    pfd = pango_font_description_new();
    pango_font_description_set_family_static( pfd, "fixed" );
    gtk_widget_modify_font( pwText, pfd );
    pango_font_description_free( pfd );

    gtk_window_set_default_size( GTK_WINDOW( pw ), -1, 400 );

    gtk_widget_show_all( pw );
    
    gtk_text_buffer_get_start_iter( ptb, &i );
    gtk_text_buffer_insert( ptb, &i, sz, -1 );

    gtk_dialog_run( GTK_DIALOG( pw ) );
    gtk_widget_destroy( pw );
}

static void AboutResponse( GtkWidget *pw, aboutresponse ar ) {

    switch( ar ) {
    case RESPONSE_COPYING:
	AboutConditions( "Free V-AMP - Copying conditions", szCopying );
	break;

    case RESPONSE_WARRANTY:
	AboutConditions( "Free V-AMP - Warranty", szWarranty );
	break;
	
    default:
	gtk_widget_destroy( pw );
    }
}

static void About( gpointer *p, guint n, GtkWidget *pwItem ) {

    static GtkWidget *pw, *pwHbox, *pwLabel;
    PangoAttrList *pal;
    PangoAttribute *pa;

    if( pw ) {
	gtk_window_present( GTK_WINDOW( pw ) );
	return;
    }

    pw = gtk_dialog_new_with_buttons( "About Free V-AMP", NULL, 0,
				      "Copying conditions", RESPONSE_COPYING,
				      "Warranty", RESPONSE_WARRANTY,
				      GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
				      NULL );

    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       pwHbox = gtk_hbox_new( FALSE, 16 ) );
    gtk_box_pack_start( GTK_BOX( pwHbox ),
			gtk_image_new_from_stock( szIcon,
						  GTK_ICON_SIZE_DIALOG ),
			FALSE, FALSE, 0 );
    pwLabel = gtk_label_new( "Free V-AMP" );
    pal = pango_attr_list_new();
    pa = pango_attr_size_new( 48 * PANGO_SCALE );
    pa->start_index = 0;
    pa->end_index = G_MAXINT;
    pango_attr_list_insert( pal, pa );
    pa = pango_attr_weight_new( PANGO_WEIGHT_BOLD );
    pa->start_index = 0;
    pa->end_index = G_MAXINT;
    pango_attr_list_insert( pal, pa );
    pa = pango_attr_family_new( "serif" );
    pa->start_index = 0;
    pa->end_index = G_MAXINT;
    pango_attr_list_insert( pal, pa );
    gtk_label_set_attributes( GTK_LABEL( pwLabel ), pal );
    pango_attr_list_unref( pal );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwLabel );

    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       gtk_label_new( "version " VERSION ) );
    
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       gtk_label_new( "\xC2\xA9 Copyright Gary Wong, 2002" ) );

    pwLabel = gtk_label_new( "Free V-AMP is free software, covered "
			     "by the GNU General Public License "
			     "version 2, and you are welcome to "
			     "change it and/or distribute copies of "
			     "it under certain conditions.  There is "
			     "absolutely no warranty for Free "
			     "V-AMP." );
    pal = pango_attr_list_new();
    pa = pango_attr_scale_new( PANGO_SCALE_SMALL );
    pa->start_index = 0;
    pa->end_index = G_MAXINT;
    pango_attr_list_insert( pal, pa );
    gtk_label_set_attributes( GTK_LABEL( pwLabel ), pal );
    pango_attr_list_unref( pal );
    gtk_label_set_line_wrap( GTK_LABEL( pwLabel ), TRUE );
    gtk_label_set_justify( GTK_LABEL( pwLabel ), GTK_JUSTIFY_CENTER );
    gtk_box_pack_end( GTK_BOX( GTK_DIALOG( pw )->vbox ), pwLabel,
		      TRUE, TRUE, 8 );
    
    gtk_dialog_set_default_response( GTK_DIALOG( pw ), GTK_RESPONSE_ACCEPT );
    g_signal_connect( pw, "response", G_CALLBACK( AboutResponse ), NULL );

    g_object_add_weak_pointer( G_OBJECT( pw ), (gpointer *) &pw );
    
    gtk_widget_show_all( pw );
}

static void Copy( vamp *pva, guint n, GtkWidget *pwItem ) {

    pva->fClipboard = TRUE;
    memcpy( pva->achClipboard, pva->achPreset[ PRESET_CURRENT ],
	    sizeof( pva->achClipboard ) );
    gtk_widget_set_sensitive( pva->plw->pwPaste, TRUE );
}

static void Open( vamp *pva, guint n, GtkWidget *pwItem ) {

    GtkWidget *pw;
    const char *pch;
    
    pw = gtk_file_selection_new( "Free V-AMP - Open" );
    
    gtk_window_set_transient_for( GTK_WINDOW( pw ),
				  GTK_WINDOW( pva->pwList ) );
    gtk_widget_show_all( pw );

    if( gtk_dialog_run( GTK_DIALOG( pw ) ) == GTK_RESPONSE_OK ) {
	pch = gtk_file_selection_get_filename( GTK_FILE_SELECTION( pw ) );

	if( !ReadPresetsFile( pva, GTK_WINDOW( pw ), pch ) )
	    Refresh( pva, n, pwItem );
    }
    
    gtk_widget_destroy( pw );
}

static void Paste( vamp *pva, guint n, GtkWidget *pwItem ) {

    char *achPreset = pva->achPreset[ (int) pva->iProgram ];
	    
    memcpy( achPreset, pva->achClipboard, sizeof( pva->achClipboard ) );

    WritePreset( pva, pva->iProgram, pva->achClipboard );
    RequestPreset( pva, GTK_WINDOW( pva->pwList ), pva->iProgram, achPreset );
    
    gtk_label_set_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN(
        pva->plw->apw[ (int) pva->iProgram ] ) ) ), PresetName( achPreset ) );
    
    if( pva->pwEditor )
	UpdateEditorWindow( pva->pwEditor, achPreset, pva );
}

static void LogToggled( GtkWidget *pw, GtkWidget *pwBox ) {

    gtk_widget_set_sensitive( pwBox, gtk_toggle_button_get_active(
	GTK_TOGGLE_BUTTON( pw ) ) );
}

static void Preferences( vamp *pva, guint n, GtkWidget *pwItem ) {

    GtkWidget *pwWindow, *pwFrame, *pwLog, *pwLogMax, *pwLogScroll, *pwPrint,
	*pwVbox, *pwHbox, *pwA4, *pwLetter;
    char *pchRC;
    FILE *pf;
    
    pwWindow = gtk_dialog_new_with_buttons( "Free V-AMP - Preferences",
					    GTK_WINDOW( pva->pwList ),
					    GTK_DIALOG_MODAL, GTK_STOCK_OK,
					    GTK_RESPONSE_ACCEPT,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_REJECT, NULL);
    gtk_dialog_set_default_response( GTK_DIALOG( pwWindow ),
				     GTK_RESPONSE_ACCEPT );
    pwFrame = gtk_frame_new( NULL );
    gtk_container_set_border_width( GTK_CONTAINER( pwFrame ), 8 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pwWindow )->vbox ),
		       pwFrame );
    pwLog = gtk_check_button_new_with_label( "Log MIDI events" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pwLog ), pref.fLog );
    gtk_frame_set_label_widget( GTK_FRAME( pwFrame ), pwLog );

    pwVbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_set_border_width( GTK_CONTAINER( pwVbox ), 8 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwVbox );

    pwHbox = gtk_hbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwHbox );

    gtk_container_add( GTK_CONTAINER( pwHbox ),
		       gtk_label_new( "Maximum log length:" ) );
    
    pwLogMax = gtk_spin_button_new_with_range( 10, 5000, 1 );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( pwLogMax ), pref.cLogMax );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwLogMax );
    
    pwLogScroll = gtk_check_button_new_with_label( "Scroll to show new "
						   "events" );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pwLogScroll ),
				  pref.fLogScroll );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwLogScroll );
    
    g_signal_connect( pwLog, "toggled", G_CALLBACK( LogToggled ), pwVbox );
    LogToggled( pwLog, pwVbox );
    
    pwFrame = gtk_frame_new( "Printing" );
    gtk_container_set_border_width( GTK_CONTAINER( pwFrame ), 8 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pwWindow )->vbox ),
		       pwFrame );
    
    pwVbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_set_border_width( GTK_CONTAINER( pwVbox ), 8 );
    gtk_container_add( GTK_CONTAINER( pwFrame ), pwVbox );

    pwHbox = gtk_hbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwHbox );
    gtk_container_add( GTK_CONTAINER( pwHbox ),
		       gtk_label_new( "Print command:" ) );
    pwPrint = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY( pwPrint ), pref.szPrint );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwPrint );

    pwA4 = gtk_radio_button_new_with_label( NULL, "ISO A4" );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwA4 );
    
    pwLetter = gtk_radio_button_new_with_label_from_widget(
	GTK_RADIO_BUTTON( pwA4 ), "U.S. Letter" );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwLetter );

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pwLetter ),
				  pref.cxPaper == PAPER_US_LETTER_X &&
				  pref.cyPaper == PAPER_US_LETTER_Y );
    
    gtk_widget_show_all( pwWindow );
    
    if( gtk_dialog_run( GTK_DIALOG( pwWindow ) ) == GTK_RESPONSE_ACCEPT ) {
	pref.fLog = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pwLog ) );
	
	if( pref.fLog && !pva->pwLog )
	    CreateLogWindow( pva );
	else if( !pref.fLog && pva->pwLog )
	    gtk_widget_destroy( pva->pwLog );

	pref.cLogMax = gtk_spin_button_get_value( GTK_SPIN_BUTTON(
	    pwLogMax ) );
	pref.fLogScroll = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(
	    pwLogScroll ) );
	
	LogTrimScroll( pva );

	g_free( pref.szPrint );
	pref.szPrint = gtk_editable_get_chars( GTK_EDITABLE( pwPrint ),
					       0, -1 );

	pref.cxPaper = gtk_toggle_button_get_active(
	    GTK_TOGGLE_BUTTON( pwLetter ) ) ?
	    PAPER_US_LETTER_X : PAPER_ISO_A4_X;
	pref.cyPaper = gtk_toggle_button_get_active(
	    GTK_TOGGLE_BUTTON( pwLetter ) ) ?
	    PAPER_US_LETTER_Y : PAPER_ISO_A4_Y;
	
	pchRC = g_build_filename( g_get_home_dir(), RCFILE, NULL );
	pf = fopen( pchRC, "w" );
	g_free( pchRC );
	fprintf( pf,
		 "log %c\n"
		 "log-max %d\n"
		 "log-scroll %c\n"
		 "print-command %s\n"
		 "print-paper-x %d\n"
		 "print-paper-y %d\n",
		 pref.fLog ? 'y' : 'n',
		 pref.cLogMax,
		 pref.fLogScroll ? 'y' : 'n',
		 pref.szPrint,
		 pref.cxPaper,
		 pref.cyPaper );
	fclose( pf );
    }

    gtk_widget_destroy( pwWindow );
}

static void PostScriptEscape( FILE *pf, char *pch ) {

    while( *pch ) {
	switch( *pch ) {
	case '\\':
	    fputs( "\\\\", pf );
	    break;
	case '(':
	    fputs( "\\(", pf );
	    break;
	case ')':
	    fputs( "\\)", pf );
	    break;
	default:
	    if( (unsigned char) *pch >= 0x80 )
		fprintf( pf, "\\%030o", *pch );
	    else
		putc( *pch, pf );
	    break;
	}
	pch++;
    }
}

static void Print( vamp *pva, guint nIgnore, GtkWidget *pwItem ) {

    static int acxColumn[ 28 ] = {
	88, /* name */
	12,
	37, /* pre FX */
	12,
	12,
	12,
	12,
	40, /* modulation */
	15,
	12,
	12,
	12,
	30, /* delay */
	15,
	12,
	12,
	12,
	45, /* reverb */
	12,
	55, /* amp */
	7,
	12,
	12,
	12,
	12,
	12,
	12,
	65 /* cabinet */
    };
    static int anLineColumns[] = {
	2, 7, 12, 17, 19, 27, 28, -1
    };
    static char *aszCompressorRatio[ 8 ] = { "1.2", "1.4", "2", "2.5",
					     "3", "4.5", "6", "\x80" };
    FILE *pf;
    time_t t;
    char *sz, *pch;
    int i, n, x, *pnLine;
    
    if( pref.cxPaper < PRINT_AREA_X || pref.cyPaper < PRINT_AREA_Y ) {
	Message( GTK_WINDOW( pva->pwList ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "The paper size selected is too small." );
	return;	
    }
    
    if( !( pf = popen( pref.szPrint, "w" ) ) ) {
	Message( GTK_WINDOW( pva->pwList ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pref.szPrint,
		 g_strerror( errno ) );
	return;
    }

    fputs( "%!PS-Adobe-3.0\n", pf );
    
    fputs( "%%Title: (", pf );
    PostScriptEscape( pf, pva->szFileName ? pva->szFileName :
		      "Free V-AMP (Untitled)" );
    fputs( ")\n", pf );

    fprintf( pf, "%%%%BoundingBox: %d %d %d %d\n",
	     ( pref.cxPaper - PRINT_AREA_X ) >> 1,
	     ( pref.cyPaper - PRINT_AREA_Y ) >> 1,
	     ( pref.cxPaper + PRINT_AREA_X ) >> 1,
	     ( pref.cyPaper + PRINT_AREA_Y ) >> 1 );

    time( &t );
    sz = ctime( &t );
    if( ( pch = strchr( sz, '\n' ) ) )
	*pch = 0;
    fprintf( pf, "%%%%CreationDate: (%s)\n", sz );
    
    fputs( "%%Creator: (Free V-AMP " VERSION ")\n"
	   "%%DocumentData: Clean7Bit\n"
	   "%%DocumentNeededResources: font Helvetica Symbol\n"
	   "%%LanguageLevel: 1\n"
	   "%%Orientation: Landscape\n"
	   "%%Pages: 3\n"
	   "%%PageOrder: Ascend\n"
	   "%%EndComments\n"
	   "%%BeginSetup\n"
	   "%%IncludeResource: font Helvetica\n"
	   "%%IncludeResource: font Symbol\n"
	   "%%EndSetup\n", pf );

    fputs( "%%BeginProlog\n"
	   "/Helvetica findfont dup length dict begin\n"
	   "{ 1 index /FID ne {def} {pop pop} ifelse } forall\n"
	   "/Helvetica findfont /CharStrings get dup length 1 add dict copy\n"
	   "dup /infinity /Symbol findfont /CharStrings get /infinity get \n"
	   "put /CharStrings exch def\n"
	   "/Helvetica findfont /Encoding get dup length array copy\n"
	   "dup 128 /infinity put /Encoding exch def currentdict end\n"
	   "/Helvetica-infinity exch definefont pop\n\n", pf );

    fputs( "/presetnames [\n", pf );
    for( i = 0; i < NUM_PRESETS; i++ ) {
	putc( '(', pf );
	PostScriptEscape( pf, PresetName( pva->achPreset[ i ] ) );
	putc( ')', pf );
	putc( '\n', pf );
    }
    fputs( "] def\n\n", pf );

    fputs( "/settings [\n", pf );
    for( i = 0; i < NUM_PRESETS; i++ ) {
	if( pva->achPreset[ i ][ PARM_NOISE_GATE ] )
	    fprintf( pf, "[ (%d) ",
		     3 * pva->achPreset[ i ][ PARM_NOISE_GATE ] - 93 );
	else
	    fputs( "[ (-\x80) ", pf );
	
	switch( pva->achPreset[ i ][ PARM_PRE_FX_TYPE ] ) {
	case 1:
	    /* compressor */
	    fprintf( pf, "(Compressor) () (%s:1) () ()\n", aszCompressorRatio[
		pva->achPreset[ i ][ PARM_PRE_FX_1 ] & 7 ] );
	    break;
	    
	case 2:
	    /* auto-wah */
	    fprintf( pf, "(Auto wah) (%d) (%d) (%d) (%d)\n",
		     AutoWahSpeed( pva->achPreset[ i ][ PARM_PRE_FX_1 ] ),
		     pva->achPreset[ i ][ PARM_PRE_FX_2 ],
		     pva->achPreset[ i ][ PARM_PRE_FX_3 ],
		     pva->achPreset[ i ][ PARM_PRE_FX_4 ] );
	    break;

	default:
	    /* none */
	    fputs( "() () () () ()\n", pf );
	}

	n = pva->achPreset[ i ][ PARM_POST_FX_1 ];
	switch( pva->achPreset[ i ][ PARM_POST_FX_MODE ] ) {
	case POST_FX_ROTARY:
	case POST_FX_PHASER:
	    n = 0.59 * n * n + 0.5;
	    break;
	case POST_FX_TREMOLO:
	    n = 0.62 * n * n + 0.5;
	    break;
	default:
	    n = 0.31 * n * n + 0.5;
	}
	if( pva->achPreset[ i ][ PARM_POST_FX ] )
	    fprintf( pf, " (%s) (%d) (%d) (%d) (%d) ",
		     ModulationName( pva->achPreset[ i ]
				     [ PARM_POST_FX_MODE ] ), n,
		     pva->achPreset[ i ][ PARM_POST_FX_2 ],
		     pva->achPreset[ i ][ PARM_POST_FX_3 ],
		     pva->achPreset[ i ][ PARM_POST_FX ] );
	else
	    fputs( " () () () () () ", pf );

	if( pva->achPreset[ i ][ PARM_DELAY_MIX ] )
	    fprintf( pf, "(%s) (%d) (%d) (%d) (%d)\n",
		     DelayName( pva->achPreset[ i ][ PARM_DELAY_TYPE ] ),
		     (int) ( ( pva->achPreset[ i ][ PARM_DELAY_TIME_HI ] * 128
			       + pva->achPreset[ i ][ PARM_DELAY_TIME_LO ] ) *
			     0.128 + 0.5 ),
		     pva->achPreset[ i ][ PARM_DELAY_SPREAD ],
		     pva->achPreset[ i ][ PARM_DELAY_FEEDBACK ],
		     pva->achPreset[ i ][ PARM_DELAY_MIX ] );
	else
	    fputs( "() () () () ()\n", pf );

	if( pva->achPreset[ i ][ PARM_REVERB_MIX ] )
	    fprintf( pf, " (%s) (%d) ",
		     ReverbName( pva->achPreset[ i ][ PARM_REVERB_TYPE ] ),
		     pva->achPreset[ i ][ PARM_REVERB_MIX ] );
	else
	    fputs( " () () ", pf );
	    
	fprintf( pf, "(%s) (%s) (%d) (%d) (%d) (%d) (%d) (%d)\n",
		 AmpName( pva->achPreset[ i ][ PARM_AMP_TYPE ] ),
		 pva->achPreset[ i ][ PARM_DRIVE ] ? "\267" : "",
		 pva->achPreset[ i ][ PARM_AMP_GAIN ],
		 pva->achPreset[ i ][ PARM_AMP_BASS ],
		 pva->achPreset[ i ][ PARM_AMP_MID ],
		 pva->achPreset[ i ][ PARM_AMP_TREBLE ],
		 pva->achPreset[ i ][ PARM_AMP_PRESENCE ],
		 pva->achPreset[ i ][ PARM_AMP_VOL ] );
	fprintf( pf, " (%s) (%s)]\n",
		 CabinetName( pva->achPreset[ i ][ PARM_CABINET_TYPE ] ),
		 EffectsAssignName( pva->achPreset[ i ]
				    [ PARM_FX_MIX_ASSIGN ] ) );
    }
    fputs( "] def\n\n", pf );

    fputs( "/columntitle [ (Noise gate) (Pre Effect) () () () () \n"
	   "  (Post Effect) () () () () (Delay) () () () () (Reverb) ()\n"
	   "  (Amplifier) () () () () () () () (Cabinet)\n"
	   "  (Effects Assign) ] def\n\n", pf );
    fputs( "/columnsubtitle [ ((dB)) () ((ms)) (D) (O) (F) \n"
	   "  () ((ms)) (D) (F) (M) () ((ms)) (S) (F) (M) () (M)\n"
	   "  () (Drive) (G) (B) (M) (T) (P) (V) ()\n"
	   "  () ] def\n\n", pf );
    
    fputs( "/columnpos [\n  ", pf );
    for( i = 0, x = 0; i < 28; i++ )
	fprintf( pf, "%d ", ( x += acxColumn[ i ] ) +
		 ( i < 27 && acxColumn[ i + 1 ] <= 15 ?
		   acxColumn[ i + 1 ] - 3 : 0 ) );
    fputs( "\n] def\n\n", pf );

    fputs( "/columnrightjust [\n  ", pf );
    for( i = 0; i < 28; i++ )
	fprintf( pf, "%s ", i < 27 && acxColumn[ i + 1 ] <= 15 ?
		 "true" : "false" );
    fputs( "\n] def\n", pf );
    
    fputs( "%%EndProlog\n\n", pf );
    
    fputs( "%%Page: 1 1\n"
	   "%%BeginPageSetup\n", pf );
    fprintf( pf, "/pageobj save def 90 rotate %d %d translate 1 setlinecap\n",
	     ( pref.cyPaper - PRINT_AREA_Y ) >> 1, 
	     -( ( pref.cxPaper + PRINT_AREA_X ) >> 1 ) );
    fputs( "%%EndPageSetup\n", pf );

    fputs( "/Helvetica findfont 9 scalefont setfont\n", pf );
    
    fputs( "0 1 124 { "
	   "dup dup "
	   "5 mod 123 mul 95 add "
	   "exch 5 idiv -17 mul 413 add "
	   "moveto\n "
	   "presetnames exch get "
	   "dup "
	   "stringwidth pop -2 div 0 rmoveto "
	   "show "
	   "} for\n", pf );

    fputs( "1 1 25 { dup 29 exch -17 mul 430 add moveto 2 string cvs\n "
	   "dup stringwidth pop neg 0 rmoveto show } for\n", pf );
    fputs( "0 1 4 { dup 123 mul 95 add 435 moveto [ (A) (B) (C) (D) (E) ]\n "
	   "exch get dup stringwidth pop -2 div 0 rmoveto show } for\n", pf );
    
    fputs( "0 17 425 { 0 exch moveto 648 0 rlineto stroke } for\n", pf );
    fputs( "33 123 648 { 0 moveto 0 451 rlineto stroke } for\n", pf );
    fputs( "0 0 moveto 0 425 lineto stroke\n", pf );
    fputs( "33 451 moveto 648 451 lineto stroke\n", pf );
    
    fputs( "pageobj restore showpage\n\n", pf );

    fputs( "%%Page: 2 2\n"
	   "%%BeginPageSetup\n", pf );
    fprintf( pf, "/pageobj save def 90 rotate %d %d translate 0.97 1 scale "
	     "1 setlinecap\n",
	     ( pref.cyPaper - PRINT_AREA_Y ) >> 1, 
	     -( ( pref.cxPaper + PRINT_AREA_X ) >> 1 ) );
    fputs( "%%EndPageSetup\n", pf );

    fputs( "/Helvetica-infinity findfont 6 scalefont setfont\n", pf );

    fputs( "2 444 moveto (Name) show\n", pf );
    fputs( "0 1 27 { dup dup columnpos exch get 444 moveto\n"
	   /* stack: col col */
	   "  columntitle exch get exch\n"
	   /* stack: title col */
	   "  columnrightjust exch get\n"
	   /* stack: title rightjust */
	   "  { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: title */
	   "show } for\n", pf );
    fputs( "0 1 27 { dup dup columnpos exch get 438 moveto\n"
	   /* stack: col col */
	   "  columnsubtitle exch get exch\n"
	   /* stack: title col */
	   "  columnrightjust exch get\n"
	   /* stack: title rightjust */
	   "  { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: title */
	   "show } for\n", pf );
    
    fputs( "0 1 64 {\n"
	   /* stack: row */
	   "  dup 8 exch -6.7 mul 430 add moveto\n"
	   /* stack: row */
	   "  dup presetnames exch get show\n"
	   /* stack: row */
	   "  0 1 27 {\n"
	   /* stack: row col */
	   "    dup dup columnpos exch get 3 index -6.7 mul 430 add moveto\n"
	   /* stack: row col col */
	   "    settings 3 index get exch get exch\n"
	   /* stack: row text col */
	   "    columnrightjust exch get\n"
	   /* stack: row text rightjust */
	   "    { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: row text */
	   "    show \n" 
	   /* stack: row */
	   "  } for\n"
	   "  pop\n"
	   "} for\n", pf );

    fputs( "1 1 13 {\n"
	   /* stack: bank */
	   "  dup 7 exch -33.5 mul 450.1 add moveto\n"
	   /* stack: bank */
	   "  2 string cvs dup stringwidth pop neg 0 rmoveto show\n"
	   "} for\n", pf );
    
    fputs( "0 0 moveto 668 0 lineto 668 451 lineto 0 451 lineto "
	   "closepath stroke\n", pf );
    fputs( "0 435.5 moveto 668 435.5 lineto stroke\n", pf );
    
    fputs( "0.5 setlinewidth\n", pf );
    fputs( "[ ", pf );
    for( i = 0, pnLine = anLineColumns, x = 0; ; i++ ) {
	if( i == *pnLine ) {
	    fprintf( pf, "%d.5 ", x - 2 );
	    if( *++pnLine < 0 )
		break;
	}

	x += acxColumn[ i ];
    }
    fputs( "] { dup 0 moveto 451 lineto stroke } forall\n", pf );
    fputs( "0 33.5 436 { dup 0 exch moveto 668 exch lineto stroke } for\n",
	   pf );
    fputs( "pageobj restore showpage\n\n", pf );

    fputs( "%%Page: 3 3\n"
	   "%%BeginPageSetup\n", pf );
    fprintf( pf, "/pageobj save def 90 rotate %d %d translate 0.97 1 scale "
	     "1 setlinecap\n",
	     ( pref.cyPaper - PRINT_AREA_Y ) >> 1, 
	     -( ( pref.cxPaper + PRINT_AREA_X ) >> 1 ) );
    fputs( "%%EndPageSetup\n", pf );

    fputs( "/Helvetica-infinity findfont 6 scalefont setfont\n", pf );

    fputs( "2 444 moveto (Name) show\n", pf );
    fputs( "0 1 27 { dup dup columnpos exch get 444 moveto\n"
	   /* stack: col col */
	   "  columntitle exch get exch\n"
	   /* stack: title col */
	   "  columnrightjust exch get\n"
	   /* stack: title rightjust */
	   "  { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: title */
	   "show } for\n", pf );
    fputs( "0 1 27 { dup dup columnpos exch get 438 moveto\n"
	   /* stack: col col */
	   "  columnsubtitle exch get exch\n"
	   /* stack: title col */
	   "  columnrightjust exch get\n"
	   /* stack: title rightjust */
	   "  { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: title */
	   "show } for\n", pf );
    
    fputs( "65 1 124 {\n"
	   /* stack: row */
	   "  dup 8 exch -6.7 mul 865.5 add moveto\n"
	   /* stack: row */
	   "  dup presetnames exch get show\n"
	   /* stack: row */
	   "  0 1 27 {\n"
	   /* stack: row col */
	   "    dup dup columnpos exch get 3 index -6.7 mul 865.5 add moveto\n"
	   /* stack: row col col */
	   "    settings 3 index get exch get exch\n"
	   /* stack: row text col */
	   "    columnrightjust exch get\n"
	   /* stack: row text rightjust */
	   "    { dup stringwidth pop neg 0 rmoveto } if\n"
	   /* stack: row text */
	   "    show \n" 
	   /* stack: row */
	   "  } for\n"
	   "  pop\n"
	   "} for\n", pf );

    fputs( "14 1 25 {\n"
	   /* stack: bank */
	   "  dup 7 exch -33.5 mul 885.6 add moveto\n"
	   /* stack: bank */
	   "  2 string cvs dup stringwidth pop neg 0 rmoveto show\n"
	   "} for\n", pf );
    
    fputs( "0 33.5 moveto 668 33.5 lineto 668 451 lineto 0 451 lineto "
	   "closepath stroke\n", pf );
    fputs( "0 435.5 moveto 668 435.5 lineto stroke\n", pf );
    
    fputs( "0.5 setlinewidth\n", pf );
    fputs( "[ ", pf );
    for( i = 0, pnLine = anLineColumns, x = 0; ; i++ ) {
	if( i == *pnLine ) {
	    fprintf( pf, "%d.5 ", x - 2 );
	    if( *++pnLine < 0 )
		break;
	}

	x += acxColumn[ i ];
    }
    fputs( "] { dup 33.5 moveto 451 lineto stroke } forall\n", pf );
    fputs( "33.5 33.5 436 { dup 0 exch moveto 668 exch lineto stroke } for\n",
	   pf );
    fputs( "pageobj restore showpage\n"
	   "%%Trailer\n"
	   "%%EOF\n", pf );

    if( pclose( pf ) < 0 ) 
	Message( GTK_WINDOW( pva->pwList ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pref.szPrint,
		 g_strerror( errno ) );
}

static void Refresh( vamp *pva, guint n, GtkWidget *pwItem ) {
    
    RequestAllPresets( pva, GTK_WINDOW( pva->pwList ) );

    UpdateListWindow( pva );
	    
    if( pva->pwEditor )
	RequestAllControllers( pva );
}

static void SaveAs( vamp *pva, guint n, GtkWidget *pwItem );

static void Save( vamp *pva, guint n, GtkWidget *pwItem ) {

    int h;

    if( !pva->szFileName )
	return SaveAs( pva, n, pwItem );
    
    if( ( h = open( pva->szFileName, O_WRONLY | O_CREAT, 0666 ) ) < 0 ) {
	Message( GTK_WINDOW( pva->pwList ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pva->szFileName,
		 g_strerror( errno ) );
	return;
    }
	
    if( write( h, pva->achPreset, NUM_PRESETS * NUM_PARMS ) < 0 )
	Message( GTK_WINDOW( pva->pwList ), GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_CLOSE, "%s: %s", pva->szFileName,
		 g_strerror( errno ) );
    
    close( h );
}

static void SaveAs( vamp *pva, guint n, GtkWidget *pwItem ) {

    GtkWidget *pw;
    const char *pch;
    struct stat s;
    
    pw = gtk_file_selection_new( "Free V-AMP - Save" );
    
    gtk_window_set_transient_for( GTK_WINDOW( pw ),
				  GTK_WINDOW( pva->pwList ) );
    gtk_widget_show_all( pw );

    while( gtk_dialog_run( GTK_DIALOG( pw ) ) == GTK_RESPONSE_OK ) {
	pch = gtk_file_selection_get_filename( GTK_FILE_SELECTION( pw ) );

	if( !( stat( pch, &s ) ) &&
	    Message( GTK_WINDOW( pw ), GTK_MESSAGE_QUESTION,
		     GTK_BUTTONS_YES_NO,
		     "Are you sure you want to overwrite file %s?", pch ) !=
	    GTK_RESPONSE_YES )
	    continue;

	SetFileName( pva, pch );

	Save( pva, n, pwItem );
	
	break;
    }

    gtk_widget_destroy( pw );
}

static GtkWidget *CreateListWindow( vamp *pva ) {
    
    GtkWidget *pwWindow, *pwVbox, *pwMenu, *pw, *pwTable;
    GtkItemFactory *pif;
    listwindow *plw;
    char achLabel[ 3 ];
    int i;
    static GtkItemFactoryEntry aife[] = {
	{ "/_File", NULL, NULL, 0, "<Branch>" },
	{ "/_File/_Open...", "<control>O", Open, 0, "<StockItem>",
	  GTK_STOCK_OPEN },
	{ "/_File/_Save", "<control>S", Save, 0, "<StockItem>",
	  GTK_STOCK_SAVE },
	{ "/_File/_Save As...", NULL, SaveAs, 0, "<StockItem>",
	  GTK_STOCK_SAVE_AS },
	{ "/_File/-", NULL, NULL, 0, "<Separator>" },
	{ "/_File/_Print", "<control>P", Print, 0, "<StockItem>",
	  GTK_STOCK_PRINT },
	{ "/_File/-", NULL, NULL, 0, "<Separator>" },
	{ "/_File/_Refresh", "<control>R", Refresh, 0, "<StockItem>",
	  GTK_STOCK_REFRESH },
	{ "/_File/-", NULL, NULL, 0, "<Separator>" },
	{ "/_File/_Quit", "<control>Q", gtk_main_quit, 0, "<StockItem>",
	  GTK_STOCK_QUIT },
	{ "/_Edit", NULL, NULL, 0, "<Branch>" },
	{ "/_Edit/_Copy", "<control>C", Copy, 0, "<StockItem>",
	  GTK_STOCK_COPY },
	{ "/_Edit/_Paste", "<control>V", Paste, 0, "<StockItem>",
	  GTK_STOCK_PASTE },
	{ "/_Edit/-", NULL, NULL, 0, "<Separator>" },
	{ "/_Edit/P_references...", NULL, Preferences, 0, "<StockItem>",
	  GTK_STOCK_PREFERENCES },
	{ "/_Help", NULL, NULL, 0, "<Branch>" },
	{ "/_Help/About Free V-AMP", NULL, About, 0, "<StockItem>",
	  szIcon }
    };
    
    if( !( pva->plw = plw = malloc( sizeof( *plw ) ) ) )
	return NULL;
    
    plw->pva = pva;
    
    pva->pwList = pwWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_object_add_weak_pointer( G_OBJECT( pwWindow ),
			       (gpointer *) &pva->pwList );
    g_object_add_weak_pointer( G_OBJECT( pwWindow ),
			       (gpointer *) &pva->plw );
    g_signal_connect_swapped( pwWindow, "destroy", G_CALLBACK( free ), plw );

    pwVbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( pwWindow ), pwVbox );
    
    pva->pag = gtk_accel_group_new();
    pif = gtk_item_factory_new( GTK_TYPE_MENU_BAR, "<main>", pva->pag );
    gtk_item_factory_create_items( pif, sizeof( aife ) / sizeof( aife[ 0 ] ),
				   aife, pva );
    gtk_window_add_accel_group( GTK_WINDOW( pwWindow ), pva->pag );
    g_object_unref( pva->pag );
    gtk_box_pack_start( GTK_BOX( pwVbox ),
			pwMenu = gtk_item_factory_get_widget( pif, "<main>" ),
			FALSE, FALSE, 0 );
    gtk_widget_set_sensitive( plw->pwPaste = gtk_item_factory_get_item(
				  pif, "/Edit/Paste" ), FALSE );

    UpdateListWindowTitle( pva );

    pwTable = gtk_table_new( NUM_BANKS + 1, PRESETS_PER_BANK + 1, FALSE );
    gtk_container_add( GTK_CONTAINER( pwVbox ), pwTable );

    achLabel[ 1 ] = 0;
    for( i = 0; i < PRESETS_PER_BANK; i++ ) {
	achLabel[ 0 ] = 'A' + i;
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   gtk_label_new( achLabel ), i + 1, i + 2,
				   0, 1 );
    }
    
    for( i = 0; i < NUM_BANKS; i++ ) {
	sprintf( achLabel, "%d", i + 1 );
	gtk_table_attach_defaults( GTK_TABLE( pwTable ),
				   gtk_label_new( achLabel ), 0, 1,
				   i + 1, i + 2 );
    }
    
    for( i = 0; i < NUM_PRESETS; i++ ) {
	pw = gtk_toggle_button_new_with_label( PresetName(
	    pva->achPreset[ i ] ) );
	g_object_set_data( G_OBJECT( pw ), "preset", pva->achPreset[ i ] );
	g_signal_connect( pw, "clicked", G_CALLBACK( ActivatePreset ),
			  pva );
	gtk_table_attach_defaults( GTK_TABLE( pwTable ), pw,
				   i % PRESETS_PER_BANK + 1,
				   i % PRESETS_PER_BANK + 2,
				   i / PRESETS_PER_BANK + 1,
				   i / PRESETS_PER_BANK + 2 );
	plw->apw[ i ] = pw;
    }
	    
    gtk_widget_show_all( pwWindow );

    if( pref.fLog )
	CreateLogWindow( pva );
    
    return pwWindow;
}

static void LoadPreferences( void ) {

    char *pchRC, *pchData, *pch, *pchEnd, ch;
    char szKeyword[ 32 ];
    
    pchRC = g_build_filename( g_get_home_dir(), RCFILE, NULL );
    g_file_get_contents( pchRC, &pchData, NULL, NULL );
    g_free( pchRC );
    
    for( pch = pchData; pch; ) {
	sscanf( pch, "%31s", szKeyword );

	if( !strcmp( szKeyword, "log" ) && sscanf( pch, "log %c", &ch ) == 1 )
	    pref.fLog = toupper( ch ) == 'Y';
	else if( !strcmp( szKeyword, "log-max" ) &&
		 sscanf( pch, "log-max %d", &pref.cLogMax ) == 1 )
	    ;
	else if( !strcmp( szKeyword, "log-scroll" ) &&
		 sscanf( pch, "log-scroll %c", &ch ) == 1 )
	    pref.fLogScroll = toupper( ch ) == 'Y';
	else if( !strcmp( szKeyword, "print-command" ) &&
		 isspace( pch[ 13 ] ) ) {
	    if( !( pchEnd = strchr( pch + 14, '\n' ) ) )
		pchEnd = strchr( pch + 14, 0 );
	    g_free( pref.szPrint );
	    pref.szPrint = g_strndup( pch + 14, pchEnd - pch - 14 );
	} else if( !strcmp( szKeyword, "print-paper-x" ) &&
		   sscanf( pch, "print-paper-x %d", &pref.cxPaper ) == 1 )
	    ;
	else if( !strcmp( szKeyword, "print-paper-y" ) &&
		 sscanf( pch, "print-paper-y %d", &pref.cyPaper ) == 1 )
	    ;
			  
	if( ( pch = strchr( pch, '\n' ) ) )
	    pch++;
    }

    g_free( pchData );
}

static void HandleControlChange( vamp *pva, char iController, char nValue ) {

    char *pchPreset;
    
    pchPreset = pva->achPreset[ PRESET_CURRENT ];

    if( ( iController < CTRL_AMP_GAIN ) ||
	( iController > CTRL_AMP_TYPE ) ||
	( ( iController > CTRL_WAH ) && ( iController < CTRL_PRE_FX_TYPE ) ) )
	/* ignore */
	return;
    else if( ( iController == CTRL_AMP_TYPE_DEFAULT ) ||
	     ( iController == CTRL_FX_TYPE_DEFAULT ) )
	RequestAllControllers( pva );
    else if( iController == CTRL_FX_ASSIGN )
	pchPreset[ PARM_FX_MIX_ASSIGN ] = nValue;
    else if( iController == CTRL_AMP_TYPE )
	pchPreset[ PARM_AMP_TYPE ] = nValue;
    else if( iController < CTRL_PRE_FX_TYPE )
	pchPreset[ iController - CTRL_AMP_GAIN + PARM_AMP_GAIN ] = nValue;
    else
	pchPreset[ iController - CTRL_PRE_FX_TYPE + PARM_PRE_FX_TYPE ] =
	    nValue;

    if( pva->pwEditor )
	UpdateEditorWindow( pva->pwEditor, pchPreset, pva );
}

static void HandleProgramChange( vamp *pva, char iProgram ) {

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(
	pva->plw->apw[ (int) iProgram ] ), TRUE );
}

static gboolean ReadIOC( GIOChannel *piocSource, GIOCondition ioc,
			  vamp *pva ) {
    
    if( ReadMIDI( pva ) < 0 ) {
	Message( pva->pwList ? GTK_WINDOW( pva->pwList ) : NULL,
		 GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
		 "%s: %s", pva->szMidiDevice, g_strerror( errno ) );

	close( pva->h );
	pva->h = -1;
	
	return FALSE;
    }

    return TRUE;
}

extern int main( int argc, char *argv[] ) {

    GtkWidget *pw;
    GdkPixbuf *ppb;
    GtkIconFactory *pif;
    char szDevice[ 256 ];
    int i;
    vamp va;
    int ch;
    static struct option ao[] = {
	{ "channel", required_argument, NULL, 'c' },
	{ "device", required_argument, NULL, 'd' },
        { "help", no_argument, NULL, 'h' },
        { "no-midi", no_argument, NULL, 'n' },
        { "version", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0 }
    };
    
    gtk_init( &argc, &argv );

    pif = gtk_icon_factory_new();
    ppb = gdk_pixbuf_new_from_inline( sizeof achFreeVAMPIcon,
				      achFreeVAMPIcon, FALSE, NULL );
    gtk_icon_factory_add( pif, szIcon, gtk_icon_set_new_from_pixbuf( ppb ) );
    gtk_icon_factory_add_default( pif );
    
    va.szMidiDevice = "/dev/midi00";
    va.szFileName = NULL;
    va.nDeviceID = DEV_BROADCAST;
    va.nModelID = MOD_BROADCAST;
    va.iChannel = 0;
    va.iProgram = PRESET_CURRENT;
    va.nRunningStatus = va.fSysex = 0;
    va.cchCommand = 0;
    va.control = HandleControlChange;
    va.program = HandleProgramChange;
    va.sysex = NULL;
    va.pwEditor = va.pwList = va.pwLog = NULL;
    va.plsLog = NULL;
    va.fClipboard = FALSE;

    pref.szPrint = g_strdup( "lpr" );
    
    LoadPreferences();
    
    while( ( ch = getopt_long( argc, argv, "c:d:hnv", ao, NULL ) ) >= 0 )
	switch( ch ) {
	case 'c':
	    /* select MIDI channel */
	    if( ( va.iChannel = atoi( optarg ) - 1 ) > 0x0F ) {
		fprintf( stderr, "%s: channel \"%s\" invalid\n",
			 argv[ 0 ], optarg );
		va.iChannel = 0;
	    }
	    break;
	    
	case 'd':
	    /* select MIDI device */
	    va.szMidiDevice = optarg;
	    break;

	case 'h':
	    /* help */
	    printf( "Usage: %s [option ...] [preset file]\n"
		    "Options:\n"
		    "  -c C, --channel=C  Use MIDI channel C\n"
		    "  -d D, --device=D   Use MIDI device D\n"
		    "  -h, --help         Display usage and exit\n"
		    "  -n, --no-midi      Do not use external V-AMP\n"
		    "  -v, --version      Show version information and exit\n"
		    "\n"
		    "Please report bugs to <gtw@gnu.org>.\n", argv[ 0 ] );
	    return EXIT_SUCCESS;
	    
	case 'n':
	    /* disable MIDI */
	    va.szMidiDevice = NULL;
	    break;
	    
	case 'v':
	    /* version */
	    puts( "Free V-AMP " VERSION );
	    return EXIT_SUCCESS;
	    
	default:
	    return EXIT_FAILURE;
	}

    if( argc > optind + 1 )
	fprintf( stderr, "%s: only one preset file may be specified\n",
		 argv[ 0 ] );
    
    if( !va.szMidiDevice )
	va.h = -1;
    else if( ( va.h = open( va.szMidiDevice, O_RDWR ) ) < 0 ) {
	Message( NULL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s",
		 va.szMidiDevice, g_strerror( errno ) );
	va.szMidiDevice = NULL;
    } else {
	va.pioc = g_io_channel_unix_new( va.h );
	g_io_add_watch( va.pioc, G_IO_IN, (GIOFunc) ReadIOC, &va );
    }
    
    if( !IdentifyDevice( &va, NULL, szDevice ) )
	va.szDevice = szDevice;
    else {
	va.szDevice = "No device";
	va.h = -1;
    }
    
    if( argv[ optind ] && !ReadPresetsFile( &va, NULL, argv[ optind ] ) )
	;
    else if( RequestAllPresets( &va, NULL ) )
	for( i = 0; i < NUM_PRESETS; i++ )
	    memcpy( va.achPreset[ i ], achDefaultParm, NUM_PARMS );

    pw = CreateListWindow( &va );

    g_signal_connect( pw, "destroy", G_CALLBACK( gtk_main_quit ),
		      NULL );

    gtk_main();
    
    return EXIT_SUCCESS;
}

/* FIXME when logging MIDI events, don't interpret controller/program names
   on channels other than a V-AMP */
/* FIXME add "revert to saved" menu item */
/* FIXME ask for confirmation when discarding unsaved changes? */
/* FIXME add other import formats (e.g. sysex) */
/* FIXME make "open" read all file formats */
/* FIXME allow different devices for in and out */
/* FIXME make the log window global, not part of vamp */
/* FIXME add other prefs (e.g. midi device, channel, ... ) */
/* FIXME when changing between auto wah/compressor, first param doesn't update
   until mouse moves over it (dependent on GTK+ version?) */
/* FIXME allow printing only a subset of pages */
