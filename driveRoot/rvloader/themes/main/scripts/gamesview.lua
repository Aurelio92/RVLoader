require 'scripts/enum'
require 'scripts/class'
require 'scripts/menuSystem'
require 'scripts/gc2wiimote'
require 'scripts/gc+map'
require 'scripts/topbarcmd'

menuSystem = MenuSystem()
cheatsMenuSystem = MenuSystem()

function init()
    showingGameConfig = false
    showingGameCheats = false
    showingGC2Wiimote = false
    showingGCPMapping = false

    screensize = Gui.getScreenSize()
    if screensize.x == 640 then
        gamesPerRow = 4
    else
        gamesPerRow = 5
    end
    gamesPerPage = 2 * gamesPerRow
    coverWidth = 121
    coverHeight = 170
    selectorBorder = 3
    gridGapX = (screensize.x - gamesPerRow * coverWidth) // (gamesPerRow + 1)
    gridGapY = 10
    selectedGame = 0
    columnScroll = 0
    columnsCount = 0
    GamesView.setCoverSize(coverWidth, coverHeight)

    gamesCount = GamesView.getGamesCount()

    if gamesCount <= gamesPerRow then
        columnsCount = gamesCount
    elseif gamesCount <= gamesPerPage then
        columnsCount = gamesPerRow
    else
        columnsCount = gamesPerRow + (gamesCount - gamesPerPage + 1) // 2
    end

    button_a_icon = Gfx.loadImage("assets/button_a.png")
    button_b_icon = Gfx.loadImage("assets/button_b.png")
    button_x_icon = Gfx.loadImage("assets/button_x.png")
    button_y_icon = Gfx.loadImage("assets/button_y.png")
    button_s_icon = Gfx.loadImage("assets/button_s.png")

    SETTING_FONT_SIZE = 20
    SETTING_EL_HEIGHT = 32
    SETTINGS_WIN_WIDTH = 400
    SETTINGS_WIN_HEIGHT = 300
    SETTINGS_SIDE_MARGIN = 16

    if GamesView.getGamesType() == GamesView.gameType.GC_GAME then
        menuSystem:addYesNoEntry("Force progressive", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Force widescreen", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Force RVL-DD stretching", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Native SI", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Video mode", false)
        menuSystem:addEntryOption("Auto", GamesView.nintendont.VIDEO_AUTO)
        menuSystem:addEntryOption("NTSC", GamesView.nintendont.VIDEO_NTSC)
        menuSystem:addEntryOption("PAL50", GamesView.nintendont.VIDEO_PAL50)
        menuSystem:addEntryOption("PAL60", GamesView.nintendont.VIDEO_PAL60)
        menuSystem:addEntryOption("MPAL", GamesView.nintendont.VIDEO_MPAL)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Language", false)
        menuSystem:addEntryOption("Auto", GamesView.nintendont.LANG_AUTO)
        menuSystem:addEntryOption("English", GamesView.nintendont.LANG_ENGLISH)
        menuSystem:addEntryOption("German", GamesView.nintendont.LANG_GERMAN)
        menuSystem:addEntryOption("French", GamesView.nintendont.LANG_FRENCH)
        menuSystem:addEntryOption("Spanish", GamesView.nintendont.LANG_SPANISH)
        menuSystem:addEntryOption("Italian", GamesView.nintendont.LANG_ITALIAN)
        menuSystem:addEntryOption("Dutch", GamesView.nintendont.LANG_DUTCH)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addYesNoEntry("Enable Cheats", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Memory Card Emulation", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Max Pads", false)
        menuSystem:addRangeToOptions(1, 4, 1)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Video Width", false)
        menuSystem:addEntryOption("Auto", 0)
        menuSystem:addRangeToOptions(640, 720, 1)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        if Gcp.isV2() then
            menuSystem:addEntry("Configure GC+2.0 map", false)
            menuSystem:setEntrySelectAction(activateGCPMapping)
        end
    elseif GamesView.getGamesType() == GamesView.gameType.WII_GAME then
        menuSystem:addYesNoEntry("Enable WiFi", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable Bluetooth", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable USB saves", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable GC2Wiimote", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Configure GC2Wiimote", false)
        menuSystem:setEntrySelectAction(activateGC2WMapping)
        menuSystem:addYesNoEntry("Patch MX chip", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("PADRead Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.PADREAD_AUTO)
        menuSystem:addEntryOption("Bypass", GamesView.hiidra.PADREAD_BYPASS)
        menuSystem:addEntryOption("Redirect", GamesView.hiidra.PADREAD_REDIRECT)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Deflicker Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.DEFLICKER_AUTO)
        menuSystem:addEntryOption("OFF (Safe)", GamesView.hiidra.DEFLICKER_OFF)
        menuSystem:addEntryOption("OFF (Extended)", GamesView.hiidra.DEFLICKER_OFF_EXTENDED)
        menuSystem:addEntryOption("ON (Low)", GamesView.hiidra.DEFLICKER_ON_LOW)
        menuSystem:addEntryOption("ON (Medium)", GamesView.hiidra.DEFLICKER_ON_MEDIUM)
        menuSystem:addEntryOption("ON (High)", GamesView.hiidra.DEFLICKER_ON_HIGH)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addYesNoEntry("Enable Cheats", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Cheats Hooktype", false)
        menuSystem:addEntryOption("VBI", GamesView.hiidra.HOOKTYPE_VBI)
        menuSystem:addEntryOption("KPADRead", GamesView.hiidra.HOOKTYPE_KPADRead)
        menuSystem:addEntryOption("Joypad", GamesView.hiidra.HOOKTYPE_Joypad)
        menuSystem:addEntryOption("GXDraw", GamesView.hiidra.HOOKTYPE_GXDraw)
        menuSystem:addEntryOption("GXFlush", GamesView.hiidra.HOOKTYPE_GXFlush)
        menuSystem:addEntryOption("OSSleepThread", GamesView.hiidra.HOOKTYPE_OSSleepThread)
        menuSystem:addEntryOption("AXNextFrame", GamesView.hiidra.HOOKTYPE_AXNextFrame)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Configure cheats", false)
        menuSystem:setEntrySelectAction(activateCheatsConfig)
    elseif GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL then
        menuSystem:addYesNoEntry("Enable WiFi", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable Bluetooth", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable GC2Wiimote", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Configure GC2Wiimote", false)
        menuSystem:setEntrySelectAction(activateGC2WMapping)
        menuSystem:addYesNoEntry("Patch MX chip", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("PADRead Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.PADREAD_AUTO)
        menuSystem:addEntryOption("Bypass", GamesView.hiidra.PADREAD_BYPASS)
        menuSystem:addEntryOption("Redirect", GamesView.hiidra.PADREAD_REDIRECT)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Deflicker Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.DEFLICKER_AUTO)
        menuSystem:addEntryOption("OFF (Safe)", GamesView.hiidra.DEFLICKER_OFF)
        menuSystem:addEntryOption("OFF (Extended)", GamesView.hiidra.DEFLICKER_OFF_EXTENDED)
        menuSystem:addEntryOption("ON (Low)", GamesView.hiidra.DEFLICKER_ON_LOW)
        menuSystem:addEntryOption("ON (Medium)", GamesView.hiidra.DEFLICKER_ON_MEDIUM)
        menuSystem:addEntryOption("ON (High)", GamesView.hiidra.DEFLICKER_ON_HIGH)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addYesNoEntry("Enable Cheats", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Cheats Hooktype", false)
        menuSystem:addEntryOption("VBI", GamesView.hiidra.HOOKTYPE_VBI)
        menuSystem:addEntryOption("KPADRead", GamesView.hiidra.HOOKTYPE_KPADRead)
        menuSystem:addEntryOption("Joypad", GamesView.hiidra.HOOKTYPE_Joypad)
        menuSystem:addEntryOption("GXDraw", GamesView.hiidra.HOOKTYPE_GXDraw)
        menuSystem:addEntryOption("GXFlush", GamesView.hiidra.HOOKTYPE_GXFlush)
        menuSystem:addEntryOption("OSSleepThread", GamesView.hiidra.HOOKTYPE_OSSleepThread)
        menuSystem:addEntryOption("AXNextFrame", GamesView.hiidra.HOOKTYPE_AXNextFrame)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Configure cheats", false)
        menuSystem:setEntrySelectAction(activateCheatsConfig)
    elseif GamesView.getGamesType() == GamesView.gameType.WII_VC then
        menuSystem:addYesNoEntry("Enable WiFi", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable Bluetooth", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addYesNoEntry("Enable GC2Wiimote", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Configure GC2Wiimote", false)
        menuSystem:setEntrySelectAction(activateGC2WMapping)
        menuSystem:addYesNoEntry("Patch MX chip", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("PADRead Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.PADREAD_AUTO)
        menuSystem:addEntryOption("Bypass", GamesView.hiidra.PADREAD_BYPASS)
        menuSystem:addEntryOption("Redirect", GamesView.hiidra.PADREAD_REDIRECT)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Deflicker Mode", false)
        menuSystem:addEntryOption("Auto", GamesView.hiidra.DEFLICKER_AUTO)
        menuSystem:addEntryOption("OFF (Safe)", GamesView.hiidra.DEFLICKER_OFF)
        menuSystem:addEntryOption("OFF (Extended)", GamesView.hiidra.DEFLICKER_OFF_EXTENDED)
        menuSystem:addEntryOption("ON (Low)", GamesView.hiidra.DEFLICKER_ON_LOW)
        menuSystem:addEntryOption("ON (Medium)", GamesView.hiidra.DEFLICKER_ON_MEDIUM)
        menuSystem:addEntryOption("ON (High)", GamesView.hiidra.DEFLICKER_ON_HIGH)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addYesNoEntry("Enable Cheats", false, GamesView.config.YES, GamesView.config.NO)
        menuSystem:addEntry("Cheats Hooktype", false)
        menuSystem:addEntryOption("VBI", GamesView.hiidra.HOOKTYPE_VBI)
        menuSystem:addEntryOption("KPADRead", GamesView.hiidra.HOOKTYPE_KPADRead)
        menuSystem:addEntryOption("Joypad", GamesView.hiidra.HOOKTYPE_Joypad)
        menuSystem:addEntryOption("GXDraw", GamesView.hiidra.HOOKTYPE_GXDraw)
        menuSystem:addEntryOption("GXFlush", GamesView.hiidra.HOOKTYPE_GXFlush)
        menuSystem:addEntryOption("OSSleepThread", GamesView.hiidra.HOOKTYPE_OSSleepThread)
        menuSystem:addEntryOption("AXNextFrame", GamesView.hiidra.HOOKTYPE_AXNextFrame)
        menuSystem:setEntryIncreaseAction(menuSystem.increaseEntryValue)
        menuSystem:setEntryDecreaseAction(menuSystem.decreaseEntryValue)
        menuSystem:addEntry("Configure cheats", false)
        menuSystem:setEntrySelectAction(activateCheatsConfig)
    end

    fonts = {}
    fonts[16] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 16)
    fonts[20] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 20)

    menuSystem.font = fonts[SETTING_FONT_SIZE]
    menuSystem.lineHeight = SETTING_EL_HEIGHT
    menuSystem.columnWidth = SETTINGS_WIN_WIDTH
    menuSystem.sideMargin = SETTINGS_SIDE_MARGIN

    cheatsMenuSystem.font = fonts[SETTING_FONT_SIZE]
    cheatsMenuSystem.lineHeight = SETTING_EL_HEIGHT
    cheatsMenuSystem.columnWidth = SETTINGS_WIN_WIDTH
    cheatsMenuSystem.sideMargin = SETTINGS_SIDE_MARGIN
    cheatsMenuSystem.lineWidth = 280

    initGC2WiimoteConf()
    initGCPMapConf()
end

function activateGCPMapping(index)
    GamesView.openGCPMapGameConfig()
    showingGCPMapping = true
end

function activateGC2WMapping(index)
    resetGC2WiimoteConf()
    GamesView.openGC2WiimoteGameConfig(selectedGame)
    showingGC2Wiimote = true
end

function activateCheatsConfig(index)
    gameCheats = GamesView.readGameCheats(selectedGame)
    cheatsMenuSystem:clearEntries()
    cheatsMenuSystem:reset()
    for i = 1, #gameCheats do
        cheatsMenuSystem:addYesNoEntry(gameCheats[i], false, GamesView.config.YES, GamesView.config.NO)
        cheatsMenuSystem:setEntryValue(gameCheats[i], GamesView.getGameConfigValue("Cheat_" .. GamesView.getCheatNameHash(gameCheats[i])))
    end
    showingGameCheats = true
end

function draw(onFocus)
    if showingGC2Wiimote then
        topBarSetText("GC2Wiimote: " .. GamesView.getGameName(selectedGame))
        drawGC2WiimoteConf()
        return
    end

    if showingGCPMapping then
        topBarSetText("GC+2.0 map: " .. GamesView.getGameName(selectedGame))
        drawGCPMapConf()
        return
    end

    if onFocus then
        if GamesView.getGamesType() == GamesView.gameType.GC_GAME then
            topBarSetText("GC games")
        elseif GamesView.getGamesType() == GamesView.gameType.WII_GAME then
            topBarSetText("Wii games")
        elseif GamesView.getGamesType() == GamesView.gameType.WII_VC then
            topBarSetText("Wii VC")
        elseif GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL then
            topBarSetText("Wii channels")
        end
    end

    Gfx.pushMatrix()
    local rowWidth = gamesPerRow * coverWidth + (gamesPerRow - 1) * gridGapX + 2 * selectorBorder
    local colHeight = 2 * coverHeight + gridGapY + 2 * selectorBorder
    Gfx.translate((getDimensions()[1] - rowWidth) / 2, 0)
    if onFocus then
        local oldAlignment = Gfx.getFontVerticalAlignment(fonts[20])
        Gfx.setFontVerticalAlignment(fonts[20], Gfx.TOP_ALIGN)
        Gfx.print(fonts[20], 0, 0, GamesView.getGameName(selectedGame))
        Gfx.setFontVerticalAlignment(fonts[20], oldAlignment)
    end
    Gfx.translate(0, 28)
    Gfx.pushScissorBox(rowWidth, colHeight)
    Gfx.translate(selectorBorder, selectorBorder)
    Gfx.translate(-columnScroll * (coverWidth + gridGapX), 0)
    for i = 0, (gamesCount - 1) do
        local x = 0
        local y = 0
        if i < gamesPerPage then
            x = (i % gamesPerRow) * (coverWidth + gridGapX)
            y = (i // gamesPerRow) * (coverHeight + gridGapY)
        else
            x = (gamesPerRow + (i - gamesPerPage) // 2) * (coverWidth + gridGapX)
            y = ((i - gamesPerPage) % 2) * (coverHeight + gridGapY)
        end

        if i == selectedGame and onFocus then
            Gfx.drawRectangle(x - selectorBorder, y - selectorBorder, coverWidth + 2 * selectorBorder, coverHeight + 2 * selectorBorder, Gfx.RGBA8(0x25, 0xcb, 0xf9, 0xff))
        end

        GamesView.drawGameCover(x, y, i)
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
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Boot game")
    tempX = tempX + 16

    Gfx.drawImage(button_b_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Configure game")
    tempX = tempX + 16

    if GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL or GamesView.getGamesType() == GamesView.gameType.WII_VC then
        Gfx.drawImage(button_y_icon, tempX, 0)
        tempX = tempX + 40
        tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Reinstall & boot")
        tempX = tempX + 16
    end

    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popScissorBox()
    Gfx.popMatrix()

    if showingGameCheats then
        drawGameCheats(onFocus)
    elseif showingGameConfig then
        drawGameConfig(onFocus)
    end
end

function handleInputs(onFocus)
    if onFocus then
        if showingGC2Wiimote then
            topBarDisableWheel()
            handleGC2WiimoteConf()
            return
        else
            topBarEnableWheel()
        end

        if showingGCPMapping then
            handleGCPMapConf()
            return
        end

        if showingGameCheats then
            handleGameCheats()
            return
        elseif showingGameConfig then
            handleGameConfig()
            return
        end

        local down = Pad.gendown(0)
        local curColumn = 0
        local curRow = 0

        if selectedGame < gamesPerPage then
            curColumn = selectedGame % gamesPerRow
            curRow = selectedGame // gamesPerRow
        else
            curColumn = gamesPerRow + (selectedGame - gamesPerPage) // 2
            curRow = (selectedGame - gamesPerPage) % 2
        end

        if down.BUTTON_RIGHT then
            if (curColumn == columnScroll + gamesPerRow - 1) and (columnScroll + gamesPerRow < columnsCount) then
                columnScroll = columnScroll + 1
            end
            local nextSelGame = 0
            if selectedGame < gamesPerRow - 1 then
                nextSelGame = selectedGame + 1
            elseif selectedGame == gamesPerRow - 1 then
                nextSelGame = selectedGame + gamesPerRow + 1
            elseif selectedGame < gamesPerPage - 1 then
                nextSelGame = selectedGame + 1
            else
                nextSelGame = selectedGame + 2
            end
            if nextSelGame < gamesCount then
                selectedGame = nextSelGame
            end
        end

        if down.BUTTON_LEFT then
            if (curColumn == columnScroll) and (columnScroll > 0) then
                columnScroll = columnScroll - 1
            end
            if selectedGame > gamesPerPage then
                selectedGame =  selectedGame - 2
            elseif selectedGame == gamesPerPage then
                selectedGame = selectedGame - gamesPerRow - 1
            elseif selectedGame > gamesPerRow then
                selectedGame = selectedGame - 1
            elseif selectedGame == gamesPerRow then
                selectedGame = selectedGame
            elseif selectedGame > 0 then
                selectedGame = selectedGame - 1
            end
        end

        if down.BUTTON_DOWN and curRow == 0 then
            --We don't need to check every range because we know curRow value
            local nextSelGame = 0
            if selectedGame < gamesPerRow then
                nextSelGame = selectedGame + gamesPerRow
            else
                nextSelGame = selectedGame + 1
            end
            if nextSelGame < gamesCount then
                selectedGame = nextSelGame
            end
        end

        if down.BUTTON_UP and curRow == 1 then
            --We don't need to check every range because we know curRow value
            if selectedGame > gamesPerPage then
                selectedGame = selectedGame - 1
            else
                selectedGame = selectedGame - gamesPerRow
            end
        end

        if gamesCount > 0 then
            --Boot game if requested
            if down.BUTTON_A then
                GamesView.bootGame(selectedGame)
            end

            if down.BUTTON_Y and (GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL or GamesView.getGamesType() == GamesView.gameType.WII_VC) then
                GamesView.bootGame(selectedGame, true)
            end

            --Show game config if requested
            if down.BUTTON_B then
                GamesView.openGameConfig(selectedGame)
                configOptions = menuSystem:getEntriesWithOptions()
                for i = 1, #configOptions do
                    menuSystem:setEntryValue(configOptions[i], GamesView.getGameConfigValue(configOptions[i]))
                end
                menuSystem:reset()
                showingGameConfig = true
                return
            end
        end
    end
end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y - 40
    return {w, h}
end

function drawGameConfig(onFocus)
    Gfx.pushMatrix()
    Gfx.identity()
    Gfx.pushIdentityScissorBox()
    Gfx.drawRectangle(0, 0, Gui.getScreenSize().x, Gui.getScreenSize().y, Gfx.RGBA8(0x00, 0x00, 0x00, 0xA0))
    Gfx.translate((Gui.getScreenSize().x - SETTINGS_WIN_WIDTH) / 2, (Gui.getScreenSize().y - SETTINGS_WIN_HEIGHT) / 2)
    Gfx.pushScissorBox(SETTINGS_WIN_WIDTH, SETTINGS_WIN_HEIGHT)
    Gfx.drawRectangle(0, 0, SETTINGS_WIN_WIDTH, SETTINGS_WIN_HEIGHT, Gfx.RGBA8(0x2D, 0x2D, 0x2D, 0xB0))

    menuSystem:printMenu(onFocus)

    Gfx.popScissorBox()
    Gfx.popScissorBox()
    Gfx.popMatrix()
end

function handleGameConfig()
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        configOptions = menuSystem:getEntriesWithOptions()
        for i = 1, #configOptions do
            GamesView.setGameConfigValue(configOptions[i], menuSystem:getEntryValue(configOptions[i]))
        end
        GamesView.saveGameConfig()
        showingGameConfig = false
        return
    end

    menuSystem:handleInputs()
end

function drawGameCheats(onFocus)
    Gfx.pushMatrix()
    Gfx.identity()
    Gfx.pushIdentityScissorBox()
    Gfx.drawRectangle(0, 0, Gui.getScreenSize().x, Gui.getScreenSize().y, Gfx.RGBA8(0x00, 0x00, 0x00, 0xA0))
    Gfx.translate((Gui.getScreenSize().x - SETTINGS_WIN_WIDTH) / 2, (Gui.getScreenSize().y - SETTINGS_WIN_HEIGHT) / 2)
    Gfx.pushScissorBox(SETTINGS_WIN_WIDTH, SETTINGS_WIN_HEIGHT)
    Gfx.drawRectangle(0, 0, SETTINGS_WIN_WIDTH, SETTINGS_WIN_HEIGHT, Gfx.RGBA8(0x2D, 0x2D, 0x2D, 0xB0))

    cheatsMenuSystem:printMenu(onFocus)

    Gfx.popScissorBox()
    Gfx.popScissorBox()
    Gfx.popMatrix()
end

function handleGameCheats()
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        for i = 1, #gameCheats do
            GamesView.setGameConfigValue("Cheat_" .. GamesView.getCheatNameHash(gameCheats[i]), cheatsMenuSystem:getEntryValue(gameCheats[i]))
        end
        showingGameCheats = false
        return
    end

    cheatsMenuSystem:handleInputs()
end
