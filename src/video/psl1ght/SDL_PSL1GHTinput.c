/* Input routines on PSL1GHT.
 * Keyboard, game pad, mouse(?) 
 *
 * Author: kraiskil, 2011. No rights reserved.
 */
#include <io/kb.h>
#include <stdio.h>
#include "../../events/SDL_events_c.h"

static int have_keyboard;

// memory buffer for the pressed keys from last round.
static u16 prev_keys[4];

int
PSL1GHT_InitKeyboard(_THIS)
{
	KbInfo kbinfo;
	// currently only support one keyboard.
	// possibly expand to support multiple once one work
	if(ioKbInit(1))
	{	
		// TODO: make an SDL error
		printf("SDL: ioKbInit failure!\n");fflush(stdout);
		return 1;
	}
	if(ioKbGetInfo(&kbinfo))	
	{
		// TODO: make an SDL error
		printf("SDL: ioKbGetInfo failure!\n");fflush(stdout);
		return 1;
	}
	// TODO: should we poll on each PumpEvent if a 
	// keyboard would have been attached?
	// for now, expect the user to have attached the 
	// keyboard before starting the program.
	// Yes I know - it is not nice
	if(kbinfo.connected==0)
	{
		printf("SDL: no keyboard found!\n");fflush(stdout);
		have_keyboard=0;
	}
	else {
		// Set this to get all the currently pressed keys upon request.
		// (othervise we get the scan code when *a* key is pressed, and 
		//  0 when *any* key is unpressed)
		ioKbSetReadMode( 0, KB_RMODE_PACKET );
		// get the USB standard key values - this is what SDL expects
		ioKbSetCodeType( 0, KB_CODETYPE_RAW );
		have_keyboard=1; //??
	}

	memset(prev_keys, 0, sizeof(prev_keys));
	return 0;
}

void
PSL1GHT_QuitKeyboard(_THIS)
{
	ioKbEnd();
}


/* Helper function for PSL1GHT_DispatchKeyboardEvents. 
 * Check if given key is in given key buffer 
 * return 1 if is, 0 if is not
 */
inline int
is_in_buf( u16 needle, u16* haystack, int size)
{
	int i;
	int retval = 0;
	for( i=0; i<size; i++)
	{
		if( haystack[i]==needle )
		{
			retval=1;
			break;
		}
	}
	return retval;
}

/* Translate keyboard status to SDL events
 *
 * -psl1ght code is u16 - gets messed up as a SDL event
 * if more than 4 keys are pressed simultaneously - 8 keys are reported
 * all with code 32769
 */
void
PSL1GHT_DispatchKeyboardEvents()
{
	int i;
	int num_prev_keys=0;
	u16 new_key;

	KbData kbdata;
	ioKbRead(0, &kbdata);
	
	if(!have_keyboard)return;

	// get the number of keys that was pressed last round.
	for( i=0; i<4; i++ )
		if(prev_keys[i]) num_prev_keys++;
		else break;

	// a key has been pressed
	if( num_prev_keys < kbdata.nb_keycode )
	{
		for( i=0; i<kbdata.nb_keycode; ++i)
			if( !is_in_buf( kbdata.keycode[i], prev_keys, 4 ) )
				SDL_SendKeyboardKey(SDL_PRESSED, kbdata.keycode[i]);
	}
	else if (num_prev_keys > kbdata.nb_keycode)
	{
		for( i=0; i<num_prev_keys; ++i)
			if( !is_in_buf( prev_keys[i], kbdata.keycode, 4 ) )
				SDL_SendKeyboardKey(SDL_RELEASED, prev_keys[i]);

	}
	//else same amount of keys are pressed

	// update kb status in preparation for next round
	num_prev_keys = kbdata.nb_keycode;
	memset( prev_keys, 0, sizeof(prev_keys) );
	memcpy(prev_keys, kbdata.keycode, num_prev_keys * sizeof(u16));
}
