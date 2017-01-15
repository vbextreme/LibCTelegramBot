#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <termios.h>
#include <wchar.h>
#include <tgb/terminale.h>
#include <tgb/utf8_extra.h>

extern utf8_t boxStyle[11][11];
extern utf8_t barStyle[3][9];
extern utf8_t arrowStyle[18][4];

__private struct termios initial_settings, new_settings;
__private uint_t asyncmode = 0;
__private uint_t _mouse_x = 0;
__private uint_t _mouse_y = 0;
__private uint_t _mouse_b = 0;
__private int_t  _unget   = 0;
 
#define  RD_EOF -1
#define  RD_EIO -2
#define DRD_EOF -1
#define DRD_EIO -2

#define con_puts(S) print(S)

struct cdirectrw
{
    int fd;
    int saved_errno;
    struct termios  saved;
    struct termios  temporary;
};


int con_drd(struct cdirectrw* dc)
{
    unsigned char   buffer[4];
    ssize_t         n;

    while (1) {

        n = read(dc->fd, buffer, 1);
        if (n > (ssize_t)0)
            return buffer[0];

        else
        if (n == (ssize_t)0)
            return DRD_EOF;

        else
        if (n != (ssize_t)-1)
            return DRD_EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return DRD_EIO;
    }
}

int con_dwr(struct cdirectrw* dc, const char *const data, const size_t bytes)
{
    const char       *head = data;
    const char *const tail = data + bytes;
    ssize_t           n;

    while (head < tail) {

        n = write(dc->fd, head, (size_t)(tail - head));
        if (n > (ssize_t)0)
            head += n;

        else
        if (n != (ssize_t)-1)
            return EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return errno;
    }

    return 0;
}

void con_dsrwhyde(struct cdirectrw* dc)
{
    dc->temporary.c_lflag &= ~ICANON;
    dc->temporary.c_lflag &= ~ECHO;
    dc->temporary.c_cflag &= ~CREAD;
}

int con_dsetting(struct cdirectrw* dc)
{
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->temporary);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;

    return 0;
}

int con_drestore(struct cdirectrw* dc)
{
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->saved);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;
    errno = dc->saved_errno;
    return 0;
}

