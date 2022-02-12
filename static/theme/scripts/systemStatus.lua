require 'scripts/enum'
require 'scripts/class'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'

statusSettings = class(SettingsMenu)

function statusSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)
end

function statusSettings:draw(onFocus)
    self.menuSystem:start(onFocus)
    if Gcp.isV1() then
        self.menuSystem:printLine("GC+1.0 detected")
    elseif Gcp.isV2() then
        self.menuSystem:printLine("GC+2.0 detected")
    end
    if UAMP.isConnected() then
        self.menuSystem:printLine("UAMP HUD detected")
    end
    if Time.available() then
        self.menuSystem:printLine("MX-Chip detected")
    end

    if PMS2.isConnected() then
        local chargeStatus = PMS2.getChargeStatus()

        if PMS2.isLite() then
            self.menuSystem:printLine("PMS Lite detected")
        else
            self.menuSystem:printLine("PMS2 detected")
        end

        if chargeStatus == PMS2.CHG_STAT_NOT_CHG then
            self.menuSystem:printLine("Not charging")
        elseif chargeStatus == PMS2.CHG_STAT_PRE_CHG or chargeStatus == PMS2.CHG_STAT_FAST_CHG then
            self.menuSystem:printLine("Charging")
        else
            self.menuSystem:printLine("Charging complete")
        end
        self.menuSystem:printLine(string.format("Battery SOC: %.1f", PMS2.getSOC()) .. "%%")
        self.menuSystem:printLine(string.format("Battery voltage: %.0f mV", PMS2.getVCell()))
        if not PMS2.isLite() then
            self.menuSystem:printLine(string.format("Battery current: %.0f mA", PMS2.getCurrent()))
        end
    end
    self.menuSystem:finish()
end

function statusSettings:handleInputs(onFocus)
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end
end
