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
#include <unistd.h>

#include "fvicon.h"
#include "getopt.h"

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

#define MIDI_STATUS 0x80
#define MIDI_OPERATION 0xF0
#define MIDI_CHANNEL 0x0F
#define MIDI_CTRLCHANGE 0xB0
#define MIDI_PROGCHANGE 0xC0
#define MIDI_SYSTEM 0xF0
#define MIDI_SYSEX 0xF0
#define MIDI_SYSEX_END 0xF7
#define MIDI_SYSREALTIME 0xF8

#define PROG_TUNER 0x7F

#define ID_BEHRINGER1 0x00
#define ID_BEHRINGER2 0x20
#define ID_BEHRINGER3 0x32

#define DEV_BROADCAST 0x7F
#define MOD_BROADCAST 0x7F

typedef struct _vamp {
    int h; /* MIDI device file descriptor */
    GIOChannel *pioc;
    char *szMidiDevice, *szDevice;
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
    int fLog, fLogScroll, cLogMax;
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
    64, 64, 64, 64, 64, 0, 64, 32, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    'U','n','n','a','m','e','d',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

static preferences pref = { FALSE, TRUE, 100 };

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

    if( pva->h < 0 ) {
	errno = EBADF;
	return -1;
    }
    
    if( ( n = read( pva->h, ach, sizeof ach ) ) < 0 )
	return -1;

    for( pch = ach; n; pch++, n-- ) {
	if( pva->fSysex && pva->sysex )
	    pva->sysex( pva, *pch, pva->pvSysex, n - 1 );

	if( *pch & MIDI_STATUS ) {
	    if( ( *pch & MIDI_SYSREALTIME ) == MIDI_SYSREALTIME )
		Log( pva, *pch, "System Real-time" );
	    else if( pva->nRunningStatus == MIDI_SYSEX &&
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
	    case MIDI_CTRLCHANGE:
		/* FIXME don't try to interpret events on other channels */
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

	    default:
		/* ignore */
		/* FIXME log known events */
	    }
	}
    }

    return 0;
}

typedef struct _sysexstate {
    int cch, cchAlloc, cchExpected, cchOld, fFailed;
    char *pch;
    GtkWidget *pwProgress;
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
	gtk_main_quit();
	return;
    } else if( !( ch & MIDI_STATUS ) ) {
	pses->pch[ pses->cch++ ] = ch;

	if( pses->cch < pses->cchAlloc )
	    return;

	if( ( pses->pch = realloc( pses->pch, pses->cchAlloc <<= 1 ) ) )
	    return;
    }

    pses->fFailed = TRUE;
    free( pses->pch );
    pses->pch = NULL;
    gtk_main_quit();
}

static void CancelSysex( GtkWidget *pw, gint n, sysexstate *pses ) {

    pses->fFailed = TRUE;
    free( pses->pch );
    pses->pch = NULL;
    gtk_main_quit();
}

static unsigned char *ReadSysex( vamp *pva, GtkWindow *pwParent, int *cb,
				 int cbExpected ) {

    sysexstate ses;
    GtkWidget *pw;
    
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
    g_signal_connect( G_OBJECT( pw ), "response", G_CALLBACK( CancelSysex ),
		      &ses );
    ses.pwProgress = gtk_progress_bar_new();
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       ses.pwProgress );
    gtk_widget_show_all( pw );
    if( ( ses.pch = malloc( ses.cchAlloc = 1024 ) ) ) {
	pva->pvSysex = &ses;
	pva->sysex = HandleSysex;

	gtk_main();

	pva->pvSysex = NULL;
	pva->sysex = NULL;
    }

    if( !ses.pch )
	ses.cch = 0;
    
    if( cb )
	*cb = ses.cch;

    gtk_widget_destroy( pw );
    
    return realloc( ses.pch, ses.cch );
}

static int ControlChange( vamp *pva, char iController, char nValue ) {

    unsigned char ach[ 3 ] = { MIDI_CTRLCHANGE | pva->iChannel, iController,
			       nValue };

    write( pva->h, ach, 3 );

    pva->control( pva, iController, nValue );
    
    return 0;
}

static int RequestAllControllers( vamp *pva ) {

    return ControlChange( pva, CTRL_REQUEST, CTRL_REQUEST );
}

