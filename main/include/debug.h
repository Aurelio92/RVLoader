#ifndef _STFDEBUG_
#define _STFDEBUG_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    s32 DebugStart ();
    void Debug (const char *text, ...);
    void Debug_hexdump(void *d, int len);
    void gprintf (const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif