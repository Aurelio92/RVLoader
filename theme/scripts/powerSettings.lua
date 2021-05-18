--Code for PMS2 configuration

function initPower()
    pms2UpdateTimeout = 2000
    pms2PrevUpdating = false
    pms2OldVersion = 0.0
    pms2NewVersion = 0.0
    pms2UpdateTime = Time.getms() - pms2UpdateTimeout

    powerAUTO_INCREASE_DELAY = 500
    powerAUTO_INCREASE_TIME = 100
    powerAutoIncreaseTime = 0

    powerSelectedEnum = enum({"batCapacity", "chgCurrent", "termCurrent", "preCurrent", "chargeVoltage", "TREG", "pwrBtnType", "pwrBtnPol", "statLEDType", "saveConfig", "firmwareUpdate"})
    powerSelected = powerSelectedEnum[1]

    powerBatteryCapacity = PMS2.getBatDesignCapacity()
    powerChargingCurrent = PMS2.getChargeCurrent()
    powerTermCurrent = PMS2.getTermCurrent()
    powerPreChgCurrent = PMS2.getPreChargeCurrent()
    powerChargeVoltage = PMS2.getChargeVoltage()
    powerTREG = PMS2.getTREG()
    powerConf0 = PMS2.getConf0()
    powerOldBatteryCapacity = powerBatteryCapacity
    powerOldChargingCurrent = powerChargingCurrent
    powerOldTermCurrent = powerTermCurrent
    powerOldPreChgCurrent = powerPreChgCurrent
    powerOldChargeVoltage = powerChargeVoltage
    powerOldTREG = powerTREG
    powerOldConf0 = powerConf0
end