int con_dopen(struct cdirectrw* dc)
{
    const char *dev;
    int result,retval;

    dev = ttyname(STDIN_FILENO);
    if (!dev)
        dev = ttyname(STDOUT_FILENO);
    if (!dev)
        dev = ttyname(STDERR_FILENO);
    if (!dev) {errno = ENOTTY;return -1;}

    do {
        dc->fd = open(dev, O_RDWR | O_NOCTTY);
    } while (dc->fd == -1 && errno == EINTR);
    if (dc->fd == -1)return -1;

    /* Bad tty? */

    dc->saved_errno = errno;

    /* Save current terminal settings. */
    do {
        result = tcgetattr(dc->fd, &dc->saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    /* Get current terminal settings for basis, too. */
    do {
        result = tcgetattr(dc->fd, &dc->temporary);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    return 0;
}


int_t con_delay(int_t ms)
{
	struct timespec tv;

	tv.tv_sec = (time_t) ms / 1000;
	
	tv.tv_nsec = (long_t) ((ms - (tv.tv_sec * 1000)) * 1000000L);

	while (1)
	{
		int rval = nanosleep (&tv, &tv);
		if (rval == 0)
			return 0;
		else if (errno == EINTR)
			continue;
		else
			return rval;
	}
	return 0;
}

void con_flushin(void)
{
    tcflush(0, TCIFLUSH);
}

void con_async(int_t enable)
{
    if ( enable )
    {
        if ( asyncmode ) return;
		asyncmode = enable;
        flush();
        con_flushin();
        tcgetattr(0,&initial_settings);
        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_lflag &= ~ISIG;
        new_settings.c_lflag &= ~IXON;
        if ( enable < 2 )
        {
			new_settings.c_cc[VMIN] = 1;
			new_settings.c_cc[VTIME] = 0;
		}
        tcsetattr(0, TCSANOW, &new_settings);
    }
    else
    {
        if ( !asyncmode ) return;
        flush();
        con_flushin();
        tcsetattr(0, TCSANOW, &initial_settings);
        asyncmode = 0;
    }
}

int_t con_kbhit(void)
{	
	if ( !asyncmode ) return 0;
	int_t toread;
	ioctl(0, FIONREAD, &toread);
    return toread;
}

__private int_t _kbhit(void)
{	
	int_t toread;
	ioctl(0, FIONREAD, &toread);
	return (_unget) ? toread + 1 : toread;
}

void con_unget(int_t ch)
{
	_unget = ch;
}

int_t con_getch(void)
{
    if ( _unget )
    {
		int_t ret = _unget;
		_unget = 0;
		return ret;
	}
	
	char ch;
	if ( read(0,&ch,1) < 1 ) 
	{
		return EOF;
	}
	
    return ch;
}

key_s con_getkey()
{
	int32_t c;
    key_s ret;
    
	if ( EOF == (c = con_getch()) )
	{
		ret.special = CON_SPECIAL_NORMAL;
		ret.value = EOF;
		return ret;
	}
	
	if ( c == CON_KEY_ESC )
	{
		dbg("vt100");
		ret.special = CON_SPECIAL_CONTROL;
		ret.value = c;
		
		while( 1 )
		{
			
			
			if ( !_kbhit() ) con_delay(TIME_RELAX);
			if ( !_kbhit() ) break;
			if ( EOF == (c = con_getch()) ) 
			{
				break;
			}
			if ( CON_KEY_ESC == c )
			{
				con_unget(c);
				break;
			}
			
			ret.value <<= 8;
			ret.value |= c;
			
			if ( ret.value == CON_KEY_MOUSE )
			{
				ret.special = CON_SPECIAL_MOUSE;
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_b = con_getch();
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_x = con_getch() - 32;
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_y = con_getch() - 32;
				break;
			}
		}
		
		dbg("return s:%u c:0x%x", ret.special, ret.value);
		return ret;
	}
	
	ret.special = 0;
	ret.value = c;
	
	uint8_t uBit = c;
	if ( !(uBit & 0x80) ) return ret;
	
	uBit <<= 1;
	while( (uBit & 0x80) )
	{
		ret.value <<= 8;
		uBit <<= 1;
		ret.value |= (uint8_t)con_getch();
	}
	return ret;
}

void con_getmaxrc(uint_t* r, uint_t* c)
{
    struct winsize ws;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws);
    *r = ws.ws_row;
    *c = ws.ws_col;
}

void con_gotorc(uint_t r, uint_t c)
{
    printf("\033[%u;%uf",r,c);
}

void con_getrc(uint_t* r, uint_t* c)
{
	bool_t restore = FALSE;
	
	if ( asyncmode )
	{
		restore = TRUE;
		con_async(0);
	}
	
    struct cdirectrw dc;

    if ( con_dopen(&dc) ) return;

    con_dsrwhyde(&dc);

    int rows,cols,result;

    do {

        if ( con_dsetting(&dc) ) break;

        if ( con_dwr(&dc, "\033[6n", 4) ) break;


        if ( (result = con_drd(&dc)) != 27) break;
        if ( (result = con_drd(&dc)) != '[')  break;

        /* Parse rows. */
        rows = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            rows = 10 * rows + result - '0';
            result = con_drd(&dc);
        }

        if (result != ';') break;

        /* Parse cols. */
        cols = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            cols = 10 * cols + result - '0';
            result = con_drd(&dc);
        }

        if (result != 'R') break;
        /* Success! */
        if (r) *r = rows;
        if (c) *c = cols;

    } while (0);

    /* Restore saved terminal settings. */
    con_drestore(&dc);
    
    if ( restore )
    {
		con_async(1);
	}
}

void con_cls()
{
    con_puts("\033[H\033[J");
}

void con_clsline(char* mode)
{
    printf("\033[%s",mode);
}


void con_setcolor(uint8_t b, uint8_t f)
{
    if (!b && !f)
    {
        con_puts("\033[m");
        return;
    }

    if (b) printf("\033[%um",b);
    if (f) printf("\033[%um",f);
}

void con_setcolor256(uint8_t b, uint8_t f)
{
    if (!b && !f)
    {
        con_puts("\033[m");
        return;
    }

    if (b) printf("\033[48;5;%um",b);
    if (f) printf("\033[38;5;%um",f);
}

void con_showcursor(bool_t enable)
{
	printf("\033[?25%c",(enable)?'h':'l');
}

void con_special(char v)
{
    printf("\033(0%c\033(B",v);
}

void con_carret_up(uint_t n)
{
	printf("\033[%dA",n);
}

