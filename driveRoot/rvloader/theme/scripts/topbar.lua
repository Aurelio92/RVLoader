dofile("scripts/enum.lua")
dofile("scripts/topbarcmd.lua")

function myAtan2(y, x) --This always returns an angle in [0; 2pi)
    local angle = math.atan(y, x) + 2 * math.pi
    return angle % (2 * math.pi)
end

function modAngle(angle)
    return angle % (2 * math.pi)
end

function init()
    menuWheelEnabled = true
    sideMargin = 16
    batteryIcon = Gfx.loadImage("assets/battery_empty.png")
    topBarMessage = ""
    fonts = {}
    fonts[16] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 16)


    stickLoHiThreshold = 30
    stickHiLoThreshold = 20
    curSelection = -1
    timeout = 1000 --ms
    blinkPeriod = 250 --ms
    radius = 100
    cx = 10
    cy = 0
    states = enum({"idle", "active", "stickReleased", "selectionMade", "waitForRelease"})
    state = states.idle
    stateTimer = 0

    c_stick_inner = Gfx.loadImage("assets/switcher/c-stick_inner.png")
    select_slice = Gfx.loadImage("assets/switcher/select.png")
    main_wheel = Gfx.loadImage("assets/switcher/wheel.png")

    nIcons = 8
end

