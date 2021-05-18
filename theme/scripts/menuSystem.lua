menuSystem = {
    font = 0,
    lineHeight = 32,
    columnWidth = 100,
    sideMargin = 16,
    lineI = 1,
    lineColor = Gfx.RGBA8(0x86, 0x86, 0x86, 0xFF)
}

menuSystem.reset = function()
    menuSystem.lineI = 1
end

menuSystem.printLine = function(str, sel)
    local oldAlignment = Gfx.getFontVerticalAlignment(menuSystem.font)
    Gfx.setFontVerticalAlignment(menuSystem.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(menuSystem.font)
    if menuSystem.lineI == sel then
        Gfx.setFontColor(menuSystem.font, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0xFF))
    else
        Gfx.setFontColor(menuSystem.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    end
    Gfx.print(menuSystem.font, 2 * menuSystem.sideMargin, (menuSystem.lineI - 0.5) * menuSystem.lineHeight, str)
    Gfx.drawLine(0, menuSystem.lineI * menuSystem.lineHeight, menuSystem.columnWidth, menuSystem.lineI * menuSystem.lineHeight, 1, menuSystem.lineColor)
    Gfx.setFontColor(menuSystem.font, oldColor)
    menuSystem.lineI = menuSystem.lineI + 1
    Gfx.setFontVerticalAlignment(menuSystem.font, oldAlignment)
end

menuSystem.printLineValue = function(str, changed)
    local oldAlignment = Gfx.getFontVerticalAlignment(menuSystem.font)
    Gfx.setFontVerticalAlignment(menuSystem.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(menuSystem.font)
    local tempLineI = menuSystem.lineI - 1
    if changed then
        Gfx.setFontColor(menuSystem.font, Gfx.RGBA8(0xFF, 0x00, 0x00, 0xFF))
    else
        Gfx.setFontColor(menuSystem.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    end
    Gfx.print(menuSystem.font, menuSystem.columnWidth - 2 * menuSystem.sideMargin - Gfx.getTextWidth(menuSystem.font, str), (tempLineI - 0.5) * menuSystem.lineHeight, str)
    Gfx.setFontColor(menuSystem.font, oldColor)
    Gfx.setFontVerticalAlignment(menuSystem.font, oldAlignment)
end