void con_carret_down(uint_t n)
{
	printf("\033[%dB",n);
}

void con_carret_next(uint_t n)
{
	printf("\033[%dC",n);
}

void con_carret_prev(uint_t n)
{
	printf("\033[%dD",n);
}

void con_carret_home()
{
	con_puts("\033[H");
}

void con_carret_end()
{
	con_puts("\033[H");
}

void con_carret_save()
{
	con_puts("\033[s");
}

void con_carret_restore()
{
	con_puts("\033[u");
}

void con_scrool_up()
{
	con_puts("\033[M");
}

void con_scrool_down()
{
	con_puts("\033[D");
}

void con_carret_delete(uint_t n)
{
	printf("\033[%dP",n);
}

void con_mode_ins(bool_t enable)
{
	printf("\033[4%c",(enable)?'h':'l');
}

void con_linewrap(bool_t enable)
{
	printf("\033[?7%c",(enable)?'h':'l');
}

void con_vt100_reset()
{
	con_puts("\033[c");
}

void con_font_attribute(uint_t a)
{
	printf("\033[%dm",a);
}

void con_pause(void)
{
	puts("Press any key to continue");
	con_async(2);
		(void)con_getkey();
	con_async(0);
}

void utf8_putch(utf8_t ch)
{	
	dbg("putu8::0x%X",ch);
	if ( ch == 0 ) return;
	
	while( (ch & 0xFF000000) == 0 )
		ch <<= 8;
	
	while ( ch != 0 )
	{
		putchar( (ch & 0xFF000000) >> 24 );
		ch <<= 8;
	}
}

char_t* utf8_char_putch(char_t* ch)
{	
	if ( *ch == 0 )
	{
		dbg("end");
		return NULL;
	}
	
	if ( (*ch & 0xF0) == 0xF0)
	{
		dbg("4");
		utf8_t u = (utf8_t)*ch << 24;
		u |= (*(ch+1)) << 16;
		u |= (*(ch+2)) << 8;
		u |= *(ch+3);
		utf8_putch(u);
		return ch + 4;
	}
	
	if ( (*ch & 0xE0) == 0xE0)
	{
		dbg("3");
		utf8_t u = (utf8_t)*ch << 16;
		u |= (*(ch+1)) << 8;
		u |= *(ch+2);
		utf8_putch(u);
		return ch + 3;
	}
	
	if ( (*ch & 0xC0) == 0xC0 )
	{
		dbg("2");
		utf8_t u = (utf8_t)*ch << 8;
		u |= *(ch+1);
		utf8_putch(u);
		return ch + 2;
	}
	
	dbg("1");
	putchar(*ch);
	return ch + 1;
}


char_t* utf8_write(char_t* d, utf8_t ch)
{
	dbg("");	
	if ( ch == 0 ) return d;
	
	while( (ch & 0xFF000000) == 0 )
		ch <<= 8;
	
	while ( ch != 0 )
	{
		*d++ = (ch & 0xFF000000) >> 24;
		ch <<= 8;
	}
	
	return d;
}

utf8_t itoutf8(uint_t i)
{
	utf8_t ret;
	
	if ( i < 0x80 )
	{
		ret = (i & 0x7F);
		return ret;
	}
	
	if ( i < 0x800 )
	{
		ret = (0xC0 | (i >> 6)) << 8;
		ret |= 0x80 | (i & 0x3F);
		return ret;
	}
	
	if ( i < 0x010000 )
	{
		ret = (0xE0 | (i >> 12)) << 16;
		ret |= (0x80 | ((i >> 6) & 0x3F)) << 8;
		ret |= 0x80 | (i & 0x3F);
		return ret;
	}
	
	ret = (0xF0 | (i >> 18)) << 24;
	ret |= (0x80 | ((i >> 12) & 0x3F)) << 16;
	ret |= (0x80 | ((i >> 6) & 0x3F)) << 8;
	ret |= 0x80 | (i & 0x3F);
	
	return ret;
}

