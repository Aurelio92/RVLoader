--Code for PMS2 configuration

require 'scripts/enum'
require 'scripts/class'
require 'scripts/topbarcmd'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'

powerSettings = class(SettingsMenu)
fanCalibrationMenu = class(SettingsMenu)

function powerSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.updateMenuSystem = MenuSystem()
    self.updateMenuSystem.font = font
    self.updateMenuSystem.lineHeight = lineHeight
    self.updateMenuSystem.columnWidth = columnWidth
    self.updateMenuSystem.sideMargin = sideMargin

    self.updateTimeout = 2000
    self.prevUpdating = false
    self.updateTime = Time.getms() - self.updateTimeout

    self.showingShippingModeDialog = false

    self.AUTO_INCREASE_DELAY = 500
    self.AUTO_INCREASE_TIME = 100
    self.autoIncreaseTime = 0

    if PMS2.isLite() then
        if PMS2.getVer() >= 1.2 then
            self.selectionEnum = enum({"chgCurrent", "termCurrent", "preCurrent", "chargeVoltage", "TREG", "pwrBtnType", "pwrBtnPol", "statLEDType", "statLEDIntensity", "fanMin", "fanMax", "PIDTarget", "PIDkP", "PIDkI", "PIDkD", "runFanCalibration", "saveConfig", "firmwareUpdate", "shippingMode"})
        else
            self.selectionEnum = enum({"chgCurrent", "termCurrent", "preCurrent", "chargeVoltage", "TREG", "pwrBtnType", "pwrBtnPol", "statLEDType", "saveConfig", "firmwareUpdate", "shippingMode"})
        end
    else
        if PMS2.getVer() >= 1.2 then
            self.selectionEnum = enum({"batCapacity", "chgCurrent", "termCurrent", "preCurrent", "chargeVoltage", "TREG", "pwrBtnType", "pwrBtnPol", "statLEDType", "statLEDIntensity", "fanMin", "fanMax", "PIDTarget", "PIDkP", "PIDkI", "PIDkD", "runFanCalibration", "saveConfig", "firmwareUpdate", "shippingMode"})
        else
            self.selectionEnum = enum({"batCapacity", "chgCurrent", "termCurrent", "preCurrent", "chargeVoltage", "TREG", "pwrBtnType", "pwrBtnPol", "statLEDType", "saveConfig", "firmwareUpdate", "shippingMode"})
        end
    end
    self.selected = self.selectionEnum[1]

    self.batteryCapacity = PMS2.getBatDesignCapacity()
    self.chargingCurrent = PMS2.getChargeCurrent()
    self.termCurrent = PMS2.getTermCurrent()
    self.preChgCurrent = PMS2.getPreChargeCurrent()
    self.chargeVoltage = PMS2.getChargeVoltage()
    self.TREG = PMS2.getTREG()
    self.conf0 = PMS2.getConf0()
    self.LEDIntensity = PMS2.getLEDIntensity()
    self.pms2Version = PMS2.getVer()
    if PMS2.getVer() >= 1.2 then
        self.fanRange = PMS2.getFanRange()
        self.fankP = PMS2.getFanPIDkP()
        self.fankI = PMS2.getFanPIDkI()
        self.fankD = PMS2.getFanPIDkD()
        self.PIDTarget = PMS2.getFanPIDTarget()
    end

    self.oldBatteryCapacity = self.batteryCapacity
    self.oldChargingCurrent = self.chargingCurrent
    self.oldTermCurrent = self.termCurrent
    self.oldPreChgCurrent = self.preChgCurrent
    self.oldChargeVoltage = self.chargeVoltage
    self.oldTREG = self.TREG
    self.oldConf0 = self.conf0
    self.oldLEDIntensity = self.LEDIntensity
    self.oldVersion = self.pms2Version
    if PMS2.getVer() >= 1.2 then
        self.fanOldRange = PMS2.getFanRange()
        self.oldkP = self.fankP
        self.oldkI = self.fankI
        self.oldkD = self.fankD
        self.oldPIDTarget = self.PIDTarget
    end

    self.runningFanCalibration = false
    self.fanCalibrationMenu = fanCalibrationMenu(font, lineHeight, columnWidth, sideMargin, self)
