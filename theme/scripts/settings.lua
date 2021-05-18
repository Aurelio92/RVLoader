dofile("scripts/enum.lua")
dofile("scripts/topbarcmd.lua")
dofile("scripts/menuSystem.lua")
dofile("scripts/controllersSettings.lua")
dofile("scripts/powerSettings.lua")
dofile("scripts/loaderSettings.lua")
dofile("scripts/systemStatus.lua")

function initAbout()
    luaLogo_Img = Gfx.loadImage("assets/luaLogo.png")
end

function drawAbout(onFocus)
    menuSystem.reset()
    menuSystem.printLine(string.format("RVLoader v%.1f", Sys.getVersion()))
    menuSystem.printLine("Developed by Aurelio")
    menuSystem.printLine("Powered by LUA")

    Gfx.drawImage(luaLogo_Img, leftMargin, (menuSystem.lineI - 1) * menuSystem.lineHeight)
end

settingMenus = {
    names = {},
    initFunctions = {},
    drawFunctions = {},
    handleFunctions = {}
}

function init()
    --Initialize settingMenus
    table.insert(settingMenus.names, "Loader")
    table.insert(settingMenus.initFunctions, initLoader)
    table.insert(settingMenus.drawFunctions, drawLoader)
    table.insert(settingMenus.handleFunctions, handleLoader)
    table.insert(settingMenus.names, "Controller")
    table.insert(settingMenus.initFunctions, initController)
    table.insert(settingMenus.drawFunctions, drawController)
    table.insert(settingMenus.handleFunctions, handleController)

    if PMS2.isConnected() then
        table.insert(settingMenus.names, "Power")
        table.insert(settingMenus.initFunctions, initPower)
        table.insert(settingMenus.drawFunctions, drawPower)
        table.insert(settingMenus.handleFunctions, handlePower)
    end

    table.insert(settingMenus.names, "About")
    table.insert(settingMenus.initFunctions, initAbout)
    table.insert(settingMenus.drawFunctions, drawAbout)
    table.insert(settingMenus.handleFunctions, nil)

    table.insert(settingMenus.names, "Status")
    table.insert(settingMenus.initFunctions, initSystemStatus)
    table.insert(settingMenus.drawFunctions, drawSystemStatus)
    table.insert(settingMenus.handleFunctions, nil)

    lineColor = Gfx.RGBA8(0x86, 0x86, 0x86, 0xFF)
    leftColumnColor = Gfx.RGBA8(0x40, 0x40, 0x40, 0xFF)
    --bgColor = Gfx.RGBA8(0x40, 0x40, 0x40, 0xFF)
    bgColor = Gfx.RGBA8(0x2D, 0x2D, 0x2D, 0xFF)
    selColorBg = Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF)
    selColorText = Gfx.RGBA8(0x00, 0xFF, 0xC8, 0xFF)
    leftColumnWidth = 150
    lineHeight = 32
    leftMargin = 16
    topMargin = 40


    selected = 1
    handlingLeftColumn = true
    fonts = {}
    fonts[16] = Gfx.loadFont("assets/notosans.ttf", 16)
    fonts[12] = Gfx.loadFont("assets/notosans.ttf", 12)
    fonts[20] = Gfx.loadFont("assets/notosans.ttf", 20)
    fonts[30] = Gfx.loadFont("assets/notosans.ttf", 30)
    Gfx.setFontVerticalAlignment(fonts[20], Gfx.CENTER_ALIGN)
    Gfx.setFontVerticalAlignment(fonts[30], Gfx.BOTTOM_ALIGN)

    menuSystem.font = fonts[20]
    menuSystem.lineHeight = lineHeight
    menuSystem.columnWidth = leftColumnWidth
    menuSystem.sideMargin = leftMargin

    --Call init functions for settingMenus
    for i = 1, #settingMenus.initFunctions do
        settingMenus.initFunctions[i]()
    end
end

function draw(onFocus)
    topBarSetText("")

    Gfx.draw4ColorsRectangle(0, 0, leftColumnWidth, getDimensions()[2], bgColor, leftColumnColor, leftColumnColor, bgColor)
    Gfx.drawRectangle(leftColumnWidth, 0, getDimensions()[1], getDimensions()[2], leftColumnColor)

    Gfx.pushMatrix()
    Gfx.translate(0, topMargin)
    menuSystem.reset()
    menuSystem.columnWidth = leftColumnWidth
    if handlingLeftColumn then
        Gfx.drawRectangle(0, (selected - 1) * lineHeight, leftColumnWidth, lineHeight, Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF));
    else
        Gfx.drawRectangle(0, (selected - 1) * lineHeight, leftColumnWidth, lineHeight, Gfx.RGBA8(0x4F, 0x52, 0x57, 0xFF));
    end
    for i = 1, #settingMenus.names do
        menuSystem.printLine(settingMenus.names[i], selected)
    end

    menuSystem.columnWidth = getDimensions()[1] - leftColumnWidth
    Gfx.pushMatrix()
    Gfx.translate(leftColumnWidth, 0)
    settingMenus.drawFunctions[selected](onFocus and not handlingLeftColumn)
    Gfx.popMatrix()
    Gfx.print(fonts[30], leftMargin, -10, "Settings")
    Gfx.drawLine(0, 0, getDimensions()[1], 0, 2, lineColor)
    Gfx.popMatrix()
    Gfx.drawLine(leftColumnWidth, topMargin, leftColumnWidth, getDimensions()[2], 2, lineColor)

end

function handleInputs(onFocus)
    local down = Pad.gendown(0)

    if not handlingLeftColumn then
        settingMenus.handleFunctions[selected](onFocus)
        return
    end

    if down.BUTTON_A and settingMenus.handleFunctions[selected] ~= nil then
        handlingLeftColumn = false
    end

    if down.BUTTON_DOWN and selected < #settingMenus.names then
        selected = selected + 1
    elseif down.BUTTON_UP and selected > 1 then
        selected = selected - 1
    end
end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y - 40
    return {w, h}
end