void con_line(uint_t r1, uint_t c1, uint_t r2, uint_t c2, char_t c)
{
    register int_t dx,dy,stepx,stepy,fraction;
    register int_t x0 = c1;
    register int_t x1 = c2;
    register int_t y0 = r1;
    register int_t y1 = r2;

    if(x0 != x1){if( x1 < x0){++x0;}else{--x0;}}
    if(y0 != y1){if( y1 < y0){++y0;}else{--y0;}}

    dx = x1 - x0;
    dy = y1 - y0;

    if (dy < 0) {dy=-dy;stepy=-1;}else{stepy=1;}
    if (dx < 0) {dx=-dx;stepx=-1;}else{stepx=1;}

    dx <<= 1;
    dy <<= 1;

    if (dx > dy)
    {
        fraction=dy - (dx >> 1);
        while (x0 != x1)
        {
            x0 += stepx;
            if (fraction >= 0)
                {y0+=stepy;fraction -= dx;}

            fraction += dy;
            con_gotorc(y0,x0);
            putchar(c);
        }
    }
    else
    {
        fraction=dx-(dy >> 1);
        while (y0 != y1)
        {
            y0 += stepy;

            if (fraction >= 0)
                {x0+=stepx;fraction-=dy;}

            fraction += dx;
            con_gotorc(y0,x0);
            putchar(c);
        }
    }
}

void con_rect(uint_t r, uint_t c, uint_t h, uint_t w, uint_t style)
{
    register uint_t ir,ic;
    utf8_t al[4];    
    uint_t connect = (style & 0xF0)>>4;
    uint_t ids = style & 0xF;
						  
    switch ( connect )
    {
		default: case UTF8_BOX_CONNECT_NONE:
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
		
		case UTF8_BOX_CONNECT_RIGHT:
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_DOWN];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_UP];
		break;
		
		case UTF8_BOX_CONNECT_LEFT:
			al[0] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_DOWN];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_UP];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
		
		case 0x3: /*UTF8_BOX_CONNECT_RIGHT & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_DOWN];
			al[1] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_DOWN];
			al[2] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_UP];
			al[3] = boxStyle[ids][UTF8_BOX_HORIZONTAL_INTERSECTION_UP];
		break;
		
		case UTF8_BOX_CONNECT_TOP:
			al[0] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[1] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
		
		case UTF8_BOX_CONNECT_BOTTOM:
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[3] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
		break;
		
		
		case 0xC: /*UTF8_BOX_CONNECT_TOP & UTF8_BOX_CONNECT_BOTTOM*/
			al[0] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[1] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
			al[2] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[3] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
		break;
		
		case 0x5: /*UTF8_BOX_CONNECT_TOP & UTF8_BOX_CONNECT_RIGHT*/
			al[0] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[1] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
    
		case 0x9: /*UTF8_BOX_CONNECT_BOTTOM & UTF8_BOX_CONNECT_RIGHT*/
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_RIGHT];
			al[3] = boxStyle[ids][UTF8_BOX_INTERSECTION];
		break;
		
		case 0x6: /*UTF8_BOX_CONNECT_TOP & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[1] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
    
		case 0xA: /*UTF8_BOX_CONNECT_BOTTOM & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[3] = boxStyle[ids][UTF8_BOX_VERTICAL_INTERSECTION_LEFT];
		break;
		
		case 0x7: /*UTF8_BOX_CONNECT_TOP & UTF8_BOX_CONNECT_RIGHT & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[1] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[2] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_LEFT];
			al[3] = boxStyle[ids][UTF8_BOX_CORNER_BOTTOM_RIGHT];
		break;
    
		case 0xB: /*UTF8_BOX_CONNECT_BOTTOM & UTF8_BOX_CONNECT_RIGHT & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_CORNER_TOP_LEFT];
			al[1] = boxStyle[ids][UTF8_BOX_CORNER_TOP_RIGHT];
			al[2] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[3] = boxStyle[ids][UTF8_BOX_INTERSECTION];
		break;
		
		case 0xF: /*UTF8_BOX_CONNECT_TOP & UTF8_BOX_CONNECT_BOTTOM & UTF8_BOX_CONNECT_RIGHT & UTF8_BOX_CONNECT_LEFT*/
			al[0] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[1] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[2] = boxStyle[ids][UTF8_BOX_INTERSECTION];
			al[3] = boxStyle[ids][UTF8_BOX_INTERSECTION];
		break;
		
	}
    
    con_gotorc(r,c);
    utf8_putch(al[0]);
    for (ic = 0; ic < w - 2 ; ++ic )
        utf8_putch(boxStyle[style][UTF8_BOX_HORIZONTAL]);
    utf8_putch(al[1]);

    con_gotorc(r + h - 1, c);
    utf8_putch(al[2]);
    for (ic = 0; ic < w - 2 ; ++ic )
        utf8_putch(boxStyle[style][UTF8_BOX_HORIZONTAL]);
    utf8_putch(al[3]);

    for (ir = r + 1; ir < r + h - 1 ; ir++ )
    {
        con_gotorc(ir,c);
        utf8_putch(boxStyle[style][UTF8_BOX_VERTICAL]);
        con_gotorc(ir,c + w - 1);
        utf8_putch(boxStyle[style][UTF8_BOX_VERTICAL]);
    }
}

