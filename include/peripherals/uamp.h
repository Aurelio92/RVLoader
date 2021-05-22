#include <gccore.h>

namespace UAMP {
    bool isConnected();
    void init();
    void setMute(bool mute);
    void setHeadphonesVolume(u8 vol);
    void setSpeakersVolume(u8 vol);
};