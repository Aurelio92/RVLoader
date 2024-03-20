function initGC2WiimoteConf()
    AUTO_INCREASE_DELAY = 500
    AUTO_INCREASE_TIME = 100
    AUTO_SCROLL_DELAY = 500
    AUTO_SCROLL_TIME = 100
    MAPPING_CONFIRM_TIME = 1000
    MAPPING_TIMEOUT = 3000

    STICKS_THRESHOLD = 30
    WM_CONFIG_LEFT_WIDTH = 250

    wmEmuEntriesEnum = enum({"Wiimote orientation", "Wiimote extension", "Motion plus enabled", "WM IR mode", "WM IR", "WM IR timeout", "WM DPAD-U", "WM DPAD-D", "WM DPAD-R", "WM DPAD-L", "WM A", "WM B", "WM 1", "WM 2", "WM Plus", "WM Minus", "WM Home", "WM Shake X", "WM Shake Y", "WM Shake Z"})
    nuEmuEntriesEnum = enum({"NU Stick", "NU C", "NU Z", "NU Shake X", "NU Shake Y", "NU Shake Z"})
    ccEmuEntriesEnum = enum({"CC DPAD-U", "CC DPAD-D", "CC DPAD-R", "CC DPAD-L", "CC A", "CC B", "CC X", "CC Y", "CC R analog", "CC L analog", "CC R digital", "CC L digital", "CC Plus", "CC Minus", "CC Home", "CC L Stick", "CC R Stick"})

    configModeEnum = enum({"GLOBAL_CONFIG", "CHANGE_MAPPING_STICK", "CHANGE_MAPPING_BUTTON", "CHANGE_MODIFIER", "CHANGE_NEG_MODIFIER"})
    configMode = configModeEnum["GLOBAL_CONFIG"]

    wmEmuScroll = 0
    wmEmuSelectedEntry = 1

    wmEmuAutoIncreaseTime = 0
    wmEmuAutoScrollTime = 0
    wmEmuPrevHeld = {}
    wmEmuConfigMenuTime = 0
end

function resetGC2WiimoteConf()
    wmEmuScroll = 0
    wmEmuSelectedEntry = 1
end

