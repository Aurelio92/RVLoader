#ifndef _APPLOADER_H_
#define _APPLOADER_H_

/* Entry point */
typedef void (*entry_point)(void);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Prototypes */
s32 Apploader_Run(entry_point *entry);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