end

function powerSettings:draw(onFocus)
    if PMS2.isUpdating() == false and self.prevUpdating then
        self.updateTime = Time.getms()
        self.pms2Version = PMS2.getVer()
    end
    self.prevUpdating = PMS2.isUpdating()

    if PMS2.isUpdating() or (Time.getms() - self.updateTime) < self.updateTimeout then
        self.updateMenuSystem:start(onFocus)
        self.updateMenuSystem:printLine("Old version:", 0)
        self.updateMenuSystem:printLineValue(string.format("%.1f", self.oldVersion), false)
        self.updateMenuSystem:printLine("Update progress:", 0)
        self.updateMenuSystem:printLineValue(PMS2.getUpdateProgress() .. " %%", false)
        if not PMS2.isUpdating() then
            topBarEnableWheel()
            if PMS2.hasUpdateSucceeded() then
                self.updateMenuSystem:printLine("New version:", 0)
                self.updateMenuSystem:printLineValue(string.format("%.1f", self.pms2Version), false)
                self.updateMenuSystem:printLine("Update complete! Will now return", 0)
            else
                self.updateMenuSystem:printLine("Update failed", 0)
                if PMS2.isLite() then
                    self.updateMenuSystem:printLine("Make sure you have /pms2_lite.bin on your USB drive", 0)
                else
                    self.updateMenuSystem:printLine("Make sure you have /pms2.bin on your USB drive", 0)
                end
            end
        else
            self.updateMenuSystem:printLine("Updating, don't power off!", 0)
        end
        self.updateMenuSystem:finish()
        return
    end
    if self.runningFanCalibration then
        self.fanCalibrationMenu:draw(onFocus)
        return
    end

    self.menuSystem:start(onFocus)
    self.menuSystem:addSpacer("Info")
    self.menuSystem:printInfoLine("PMS2 version:")
    self.menuSystem:printLineValue(string.format("%.1f", self.pms2Version), false)

    self.menuSystem:addSpacer("Charge settings")
    if not PMS2.isLite() then
        self.menuSystem:printLine("Battery capacity:", self.selected.id)
        self.menuSystem:printLineValue(self.batteryCapacity .. " mAh", self.batteryCapacity ~= self.oldBatteryCapacity)
    end
    self.menuSystem:printLine("Charging current:", self.selected.id)
    self.menuSystem:printLineValue(self.chargingCurrent .. " mA", self.chargingCurrent ~= self.oldChargingCurrent)
    self.menuSystem:printLine("Termination current:", self.selected.id)
    self.menuSystem:printLineValue(self.termCurrent .. " mA", self.termCurrent ~= self.oldTermCurrent)
    self.menuSystem:printLine("Precharge current:", self.selected.id)
    self.menuSystem:printLineValue(self.preChgCurrent .. " mA", self.preChgCurrent ~= self.oldPreChgCurrent)
    self.menuSystem:printLine("Charge voltage:", self.selected.id)
    self.menuSystem:printLineValue(self.chargeVoltage .. " mV", self.chargeVoltage ~= self.oldChargeVoltage)
    self.menuSystem:printLine("TREG:", self.selected.id)
    self.menuSystem:printLineValue(self.TREG .. " °C", self.TREG ~= self.oldTREG)

    self.menuSystem:addSpacer("Misc settings")
    self.menuSystem:printLine("Power button type:", self.selected.id)
    if (self.conf0 & PMS2.PWR_BTN_TYPE) == PMS2.PWR_BTN_MOMEN then
        self.menuSystem:printLineValue("Momentary", ((self.conf0 ~ self.oldConf0) & PMS2.PWR_BTN_TYPE) ~= 0)
    else
        self.menuSystem:printLineValue("Toggle", ((self.conf0 ~ self.oldConf0) & PMS2.PWR_BTN_TYPE) ~= 0)
    end
    self.menuSystem:printLine("Power button polarity:", self.selected.id)
    if (self.conf0 & PMS2.PWR_BTN_POLARITY) == PMS2.PWR_BTN_ACTLOW then
        self.menuSystem:printLineValue("Active low", ((self.conf0 ~ self.oldConf0) & PMS2.PWR_BTN_POLARITY) ~= 0)
    else
        self.menuSystem:printLineValue("Active high", ((self.conf0 ~ self.oldConf0) & PMS2.PWR_BTN_POLARITY) ~= 0)
    end
    self.menuSystem:printLine("Status LED type:", self.selected.id)
    if (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_STD then
        self.menuSystem:printLineValue("Standard", ((self.conf0 ~ self.oldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    elseif self.pms2Version < 1.2 then
        self.menuSystem:printLineValue("Addressable", ((self.conf0 ~ self.oldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSD then
        self.menuSystem:printLineValue("Addressable type D", ((self.conf0 ~ self.oldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSB then
        self.menuSystem:printLineValue("Addressable type B", ((self.conf0 ~ self.oldConf0) & PMS2.STAT_LED_TYPE) ~= 0)
    end

    if self.pms2Version >= 1.2 then
        self.menuSystem:printLine("Status LED intensity:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%u", self.LEDIntensity), self.LEDIntensity ~= self.oldLEDIntensity)
        self.menuSystem:addSpacer("Fan")
        self.menuSystem:printLine("Fan speed min:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%u", self.fanRange.min), self.fanRange.min ~= self.fanOldRange.min)
        self.menuSystem:printLine("Fan speed max:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%u", self.fanRange.max), self.fanRange.max ~= self.fanOldRange.max)
        self.menuSystem:printLine("Target temperature:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%.1f", self.PIDTarget) .. " °C", self.PIDTarget ~= self.oldPIDTarget)
        self.menuSystem:printLine("PID kP:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%.4f", self.fankP), self.fankP ~= self.oldkP)
        self.menuSystem:printLine("PID kI:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%.4f", self.fankI), self.fankI ~= self.oldkI)
        self.menuSystem:printLine("PID kD:", self.selected.id)
        self.menuSystem:printLineValue(string.format("%.4f", self.fankD), self.fankD ~= self.oldkD)
        self.menuSystem:printLine("Run calibration", self.selected.id)
        self.menuSystem:printInfoLine("Current fan speed:")
        self.menuSystem:printLineValue(string.format("%u", PMS2.getFanSpeed()), false)
    end

    self.menuSystem:addSpacer("Save")
    self.menuSystem:printLine("Save config", self.selected.id)

    self.menuSystem:addSpacer("Firmware settings")
    self.menuSystem:printLine("Firmware update", self.selected.id)
    self.menuSystem:printLine("Enable shipping mode", self.selected.id)
    self.menuSystem:finish()

    if self.showingShippingModeDialog then
        local dialogText = {"The system will turn off.", "Power can be re-enabled only by plugging in a charger.", "Press A to confirm, B to cancel."}

        local sb = Gfx.getCurScissorBox()
        Gfx.pushMatrix()
        Gfx.identity()
        Gfx.pushIdentityScissorBox()
        Gfx.drawRectangle(0, 0, Gui.getScreenSize().x, Gui.getScreenSize().y, Gfx.RGBA8(0x00, 0x00, 0x00, 0xA0))
        for i = 1, #dialogText do
            Gfx.print(self.menuSystem.font, sb.x + (getDimensions()[1] - sb.x - Gfx.getTextWidth(self.menuSystem.font, dialogText[i])) / 2, 100 + i * self.menuSystem.lineHeight, dialogText[i])
        end
        Gfx.popScissorBox()
        Gfx.popMatrix()
    end
end

function powerSettings:handleInputs(onFocus)
    if PMS2.isUpdating() or (Time.getms() - self.updateTime) < self.updateTimeout then
        return
    end

    if self.runningFanCalibration then
        self.fanCalibrationMenu:handleInputs(onFocus)
        return
    end

    local down = Pad.gendown(0)
    local held = Pad.genheld(0)

    if self.showingShippingModeDialog then
        if down.BUTTON_A then
            PMS2.enableShippingMode()
        end

        if down.BUTTON_B then
            self.showingShippingModeDialog = false
            topBarEnableWheel()
        end
        return
    end

    local curId = self.selected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < self.selectionEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if down.BUTTON_A then
        if self.selected == self.selectionEnum.saveConfig then --Save configuration into PMS2
            PMS2.setBatDesignCapacity(self.batteryCapacity)
            PMS2.setChargeCurrent(self.chargingCurrent)
            PMS2.setTermCurrent(self.termCurrent)
            PMS2.setPreChargeCurrent(self.preChgCurrent)
            PMS2.setChargeVoltage(self.chargeVoltage)
            PMS2.setTREG(self.TREG)
            PMS2.setConf0(self.conf0)
            if PMS2.getVer() >= 1.2 then
                PMS2.setLEDIntensity(self.LEDIntensity)
                PMS2.setFanPIDkP(self.fankP)
                PMS2.setFanPIDkI(self.fankI)
                PMS2.setFanPIDkD(self.fankD)
                PMS2.setFanRange(self.fanRange)
                PMS2.setFanPIDTarget(self.PIDTarget)
            end

            PMS2.flashConfig()
            if self.batteryCapacity ~= self.oldBatteryCapacity or self.termCurrent ~= self.oldTermCurrent or self.chargeVoltage ~= self.oldChargeVoltage then
                PMS2.reconfigureMAX()
            end

            self.oldBatteryCapacity = self.batteryCapacity
            self.oldChargingCurrent = self.chargingCurrent
            self.oldTermCurrent = self.termCurrent
            self.oldPreChgCurrent = self.preChgCurrent
            self.oldChargeVoltage = self.chargeVoltage
            self.oldTREG = self.TREG
            self.oldConf0 = self.conf0
            if PMS2.getVer() >= 1.2 then
                self.oldLEDIntensity = self.LEDIntensity
                self.fanOldRange = PMS2.getFanRange()
                self.oldkP = self.fankP
                self.oldkI = self.fankI
                self.oldkD = self.fankD
                self.oldPIDTarget = self.PIDTarget
            end
        elseif self.selected == self.selectionEnum.runFanCalibration then
            self.runningFanCalibration = true
        elseif self.selected == self.selectionEnum.firmwareUpdate then
            topBarDisableWheel()
            if PMS2.isLite() then
                PMS2.startUpdate("/pms2_lite.bin")
            else
                PMS2.startUpdate("/pms2.bin")
            end
        elseif self.selected == self.selectionEnum.shippingMode then
            topBarDisableWheel()
            self.showingShippingModeDialog = true
        end
    elseif down.BUTTON_RIGHT or (held.BUTTON_RIGHT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.batCapacity then
            self.batteryCapacity = self.batteryCapacity + 50
            if self.batteryCapacity > 30000 then --Could handle higher capacities, but seriously?
                self.batteryCapacity = 30000
            end
        elseif self.selected == self.selectionEnum.chgCurrent then
            self.chargingCurrent = self.chargingCurrent + 64
            if self.chargingCurrent > 4544 then
                self.chargingCurrent = 4544
            end
        elseif self.selected == self.selectionEnum.termCurrent then
            self.termCurrent = self.termCurrent + 128
            if self.termCurrent > 2048 then
                self.termCurrent = 2048
            end
        elseif self.selected == self.selectionEnum.preCurrent then
            self.preChgCurrent = self.preChgCurrent + 128
            if self.preChgCurrent > 640 then
                self.preChgCurrent = 640
            end
        elseif self.selected == self.selectionEnum.chargeVoltage then
            self.chargeVoltage = self.chargeVoltage + 16
            if self.chargeVoltage > 4400 then
                self.chargeVoltage = 4400
            end
        elseif self.selected == self.selectionEnum.TREG then
            self.TREG = self.TREG + 20
            if self.TREG > 120 then
                self.TREG = 120
            end
        elseif self.selected == self.selectionEnum.pwrBtnType then
            self.conf0 = self.conf0 ~ PMS2.PWR_BTN_TYPE
        elseif self.selected == self.selectionEnum.pwrBtnPol then
            self.conf0 = self.conf0 ~ PMS2.PWR_BTN_POLARITY
        elseif self.selected == self.selectionEnum.statLEDType then
            if (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_STD then
                self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_WSD
            elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSD then
                if self.pms2Version < 1.2 then
                    self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_STD
                else
                    self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_WSB
                end
            elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSB then
                self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_STD
            end
        elseif self.selected == self.selectionEnum.statLEDIntensity then
            self.LEDIntensity = self.LEDIntensity + 5
            if self.LEDIntensity > 255 then
                self.LEDIntensity = 255
            end
        elseif self.selected == self.selectionEnum.fanMin then
            self.fanRange.min = self.fanRange.min + 5
            if self.fanRange.min > 255 then
                self.fanRange.min = 255
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.fanMax then
            self.fanRange.max = self.fanRange.max + 5
            if self.fanRange.max > 255 then
                self.fanRange.max = 255
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.PIDTarget then
            self.PIDTarget = self.PIDTarget + 1
            if self.PIDTarget > 50 then
                self.PIDTarget = 50
            end
        elseif self.selected == self.selectionEnum.PIDkP then
            self.fankP = self.fankP + 0.2
            if self.fankP > 100 then
                self.fankP = 100
            end
        elseif self.selected == self.selectionEnum.PIDkI then
            self.fankI = self.fankI + 0.02
            if self.fankI > 100 then
                self.fankI = 100
            end
        elseif self.selected == self.selectionEnum.PIDkD then
            self.fankD = self.fankD + 0.02
            if self.fankD > 100 then
                self.fankD = 100
            end
        end

        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    elseif down.BUTTON_LEFT or (held.BUTTON_LEFT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.batCapacity then
            self.batteryCapacity = self.batteryCapacity - 50
            if self.batteryCapacity < 100 then
                self.batteryCapacity = 100
            end
        elseif self.selected == self.selectionEnum.chgCurrent then
            self.chargingCurrent = self.chargingCurrent - 64
            if self.chargingCurrent < 512 then
                self.chargingCurrent = 512
            end
        elseif self.selected == self.selectionEnum.termCurrent then
            self.termCurrent = self.termCurrent - 128
            if self.termCurrent < 128 then
                self.termCurrent = 128
            end
        elseif self.selected == self.selectionEnum.preCurrent then
            self.preChgCurrent = self.preChgCurrent - 128
            if self.preChgCurrent < 128 then
                self.preChgCurrent = 128
            end
        elseif self.selected == self.selectionEnum.chargeVoltage then
            self.chargeVoltage = self.chargeVoltage - 16
            if self.chargeVoltage < 3504 then
                self.chargeVoltage = 3504
            end
        elseif self.selected == self.selectionEnum.TREG then
            self.TREG = self.TREG - 20
            if self.TREG < 60 then
                self.TREG = 60
            end
        elseif self.selected == self.selectionEnum.pwrBtnType then
            self.conf0 = self.conf0 ~ PMS2.PWR_BTN_TYPE
        elseif self.selected == self.selectionEnum.pwrBtnPol then
            self.conf0 = self.conf0 ~ PMS2.PWR_BTN_POLARITY
        elseif self.selected == self.selectionEnum.statLEDType then
            if (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_STD then
                if self.pms2Version < 1.2 then
                    self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_WSD
                else
                    self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_WSB
                end
            elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSD then
                self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_STD
            elseif (self.conf0 & PMS2.STAT_LED_TYPE) == PMS2.STAT_LED_WSB then
                self.conf0 = (self.conf0 & ~PMS2.STAT_LED_TYPE) | PMS2.STAT_LED_WSD
            end
        elseif self.selected == self.selectionEnum.statLEDIntensity then
            self.LEDIntensity = self.LEDIntensity - 5
            if self.LEDIntensity < 0 then
                self.LEDIntensity = 0
            end
        elseif self.selected == self.selectionEnum.fanMin then
            self.fanRange.min = self.fanRange.min - 5
            if self.fanRange.min < 0 then
                self.fanRange.min = 0
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.fanMax then
            self.fanRange.max = self.fanRange.max - 5
            if self.fanRange.max < 130 then
                self.fanRange.max = 130
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.PIDTarget then
            self.PIDTarget = self.PIDTarget - 1
            if self.PIDTarget < 25 then
                self.PIDTarget = 25
            end
        elseif self.selected == self.selectionEnum.PIDkP then
            self.fankP = self.fankP - 0.2
            if self.fankP < 0 then
                self.fankP = 0
            end
        elseif self.selected == self.selectionEnum.PIDkI then
            self.fankI = self.fankI - 0.02
            if self.fankI < 0 then
                self.fankI = 0
            end
        elseif self.selected == self.selectionEnum.PIDkD then
            self.fankD = self.fankD - 0.02
            if self.fankD < 0 then
                self.fankD = 0
            end
        end

        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    end

    self.selected = self.selectionEnum[curId]
end

function fanCalibrationMenu:init(font, lineHeight, columnWidth, sideMargin, parent)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.powerSettings = parent

    self.calibrationStateEnum = enum({"init", "rangeWizard", "heating", "stablizing", "done"})
    self.calibrationState = self.calibrationStateEnum[1]
    --self.reset()
end

function fanCalibrationMenu:reset()
    self.calibrationTimer = 0
    self.CALIBRATION_TIME = 2 * 60000
    self.calibrationLoop = 0
    self.CALIBRATION_LOOPS = 3

    self.fanMinSpeedWizard = 128
    self.fanMinSpeedWizardRange = {0, 255}
    self.fanLastMinSpinningSpeed = 0
    self.fanMinSpeedWizardRestartTime = Time.getms()
    self.fanMinSpeedWizardRestartTimeout = 1000
    PMS2.setFanSpeed(self.fanMinSpeedWizardVar)
end

function fanCalibrationMenu:draw(onFocus)
    self.menuSystem:start(onFocus)

    if self.calibrationState == self.calibrationStateEnum.rangeWizard then
        self.menuSystem:printInfoLine("Current fan speed:")
        self.menuSystem:printLineValue(string.format("%u", self.fanMinSpeedWizard), false)
        self.menuSystem:printInfoLine("Press A if you hear the fan spinning")
        self.menuSystem:printInfoLine("Otherwise press B")
    end

    self.menuSystem:printInfoLine("Hold L+R to cancel calibration")

    self.menuSystem:finish()
end

function fanCalibrationMenu:handleInputs(onFocus)
    local down = Pad.gendown(0)
    local held = Pad.genheld(0)

    if held.TRIGGER_R and held.TRIGGER_L then
        --Restore old fan settings
        PMS2.setFanRange(self.powerSettings.fanOldRange)
        PMS2.setFanPIDkP(self.powerSettings.oldkP)
        PMS2.setFanPIDkI(self.powerSettings.oldkI)
        PMS2.setFanPIDkD(self.powerSettings.oldkD)
        PMS2.setFanPIDTarget(self.powerSettings.oldPIDTarget)
        PMS2.freeFan()
        topBarEnableWheel()
        self.powerSettings.runningFanCalibration = false
        return
    end

    if self.calibrationState == self.calibrationStateEnum.init then
        self.reset()
        self.calibrationState = self.calibrationStateEnum.rangeWizard
    elseif self.calibrationState == self.calibrationStateEnum.rangeWizard then
        --Set fan speed
        if (Time.getms() - self.fanMinSpeedWizardRestartTime) < self.fanMinSpeedWizardRestartTimeout then
            PMS2.setFanSpeed(0)
        else
            PMS2.setFanSpeed(self.fanMinSpeedWizardVar)
        end

        if down.BUTTON_A then
            --Fan spinning, try reducing min speed
            self.fanLastMinSpinningSpeed = self.fanMinSpeedWizard
            self.fanMinSpeedWizardRange = {self.fanMinSpeedWizardRange[1], self.fanMinSpeedWizard}
            self.fanMinSpeedWizard = (self.fanMinSpeedWizardRange[1] + self.fanMinSpeedWizardRange[2]) // 2
            self.fanMinSpeedWizardRestartTime = Time.getms()
        elseif down.BUTTON_B then
            --Fan not spinning, try increasing max speed
            self.fanMinSpeedWizardRange = {self.fanMinSpeedWizard, self.fanMinSpeedWizardRange[2]}
            self.fanMinSpeedWizard = (self.fanMinSpeedWizardRange[1] + self.fanMinSpeedWizardRange[2]) // 2
            self.fanMinSpeedWizardRestartTime = Time.getms()
        end

        --Min speed calibration done. Go to next step
        if (down.BUTTON_A or down.BUTTON_B) and (self.fanMinSpeedWizardRange[2] - self.fanMinSpeedWizardRange[1]) < 8 then

        end
    end
end