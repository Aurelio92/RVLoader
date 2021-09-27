function initSystemStatus()
end

function drawSystemStatus(onFocus)
    menuSystem.reset()
    if Gcp.isV1() then
        menuSystem.printLine("GC+1.0 detected")
    elseif Gcp.isV2() then
        menuSystem.printLine("GC+2.0 detected")
    end
    if UAMP.isConnected() then
        menuSystem.printLine("UAMP HUD detected")
    end
    if Time.available() then
        menuSystem.printLine("MX-Chip detected")
    end

    if PMS2.isConnected() then
        local chargeStatus = PMS2.getChargeStatus()

        if PMS2.isLite() then
            menuSystem.printLine("PMS Lite detected")
        else
            menuSystem.printLine("PMS2 detected")
        end

        if chargeStatus == PMS2.CHG_STAT_NOT_CHG then
            menuSystem.printLine("Not charging")
        elseif chargeStatus == PMS2.CHG_STAT_PRE_CHG or chargeStatus == PMS2.CHG_STAT_FAST_CHG then
            menuSystem.printLine("Charging")
        else
            menuSystem.printLine("Charging complete")
        end
        menuSystem.printLine(string.format("Battery SOC: %.1f", PMS2.getSOC()) .. "%%")
        menuSystem.printLine(string.format("Battery voltage: %.0f mV", PMS2.getVCell()))
        if not PMS2.isLite() then
            menuSystem.printLine(string.format("Battery current: %.0f mA", PMS2.getCurrent()))
        end
    end
end
