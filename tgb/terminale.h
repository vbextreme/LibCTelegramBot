#ifndef __TERMINALE_H__
#define __TERMINALE_H__

#include <tgb/stdextra.h>
#include <tgb/utf8_extra.h>

/* NOT SPECIAL
 * backspace
 * key ctrl
*/

#define TIME_RELAX 30

#define CON_COLOR_RESET    0
#define CON_COLOR_BK       10

#define CON_COLOR_BLACK    30
#define CON_COLOR_RED      31
#define CON_COLOR_GREEN    32
#define CON_COLOR_YELLOW   33
#define CON_COLOR_BLUE     34
#define CON_COLOR_MAGENTA  35
#define CON_COLOR_CYAN     36
#define CON_COLOR_LGRAY    37
#define CON_COLOR_DGRAY    90
#define CON_COLOR_LRED     91
#define CON_COLOR_LGREEN   92
#define CON_COLOR_LYELLOW  93
#define CON_COLOR_LBLUE    94
#define CON_COLOR_LMAGENTA 95
#define CON_COLOR_LCYAN    96
#define CON_COLOR_WHYTE    97

#define CON_COLOR_BK_BLACK    40
#define CON_COLOR_BK_RED      41
#define CON_COLOR_BK_GREEN    42
#define CON_COLOR_BK_YELLOW   43
#define CON_COLOR_BK_BLUE     44
#define CON_COLOR_BK_MAGENTA  45
#define CON_COLOR_BK_CYAN     46
#define CON_COLOR_BK_LGRAY    47
#define CON_COLOR_BK_DGRAY    100
#define CON_COLOR_BK_LRED     101
#define CON_COLOR_BK_LGREEN   102
#define CON_COLOR_BK_LYELLOW  103
#define CON_COLOR_BK_LBLUE    104
#define CON_COLOR_BK_LMAGENTA 105
#define CON_COLOR_BK_LCYAN    106
#define CON_COLOR_BK_WHYTE    107

#define CON_FONT_RESET      0
#define CON_FONT_BOLD       1
#define CON_FONT_HALFBRIGHT 2
#define CON_FONT_ITALIC     3 /*not work for many terminal*/
#define CON_FONT_UNDERLINE  4
#define CON_FONT_CROSSED    9

#define CON_SPECIAL_NORMAL  0
#define CON_SPECIAL_CONTROL 1
#define CON_SPECIAL_MOUSE   2

#define CON_KET_GETCH_VT100_WAIT 5
#define CON_KET_WAIT             10