function drawPower(onFocus)
    if PMS2.isUpdating() == false and pms2PrevUpdating then
        pms2UpdateTime = Time.getms()
        pms2NewVersion = PMS2.getVer()
    end
    pms2PrevUpdating = PMS2.isUpdating()

    if PMS2.isUpdating() or (Time.getms() - pms2UpdateTime) < pms2UpdateTimeout then
        menuSystem.reset()
        menuSystem.printLine("Old version:", 0)
        menuSystem.printLineValue(string.format("%.1f", pms2OldVersion), false)
        menuSystem.printLine("Update progress:", 0)
        menuSystem.printLineValue(PMS2.getUpdateProgress() .. " %%", false)
        if not PMS2.isUpdating() then
            topBarEnableWheel()
            if PMS2.hasUpdateSucceeded then
                menuSystem.printLine("New version:", 0)
                menuSystem.printLineValue(string.format("%.1f", pms2NewVersion), false)
                menuSystem.printLine("Update complete! Will now return", 0)
            else
                menuSystem.printLine("Update failed", 0)
                menuSystem.printLine("Make sure you have /pms2.bin on your USB drive", 0)
            end
        else
            menuSystem.printLine("Updating, don't power off!", 0)
        end
        return
    end

    local colWidth = getDimensions()[1] - leftColumnWidth
    if onFocus then
        Gfx.drawRectangle(0, (powerSelected.id - 1) * lineHeight, colWidth, lineHeight, Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF));
    end
    menuSystem.reset()
    menuSystem.printLine("Battery capacity:", powerSelected.id)
    menuSystem.printLineValue(powerBatteryCapacity .. " mAh", powerBatteryCapacity ~= powerOldBatteryCapacity)
    menuSystem.printLine("Charging current:", powerSelected.id)
    menuSystem.printLineValue(powerChargingCurrent .. " mA", powerChargingCurrent ~= powerOldChargingCurrent)
    menuSystem.printLine("Termination current:", powerSelected.id)
    menuSystem.printLineValue(powerTermCurrent .. " mA", powerTermCurrent ~= powerOldTermCurrent)
    menuSystem.printLine("Precharge current:", powerSelected.id)
    menuSystem.printLineValue(powerPreChgCurrent .. " mA", powerPreChgCurrent ~= powerOldPreChgCurrent)
    menuSystem.printLine("Charge voltage:", powerSelected.id)
    menuSystem.printLineValue(powerChargeVoltage .. " mV", powerChargeVoltage ~= powerOldChargeVoltage)
    menuSystem.printLine("TREG:", powerSelected.id)
    menuSystem.printLineValue(powerTREG .. " °C", powerTREG ~= powerOldTREG)
    menuSystem.printLine("Power button type:", powerSelected.id)
    if (powerConf0 & PMS2.PWR_BTN_TYPE) == PMS2.PWR_BTN_MOMEN then
        menuSystem.printLineValue("Momentary", ((powerConf0 ~ powerOldConf0) & PMS2.PWR_BTN_TYPE) ~= 0)
    else
        menuSystem.printLineValue("Toggle", ((powerConf0 ~ powerOldConf0) & PMS2.PWR_BTN_TYPE) ~= 0)
    end
    menuSystem.printLine("Power button polarity:", powerSelected.id)
    if (powerConf0 & PMS2.PWR_BTN_POLARITY) == PMS2.PWR_BTN_ACTLOW then
        menuSystem.printLineValue("Active low", ((powerConf0 ~ powerOldConf0) & PMS2.PWR_BTN_POLARITY) ~= 0)
    else
        menuSystem.printLineValue("Active high", ((powerConf0 ~ powerOldConf0) & PMS2.PWR_BTN_POLARITY) ~= 0)
    end
    menuSystem.printLine("Status LED type:", powerSelected.id)
    if (powerConf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_STD then
        menuSystem.printLineValue("Standard", ((powerConf0 ~ powerOldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    else
        menuSystem.printLineValue("Addressable", ((powerConf0 ~ powerOldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    end
    menuSystem.printLine("Save config", powerSelected.id)
    menuSystem.printLine("Firmware Update", powerSelected.id)
end

function handlePower(onFocus)
    if PMS2.isUpdating() or (Time.getms() - pms2UpdateTime) < pms2UpdateTimeout then
        return
    end

    local down = Pad.gendown(0)
    local held = Pad.genheld(0)
    local curId = powerSelected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < powerSelectedEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if down.BUTTON_A then
        if powerSelected == powerSelectedEnum.saveConfig then --Save configuration into PMS2
            PMS2.setBatDesignCapacity(powerBatteryCapacity)
            PMS2.setChargeCurrent(powerChargingCurrent)
            PMS2.setTermCurrent(powerTermCurrent)
            PMS2.setPreChargeCurrent(powerPreChgCurrent)
            PMS2.setChargeVoltage(powerChargeVoltage)
            PMS2.setTREG(powerTREG)
            PMS2.setConf0(powerConf0)

            PMS2.flashConfig()
            if powerBatteryCapacity ~= powerOldBatteryCapacity or powerTermCurrent ~= powerOldTermCurrent or powerChargeVoltage ~= powerOldChargeVoltage then
                PMS2.reconfigureMAX()
            end

            powerOldBatteryCapacity = powerBatteryCapacity
            powerOldChargingCurrent = powerChargingCurrent
            powerOldTermCurrent = powerTermCurrent
            powerOldPreChgCurrent = powerPreChgCurrent
            powerOldChargeVoltage = powerChargeVoltage
            powerOldTREG = powerTREG
            powerOldConf0 = powerConf0
        elseif powerSelected == powerSelectedEnum.firmwareUpdate then
            topBarDisableWheel()
            pms2OldVersion = PMS2.getVer()
            PMS2.startUpdate("/pms2.bin")
        end
    elseif down.BUTTON_RIGHT or (held.BUTTON_RIGHT and Time.getms() > powerAutoIncreaseTime) then
        if powerSelected == powerSelectedEnum.batCapacity then
            powerBatteryCapacity = powerBatteryCapacity + 50
            if powerBatteryCapacity > 30000 then --Could handle higher capacities, but seriously?
                powerBatteryCapacity = 30000
            end
        elseif powerSelected == powerSelectedEnum.chgCurrent then
            powerChargingCurrent = powerChargingCurrent + 64
            if powerChargingCurrent > 4544 then
                powerChargingCurrent = 4544
            end
        elseif powerSelected == powerSelectedEnum.termCurrent then
            powerTermCurrent = powerTermCurrent + 128
            if powerTermCurrent > 2048 then
                powerTermCurrent = 2048
            end
        elseif powerSelected == powerSelectedEnum.preCurrent then
            powerPreChgCurrent = powerPreChgCurrent + 128
            if powerPreChgCurrent > 640 then
                powerPreChgCurrent = 640
            end
        elseif powerSelected == powerSelectedEnum.chargeVoltage then
            powerChargeVoltage = powerChargeVoltage + 16
            if powerChargeVoltage > 4400 then
                powerChargeVoltage = 4400
            end
        elseif powerSelected == powerSelectedEnum.TREG then
            powerTREG = powerTREG + 20
            if powerTREG > 120 then
                powerTREG = 120
            end
        elseif powerSelected == powerSelectedEnum.pwrBtnType then
            powerConf0 = powerConf0 ~ PMS2.PWR_BTN_TYPE
        elseif powerSelected == powerSelectedEnum.pwrBtnPol then
            powerConf0 = powerConf0 ~ PMS2.PWR_BTN_POLARITY
        elseif powerSelected == powerSelectedEnum.statLEDType then
            powerConf0 = powerConf0 ~ PMS2.STAT_LED_TYPE
        end

        if down.BUTTON_RIGHT then
            powerAutoIncreaseTime = Time.getms() + powerAUTO_INCREASE_DELAY
        else
            powerAutoIncreaseTime = Time.getms() + powerAUTO_INCREASE_TIME
        end
    elseif down.BUTTON_LEFT or (held.BUTTON_LEFT and Time.getms() > powerAutoIncreaseTime) then
        if powerSelected == powerSelectedEnum.batCapacity then
            powerBatteryCapacity = powerBatteryCapacity - 50
            if powerBatteryCapacity < 100 then
                powerBatteryCapacity = 100
            end
        elseif powerSelected == powerSelectedEnum.chgCurrent then
            powerChargingCurrent = powerChargingCurrent - 64
            if powerChargingCurrent < 512 then
                powerChargingCurrent = 512
            end
        elseif powerSelected == powerSelectedEnum.termCurrent then
            powerTermCurrent = powerTermCurrent - 128
            if powerTermCurrent < 128 then
                powerTermCurrent = 128
            end
        elseif powerSelected == powerSelectedEnum.preCurrent then
            powerPreChgCurrent = powerPreChgCurrent - 128
            if powerPreChgCurrent < 128 then
                powerPreChgCurrent = 128
            end
        elseif powerSelected == powerSelectedEnum.chargeVoltage then
            powerChargeVoltage = powerChargeVoltage - 16
            if powerChargeVoltage < 3504 then
                powerChargeVoltage = 3504
            end
        elseif powerSelected == powerSelectedEnum.TREG then
            powerTREG = powerTREG - 20
            if powerTREG < 60 then
                powerTREG = 60
            end
        elseif powerSelected == powerSelectedEnum.pwrBtnType then
            powerConf0 = powerConf0 ~ PMS2.PWR_BTN_TYPE
        elseif powerSelected == powerSelectedEnum.pwrBtnPol then
            powerConf0 = powerConf0 ~ PMS2.PWR_BTN_POLARITY
        elseif powerSelected == powerSelectedEnum.statLEDType then
            powerConf0 = powerConf0 ~ PMS2.STAT_LED_TYPE
        end

        if down.BUTTON_RIGHT then
            powerAutoIncreaseTime = Time.getms() + powerAUTO_INCREASE_DELAY
        else
            powerAutoIncreaseTime = Time.getms() + powerAUTO_INCREASE_TIME
        end
    end

    powerSelected = powerSelectedEnum[curId]
end