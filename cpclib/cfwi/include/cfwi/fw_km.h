#ifndef  __FW_KM_H__
#define __FW_KM_H__

#include <stdbool.h>
#include <stdint.h>

#include "cfwi_byte_shuffling.h"

/**
    TODO stronger typing

    * enum key_number with all hardware keys
*/

/** 0: KM INITIALISE
    #BB00
    Initialize the Key Manager
    Action:
    Full initialization of the Key Manager (as during EMS). All Key Manager variables,
    buffers and indirections are initialized. The previous state of the Key Manager is lost.
    Entry conditions:
    No conditions.
    Exit conditions:
    AF,BC,DE and HL corrupt. All other registers preserved.
    Notes:
    The Key Manager indirection (KM TEST KEY) is set to its default routine.
    The key buffer is set up (to be empty).
    The expansion buffer is set up and the expansions are set to their default strings.
    The translation table are initialized to their default translations.
    The repeating key map is initialized to its default state.
    The repeat speeds are set to their default values.
    Shift and caps lock are turned off.
    The break event is disarmed.
    See Appendices II, III and IV for the default translation tables, repeating key table
    and expansion strings.
    This routine enables interrupts.
    Related entries:
    KM RESET
*/
void fw_km_initialise(void) __preserves_regs(iyh, iyl);

/** 1: KM RESET
    #BB03
    Reset the Key Manager.
    Action:
    Reinitializes the Key Manager indirections and buffers.
    Entry conditions:
    No conditions.
    Exit conditions:
    AF,BC,DE and HL corrupt. All other registers preserved.
    Notes:
    The Key Manager indirection (KM TEST KEY) is set to its default routine.
    The key buffer is set up (to be empty).
    The expansion buffer is set up and the expansions are set to their default strings (see
    Appendix IV).
    The break event is disarmed.
    All pending keys and characters are discarded.
    This routine enables interrupts.
    Related entries:
    KM DISARM BREAK
    KM EXP BUFFER
    KM INITIALISE
*/
void fw_km_reset(void) __preserves_regs(iyh, iyl);