static int ProgramChange( vamp *pva, char iProgram ) {

    unsigned char ach[ 2 ] = { MIDI_PROGCHANGE | pva->iChannel, iProgram };

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

static int WritePreset( vamp *pva, char i, char ach[ NUM_PARMS ] ) {

    char achCommand[ NUM_PARMS + 2 ];

    achCommand[ 0 ] = i;
    achCommand[ 1 ] = NUM_PARMS;
    memcpy( achCommand + 2, ach, NUM_PARMS );
    
    if( StartSysEx( pva, CMD_WRITE_PRESET ) < 0 )
	return -1;
    
    if( write( pva->h, achCommand, NUM_PARMS + 2 ) < 0 )
	return -1;

    return EndSysEx( pva );
}

static int WriteAllPresets( vamp *pva, char ach[ NUM_PRESETS ][ NUM_PARMS ] ) {

    static char achCommand[ 2 ] = { NUM_PRESETS, NUM_PARMS };

    if( StartSysEx( pva, CMD_WRITE_ALL_PRESETS ) < 0 )
	return -1;
    
    if( write( pva->h, achCommand, 2 ) < 0 )
	return -1;

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
    pch = g_strdup_printf( "Free V-AMP - %s\n", PresetName( achPreset ) );
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
    static int anAutoWahSpeed[ 10 ] = { 10, 20, 50, 100, 200, 300, 400, 500,
					750, 1000 };
	
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
	    return g_strdup_printf( "%d", anAutoWahSpeed[ (int) f / 13 ] );
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

static void CloseEditor( GtkWidget *pw, gint nResponse, vamp *pva ) {

    char *pchPreset = pva->achPreset[ (int) pva->iProgram ];
    
    if( nResponse == GTK_RESPONSE_APPLY ) {
	WritePreset( pva, pva->iProgram, pva->achPreset[ PRESET_CURRENT ] );
    
	RequestPreset( pva, GTK_WINDOW( pva->pwList ), pva->iProgram,
		       pchPreset );
    
	gtk_label_set_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN(
	    pva->plw->apw[ (int) pva->iProgram ] ) ) ),
			    PresetName( pchPreset ) );
    }
    
    gtk_widget_destroy( pw );
}

static GtkWidget *CreateEditorWindow( vamp *pva,
				      char achPreset[ NUM_PARMS ] ) {
    
    GtkWidget *pwWindow, *pwVbox, *pwHbox, *pwFrame, *pwTable, *pwAlign, *pw;
    editorwindow *pew;
    int i;
    
    if( !( pva->pew = pew = malloc( sizeof( *pew ) ) ) )
	return NULL;
	
    pwWindow = gtk_dialog_new_with_buttons( PresetName( achPreset ), NULL, 0,
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
		       G_CALLBACK( CloseEditor ), pva );
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
    
    pch = g_strdup_printf( "Free V-AMP - Log (%s)\n", pva->szDevice );
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

static void Refresh( vamp *pva, guint n, GtkWidget *pwItem );

static void About( gpointer *p, guint n, GtkWidget *pwItem ) {

    static GtkWidget *pw, *pwHbox, *pwLabel;

    if( pw ) {
	gtk_window_present( GTK_WINDOW( pw ) );
	return;
    }

    pw = gtk_dialog_new_with_buttons( "About Free V-AMP", NULL, 0,
				      GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
				      NULL );

    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       pwHbox = gtk_hbox_new( FALSE, 16 ) );
    gtk_box_pack_start( GTK_BOX( pwHbox ),
			gtk_image_new_from_stock( szIcon,
						  GTK_ICON_SIZE_DIALOG ),
			FALSE, FALSE, 0 );
    pwLabel = gtk_label_new( NULL );
    gtk_label_set_markup( GTK_LABEL( pwLabel ),
			  "<span font_desc=\"serif bold 48\">Free "
			  "V-AMP</span>" );
    gtk_container_add( GTK_CONTAINER( pwHbox ), pwLabel );

    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       gtk_label_new( "version " VERSION ) );
    
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( pw )->vbox ),
		       gtk_label_new( "by Gary Wong, 2002" ) );
    
    g_signal_connect_swapped( pw, "response", G_CALLBACK( gtk_widget_destroy ),
			      pw );
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

    GtkWidget *pw, *pwConfirm;
    const char *pch;
    int f, h, c;
    char achPreset[ NUM_PRESETS ][ NUM_PARMS ];
    
    pw = gtk_file_selection_new( "Free V-AMP - Open" );
    
    gtk_window_set_transient_for( GTK_WINDOW( pw ),
				  GTK_WINDOW( pva->pwList ) );
    gtk_widget_show_all( pw );

    while( gtk_dialog_run( GTK_DIALOG( pw ) ) == GTK_RESPONSE_OK ) {
	pch = gtk_file_selection_get_filename( GTK_FILE_SELECTION( pw ) );

	if( ( h = open( pch, O_RDONLY ) ) < 0 ) {
	    perror( pch );
	    break;
	}
	
	if( ( c = read( h, achPreset, sizeof achPreset ) ) < 0 ) {
	    perror( pch );
	    close( h );
	    break;
	} else if( c < sizeof achPreset ) {
	    /* FIXME not a preset file */
	    break;
	}

	pwConfirm = gtk_message_dialog_new(
	    GTK_WINDOW( pw ), GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
	    "Are you sure you want to overwrite all device presets with "
	    "the contents of file %s?\n(Please note that you must "
	    "enter MIDI mode on the V-AMP before continuing.)", pch );
	f = gtk_dialog_run( GTK_DIALOG( pwConfirm ) );
	gtk_widget_destroy( pwConfirm );
	
	if( f == GTK_RESPONSE_YES ) {
	    gtk_widget_destroy( pw );
	    
	    memcpy( pva->achPreset, achPreset, sizeof achPreset );

	    WriteAllPresets( pva, achPreset );
	    return Refresh( pva, n, pwItem );
	} else
	    continue;
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

    GtkWidget *pwWindow, *pwFrame, *pwLog, *pwLogMax, *pwLogScroll, *pwVbox,
	*pwHbox;
    char *pchRC;
    FILE *pf;
    
    /* FIXME add other prefs (e.g. midi device, channel, ... ) */
    
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
    
    gtk_widget_show_all( pwWindow );
    
    g_signal_connect( pwLog, "toggled", G_CALLBACK( LogToggled ), pwVbox );
    LogToggled( pwLog, pwVbox );
    
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

	pchRC = g_build_filename( g_get_home_dir(), RCFILE, NULL );
	pf = fopen( pchRC, "w" );
	g_free( pchRC );
	fprintf( pf,
		 "log %c\n"
		 "log-max %d\n"
		 "log-scroll %c\n",
		 pref.fLog ? 'y' : 'n',
		 pref.cLogMax,
		 pref.fLogScroll ? 'y' : 'n' );
	fclose( pf );
    }

    gtk_widget_destroy( pwWindow );
}