void con_rect_fill(uint_t r, uint_t c, uint_t h, uint_t w, char_t fill)
{
    register unsigned int ir,ic;
    for (ir = r + 1 ; ir <  r + h - 1 ; ir++)
    {
        con_gotorc(ir,c + 1);
        for (ic = c + 1 ; ic < c + w - 1 ; ic++)
            putchar(fill);
    }
}

void con_circle(uint_t r, uint_t c, uint_t ra, char_t ch)
{

    register int x,y,
                 xc,yc,re;

    x=ra;
    y=0;
    xc=1-2*ra;
    yc=1;
    re=0;
    while(x>=y)
    {
        con_gotorc(r+y,c+x);
        putchar(ch);
        con_gotorc(r+y,c-x);
        putchar(ch);
        con_gotorc(r-y,c-x);
        putchar(ch);
        con_gotorc(r-y,c+x);
        putchar(ch);
        con_gotorc(r+x,c+y);
        putchar(ch);
        con_gotorc(r+x,c-y);
        putchar(ch);
        con_gotorc(r-x,c-y);
        putchar(ch);
        con_gotorc(r-x,c+y);
        putchar(ch);
        ++y;
        re+=yc;
        yc+=2;
        if (2*re+xc>0){
            --x;
            re+=xc;
            xc+=2;
        }
    }
}

void con_ellipse(uint_t cr, uint_t cc, uint_t sr, uint_t sc, char_t ch)
{
    register int rx_2,ry_2,
                 p,
                 x,y,
                 two_ry_2_x,two_rx_2_y;

    rx_2 = sc * sc;
    ry_2 = sr * sr;
    p = ry_2 - rx_2 * sr + (ry_2>>2);
    x = 0;
    y = sr;
    two_ry_2_x = 0;
    two_rx_2_y = (rx_2<<1)*y;

    con_gotorc(cr+y,cc+x);
    putchar(ch);
    con_gotorc(cr+y,cc-x);
    putchar(ch);
    con_gotorc(cr-y,cc-x);
    putchar(ch);
    con_gotorc(cr-y,cc+x);
    putchar(ch);

    while(two_rx_2_y >= two_ry_2_x)
    {
        ++x;
        two_ry_2_x += (ry_2<<1);
        p +=  two_ry_2_x + ry_2;

        if(p >= 0)
        {
            --y;
            two_rx_2_y -= (rx_2<<1);
            p -= two_rx_2_y ;
        }

        con_gotorc(cr+y,cc+x);
        putchar(ch);
        con_gotorc(cr+y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc+x);
        putchar(ch);
    }

    p = (int)(ry_2*(x+1/2.0)*(x+1/2.0) + rx_2*(y-1)*(y-1) - rx_2*ry_2);
    //p = (ry_2 * (x + 1) * (x + 1)  + rx_2 * (y - 1) * ( y - 1 ) - rx_2 * ry_2);

    while (y>=0)
    {
        p += rx_2;
        --y;
        two_rx_2_y -= (rx_2<<1);
        p -= two_rx_2_y;

        if(p <= 0)
        {
             ++x;
             two_ry_2_x += (ry_2<<1);
             p += two_ry_2_x;
        }

        con_gotorc(cr+y,cc+x);
        putchar(ch);
        con_gotorc(cr+y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc-x);
        putchar(ch);
        con_gotorc(cr-y,cc+x);
        putchar(ch);
    }
}

void con_mouse(bool_t enable)
{
	//uint_t old = asyncmode;
	
	//con_async(0);
	printf("\033[?1000%c",(enable)?'h':'l');
	flush();
	//con_async(old);
}

void con_getmouse(uint_t* b, uint_t* y, uint_t* x)
{
	*b = _mouse_b;
	*y = _mouse_y;
	*x = _mouse_x;
}
