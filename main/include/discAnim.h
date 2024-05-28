#ifndef _DISCANIM_H_
#define _DISCANIM_H_

#include <gccore.h>

#define DISC_PATH   "/rvloader/discs"

extern u8 insertDisc;

int discAnim(u8* arcData, std::string gameIDString);

#endif