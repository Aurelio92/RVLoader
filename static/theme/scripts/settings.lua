require 'scripts/enum'
require 'scripts/class'
require 'scripts/topbarcmd'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'
require 'scripts/loaderSettings'
require 'scripts/controllersSettings'
require 'scripts/powerSettings'
require 'scripts/audioSettings'
require 'scripts/systemStatus'

function settingsEnableSubMenu()
    handlingLeftColumn = false
end

function settingsDisableSubMenu()
    handlingLeftColumn = true
end

menuSystem = MenuSystem()

aboutSettings = class(SettingsMenu)

function aboutSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)
    self.luaLogo_Img = Gfx.loadImage("assets/luaLogo.png")
end

function aboutSettings:draw(onFocus)
    self.menuSystem:start(onFocus)
    self.menuSystem:printLine(string.format("RVLoader v%.1f", Sys.getVersion()))
    self.menuSystem:printLine("Developed by Aurelio")
    self.menuSystem:printLine("Powered by LUA")
    Gfx.drawImage(self.luaLogo_Img, self.menuSystem.sideMargin, (self.menuSystem.lineI - 1) * self.menuSystem.lineHeight)
    self.menuSystem:finish()
end

function aboutSettings:handleInputs(onFocus)
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end
end

settingMenus = {
    names = {},
    menuClasses = {},
    initFunctions = {},
    drawFunctions = {},
    handleFunctions = {}
}

function init()
    lineColor = Gfx.RGBA8(0x86, 0x86, 0x86, 0xFF)
    leftColumnColor = Gfx.RGBA8(0x40, 0x40, 0x40, 0x38)
    --bgColor = Gfx.RGBA8(0x40, 0x40, 0x40, 0xFF)
    bgColor = Gfx.RGBA8(0x2D, 0x2D, 0x2D, 0x38)
    selColorBg = Gfx.RGBA8(0x1F, 0x22, 0x27, 0x38)
    selColorText = Gfx.RGBA8(0x00, 0xFF, 0xC8, 0x38)
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
    menuSystem.selectionBGColor = Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF)

    --Initialize settingMenus
    table.insert(settingMenus.names, "Loader")
    table.insert(settingMenus.menuClasses, loaderSettings)
    table.insert(settingMenus.names, "Controller")
    table.insert(settingMenus.menuClasses, controllerSettings)

    if PMS2.isConnected() then
        table.insert(settingMenus.names, "Power")
        table.insert(settingMenus.menuClasses, powerSettings)
    end

    if UAMP.isConnected() then
        table.insert(settingMenus.names, "Audio")
        table.insert(settingMenus.menuClasses, audioSettings)
    end


    table.insert(settingMenus.names, "Status")
    table.insert(settingMenus.menuClasses, statusSettings)

    table.insert(settingMenus.names, "About")
    table.insert(settingMenus.menuClasses, aboutSettings)

    for i = 1, #settingMenus.menuClasses do
        settingMenus.menuClasses[i] = settingMenus.menuClasses[i](fonts[20], lineHeight, getDimensions()[1] - leftColumnWidth, leftMargin)
    end
end

function draw(onFocus)
    topBarSetText("")

    Gfx.draw4ColorsRectangle(0, 0, leftColumnWidth, getDimensions()[2], bgColor, leftColumnColor, leftColumnColor, bgColor)
    Gfx.drawRectangle(leftColumnWidth, 0, getDimensions()[1], getDimensions()[2], leftColumnColor)

    Gfx.pushMatrix()
    Gfx.translate(0, topMargin)
    Gfx.pushScissorBox(leftColumnWidth, getDimensions()[2] - topMargin)
    menuSystem:start(onFocus and handlingLeftColumn)
    for i = 1, #settingMenus.names do
        menuSystem:printLine(settingMenus.names[i], selected)
    end
    menuSystem:finish()
    Gfx.popScissorBox()

    Gfx.pushMatrix()
    Gfx.translate(leftColumnWidth, 0)
    Gfx.pushMatrix()
    Gfx.pushScissorBox(getDimensions()[1] - leftColumnWidth, getDimensions()[2] - topMargin)
    --settingMenus.drawFunctions[selected](onFocus and not handlingLeftColumn)
    settingMenus.menuClasses[selected]:draw(onFocus and not handlingLeftColumn)
    Gfx.popScissorBox()
    Gfx.popMatrix()
    Gfx.popMatrix()
    Gfx.print(fonts[30], leftMargin, -10, "Settings")
    Gfx.drawLine(0, 0, getDimensions()[1], 0, 2, lineColor)
    Gfx.popMatrix()
    Gfx.drawLine(leftColumnWidth, topMargin, leftColumnWidth, getDimensions()[2], 2, lineColor)

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
    local down = Pad.gendown(0)

    if not handlingLeftColumn then
        settingMenus.menuClasses[selected]:handleInputs(onFocus)
        return
    end

    if down.BUTTON_A then
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
