#pragma once

#define CHG_STAT_NOT_CHG    0
#define CHG_STAT_PRE_CHG    1
#define CHG_STAT_FAST_CHG   2
#define CHG_STAT_DONE       3

namespace PMS2 {
    typedef struct {
        u8 min;
        u8 max;
    } fanRange_t;

    bool isLite();
    u32 getUpdateProgress();
    bool isUpdating();
    bool hasUpdateSucceeded();
    void startUpdate(std::string path);
    bool isConnected();
    float getVer();
    void getPDProfiles(u32* profiles);
    u8 getConf0();
    u16 getPot();
    u16 getChargeCurrent();
    u16 getTermCurrent();
    u16 getPreChargeCurrent();
    u16 getChargeVoltage();
    u8 getTREG();
    u8 getChargeStatus();
    u16 getBatDesignCapacity();
    float getSOC();
    u16 getSOCRaw();
    float getVCell();
    u16 getVCellRaw();
    float getCurrent();
    s16 getCurrentRaw();
    u32 getTTE();
    u32 getTTF();
    void setConf0(u8 v);
    void setChargeCurrent(u16 v);
    void setTermCurrent(u16 v);
    void setPreChargeCurrent(u16 v);
    void setChargeVoltage(u16 v);
    void setTREG(u8 v);
    void setBatDesignCapacity(u16 v);
    void enableShippingMode();
    void flashConfig();
    void reconfigureMAX();
    float NTCToCelsius(u16 ntc);
    u16 CelsiusToNTC(float temperature);
    float getNTC();
    u8 getFanSpeed();
    void setFanSpeed(u8 speed);
    void freeFan();
    float getFanPIDkP();
    void setFanPIDkP(float kP);
    float getFanPIDkI();
    void setFanPIDkI(float kI);
    float getFanPIDkD();
    void setFanPIDkD(float kD);
    float getFanPIDTarget();
    void setFanPIDTarget(float target);
    fanRange_t getFanRange();
    void setFanRange(fanRange_t range);
    uint8_t getLEDIntensity();
    void setLEDIntensity(uint8_t intensity);
};
