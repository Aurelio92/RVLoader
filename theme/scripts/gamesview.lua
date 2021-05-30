dofile("scripts/menuSystem.lua")
dofile("scripts/enum.lua")
dofile("scripts/gc2wiimote.lua")
dofile("scripts/gc+map.lua")
dofile("scripts/topbarcmd.lua")

function init()
    showingGameConfig = false
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
        if Gcp.isV2() then
            gameConfigSelectedEnum = enum({"Force progressive", "Force widescreen", "Force RVL-DD stretching", "Native SI", "Video mode", "Language", "Memory Card Emulation", "Configure GC+2.0 map"})
        else
            gameConfigSelectedEnum = enum({"Force progressive", "Force widescreen", "Force RVL-DD stretching", "Native SI", "Video mode", "Memory Card Emulation", "Language"})
        end
    elseif GamesView.getGamesType() == GamesView.gameType.WII_GAME then
        gameConfigSelectedEnum = enum({"Enable WiFi", "Enable Bluetooth", "Enable USB saves", "Enable GC2Wiimote", "Configure GC2Wiimote"})
    elseif GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL then
        gameConfigSelectedEnum = enum({"Enable WiFi", "Enable Bluetooth", "Enable GC2Wiimote", "Configure GC2Wiimote"})
    elseif GamesView.getGamesType() == GamesView.gameType.WII_VC then
        gameConfigSelectedEnum = enum({"Enable WiFi", "Enable Bluetooth", "Enable GC2Wiimote", "Configure GC2Wiimote"})
    end

    gameConfigSelected = gameConfigSelectedEnum[1]

    fonts = {}
    fonts[16] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 16)
    fonts[20] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 20)

    menuSystem.font = fonts[SETTING_FONT_SIZE]
    menuSystem.lineHeight = SETTING_EL_HEIGHT
    menuSystem.columnWidth = SETTINGS_WIN_WIDTH
    menuSystem.sideMargin = SETTINGS_SIDE_MARGIN

    initGC2WiimoteConf()
    initGCPMapConf()
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

    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popScissorBox()
    Gfx.popMatrix()

    if showingGameConfig then
        drawGameConfig()
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

        if showingGameConfig then
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

            --Show game config if requested
            if down.BUTTON_B then
                GamesView.openGameConfig(selectedGame)
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

function drawGameConfig()
    Gfx.pushMatrix()
    Gfx.identity()
    Gfx.pushIdentityScissorBox()
    Gfx.drawRectangle(0, 0, Gui.getScreenSize().x, Gui.getScreenSize().y, Gfx.RGBA8(0x00, 0x00, 0x00, 0xA0))
    Gfx.translate((Gui.getScreenSize().x - SETTINGS_WIN_WIDTH) / 2, (Gui.getScreenSize().y - SETTINGS_WIN_HEIGHT) / 2)
    Gfx.drawRectangle(0, 0, SETTINGS_WIN_WIDTH, SETTINGS_WIN_HEIGHT, Gfx.RGBA8(0x2D, 0x2D, 0x2D, 0xB0))

    Gfx.drawRectangle(0, (gameConfigSelected.id - 1) * menuSystem.lineHeight, menuSystem.columnWidth, menuSystem.lineHeight, Gfx.RGBA8(0x1F, 0x22, 0x27, 0xB0))

    menuSystem.reset()
    if GamesView.getGamesType() == GamesView.gameType.GC_GAME then
        menuSystem.printLine("Force progressive", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Force progressive")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Force widescreen", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Force widescreen")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Force RVL-DD stretching", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Force RVL-DD stretching")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Native SI", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Native SI")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Video mode", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Video mode")
        if val == GamesView.nintendont.VIDEO_AUTO then
            val = "Auto"
        elseif val == GamesView.nintendont.VIDEO_NTSC then
            val = "NTSC"
        elseif val == GamesView.nintendont.VIDEO_PAL50 then
            val = "PAL50"
        elseif val == GamesView.nintendont.VIDEO_PAL60 then
            val = "PAL60"
        elseif val == GamesView.nintendont.VIDEO_MPAL then
            val = "MPAL"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Language", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Language")
        if val == GamesView.nintendont.LANG_AUTO then
            val = "Auto"
        elseif val == GamesView.nintendont.LANG_ENGLISH then
            val = "English"
        elseif val == GamesView.nintendont.LANG_GERMAN then
            val = "German"
        elseif val == GamesView.nintendont.LANG_FRENCH then
            val = "French"
        elseif val == GamesView.nintendont.LANG_SPANISH then
            val = "Spanish"
        elseif val == GamesView.nintendont.LANG_ITALIAN then
            val = "Italian"
        elseif val == GamesView.nintendont.LANG_DUTCH then
            val = "Dutch"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Memory Card Emulation", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Memory Card Emulation")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        if Gcp.isV2() then
            menuSystem.printLine("Configure GC+2.0 map", gameConfigSelected.id)
        end

    elseif GamesView.getGamesType() == GamesView.gameType.WII_GAME then
        menuSystem.printLine("Enable WiFi", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable WiFi")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable Bluetooth", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable Bluetooth")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable USB saves", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable USB saves")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable GC2Wiimote", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable GC2Wiimote")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Configure GC2Wiimote", gameConfigSelected.id)
    elseif GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL then
        menuSystem.printLine("Enable WiFi", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable WiFi")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable Bluetooth", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable Bluetooth")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable GC2Wiimote", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable GC2Wiimote")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Configure GC2Wiimote", gameConfigSelected.id)
    elseif GamesView.getGamesType() == GamesView.gameType.WII_VC then
        menuSystem.printLine("Enable WiFi", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable WiFi")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable Bluetooth", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable Bluetooth")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Enable GC2Wiimote", gameConfigSelected.id)
        local val = GamesView.getGameConfigValue("Enable GC2Wiimote")
        if val == GamesView.config.YES then
            val = "Yes"
        else
            val = "No"
        end
        menuSystem.printLineValue(val, false)

        menuSystem.printLine("Configure GC2Wiimote", gameConfigSelected.id)
    end

    Gfx.popScissorBox()
    Gfx.popMatrix()