static void Refresh( vamp *pva, guint n, GtkWidget *pwItem ) {
    
    RequestAllPresets( pva, GTK_WINDOW( pva->pwList ) );

    UpdateListWindow( pva );
	    
    if( pva->pwEditor )
	RequestAllControllers( pva );
}

static void Save( vamp *pva, guint n, GtkWidget *pwItem ) {

    GtkWidget *pw, *pwConfirm;
    const char *pch;
    struct stat s;
    int f, h;
    
    pw = gtk_file_selection_new( "Free V-AMP - Save" );
    
    gtk_window_set_transient_for( GTK_WINDOW( pw ),
				  GTK_WINDOW( pva->pwList ) );
    gtk_widget_show_all( pw );

    while( gtk_dialog_run( GTK_DIALOG( pw ) ) == GTK_RESPONSE_OK ) {
	pch = gtk_file_selection_get_filename( GTK_FILE_SELECTION( pw ) );

	if( !( stat( pch, &s ) ) ) {
	    pwConfirm = gtk_message_dialog_new(
		GTK_WINDOW( pw ), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"Are you sure you want to overwrite file %s?", pch );
	    f = gtk_dialog_run( GTK_DIALOG( pwConfirm ) );
	    gtk_widget_destroy( pwConfirm );

	    if( f != GTK_RESPONSE_YES )
		continue;
	}

	if( ( h = open( pch, O_WRONLY | O_CREAT, 0666 ) ) < 0 ) {
	    perror( pch );
	    break;
	}
	
	if( write( h, pva->achPreset, NUM_PRESETS * NUM_PARMS ) < 0 )
	    perror( pch );

	close( h );
	break;
    }

    gtk_widget_destroy( pw );
}

static GtkWidget *CreateListWindow( vamp *pva ) {
    
    GtkWidget *pwWindow, *pwVbox, *pwMenu, *pw, *pwTable;
    GtkItemFactory *pif;
    listwindow *plw;
    char *pch, achLabel[ 3 ];
    int i;
    static GtkItemFactoryEntry aife[] = {
	{ "/_File", NULL, NULL, 0, "<Branch>" },
	{ "/_File/_Open", "<control>O", Open, 0, "<StockItem>",
	  GTK_STOCK_OPEN },
	{ "/_File/_Save", "<control>S", Save, 0, "<StockItem>",
	  GTK_STOCK_SAVE },
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
	{ "/_Edit/P_references", NULL, Preferences, 0, "<StockItem>",
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

    pch = g_strdup_printf( "Free V-AMP - %s\n", pva->szDevice );
    gtk_window_set_title( GTK_WINDOW( pwWindow ), pch );
    g_free( pch );

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

    char *pchRC, *pchData, *pch, ch;
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
    ReadMIDI( pva );
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
	    printf( "Usage: %s [option ...]\n"
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

    if( !va.szMidiDevice )
	va.h = -1;
    else if( ( va.h = open( va.szMidiDevice, O_RDWR ) ) < 0 ) {
	perror( va.szMidiDevice );
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
    
    if( RequestAllPresets( &va, NULL ) )
	for( i = 0; i < NUM_PRESETS; i++ )
	    memcpy( va.achPreset[ i ], achDefaultParm, NUM_PARMS );

    pw = CreateListWindow( &va );

    g_signal_connect( pw, "destroy", G_CALLBACK( gtk_main_quit ),
		      NULL );

    /* FIXME handle files given on command line */
    
    gtk_main();
    
    return EXIT_SUCCESS;
}

/* FIXME implement error handling throughout! */
/* FIXME import/export patches (.vlb and sysex) */
/* FIXME add printing */