function drawGC2WiimoteConf()
    local divColor = Gfx.RGBA8(0x51, 0x51, 0x51, 0xFF)

    Gfx.pushMatrix()
    Gfx.pushScissorBox(getDimensions()[1], getDimensions()[2] - 32 - SETTINGS_SIDE_MARGIN)
    --Check scroll boundaries
    local tempMtx = Gfx.getCurMatrix()
    local sb = Gfx.getCurScissorBox()
    local nEntries = wmEmuEntriesEnum.size

    if GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
        nEntries = nEntries + nuEmuEntriesEnum.size
    elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
        nEntries = nEntries + ccEmuEntriesEnum.size
    end

    --Check if the selected entry is above the current view

    local centerY = tempMtx[2][4] + (wmEmuSelectedEntry - 1) * SETTING_EL_HEIGHT

    if (wmEmuScroll + centerY < sb.y + sb.height / 2) and wmEmuScroll < 0 then
        wmEmuScroll = sb.y + sb.height / 2 - centerY
    end

    if (wmEmuScroll + centerY > sb.y + sb.height / 2) and wmEmuScroll > -(nEntries * SETTING_EL_HEIGHT - sb.height) then
        wmEmuScroll = sb.y + sb.height / 2 - centerY
    end

    if wmEmuScroll > 0 then
        wmEmuScroll = 0
    end

    Gfx.pushMatrix()
    Gfx.translate(0, wmEmuScroll)

    Gfx.pushMatrix()
    local oldAlignment = Gfx.getFontVerticalAlignment(fonts[SETTING_FONT_SIZE])
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], Gfx.CENTER_ALIGN)

    for el in wmEmuEntriesEnum.all() do
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
        Gfx.print(fonts[SETTING_FONT_SIZE], SETTINGS_SIDE_MARGIN, SETTING_EL_HEIGHT / 2, el.name)
        local elValue = GamesView.getGC2WiimoteGameConfigValue(el.name)
        if el.name == "Wiimote orientation" then
            if elValue == GamesView.GC2Wiimote.ORIENT_STANDARD then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Standard")
            else
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Sideways")
            end
        elseif el.name == "Wiimote extension" then
            if elValue == GamesView.GC2Wiimote.EXT_NONE then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "None")
            elseif elValue == GamesView.GC2Wiimote.EXT_NUNCHUK then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Nunchuk")
            elseif elValue == GamesView.GC2Wiimote.EXT_CLASSIC then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Classic controller")
            end
        elseif el.name == "Motion plus enabled" then
            Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, ((elValue == 1) and "Yes" or "No"))
        elseif el.name == "WM IR mode" then
            if elValue == GamesView.GC2Wiimote.IRMODE_DIRECT then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Direct")
            else
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Shift")
            end
        elseif el.name == "WM IR timeout" then
            if elValue ~= 0 then
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, elValue .. " ms")
            else
                Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, "Never")
            end
        else
            Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, GamesView.getGC2WiimoteMapString(el.name))
        end
        Gfx.translate(0, SETTING_EL_HEIGHT)
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
    end
    if GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
        for el in nuEmuEntriesEnum.all() do
            Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
            Gfx.print(fonts[SETTING_FONT_SIZE], SETTINGS_SIDE_MARGIN, SETTING_EL_HEIGHT / 2, el.name)
            Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, GamesView.getGC2WiimoteMapString(el.name))
        Gfx.translate(0, SETTING_EL_HEIGHT)
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
        end
    end
    if GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
        for el in ccEmuEntriesEnum.all() do
            Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
            Gfx.print(fonts[SETTING_FONT_SIZE], SETTINGS_SIDE_MARGIN, SETTING_EL_HEIGHT / 2, el.name)
            Gfx.print(fonts[SETTING_FONT_SIZE], WM_CONFIG_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, GamesView.getGC2WiimoteMapString(el.name))
        Gfx.translate(0, SETTING_EL_HEIGHT)
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
        end
    end
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popMatrix()
    Gfx.drawRectangle(0, (wmEmuSelectedEntry - 1) * SETTING_EL_HEIGHT, getDimensions()[1], SETTING_EL_HEIGHT, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0x30))
    Gfx.popMatrix()

    if configMode ~= configModeEnum["GLOBAL_CONFIG"] then
        local down = Pad.down(0)
        local held = Pad.held(0)
        local stick = Pad.stick(0)
        local cStick = Pad.subStick(0)
        local s = stick.x * stick.x + stick.y * stick.y
        local c = cStick.x * cStick.x + cStick.y * cStick.y

        if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] then
            if stick.x > STICKS_THRESHOLD then
                held.STICKRIGHT = true
                held.STICKLEFT = false
            elseif stick.x < -STICKS_THRESHOLD then
                held.STICKRIGHT = false
                held.STICKLEFT = true
            else
                held.STICKRIGHT = false
                held.STICKLEFT = false
            end
            if stick.y > STICKS_THRESHOLD then
                held.STICKUP = true
                held.STICKDOWN = false
            elseif stick.y < -STICKS_THRESHOLD then
                held.STICKUP = false
                held.STICKDOWN = true
            else
                held.STICKUP = false
                held.STICKDOWN = false
            end

            if cStick.x > STICKS_THRESHOLD then
                held.SUBSTICKRIGHT = true
                held.SUBSTICKLEFT = false
            elseif cStick.x < -STICKS_THRESHOLD then
                held.SUBSTICKRIGHT = false
                held.SUBSTICKLEFT = true
            else
                held.SUBSTICKRIGHT = false
                held.SUBSTICKLEFT = false
            end
            if cStick.y > STICKS_THRESHOLD then
                held.SUBSTICKUP = true
                held.SUBSTICKDOWN = false
            elseif cStick.y < -STICKS_THRESHOLD then
                held.SUBSTICKUP = false
                held.SUBSTICKDOWN = true
            else
                held.SUBSTICKUP = false
                held.SUBSTICKDOWN = false
            end
        elseif configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
            held = {}
            if s > STICKS_THRESHOLD * STICKS_THRESHOLD then
                held.STICK = true
            else
                held.STICK = false
            end

            if c > STICKS_THRESHOLD * STICKS_THRESHOLD then
                held.SUBSTICK = true
            else
                held.SUBSTICK = false
            end
        end

        local padsStrings = {}
        if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] then
            padsStrings[1] = "Hold the buttons for 1 second to map to the selected action."
            padsStrings[2] = "Multiple selections will be considered as independently valid."
        elseif configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
            padsStrings[1] = "Hold the stick for 1 second to map to the selected action."
            padsStrings[2] = "Only one stick can be used."
        elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
            padsStrings[1] = "Hold the button for 1 second selected as modifier."
            padsStrings[2] = "Only one button can be used as modifier."
        elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
            padsStrings[1] = "Hold the button for 1 seconds selected as negative modifier."
            padsStrings[2] = "Only one button can be used as negative modifier."
        end

        padsStrings[4] = ""
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_LEFT and "DL|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_RIGHT and "DR|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_DOWN and "DD|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_UP and "DU|" or "")
        padsStrings[4] = padsStrings[4] .. (held.TRIGGER_Z and "Z|" or "")
        padsStrings[4] = padsStrings[4] .. (held.TRIGGER_R and "R|" or "")
        padsStrings[4] = padsStrings[4] .. (held.TRIGGER_L and "L|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_A and "A|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_B and "B|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_X and "X|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_Y and "Y|" or "")
        padsStrings[4] = padsStrings[4] .. (held.BUTTON_START and "Start|" or "")
        padsStrings[4] = padsStrings[4] .. (held.STICK and "Stick|" or "")
        padsStrings[4] = padsStrings[4] .. (held.SUBSTICK and "C-Stick|" or "")
        padsStrings[4] = padsStrings[4] .. (held.STICKLEFT and "StickRight|" or "")
        padsStrings[4] = padsStrings[4] .. (held.STICKRIGHT and "StickLeft|" or "")
        padsStrings[4] = padsStrings[4] .. (held.STICKUP and "StickUp|" or "")
        padsStrings[4] = padsStrings[4] .. (held.STICKDOWN and "StickDown|" or "")
        padsStrings[4] = padsStrings[4] .. (held.SUBSTICKLEFT and "C-StickRight|" or "")
        padsStrings[4] = padsStrings[4] .. (held.SUBSTICKRIGHT and "C-StickLeft|" or "")
        padsStrings[4] = padsStrings[4] .. (held.SUBSTICKUP and "C-StickUp|" or "")
        padsStrings[4] = padsStrings[4] .. (held.SUBSTICKDOWN and "C-StickDown|" or "")
        padsStrings[4] = padsStrings[4]:sub(1, #padsStrings[4] - 1) --Removes last '|'
        padsStrings[3] = "If nothing is held for 3 seconds this menu will disappear."

        Gfx.pushMatrix()
        Gfx.identity()
        Gfx.pushIdentityScissorBox()
        Gfx.drawRectangle(0, 0, Gui.getScreenSize().x, Gui.getScreenSize().y, Gfx.RGBA8(0x00, 0x00, 0x00, 0xA0))
        for i = 1, 4 do
            Gfx.print(fonts[SETTING_FONT_SIZE], (Gui.getScreenSize().x - Gfx.getTextWidth(fonts[SETTING_FONT_SIZE], padsStrings[i])) / 2, 100 + i * SETTING_EL_HEIGHT, padsStrings[i])
        end
        Gfx.popScissorBox()
        Gfx.popMatrix()

    end

    Gfx.popScissorBox()
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(SETTINGS_SIDE_MARGIN, getDimensions()[2] - 32 - SETTINGS_SIDE_MARGIN)
    Gfx.pushScissorBox(getDimensions()[1] - SETTINGS_SIDE_MARGIN, 32)
    local oldAlignment = Gfx.getFontVerticalAlignment(fonts[SETTING_FONT_SIZE])
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], Gfx.CENTER_ALIGN)
    local tempX = 0
    Gfx.drawImage(button_a_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Map")
    tempX = tempX + 16

    Gfx.drawImage(button_x_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Modifier")
    tempX = tempX + 16

    Gfx.drawImage(button_y_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Neg. modifier")
    tempX = tempX + 16

    Gfx.drawImage(button_s_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Save")
    tempX = tempX + 16

    Gfx.drawImage(button_b_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Cancel")
    tempX = tempX + 16
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popScissorBox()
    Gfx.popMatrix()
end

function handleGC2WiimoteConf()
    local nEntries = wmEmuEntriesEnum.size

    if GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
        nEntries = nEntries + nuEmuEntriesEnum.size
    elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
        nEntries = nEntries + ccEmuEntriesEnum.size
    end

    local wmEmuSelectedEntryName = ""
    if wmEmuSelectedEntry <= wmEmuEntriesEnum.size then
        wmEmuSelectedEntryName = wmEmuEntriesEnum[wmEmuSelectedEntry].name
    elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
        wmEmuSelectedEntryName = nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name
    elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
        wmEmuSelectedEntryName = ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name
    end

    if configMode == configModeEnum["GLOBAL_CONFIG"] then
        local down = Pad.gendown(0)
        local held = Pad.genheld(0)

        if down.BUTTON_DOWN and wmEmuSelectedEntry < nEntries then
            wmEmuSelectedEntry = wmEmuSelectedEntry + 1
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_DELAY
        end

        if down.BUTTON_UP and wmEmuSelectedEntry > 1 then
            wmEmuSelectedEntry = wmEmuSelectedEntry - 1
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_DELAY
        end

        if held.BUTTON_DOWN and wmEmuSelectedEntry < nEntries and Time.getms() > wmEmuAutoScrollTime then
            wmEmuSelectedEntry = wmEmuSelectedEntry + 1
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_TIME
        end

        if held.BUTTON_UP and wmEmuSelectedEntry > 1 and Time.getms() > wmEmuAutoScrollTime then
            wmEmuSelectedEntry = wmEmuSelectedEntry - 1
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_TIME
        end

        if down.BUTTON_A then
            if wmEmuSelectedEntryName == "Wiimote orientation" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.ORIENT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Wiimote extension" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.EXT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Motion plus enabled" then
                --TODO
            elseif wmEmuSelectedEntryName == "WM IR mode" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.IRMODE_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "WM IR timeout" then
                --Nothing to do here
            elseif  wmEmuSelectedEntryName == "WM IR" or --Sticks mapping
                    wmEmuSelectedEntryName == "NU Stick" or
                    wmEmuSelectedEntryName == "CC L Stick" or
                    wmEmuSelectedEntryName == "CC R Stick" then
                configMode = configModeEnum["CHANGE_MAPPING_STICK"]
                wmEmuConfigMenuTime = Time.getms()
            else
                configMode = configModeEnum["CHANGE_MAPPING_BUTTON"]
                wmEmuConfigMenuTime = Time.getms()
            end
        elseif down.BUTTON_RIGHT or (held.BUTTON_RIGHT and Time.getms() > wmEmuAutoIncreaseTime) then
            if wmEmuSelectedEntryName == "Wiimote orientation" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.ORIENT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Wiimote extension" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.EXT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Motion plus enabled" then
                --TODO
            elseif wmEmuSelectedEntryName == "WM IR mode" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1) % GamesView.GC2Wiimote.IRMODE_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "WM IR timeout" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 50) % 1000
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            end
            if down.BUTTON_RIGHT then
                wmEmuAutoIncreaseTime = Time.getms() + AUTO_INCREASE_DELAY
            else
                wmEmuAutoIncreaseTime = Time.getms() + AUTO_INCREASE_TIME
            end
        elseif down.BUTTON_LEFT or (held.BUTTON_LEFT and Time.getms() > wmEmuAutoIncreaseTime) then
            if wmEmuSelectedEntryName == "Wiimote orientation" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + GamesView.GC2Wiimote.ORIENT_MAX - 1) % GamesView.GC2Wiimote.ORIENT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Wiimote extension" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + GamesView.GC2Wiimote.EXT_MAX - 1) % GamesView.GC2Wiimote.EXT_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "Motion plus enabled" then
                --TODO
            elseif wmEmuSelectedEntryName == "WM IR mode" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + GamesView.GC2Wiimote.IRMODE_MAX - 1) % GamesView.GC2Wiimote.IRMODE_MAX
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            elseif wmEmuSelectedEntryName == "WM IR timeout" then
                local val = (GamesView.getGC2WiimoteGameConfigValue(wmEmuSelectedEntryName) + 1000 - 50) % 1000
                GamesView.setGC2WiimoteGameConfigValue(wmEmuSelectedEntryName, val)
            end
            if down.BUTTON_LEFT then
                wmEmuAutoIncreaseTime = Time.getms() + AUTO_INCREASE_DELAY
            else
                wmEmuAutoIncreaseTime = Time.getms() + AUTO_INCREASE_TIME
            end
        elseif down.BUTTON_X then
                configMode = configModeEnum["CHANGE_MODIFIER"]
                wmEmuConfigMenuTime = Time.getms()
        elseif down.BUTTON_Y then
                configMode = configModeEnum["CHANGE_NEG_MODIFIER"]
                wmEmuConfigMenuTime = Time.getms()
        elseif down.BUTTON_B then
            showingGC2Wiimote = false
        elseif down.BUTTON_START then
            GamesView.saveGC2WiimoteGameConfig()
            showingGC2Wiimote = false
        end
    else
        local down = Pad.down(0)
        local held = Pad.held(0)
        local stick = Pad.stick(0)
        local cStick = Pad.subStick(0)
        local s = stick.x * stick.x + stick.y * stick.y
        local c = cStick.x * cStick.x + cStick.y * cStick.y

        if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] then
            if stick.x > STICKS_THRESHOLD then
                held.STICKRIGHT = true
                held.STICKLEFT = false
            elseif stick.x < -STICKS_THRESHOLD then
                held.STICKRIGHT = false
                held.STICKLEFT = true
            else
                held.STICKRIGHT = false
                held.STICKLEFT = false
            end
            if stick.y > STICKS_THRESHOLD then
                held.STICKUP = true
                held.STICKDOWN = false
            elseif stick.y < -STICKS_THRESHOLD then
                held.STICKUP = false
                held.STICKDOWN = true
            else
                held.STICKUP = false
                held.STICKDOWN = false
            end

            if cStick.x > STICKS_THRESHOLD then
                held.SUBSTICKRIGHT = true
                held.SUBSTICKLEFT = false
            elseif cStick.x < -STICKS_THRESHOLD then
                held.SUBSTICKRIGHT = false
                held.SUBSTICKLEFT = true
            else
                held.SUBSTICKRIGHT = false
                held.SUBSTICKLEFT = false
            end
            if cStick.y > STICKS_THRESHOLD then
                held.SUBSTICKUP = true
                held.SUBSTICKDOWN = false
            elseif cStick.y < -STICKS_THRESHOLD then
                held.SUBSTICKUP = false
                held.SUBSTICKDOWN = true
            else
                held.SUBSTICKUP = false
                held.SUBSTICKDOWN = false
            end
            down.STICKRIGHT = not wmEmuPrevHeld.STICKRIGHT and held.STICKRIGHT
            down.STICKLEFT = not wmEmuPrevHeld.STICKLEFT and held.STICKLEFT
            down.STICKUP = not wmEmuPrevHeld.STICKUP and held.STICKUP
            down.STICKDOWN = not wmEmuPrevHeld.STICKDOWN and held.STICKDOWN
            down.SUBSTICKRIGHT = not wmEmuPrevHeld.SUBSTICKRIGHT and held.SUBSTICKRIGHT
            down.SUBSTICKLEFT = not wmEmuPrevHeld.SUBSTICKLEFT and held.SUBSTICKLEFT
            down.SUBSTICKUP = not wmEmuPrevHeld.SUBSTICKUP and held.SUBSTICKUP
            down.SUBSTICKDOWN = not wmEmuPrevHeld.SUBSTICKDOWN and held.SUBSTICKDOWN
        elseif configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
            held = {}
            if s > STICKS_THRESHOLD * STICKS_THRESHOLD then
                held.STICK = true
            else
                held.STICK = false
            end

            if c > STICKS_THRESHOLD * STICKS_THRESHOLD then
                held.SUBSTICK = true
            else
                held.SUBSTICK = false
            end
            down.STICK = not wmEmuPrevHeld.STICK and held.STICK
            down.SUBSTICK = not wmEmuPrevHeld.SUBSTICK and held.SUBSTICK
        end

        wmEmuPrevHeld = held

        if GamesView.getGC2WiimoteMapValue(down) ~= 0 then
            wmEmuConfigMenuTime = Time.getms()
        elseif (GamesView.getGC2WiimoteMapValue(held) ~= 0) and (Time.getms() - wmEmuConfigMenuTime) > MAPPING_CONFIRM_TIME then
            if wmEmuSelectedEntry <= wmEmuEntriesEnum.size then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(wmEmuEntriesEnum[wmEmuSelectedEntry].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(wmEmuEntriesEnum[wmEmuSelectedEntry].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(wmEmuEntriesEnum[wmEmuSelectedEntry].name, GamesView.getGC2WiimoteMapValue(held))
                end
            elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                end
            elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, GamesView.getGC2WiimoteMapValue(held))
                end
            end

            configMode = configModeEnum["GLOBAL_CONFIG"]
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_DELAY
        elseif (Time.getms() - wmEmuConfigMenuTime) > MAPPING_TIMEOUT then
            --Delete saved entry
            if wmEmuSelectedEntry <= wmEmuEntriesEnum.size then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(wmEmuEntriesEnum[wmEmuSelectedEntry].name, 0)
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(wmEmuEntriesEnum[wmEmuSelectedEntry].name, 0)
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(wmEmuEntriesEnum[wmEmuSelectedEntry].name, 0)
                end
            elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_NUNCHUK then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(nuEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                end
            elseif GamesView.getGC2WiimoteGameConfigValue("Wiimote extension") == GamesView.GC2Wiimote.EXT_CLASSIC then
                if configMode == configModeEnum["CHANGE_MAPPING_BUTTON"] or configMode == configModeEnum["CHANGE_MAPPING_STICK"] then
                    GamesView.setGC2WiimoteGameConfigValue(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                elseif configMode == configModeEnum["CHANGE_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigModifier(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                elseif configMode == configModeEnum["CHANGE_NEG_MODIFIER"] then
                    GamesView.setGC2WiimoteGameConfigNegModifier(ccEmuEntriesEnum[wmEmuSelectedEntry - wmEmuEntriesEnum.size].name, 0)
                end
            end

            configMode = configModeEnum["GLOBAL_CONFIG"]
            wmEmuAutoScrollTime = Time.getms() + AUTO_SCROLL_DELAY
        end

    end
end