end

function handleGameConfig()
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        GamesView.saveGameConfig()
        showingGameConfig = false
        return
    end

    if down.BUTTON_DOWN and gameConfigSelected.id < gameConfigSelectedEnum.size then
        gameConfigSelected = gameConfigSelectedEnum[gameConfigSelected.id + 1]
    elseif down.BUTTON_UP and gameConfigSelected.id > 1 then
        gameConfigSelected = gameConfigSelectedEnum[gameConfigSelected.id - 1]
    end

    local confVal = 0
    if gameConfigSelected ~= gameConfigSelectedEnum["Configure GC2Wiimote"] and gameConfigSelected ~= gameConfigSelectedEnum["Configure GC+2.0 map"] then
        confVal = GamesView.getGameConfigValue(gameConfigSelected.name)
    end

    if GamesView.getGamesType() == GamesView.gameType.GC_GAME then
        if gameConfigSelected == gameConfigSelectedEnum["Configure GC+2.0 map"] then
            if down.BUTTON_A then
                GamesView.openGCPMapGameConfig()
                showingGCPMapping = true
            end
        elseif gameConfigSelected == gameConfigSelectedEnum["Video mode"] then
            if down.BUTTON_RIGHT then
                if confVal == GamesView.nintendont.VIDEO_AUTO then
                    confVal = GamesView.nintendont.VIDEO_NTSC
                elseif confVal == GamesView.nintendont.VIDEO_NTSC then
                    confVal = GamesView.nintendont.VIDEO_PAL50
                elseif confVal == GamesView.nintendont.VIDEO_PAL50 then
                    confVal = GamesView.nintendont.VIDEO_PAL60
                elseif confVal == GamesView.nintendont.VIDEO_PAL60 then
                    confVal = GamesView.nintendont.VIDEO_MPAL
                elseif confVal == GamesView.nintendont.VIDEO_MPAL then
                    confVal = GamesView.nintendont.VIDEO_AUTO
                end
            elseif down.BUTTON_LEFT then
                if confVal == GamesView.nintendont.VIDEO_AUTO then
                    confVal = GamesView.nintendont.VIDEO_MPAL
                elseif confVal == GamesView.nintendont.VIDEO_NTSC then
                    confVal = GamesView.nintendont.VIDEO_AUTO
                elseif confVal == GamesView.nintendont.VIDEO_PAL50 then
                    confVal = GamesView.nintendont.VIDEO_NTSC
                elseif confVal == GamesView.nintendont.VIDEO_PAL60 then
                    confVal = GamesView.nintendont.VIDEO_PAL50
                elseif confVal == GamesView.nintendont.VIDEO_MPAL then
                    confVal = GamesView.nintendont.VIDEO_PAL60
                end
            end
        elseif gameConfigSelected == gameConfigSelectedEnum["Language"] then
            if down.BUTTON_RIGHT then
                if confVal == GamesView.nintendont.LANG_AUTO then
                    confVal = GamesView.nintendont.LANG_ENGLISH
                elseif confVal == GamesView.nintendont.LANG_ENGLISH then
                    confVal = GamesView.nintendont.LANG_GERMAN
                elseif confVal == GamesView.nintendont.LANG_GERMAN then
                    confVal = GamesView.nintendont.LANG_FRENCH
                elseif confVal == GamesView.nintendont.LANG_FRENCH then
                    confVal = GamesView.nintendont.LANG_SPANISH
                elseif confVal == GamesView.nintendont.LANG_SPANISH then
                    confVal = GamesView.nintendont.LANG_ITALIAN
                elseif confVal == GamesView.nintendont.LANG_ITALIAN then
                    confVal = GamesView.nintendont.LANG_DUTCH
                elseif confVal == GamesView.nintendont.LANG_DUTCH then
                    confVal = GamesView.nintendont.LANG_AUTO
                end
            elseif down.BUTTON_LEFT then
                if confVal == GamesView.nintendont.LANG_AUTO then
                    confVal = GamesView.nintendont.LANG_DUTCH
                elseif confVal == GamesView.nintendont.LANG_ENGLISH then
                    confVal = GamesView.nintendont.LANG_AUTO
                elseif confVal == GamesView.nintendont.LANG_GERMAN then
                    confVal = GamesView.nintendont.LANG_ENGLISH
                elseif confVal == GamesView.nintendont.LANG_FRENCH then
                    confVal = GamesView.nintendont.LANG_GERMAN
                elseif confVal == GamesView.nintendont.LANG_SPANISH then
                    confVal = GamesView.nintendont.LANG_FRENCH
                elseif confVal == GamesView.nintendont.LANG_ITALIAN then
                    confVal = GamesView.nintendont.LANG_SPANISH
                elseif confVal == GamesView.nintendont.LANG_DUTCH then
                    confVal = GamesView.nintendont.LANG_ITALIAN
                end
            end
        else --Yes/No configs
            if down.BUTTON_RIGHT or down.BUTTON_LEFT or down.BUTTON_A then
                    confVal = 1 - confVal
            end
        end
    elseif GamesView.getGamesType() == GamesView.gameType.WII_GAME then
        if gameConfigSelected == gameConfigSelectedEnum["Configure GC2Wiimote"] then
            if down.BUTTON_A then
                GamesView.openGC2WiimoteGameConfig(selectedGame)
                showingGC2Wiimote = true
            end
        else --Yes/No configs
            if down.BUTTON_RIGHT or down.BUTTON_LEFT or down.BUTTON_A then
                confVal = 1 - confVal
            end
        end
    elseif GamesView.getGamesType() == GamesView.gameType.WII_CHANNEL then
        if gameConfigSelected == gameConfigSelectedEnum["Configure GC2Wiimote"] then
            if down.BUTTON_A then
                GamesView.openGC2WiimoteGameConfig(selectedGame)
                showingGC2Wiimote = true
            end
        else --Yes/No configs
            if down.BUTTON_RIGHT or down.BUTTON_LEFT or down.BUTTON_A then
                confVal = 1 - confVal
            end
        end
    elseif GamesView.getGamesType() == GamesView.gameType.WII_VC then
        if gameConfigSelected == gameConfigSelectedEnum["Configure GC2Wiimote"] then
            if down.BUTTON_A then
                GamesView.openGC2WiimoteGameConfig(selectedGame)
                showingGC2Wiimote = true
            end
        else --Yes/No configs
            if down.BUTTON_RIGHT or down.BUTTON_LEFT or down.BUTTON_A then
                confVal = 1 - confVal
            end
        end
    end

    if gameConfigSelected ~= gameConfigSelectedEnum["Configure GC2Wiimote"] and gameConfigSelected ~= gameConfigSelectedEnum["Configure GC+2.0 map"] then
        GamesView.setGameConfigValue(gameConfigSelected.name, confVal)
    end
end
