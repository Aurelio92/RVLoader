#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ogc/usbgecko.h>
#include <ogc/exi.h>

//#define LOG_TO_FILE

static int geckolog = 0;

s32 DebugStart () {
	geckolog = usb_isgeckoalive (EXI_CHANNEL_1);

	return geckolog;
}

void Debug(const char *text, ...) {
    #ifdef LOG_TO_FILE
	   if (text == NULL) return;
    #else
        if (text == NULL || !geckolog) return;
    #endif

	int i;
	char mex[1024];

	va_list argp;
	va_start (argp, text);
	vsprintf (mex, text, argp);
	va_end (argp);

	if (geckolog) {
		usb_sendbuffer(EXI_CHANNEL_1, mex, strlen(mex));
		usb_flush(EXI_CHANNEL_1);
	}

    #ifdef LOG_TO_FILE
        FILE* fp = fopen("/rvloader/log.txt", "a");
        if (fp != NULL) {
            fprintf(fp, mex);
            fclose(fp);
        }
    #endif
}

static char ascii(char s) {
    if (s < 0x20) return '.';
    if (s > 0x7E) return '.';
    return s;
}

void gprintf (const char *format, ...) {
	char * tmp = NULL;
	va_list va;
	va_start(va, format);

	if ((vasprintf(&tmp, format, va) >= 0) && tmp) {
        usb_sendbuffer(EXI_CHANNEL_1, tmp, strlen(tmp));
	}
	va_end(va);

	if(tmp)
        free(tmp);
}

void Debug_hexdump (void *d, int len)
{
    u8 *data;
    int i, off;
    data = (u8*) d;

    gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
    gprintf("\n====  ===============================================  ================\n");

    for (off = 0; off < len; off += 16)
    {
        gprintf("%04x  ", off);
        for (i = 0; i < 16; i++)
            if ((i + off) >= len)
                gprintf("   ");
            else gprintf("%02x ", data[off + i]);

        gprintf(" ");
        for (i = 0; i < 16; i++)
            if ((i + off) >= len)
                gprintf(" ");
            else gprintf("%c", ascii(data[off + i]));
        gprintf("\n");
    }
}