#define CON_KEY_NONE            '\0'
#define CON_KEY_ESC              27
#define CON_KEY_CARRIAGE        '\r'
#define CON_KEY_BACK            127
#define CON_KEY_CTRL_A 			0x1
#define CON_KEY_CTRL_B 			0x2
#define CON_KEY_CTRL_C 			0x3
#define CON_KEY_CTRL_D 			0x4
#define CON_KEY_CTRL_E          0x5
#define CON_KEY_CTRL_F 			0x6
#define CON_KEY_CTRL_G 			0x7
#define CON_KEY_CTRL_H 			0x8
#define CON_KEY_CTRL_I 			0x9
#define CON_KEY_CTRL_TAB        0x9
#define CON_KEY_TAB       		0x9
#define CON_KEY_CTRL_ENTER      0xA
#define CON_KEY_CTRL_M 			0xA
#define CON_KEY_CTRL_J 			0xA
#define CON_KEY_ENTER     		0xA
#define CON_KEY_CTRL_K 			0xB
#define CON_KEY_CTRL_L 			0xC
#define CON_KEY_CTRL_N 			0xE
#define CON_KEY_CTRL_O 			0xF
#define CON_KEY_CTRL_P 			0x10
#define CON_KEY_CTRL_R          0x12
#define CON_KEY_CTRL_T          0x14
#define CON_KEY_CTRL_U 			0x15
#define CON_KEY_CTRL_V 			0x16
#define CON_KEY_CTRL_W          0x17
#define CON_KEY_CTRL_X 			0x18
#define CON_KEY_CTRL_Y 			0x19
#define CON_KEY_CTRL_4          0x1C
#define CON_KEY_CTRL_BACKSLASH  0x1C //doppia
#define CON_KEY_CTRL_5          0x1D
#define CON_KEY_CTRL_6          0x1E
#define CON_KEY_CTRL_MINUS		0x1F 
#define CON_KEY_CTRL_7          0x1F //
#define CON_KEY_CTRL_APOSTROPHE 0x27
#define CON_KEY_CTRL_COMMA		0x2C
#define CON_KEY_CTRL_POINT   	0x2E
#define CON_KEY_CTRL_0          0x30
#define CON_KEY_CTRL_1          0x31
#define CON_KEY_CTRL_9          0x39
#define CON_KEY_CTRL_MINOR		0x3C
#define CON_KEY_CTRL_8          0x7F
#define CON_KEY_CTRL_RIGHTBRACKET 0xA8
#define CON_KEY_CTRL_UAC 		0xAC
#define CON_KEY_CTRL_Z 			0xB2
//#define CON_KEY_CTRL_LEFTBRACKET 0xC3
#define CON_KEY_CTRL_SHARP		0x27 
#define CON_KEY_CTRL_AT			0xC3 //
#define CON_KEY_CTRL_IAC 		0xC3 //
#define CON_KEY_STRESSED		0xC3
#define CON_KEY_BACKSPACE 		0x7F
#define CON_KEY_CTRL_BACKSPACE  0x7F //
#define CON_KEY_ALT_ENTER 		0x1B0A
#define CON_KEY_ALT_RIGHTBRACKET 0x1B2B
#define CON_KEY_ALT_COMMA   	0x1B2C
#define CON_KEY_ALT_MINUS		0x1B2D
#define CON_KEY_ALT_POINT		0x1B2E
#define CON_KEY_ALT_APOSTROPHE 	0x1B27
#define CON_KEY_ALT_0 			0x1B30
#define CON_KEY_ALT_1 			0x1B31
#define CON_KEY_ALT_2 			0x1B32
#define CON_KEY_ALT_3 			0x1B33
#define CON_KEY_ALT_4 			0x1B34
#define CON_KEY_ALT_5 			0x1B35
#define CON_KEY_ALT_6 			0x1B36
#define CON_KEY_ALT_7 			0x1B37
#define CON_KEY_ALT_8 			0x1B38
#define CON_KEY_ALT_9 			0x1B39
//#define CON_KEY_ALT_MINUS		0x1B3C
#define CON_KEY_ALT_BACKSLASH	0x1B5C
#define CON_KEY_ALT_B 			0x1B62
#define CON_KEY_ALT_C 			0x1B63
#define CON_KEY_ALT_D 			0x1B64
#define CON_KEY_ALT_E 			0x1B65
#define CON_KEY_ALT_G 			0x1B67
#define CON_KEY_ALT_H 			0x1B68
#define CON_KEY_ALT_I 			0x1B69
#define CON_KEY_ALT_J 			0x1B6A
#define CON_KEY_ALT_K 			0x1B6B
#define CON_KEY_ALT_L 			0x1B6C
#define CON_KEY_ALT_N 			0x1B6E
#define CON_KEY_ALT_O 			0x1B6F
#define CON_KEY_ALT_P 			0x1B70
#define CON_KEY_ALT_Q 			0x1B71
#define CON_KEY_ALT_R 			0x1B72
#define CON_KEY_ALT_T 			0x1B74
#define CON_KEY_ALT_U 			0x1B75
#define CON_KEY_ALT_V 			0x1B76
#define CON_KEY_ALT_W 			0x1B77
#define CON_KEY_ALT_X 			0x1B78
#define CON_KEY_ALT_Y 			0x1B79
#define CON_KEY_ALT_Z 			0x1B7A
#define CON_KEY_ALT_BACKSPACE 	0x1B7F
#define CON_KEY_FINE		    0x1B4F46
#define CON_KEY_ALT_FINE 		0x1B4F46 //
#define CON_KEY_HOME 	    	0x1B4F48
#define CON_KEY_ALT_HOME 		0x1B4F48 //
#define CON_KEY_F1        		0x1B4F50
#define CON_KEY_F2        		0x1B4F51
#define CON_KEY_F3        		0x1B4F52
#define CON_KEY_F4        		0x1B4F53
#define CON_KEY_UP        		0x1B5B41
#define CON_KEY_DOWN      		0x1B5B42
#define CON_KEY_RIGHT     		0x1B5B43
#define CON_KEY_LEFT      		0x1B5B44
#define CON_KEY_MOUSE			0x1B5B4D
#define CON_KEY_ALT_SHARP		0x1BC3A0
#define CON_KEY_ALT_LEFTBRACKET	0x1BC3A8
#define CON_KEY_ALT_IAC 		0x1BC3AC
#define CON_KEY_ALT_AT 			0x1BC3B2
#define CON_KEY_ALT_UAC 		0x1BC3B9
#define CON_KEY_INS       		0x1B5B327E
#define CON_KEY_CANC      		0x1B5B337E
#define CON_KEY_PAGEUP     		0x1B5B357E
#define CON_KEY_PAGEDOWN   		0x1B5B367E
#define CON_KEY_ALT_F9		 	0x303B337E
#define CON_KEY_CTRL_F8 		0x303B357E
#define CON_KEY_CTRL_F9 		0x303B357E //
#define CON_KEY_CTRL_F10 		0x303B357E //
#define CON_KEY_SHIFT_UP 		0x313B3241
#define CON_KEY_SHIFT_DOWN 		0x313B3242
#define CON_KEY_SHIFT_RIGHT 	0x313B3243
#define CON_KEY_SHIFT_LEFT 		0x313B3244
#define CON_KEY_ALT_UP 			0x313B3341
#define CON_KEY_ALT_DOWN 		0x313B3342
#define CON_KEY_ALT_RIGHT 		0x313B3343
#define CON_KEY_ALT_LEFT 		0x313B3344
#define CON_KEY_ALT_F3 		    0x313B3352
#define CON_KEY_ALT_F10 		0x313B337E
#define CON_KEY_CTRL_UP         0x313B3541
#define CON_KEY_CTRL_DOWN       0x313B3542
#define CON_KEY_CTRL_RIGHT      0x313B3543
#define CON_KEY_CTRL_FINE       0x313B3543
#define CON_KEY_CTRL_LEFT       0x313B3544
#define CON_KEY_CTRL_HOME       0x313B3544
#define CON_KEY_CTRL_F2 		0x313B3551
#define CON_KEY_CTRL_F3 		0x313B3552
#define CON_KEY_CTRL_F4 		0x313B3553
#define CON_KEY_ALT_INS 		0x323B337E
#define CON_KEY_ALT_F11 		0x333B337E
#define CON_KEY_ALT_CANC 		0x333B337E //
#define CON_KEY_CTRL_CANC       0x333B357E
#define CON_KEY_ALT_F12 		0x343B337E
#define CON_KEY_CTRL_F12 		0x343B357E
#define CON_KEY_ALT_F5		    0x353B337E
#define CON_KEY_CTRL_F5 		0x353B357E
#define CON_KEY_ALT_PAGEDOWN 	0x363B337E
#define CON_KEY_ALT_F6    		0x373B337E
#define CON_KEY_CTRL_F6 		0x373B357E
#define CON_KEY_ALT_F7    		0x383B337E
#define CON_KEY_ALT_F8 			0x393B337E
#define CON_KEY_CTRL_F7 		0x393B357E 
#define CON_KEY_F5        		0x5B31357E
#define CON_KEY_F6        		0x5B31377E
#define CON_KEY_F7        		0x5B31387E
#define CON_KEY_F8        		0x5B31397E
#define CON_KEY_F9        		0x5B32307E
#define CON_KEY_F12     		0x5B32347E
#define CON_KEY_RESIZE          0xFFFFEEEE

