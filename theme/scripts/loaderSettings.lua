--[[
Selected theme
Launch installer

]]
function initLoader()
    loaderSelectedEnum = enum({"selTheme", "saveConfig", "bootPriiloader", "runInstaller"})
    loaderSelected = loaderSelectedEnum[1]

    loaderThemes = Theme.getThemes()
    loaderCurTheme = Theme.getLoadedTheme()
    loaderCurThemeId = 1

    for i = 1, #loaderThemes do
        Sys.debug("Theme: " .. loaderThemes[i])
        if loaderThemes[i] == loaderCurTheme then
            loaderCurThemeId = i
        end
    end
end

function drawLoader(onFocus)
    local colWidth = getDimensions()[1] - leftColumnWidth
    if onFocus then
        Gfx.drawRectangle(0, (loaderSelected.id - 1) * lineHeight, colWidth, lineHeight, Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF));
    end

    menuSystem.reset()
    menuSystem.printLine("Selected theme:", loaderSelected.id)
    menuSystem.printLineValue(loaderThemes[loaderCurThemeId], loaderThemes[loaderCurThemeId] ~= loaderCurTheme)
    menuSystem.printLine("Save config", loaderSelected.id)
    menuSystem.printLine("Boot priiloader", loaderSelected.id)
    menuSystem.printLine("Run installer", loaderSelected.id)
end

function handleLoader(onFocus)
    local down = Pad.gendown(0)
    local held = Pad.genheld(0)
    local curId = loaderSelected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < loaderSelectedEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if down.BUTTON_A then
        if loaderSelected == loaderSelectedEnum.saveConfig then
            if loaderThemes[loaderCurThemeId] ~= loaderCurTheme then
                Theme.setTheme(loaderThemes[loaderCurThemeId])
                Sys.reboot()
            end
        elseif loaderSelected == loaderSelectedEnum.bootPriiloader then
            Sys.bootPriiloader()
        elseif loaderSelected == loaderSelectedEnum.runInstaller then
            Sys.bootInstaller()
        end
    elseif down.BUTTON_RIGHT then
        if loaderSelected == loaderSelectedEnum.selTheme then
            loaderCurThemeId = loaderCurThemeId + 1
            if loaderCurThemeId > #loaderThemes then
                loaderCurThemeId = 1
            end
        end
    elseif down.BUTTON_LEFT then
        if loaderSelected == loaderSelectedEnum.selTheme then
            loaderCurThemeId = loaderCurThemeId - 1
            if loaderCurThemeId < 1 then
                loaderCurThemeId = #loaderThemes
            end
        end
    end

    loaderSelected = loaderSelectedEnum[curId]
end
