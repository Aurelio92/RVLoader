function initGCPMapConf()
    GCPAUTO_SCROLL_DELAY = 500
    GCPAUTO_SCROLL_TIME = 100
    GCPMAPPING_CONFIRM_TIME = 1000
    GCPMAPPING_TIMEOUT = 3000

    GCP_MAP_LEFT_WIDTH = 250

    GCPMapEntriesEnum = enum({"A", "B", "X", "Y", "Up", "Down", "Right", "Left", "Z", "R", "L", "Start"})

    GCPConfigModeEnum = enum({"GLOBAL_CONFIG", "CHANGE_MAPPING_BUTTON"})
    GCPConfigMode = GCPConfigModeEnum["GLOBAL_CONFIG"]

    GCPScroll = 0
    GCPSelectedEntry = 1

    GCPAutoIncreaseTime = 0
    GCPAutoScrollTime = 0

    GCPConfigMenuTime = 0
end

function drawGCPMapConf()
    local divColor = Gfx.RGBA8(0x51, 0x51, 0x51, 0xFF)

    Gfx.pushMatrix()
    Gfx.pushScissorBox(getDimensions()[1], getDimensions()[2] - 32 - SETTINGS_SIDE_MARGIN)
    --Check scroll boundaries
    local tempMtx = Gfx.getCurMatrix()
    local sb = Gfx.getCurScissorBox()
    local nEntries = GCPMapEntriesEnum.size

    --Check if the selected entry is above the current view
    local centerY = tempMtx[2][4] + (GCPSelectedEntry - 1) * SETTING_EL_HEIGHT

    if (GCPScroll + centerY < sb.y + sb.height / 2) and GCPScroll < 0 then
        GCPScroll = sb.y + sb.height / 2 - centerY
    end

    if (GCPScroll + centerY > sb.y + sb.height / 2) and GCPScroll > -(nEntries * SETTING_EL_HEIGHT - sb.height) then
        GCPScroll = sb.y + sb.height / 2 - centerY
    end

    if GCPScroll > 0 then
        GCPScroll = 0
    end

    Gfx.pushMatrix()
    Gfx.translate(0, GCPScroll)

    Gfx.pushMatrix()
    local oldAlignment = Gfx.getFontVerticalAlignment(fonts[SETTING_FONT_SIZE])
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], Gfx.CENTER_ALIGN)

    for el in GCPMapEntriesEnum.all() do
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
        Gfx.print(fonts[SETTING_FONT_SIZE], SETTINGS_SIDE_MARGIN, SETTING_EL_HEIGHT / 2, el.name)
        Gfx.print(fonts[SETTING_FONT_SIZE], GCP_MAP_LEFT_WIDTH, SETTING_EL_HEIGHT / 2, GamesView.getGCPMapString(el.name))
        Gfx.translate(0, SETTING_EL_HEIGHT)
        Gfx.drawRectangle(0, 0, getDimensions()[1], 1, divColor)
    end

    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popMatrix()
    Gfx.drawRectangle(0, (GCPSelectedEntry - 1) * SETTING_EL_HEIGHT, getDimensions()[1], SETTING_EL_HEIGHT, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0x30))
    Gfx.popMatrix()

    if GCPConfigMode == GCPConfigModeEnum["CHANGE_MAPPING_BUTTON"] then
        local down = Pad.down(0)
        local held = Pad.held(0)

        local padsStrings = {}
        padsStrings[1] = "Hold the button for 1 second to map to the selected action."
        padsStrings[2] = "Only one button can be mapped at once."

        if held.BUTTON_LEFT then
            padsStrings[4] = "Left"
        elseif held.BUTTON_RIGHT then
            padsStrings[4] = "Right"
        elseif held.BUTTON_DOWN then
            padsStrings[4] = "Down"
        elseif held.BUTTON_UP then
            padsStrings[4] = "Up"
        elseif held.TRIGGER_Z then
            padsStrings[4] = "Z"
        elseif held.TRIGGER_R then
            padsStrings[4] = "R"
        elseif held.TRIGGER_L then
            padsStrings[4] = "L"
        elseif held.BUTTON_A then
            padsStrings[4] = "A"
        elseif held.BUTTON_B then
            padsStrings[4] = "B"
        elseif held.BUTTON_X then
            padsStrings[4] = "X"
        elseif held.BUTTON_Y then
            padsStrings[4] = "Y"
        elseif held.BUTTON_START then
            padsStrings[4] = "Start"
        else
            padsStrings[4] = ""
        end

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

function handleGCPMapConf()
    local nEntries = GCPMapEntriesEnum.size

    if GCPConfigMode == GCPConfigModeEnum["GLOBAL_CONFIG"] then
        local down = Pad.gendown(0)
        local held = Pad.genheld(0)

        if down.BUTTON_DOWN and GCPSelectedEntry < nEntries then
            GCPSelectedEntry = GCPSelectedEntry + 1
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_DELAY
        end

        if down.BUTTON_UP and GCPSelectedEntry > 1 then
            GCPSelectedEntry = GCPSelectedEntry - 1
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_DELAY
        end

        if held.BUTTON_DOWN and GCPSelectedEntry < nEntries and Time.getms() > GCPAutoScrollTime then
            GCPSelectedEntry = GCPSelectedEntry + 1
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_TIME
        end

        if held.BUTTON_UP and GCPSelectedEntry > 1 and Time.getms() > GCPAutoScrollTime then
            GCPSelectedEntry = GCPSelectedEntry - 1
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_TIME
        end

        if down.BUTTON_A then
            GCPConfigMode = GCPConfigModeEnum["CHANGE_MAPPING_BUTTON"]
            GCPConfigMenuTime = Time.getms()
        elseif down.BUTTON_B then
            showingGCPMapping = false
        elseif down.BUTTON_START then
            GamesView.saveGCPMapGameConfig()
            showingGCPMapping = false
        end
    else
        local down = Pad.down(0)
        local held = Pad.held(0)

        if GamesView.getGCPMapValue(down) >= 0 then
            GCPConfigMenuTime = Time.getms()
        elseif (GamesView.getGCPMapValue(held) >= 0) and (Time.getms() - GCPConfigMenuTime) > GCPMAPPING_CONFIRM_TIME then
            GamesView.setGCPMapGameConfigValue(GCPMapEntriesEnum[GCPSelectedEntry].name, GamesView.getGCPMapValue(held))

            GCPConfigMode = GCPConfigModeEnum["GLOBAL_CONFIG"]
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_DELAY
        elseif (Time.getms() - GCPConfigMenuTime) > GCPMAPPING_TIMEOUT then
            GCPConfigMode = GCPConfigModeEnum["GLOBAL_CONFIG"]
            GCPAutoScrollTime = Time.getms() + GCPAUTO_SCROLL_DELAY
        end

    end
end