/** 2: KM WAIT CHAR
    #BB06
    Wait for the next character from the keyboard.
    Action:
    Try to get a character from the key buffer or the current expansion string. This routine
    waits until a character is available if no character is immediately available.
    Entry conditions:
    No conditions.
    Exit conditions:
    Carry true. A contains the character. Other flags corrupt. All other registers preserved.
    Notes:
    The possible sources for generating the next character are, in the order that they are
    tested:
    The 'put back' character. The next character of an expansion string. The first character
    of an expansion string. A character from a key translation table.
    Expansion tokens found in the key translation table are expanded to their associated
    strings. Expansion tokens found in expansion strings are not expanded but are treated
    as characters.
    Related entries:
    KM CHAR RETURN
    KM READ CHAR
    KM WAIT KEY
*/
unsigned char fw_km_wait_char(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle carry flag, value is returned like this:

    uint16_t returned_value = fw_km_read_char();
    if (UINT_AND_BYTE_1(returned_value))
    {
    // a character was read
    unsigned char c = UINT_SELECT_BYTE_0(returned_value);
    }
    else
    {
    // no character was read
    }

    3: KM READ CHAR
    #BB09
    Test if a character is available from the keyboard.
    Action: Try to get a character from the key buffer or the current expansion string.
    This routine does not wait for a character to become available if there is no character
    available immediately.
    Entry conditions:
    No conditions.
    Exit conditions:
    If there was a character available:
    Carry true. A contains the character.
    If there was no character available.
    Carry false. A corrupt.
    Always:
    Other flags corrupt. All other registers preserved.
    Notes:
    The possible sources for generating the next character are, in the order that they are
    tested:
    The 'put back' character. The next character of an expansion string. The first character
    of an expansion string. A character from a key translation table.
    Expansion tokens in the key translation table will be expanded to their associated
    strings. Expansion tokens found in expansion strings are not expanded but are treated
    as characters.
    This routine will always return a character if one is available. It is therefore possible
    to flush out the Key Manager buffers by calling KM READ CHAR repeatedly until it
    reports that no character is available.
    Related entries:
    KM CHAR RETURN
    KM FLUSH
    KM READ KEY
    KM WAIT CHAR
*/
uint16_t fw_km_read_char(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** 4: KM CHAR RETURN #BB0C
    Return a single character to the keyboard for next time.
    Action:
    Save a character for the next call of KM READ CHAR or KM WAIT CHAR.
    Entry conditions:
    A contains the character to put back.
    Exit conditions:
    All registers and flags preserved.
    Notes:
    The 'put back' character will be returned before any other character is generated by
    the keyboard. It will not be expanded (or otherwise dealt with) but will be returned as
    it is. The 'put back' character need not have been read from the keyboard, it could be
    inserted by the user for some purpose.
    It is only possible to have one 'put back' character. If this routine is called twice
    without reading a character between these then the first (put back' will be lost.
    Furthermore, it is not possible to return character 255 (because this is used as the
    marker for no 'put back' character).
    Related entries:
    KM READ CHAR
    KM WAIT CHAR
*/
void fw_km_char_return(unsigned char c) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle carry flag, this routine returns zero if
    expansion is okay, non-zero if expansion failed.

    5: KM SET EXPAND
    #BB0F
    Set an expansion string.
    Action:
    Set the expansion string associated with an expansion token.
    Entry conditions:
    B contains the expansion token for the expansion to set. C contains the length of the
    string. HL contains the address of the string.
    Exit conditions:
    If the expansion is OK:
    Carry true.
    If the string was too long or the token was invalid:
    Carry false.
    Always:
    A,BC,DE,HL and other flags corrupt. All other registers preserved.
    Notes:
    The string to be set may lie anywhere in RAM. Expansion strings cannot be set
    directly from ROM.
    The characters in the string are not expanded (or otherwise dealt with). It is therefore
    possible to put any character into an expansion string.
    If there is insufficient room in the expansion buffer for the new string then no change
    is made to the expansions.
    If the string set is currently being used to generate characters (by KM READ CHAR
    or KM WAIT CHAR) then the unread portion of the string is discarded. The next
    character will be read from the key buffer.
    This routine enables interrupts.
    Related entries:
    KM GET EXPAND
    KM READ CHAR
    KM WAIT CHAR

*/
enum fw_byte_all_or_nothing fw_km_set_expand(uint8_t token, uint8_t string_length, unsigned char* string) __preserves_regs(iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle carry flag, this routine returns a byte
    value if a character was returned, and any value outside range
    0-255 if not.

    You can use it like this:

    uint16_t returned_value = fw_km_read_char();
    if (UINT_AND_BYTE_1(returned_value))
    {
    // a character was read
    unsigned char c = UINT_SELECT_BYTE_0(returned_value);
    }
    else
    {
    // no character was read
    }


    6: KM GET EXPAND
    #BB12
    Get a character from an expansion string.
    Action:
    Read a character from an expansion string. The characters in the string are numbered
    starting from 0.
    Entry conditions:
    A contains an expansion token.
    L contains the character number.
    Exit conditions:
    If the character was found:
    Carry true. A contains the character.
    If the token was invalid or the string was not long enough:
    Carry false. A corrupt.
    Always:
    DE and other flags corrupt. All other registers preserved.
    Notes:
    The characters in the expansion string are not expanded (or otherwise dealt with). It is
    therefore possible to put any character into an expansion string.
    Related entries:
    KM READ CHAR
    KM SET EXPAND
*/
uint16_t fw_km_get_expand(uint8_t token, uint8_t char_number) __preserves_regs(b, c, iyh, iyl);


/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle carry flag, this routine returns zero if
    operation went okay, non-zero on failure.

    7: KM EXP BUFFER
    #BB15
    Allocate a buffer for expansion strings.
    Action:
    Set the address and length of the expansion buffer. Initialize the buffer with the
    default expansion strings.
    Entry conditions:
    DE contains the address of the buffer. HL contains the length of the buffer.
    Exit conditions:
    If the buffer is OK:
    Carry true.
    If the buffer is too short.
    Carry false.
    Always:
    A,BC,DE,HL and other flags corrupt. All other registers preserved.
    Notes:
    The buffer must not be located underneath a ROM and it must be at least 49 bytes
    long (i.e. have sufficient space for the default expansion strings). If the new buffer is
    too short then the old buffer is left unchanged.
    The default expansion strings are given in Appendix IV.
    Any expansion string currently being read is discarded.
    This routine enables interrupts.
    Related entries:
    KM GET EXPAND
    KM SET EXPAND
*/
enum fw_byte_all_or_nothing fw_km_exp_buffer(unsigned char *buffer, uint16_t buffer_bytecount) __preserves_regs(iyh, iyl);

/** 8: KM WAIT KEY
    #BB18
    Wait for next key from the keyboard.
    Action:
    Try to get a key from the key buffer. This routine waits until a key is found if no key
    is immediately available.
    Entry conditions:
    No conditions.
    Exit conditions:
    Carry true. A contains the character or expansion token.
    Other flags corrupt. All registers preserved.
    Notes:
    The next key is read from the key buffer and translated using the appropriate key
    translation table. Expansion tokens are not expanded but are passed out for the user to
    deal with, as are normal characters. Other Key Manager tokens (shift lock, caps lock
    and ignore) are obeyed but are not passed out.
    Related entries:
    KM READ KEY
    KM WAIT CHAR
*/
unsigned char fw_km_wait_key(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** #### CFWI-specific information: ####

    Since C cannot handle carry flag, this routine returns a byte
    value if a character was returned, and any value outside range
    0-255 if not.

    You can use it like this:

    uint16_t returned_value = fw_km_read_key();
    if (UINT_AND_BYTE_1(returned_value))
    {
    // a character was read
    unsigned char c = UINT_SELECT_BYTE_0(returned_value);
    }
    else
    {
    // no character was read
    }

    9: KM READ KEY
    #BB1B
    Test if a key is available from the keyboard.
    Action:
    Try to get a key from the key buffer. This routine does not wait if no key is available
    immediately.
    Entry conditions:
    No conditions.
    Exit conditions.
    If a key was available:
    Carry true.
    A contains the character or expansion token.
    If no key was available:
    Carry false. A corrupt.
    Always:
    Other flags corrupt. All other registers preserved.
    Notes:
    The next key is read from the key buffer and translated using the appropriate key
    translation table. Expansion tokens are not expanded but are passed out for the user to
    deal with, as are normal characters. Other Key Manager tokens (shift lock, caps lock
    and ignore) are obeyed but are not passed out.
    This routine will always return a key if one is available. It is therefore possible to
    flush out the key buffer by calling KM READ KEY repeatedly until it claim no key is
    available. Note, however, that the 'put back' character or a partially read expansion
    string is ignored. It is advisable to use KM READ CHAR to flush these out when
    emptying the Key Manager buffers, or, in V1.1 firmware, to call KM FLUSH.
    Related entries:
    KM FLUSH
    KM READ CHAR
    KM WAIT KEY



    CFWI_TEST_FLAGS: TESTED_APP_PASS
*/
uint16_t fw_km_read_key(void) __preserves_regs(b, c, d, e, iyh, iyl);

enum
{
	fw_km_test_key_control_mask = 0x80,
	fw_km_test_key_shift_mask = 0x20,
};

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle zero flag, value is returned like this:

    uint16_t returned_value = fw_km_test_key(mykey);
    if (UINT_AND_BYTE_1(returned_value))
    {
    // key pressed
    uint8_t code = UINT_SELECT_BYTE_0(returned_value);
    bool modifier_control = (code & fw_km_test_key_control_mask);
    bool modifier_shift = (code & fw_km_test_key_shift_mask);
    }
    else
    {
    // key not pressed
    }

    10: KM TEST KEY
    #BB1E
    Test if a key is pressed.
    Action:
    Test if a particular key or joystick button is pressed. This is done using the key state
    map rather then by accessing the keyboard hardware.
    Entry conditions:
    A contains the key number.
    Exit conditions:
    If the key is pressed:
    Zero false.
    If the key is not pressed:
    Zero true.
    Always:
    Carry false. C contains the current shift and control state.
    A,HL and other flags corrupt. All other registers preserved.
    Notes:
    The shift and control states are automatically read when a key is scanned. If bit 7 is
    set then the control key is pressed and if bit 5 is set then one of the shift keys is
    pressed.
    The key number is not checked. An invalid key number will generate the correct shift
    and control states but the state of the key tested will be meaningless.
    The key state map which this routine tests is updated by the keyboard scanning
    routine. Normally this run is every fiftieth of a second and so the state may be out of
    date by that much. The key debouncing requires that a key should be released for two
    scans of the keyboard before it is marked as released in the key state map; the
    pressing of a key is detected immediately.
    Related entries:
    KM GET JOYSTICK
    KM GET STATE
    KM READ KEY
*/
uint16_t fw_km_test_key(uint8_t key_number) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Use like this:

    uint16_t returned_value = fw_km_get_state();

    if (UINT_SELECT_BYTE_0(returned_value))
    {
    // shift lock enabled
    }

    if (UINT_SELECT_BYTE_1(returned_value))
    {
    // caps lock enabled
    }

    11: KM GET STATE
    #BB21
    Fetch Caps Lock and Shift Lock states.
    Action:
    Ask if the keyboard is currently shift locked or caps locked.
    Entry conditions:
    No conditions.
    Exit conditions:
    L contains the shift lock state. H contains the caps lock state.
    AF corrupt. All other registers preserved.
    Notes:
    The lock states are:
    #00 means the lock is off
    #FF means the lock is on
    The default lock states are off.
    Related entries:
    KM SET LOCKS
    KM TEST KEY
*/
uint16_t fw_km_get_state(void) __preserves_regs(b, c, d, e, iyh, iyl);

enum fw_joystick_bits
{
	fw_joystick_bit_up = 0,
	fw_joystick_bit_down = 1,
	fw_joystick_bit_left = 2,
	fw_joystick_bit_right = 3,
	fw_joystick_bit_fire_2 = 4,
	fw_joystick_bit_fire_1 = 5,
	fw_joystick_bit_spare = 6,
};

/* for WORD in up down left right fire_2 fire_1 spare ; do echo "fw_joystick_mask_$WORD = 1 << fw_joystick_bit_$WORD," ; done */

enum fw_joystick_masks
{
	fw_joystick_mask_up = 1 << fw_joystick_bit_up,
	fw_joystick_mask_down = 1 << fw_joystick_bit_down,
	fw_joystick_mask_left = 1 << fw_joystick_bit_left,
	fw_joystick_mask_right = 1 << fw_joystick_bit_right,
	fw_joystick_mask_fire_2 = 1 << fw_joystick_bit_fire_2,
	fw_joystick_mask_fire_1 = 1 << fw_joystick_bit_fire_1,
	fw_joystick_mask_spare = 1 << fw_joystick_bit_spare,
};

#define HAS_JOY_STATE_BIT(state, bitname) ((state && fw_joystick_mask_ xstr(#bitname))!=0)

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Use like this:

    uint16_t returned_value = fw_km_get_state();
    bool joystick_1_state = UINT_SELECT_BYTE_0(returned_value);
    bool joystick_0_state = UINT_SELECT_BYTE_1(returned_value);

    // You can use fw_joystick_mask_*
    // or this macro:
    if (HAS_JOY_STATE_BIT(joystick_0_state, up))
    {
    // player pressed up
    }

    12: KM GET JOYSTICK #BB24
    Fetch current state of the joystick(s).
    Action:
    Ask what the current states of the joysticks are. These are read from the key state map
    rather than by accessing the keyboard hardware.
    Entry conditions:
    No conditions.
    Exit conditions:
    H contains the state of joystick 0. L contains the state of joystick 1. A contains the
    state of joystick 0.
    Flags corrupt. All other registers preserved.
    Notes:
    In normal operation the key state map is updated by the key scanning routine every
    fiftieth of a second so the state returned may be slightly out of date.
    The joystick states are bit significant as follows:
    Bit 0 Up.
    Bit 1 Down.
    Bit 2 Left.
    Bit 3 Right.
    Bit 4 Fire 2.
    Bit 5 Fire 1.
    Bit 6 Spare joystick button (usually unconnected).
    Bit 7 Always zero.
    If a bit is set then the appropriate button is pressed.
    Joystick 1 is indistinguishable from certain keys on the keyboard (see Appendix 1).
    Related entries:
    KM TEST KEY

    TODO create a struct with joystick bits.
    TODO create a union with both struct and uint16_t, for decoding.
*/
uint16_t fw_km_get_joystick(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    13: KM SET TRANSLATE #BB27
    Set entry in normal key translate table.
    Action:
    Set what character or token a key will be translated to when neither shift nor control
    is pressed.
    Entry conditions:
    A contains a key number. B contains the new translation.
    Exit conditions:
    AF and HL corrupt. All other registers preserved.
    Notes:
    If the key number is invalid (greater than 79) then no action is taken.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM WAIT KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET TRANSLATE
    KM SET CONTROL
    KM SET SHIFT
*/
void fw_km_set_translate(uint8_t key_number, uint8_t new_translation) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    14: KM GET TRANSLATE #BB2A
    Get entry from normal translation table.
    Action:
    Ask what character or token a key will be translated to when neither shift nor control
    is pressed.
    Entry conditions:
    A contains a key number
    Exit conditions:
    A contains the current translation.
    HL and flags corrupt. All other registers preserved.
    Notes:
    The key number is not checked. If it is invalid (greater than 79) then the translation
    returned is meaningless.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM TEST KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET CONTROL
    KM GET SHIFT
    KM SET TRANSLATE
*/
uint8_t fw_km_get_translate(uint8_t key_number) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    15: KM SET SHIFT
    #BB2D
    Set entry in shifted key translation table.
    Action:
    Set what character or token a key will be translated to when control is not pressed but
    shift is pressed or shift lock is on:
    Entry conditions:
    A contains a key number. B contains the new translation.
    Exit conditions:
    AF and HL corrupt. All other registers preserved.
    Notes:
    If the key number is invalid (greater than 79) then no action is taken.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM TEST KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET CONTROL
    KM GET SHIFT
    KM SET TRANSLATE
*/
void fw_km_set_shift(uint8_t key_number, uint8_t new_translation) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    KM GET SHIFT
    #BB30
    Get entry from shifted key translation table.
    Action:
    Ask what character or token a key will be translated to when control is not pressed
    but shift is pressed or shift lock is on.
    Entry conditions:
    A contains a key number.
    Exit conditions:
    A contains the current translation.
    HL and flags corrupt. All other registers preserved.
    Notes:
    The key number is not checked. If it is invalid (greater than 79) then the translation
    returned is meaningless.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM TEST KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET CONTROL
    KM GET SHIFT
    KM SET TRANSLATE
*/
uint8_t fw_km_get_shift(uint8_t key_number) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    17: KM SET CONTROL #BB33
    Set entry in control key translation table.
    Action:
    Set a character or token a key will be translated to when control is pressed.
    Entry conditions:
    A contains a key number. B contains the new translation.
    Exit conditions:
    AF and HL corrupt. All other registers preserved.
    Notes:
    If the key number is invalid (greater than 79) then no action is taken.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM TEST KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET CONTROL
    KM GET SHIFT
    KM SET TRANSLATE
*/
void fw_km_set_control(uint8_t key_number, uint8_t new_translation) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    18: KM GET CONTROL #BB36
    Get entry from control key translation table.
    Action:
    Ask what a character or token a key will be translated to when control is pressed.
    Entry conditions:
    A contains a key number.
    Exit conditions:
    A contains the current translation. HL and flags corrupt. All other registers preserved.
    Notes:
    The key number is not checked. If it is invalid (greater than 79) then the translation
    returned is meaningless.
    Most values in the table are treated as characters and are passed back to the user.
    However, there are certain special values:
    #80..#9F are the expansion tokens and are expanded to character strings when KM
    READ CHAR or KM WAIT CHAR is called although they are passed back like any
    other character when KM READ KEY or KM TEST KEY is called.
    #FD is the caps lock token and causes the caps lock to toggle (turn on if off and vice
    versa).
    #FE is the shift lock token and causes the shift lock to toggle (turn on if off and vice
    versa).
    #FF is the ignore token and means the key should be thrown away.
    Characters #E0..#FC have special meanings to the BASIC to do with editing,
    cursoring and breaks.
    See Appendix II for a full listing of the default translation tables.
    Related entries:
    KM GET CONTROL
    KM GET SHIFT
    KM SET TRANSLATE
*/
uint8_t fw_km_get_control(uint8_t key_number) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    19: KM SET REPEAT #BB39
    Set whether a key may repeat.
    Action:
    Set the entry in the repeating key map that determines whether a key is allowed to
    repeat or not.
    Entry conditions:
    If the key is to be allowed to repeat:
    B contains #FF.
    If the key is not to be allowed to repeat:
    B contains #00
    Always:
    A contains the key number.
    Exit conditions:
    AF,BC and HL corrupt. All other registers preserved.
    Notes:
    If the key number is invalid (greater than 79) then no action is taken.
    The default repeating keys are listed in Appendix III.
    Related entries:
    KM GET REPEAT
    KM SET DELAY
*/
uint8_t fw_km_set_repeat(uint8_t key_number, enum fw_byte_all_or_nothing repeat_allowed) __preserves_regs(d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Since C cannot handle zero flag, value is returned like this:

    uint16_t returned_value = fw_km_get_repeat(mykey);
    if (UINT_SELECT_BYTE_0(returned_value))
    {
    // key is allowed to repeat
    }
    else
    {
    // key is not allowed to repeat
    }

    20: KM GET REPEAT
    #BB3C
    Ask if a key is allowed to repeat.
    Action:
    Test the entry in the repeating key map that says whether a key is allowed to repeat or
    not.
    Entry conditions:
    A contains a key number.
    Exit conditions:
    If the key is allowed to repeat:
    Zero false.
    If the key is not allowed to repeat:
    Zero true.
    Always
    Carry false. A,HL and other flags corrupt. All other registers preserved.
    Notes:
    The key number is not checked. If it is invalid (greater than 79) then the repeat state
    returned is meaningless.
    The default repeating keys are listed in Appendix III.
    Related entries:
    KM SET REPEAT
*/
enum fw_byte_all_or_nothing fw_km_get_repeat(uint8_t key_number) __z88dk_fastcall __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    21: KM SET DELAY
    #BB3F
    Set start delay and repeat speed.
    Action:
    Set the time before keys first repeat (start up delay) and the time between repeats
    (repeat speed).
    Entry conditions:
    H contains the new start up delay. L contains the new repeat speed.
    Exit conditions:
    AF corrupt. All other registers preserved.
    Notes:
    Both delays are given in scans of the keyboard. The keyboard is scanned every fiftieth
    of a second.
    A start up delay or repeat speed of 0 is taken to mean 256.
    The default start up delay is 30 scans (0.6 seconds) and the default repeat speed is 2
    scans (0.04 seconds or 25 characters a second).
    Note that a key is prevented from repeating (by the key scanner) if the key buffer is
    not empty. Thus the actual repeat speed is the slower of the supplied repeat speed and
    the rate at which characters are removed from the buffer. This is intended to prevent
    the user from getting too far ahead of a program that is running sluggishly.
    The start up delay and repeat speed apply to all keys on the keyboard that are set to
    repeat.
    Related entries:
    KM GET DELAY
    KM SET REPEAT

*/
void fw_km_set_delay(uint8_t startup_delay, uint8_t repeat_speed) __preserves_regs(b, c, d, e, iyh, iyl);

/** WARNING DONE BUT UNTESTED, MIGHT NOT WORK

    #### CFWI-specific information: ####

    Use like this:

    uint16_t returned_value = fw_km_get_delay();
    uint8_t startup_delay = UINT_SELECT_BYTE_1(returned_value);
    uint8_t repeat_speed = UINT_SELECT_BYTE_0(returned_value);

    22: KM GET DELAY
    #BB42
    Get start up delay and repeat speed.
    Action:
    Ask the time before keys first repeat (start up delay) and the time between repeats
    (repeat speed).
    Entry conditions:
    No conditions.
    Exit conditions:
    H contains the start up delay. L contains the repeat speed.
    AF corrupt. All other registers preserved.
    Notes:
    Both delays are given in scans of the keyboard. The keyboard is scanned every fiftieth
    of a second.
    A repeat speed or start up delay of 0 means 256.
    Related entries:
    KM SET DELAY
*/
uint16_t fw_km_get_delay(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** 23: KM ARM BREAK #BB45
    Allow break events to be generated.
    Action:
    Arm the break mechanism. The next call of KM BREAK EVENT will generate a
    break event.
    Entry conditions:
    DE contains the address of the break event routine. C contains the ROM select
    address for this routine.
    Exit conditions:
    AF,BC,DE and HL corrupt. All other registers preserved.
    Notes:
    The break mechanism can be disarmed by calling KM DISARM BREAK (or KM
    RESET).
    This routine enables interrupts.
    Related entries:
    KM BREAK EVENT
    KM DISARM BREAK
*/
// TODO KM_ARM_BREAK

/** 24: KM DISARM BREAK #BB48
    Prevent break events from being generated.
    Action:
    Disarm the break mechanism. From now on the generation of break events by KM
    BREAK EVENT will be suppressed.
    Entry conditions:
    No conditions.
    Exit conditions:
    AF and HL corrupt. All other registers preserved.
    Notes:
    Break events can be rearmed by calling KM ARM BREAK.
    The default state of the break mechanism is disarmed, thus calling KM RESET will
    also disarm breaks.
    This routine enables interrupts.
    Related entries:
    KM ARM BREAK
    KM BREAK EVENT
*/
void fw_km_disarm_break(void) __preserves_regs(b, c, d, e, iyh, iyl);

/** 25: KM BREAK EVENT #BB4B
    Generate a break event (if armed).
    Action:
    Try to generate a break event.
    Entry conditions:
    No conditions.
    Exit conditions:
    AF and HL corrupt. All other registers preserved.
    Notes:
    If the break mechanism is disarmed then no action is taken. Otherwise a break event
    is generated and a special marker is placed into the key buffer. This marker generates
    a break event token (#EF) when read from the buffer. The break mechanism is
    automatically disarmed after generating a break event so that multiple breaks can be
    avoided.
    This routine may run from the interrupt path and thus does not and should not enable
    interrupts. Note, however, that using a LOW JUMP to call the routine (as the
    firmware jumpblock is set to do) does enable interrupts and so the jumpblock may not
    be used directly from interrupt routines.
    Related entries:
    KM ARM BREAK
    KM DISARM BREAK
*/
void fw_km_break_event(void) __preserves_regs(b, c, d, e, iyh, iyl);

#ifdef FW_V11_AND_ABOVE

/** 190: KM SET LOCKS
    #BD3A
    Set the shift and caps lock states.
    Action:
    Turn the shift and caps locks on or off.
    Entry conditions:
    H contains the required caps lock state.
    L contains the required shift lock state.
    Exit conditions:
    AF corrupt.
    All other registers preserved.
    Notes:
    This routine is not available on V1.0 firmware.
    The lock states are:
    #00 means that the lock is to be turned off.
    #FF means that the lock is to be turned on.
    The default lock states are off.
    Related entries:
    KM GET STATE
*/
void fw_km_set_locks(uint16_t locks) __z88dk_fastcall __preserves_regs(b,c,d,e,iyh, iyl);

/** 191: KM FLUSH
    #BD3D
    Flush the keyboard buffers.
    Action:
    Discard all pending keys from the key buffer, the 'put back' character and any current
    expansion string.
    Entry conditions:
    No conditions.
    Exit conditions:
    AF corrupt. All other registers preserved.
    Notes:
    This routine is not available on V1.0 firmware.
    The next character that will be returned by KM READ CHAR (or a similar routine)
    after KM FLUSH is called will be the first character that the user types after the call
    of KM FLUSH since all the pending characters will have been discarded.
    On V1.0 firmware the effect of this routine can be simulated by repeatedly calling
    KM READ CHAR until it comes back with carry false.
    Related entries:
    KM READ CHAR
    KM READ KEY
*/
void fw_km_flush(void) __preserves_regs(b, c, d, e, iyh, iyl);

#endif /* FW_V11_AND_ABOVE */

#endif /* __FW_KM_H__ */