#define CON_MOUSE_LEFT_DOWN   32
#define CON_MOUSE_LEFT_UP     35
#define CON_MOUSE_RIGHT       34
#define CON_MOUSE_SCROLL_UP   96
#define CON_MOUSE_SCROLL_DOWN 97
#define CON_MOUSE_CTRL_LEFT_DOWN   48
#define CON_MOUSE_CTRL_LEFT_UP     51
#define CON_MOUSE_CTRL_RIGHT       50
#define CON_MOUSE_CTRL_SCROLL_UP   112
#define CON_MOUSE_CTRL_SCROLL_DOWN 113

#define CON_CLLS_RIGHT "0K"
#define CON_CLLS_LEFT  "1K"
#define CON_CLLS_ALL   "2K"
#define CON_CLLS_DOWN  "J"
#define CON_CLLS_UP    "1J"

#define print(S) fputs(S, stdout)
#define flush() fflush(stdout)

#define   RD_EOF   -1
#define   RD_EIO   -2

typedef uint_t utf8_t;

typedef struct _key
{
	uint_t value;
	uint8_t special;
}key_s;


int_t con_delay(int_t ms);
void con_flushin(void);
void con_async(int_t enable);
int_t con_kbhit(void);
void con_unget(int_t ch);
int_t con_getch(void);
key_s con_getkey();
void con_getmaxrc(uint_t* r, uint_t* c);
void con_gotorc(uint_t r, uint_t c);
void con_getrc(uint_t* r, uint_t* c);
void con_cls();
void con_clsline(char* mode);
void con_setcolor(uint8_t b, uint8_t f);
void con_setcolor256(uint8_t b, uint8_t f);
void con_showcursor(bool_t enable);
void con_special(char v);
void con_carret_up(uint_t n);
void con_carret_down(uint_t n);
void con_carret_next(uint_t n);
void con_carret_prev(uint_t n);
void con_carret_home();
void con_carret_end();
void con_carret_save();
void con_carret_restore();
void con_scrool_up();
void con_scrool_down();
void con_carret_delete(uint_t n);
void con_mode_ins(bool_t enable);
void con_linewrap(bool_t enable);
void con_vt100_reset();
void con_font_attribute(uint_t a);
void con_pause(void);
void utf8_putch(utf8_t ch);
char_t* utf8_char_putch(char_t* ch);
char_t* utf8_write(char_t* d, utf8_t ch);
utf8_t itoutf8(uint_t i);
void con_line(uint_t r1, uint_t c1, uint_t r2, uint_t c2, char_t c);
void con_rect(uint_t r, uint_t c, uint_t h, uint_t w, uint_t style);
void con_rect_fill(uint_t r, uint_t c, uint_t h, uint_t w, char_t fill);
void con_circle(uint_t r, uint_t c, uint_t ra, char_t ch);
void con_ellipse(uint_t cr, uint_t cc, uint_t sr, uint_t sc, char_t ch);
void con_mouse(bool_t enable);
void con_getmouse(uint_t* b, uint_t* y, uint_t* x);


#endif