function draw(onFocus)
    Gfx.print(fonts[16], 16, 16, topBarMessage)

    if Time.available() then
        local timeStr = os.date("!%I:%M%p")
        Gfx.print(fonts[16], (getDimensions()[1] - Gfx.getTextWidth(fonts[16], timeStr)) // 2, 16, timeStr)
    end

    if PMS2.isConnected() then
        local batteryIcon_w, batteryIcon_h = Gfx.getImageSize(batteryIcon)
        local SOC = PMS2.getSOC()
        local SOCStr = string.format("%.0f", SOC) .. "%"
        local WiiTemperature = PMS2.getNTC()
        local WiiTemperatureStr = string.format("%.1f", WiiTemperature) .. " °C"
        local temperature_SOC_Str = ""

        if WiiTemperature ~= WiiTemperature then --Check is WiiTemperature is NaN, i.e. the NTC is disconnected
            temperature_SOC_Str = SOCStr
        else
            temperature_SOC_Str = WiiTemperatureStr .. "  |  " .. SOCStr
        end

        Gfx.pushMatrix()
        Gfx.translate(getDimensions()[1] - batteryIcon_w - sideMargin, 0)
        Gfx.print(fonts[16], -Gfx.getTextWidth(fonts[16], temperature_SOC_Str) - 8, 16, temperature_SOC_Str)
        Gfx.pushMatrix()
        Gfx.translate(0, (sideMargin + 40 - batteryIcon_h) / 2)
        Gfx.drawImage(batteryIcon)
        local chargeStatus = PMS2.getChargeStatus()

        if chargeStatus == PMS2.CHG_STAT_NOT_CHG then
            if (SOC < 10) then
                Gfx.drawRectangle(3, 3, 19 * SOC / 100, 9, Gfx.RGBA8(0xFF, 0x00, 0x00))
            else
                Gfx.drawRectangle(3, 3, 19 * SOC / 100, 9, Gfx.RGBA8(0x00, 0xFF, 0x00))
            end
        elseif chargeStatus == PMS2.CHG_STAT_PRE_CHG or chargeStatus == PMS2.CHG_STAT_FAST_CHG then
            Gfx.drawRectangle(3, 3, 19 * SOC / 100, 9, Gfx.RGBA8(0xFF, 0x80, 0x00))
        else --Charge complete
            Gfx.drawRectangle(3, 3, 19 * SOC / 100, 9, Gfx.RGBA8(0x00, 0x00, 0xFF))
        end
        Gfx.popMatrix()
        Gfx.popMatrix()
    end

    --Menuwheel
    if not menuWheelEnabled then
        return
    end

    if state == states.idle or state == states.waitForRelease then
        return
    end

    local w, h = Gfx.getImageSize(main_wheel)
    Gfx.pushMatrix()
    Gfx.translate((getDimensions()[1] - w) / 2, (getDimensions()[2] - h) / 2)
    Gfx.drawImage(main_wheel)
    local angleOffset = math.pi / nIcons
    local angleStep = 2 * math.pi / nIcons

    local w, h = Gfx.getImageSize(c_stick_inner)
    Gfx.drawImage(c_stick_inner, 157 - w // 2 + cx * 16 // 128, 157 - h // 2 - cy * 16 // 128)

    local angle = math.deg(math.floor(modAngle((myAtan2(cy, cx) + angleOffset)) / angleStep) * angleStep)
    if state == states.selectionMade then
        angle = math.deg(curSelection * angleStep)
    end
    Gfx.pushMatrix()
    Gfx.translate(157 + 52 - 16, 157 - 112)
    Gfx.pushMatrix()
    Gfx.rotate(-52 + 16, 112, -angle)
    if state == states.selectionMade then
        if (Time.getms() - stateTimer) % blinkPeriod < blinkPeriod / 2 then
            Gfx.drawImage(select_slice)
        end
    elseif stickRadius > stickLoHiThreshold then
        Gfx.drawImage(select_slice)
    end
    Gfx.popMatrix()
    Gfx.popMatrix()
    Gfx.popMatrix()
end

function handleMessage(msg)
    if msg.cmd == TOPBAR_CMD_SETTEXT then
        topBarMessage = msg.text
    elseif msg.cmd == TOPBAR_CMD_DISABLEWHEEL then
        menuWheelEnabled = false
    elseif msg.cmd == TOPBAR_CMD_ENABLEWHEEL then
        menuWheelEnabled = true
    end
end

function handleInputs(onFocus)
    if not menuWheelEnabled then
        return
    end

    local angleOffset = math.pi / nIcons
    local angleStep = 2 * math.pi / nIcons

    local substick = Pad.gensubStick(0)
    cx = substick.x
    cy = substick.y
    stickRadius = math.sqrt(cx * cx + cy * cy)

    if stickRadius > stickLoHiThreshold and state == states.idle then
        state = states.active
        stateTimer = Time.getms()
        curSelection = math.floor((math.atan(-cy, cx) + angleOffset) / angleStep)
    elseif state == states.active then
        local tempSelection = math.floor(modAngle((myAtan2(cy, cx) + angleOffset)) / angleStep)

        if stickRadius < stickHiLoThreshold then
            stateTimer = Time.getms()
            state = states.stickReleased
        end

        if tempSelection ~= curSelection then
            stateTimer = Time.getms()
            curSelection = tempSelection
        end

        if (Time.getms() - stateTimer) >= timeout then
            stateTimer = Time.getms()
            state = states.selectionMade
        end
    elseif state == states.stickReleased then
        if (Time.getms() - stateTimer) >= timeout then
            state = states.idle
        elseif stickRadius > stickLoHiThreshold then
            state = states.active
            stateTimer = Time.getms()
            curSelection = math.floor(modAngle((myAtan2(cy, cx) + angleOffset)) / angleStep)
        end
    elseif state == states.selectionMade then
        if (Time.getms() - stateTimer) >= timeout then
            stateTimer = Time.getms()
            state = states.waitForRelease
        end
    elseif state == states.waitForRelease then
        if curSelection == 0 then
            Gui.switchToElement("WiiGamesView")
        elseif curSelection == 1 then
            Gui.switchToElement("GCGamesView")
        elseif curSelection == 2 then
            Sys.bootSysMenu()
        elseif curSelection == 3 then
            Gui.switchToElement("SettingsView")
        elseif curSelection == 4 then
            Gui.switchToElement("WiiChannelsView")
        elseif curSelection == 5 then
            Gui.switchToElement("WiiHBView")
        elseif curSelection == 7 then
            Gui.switchToElement("VCGamesView")
        end
        if stickRadius < stickHiLoThreshold then
            state = states.idle
        end
    end
end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y
    return {w, h}
end
