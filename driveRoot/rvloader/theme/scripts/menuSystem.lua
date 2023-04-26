require 'scripts/class'

MenuSystem = class()

function MenuSystem:init()
    self.font = 0
    self.lineHeight = 32
    self.columnWidth = 100
    self.sideMargin = 16
    self.lineI = 1
    self.lineY = 0
    self.lineCount = 0
    self.lineColor = Gfx.RGBA8(0x86, 0x86, 0x86, 0xFF)
    self.selectionBGColor = Gfx.RGBA8(0x00, 0x00, 0x00, 0x00)
    self.selectionBGColorOnFocus = Gfx.RGBA8(0x4F, 0x52, 0x57, 0xFF)
    self.scroll = 0
    self.selLineY = 0
end

function MenuSystem:drawSelectionRectangle()
    Gfx.drawRectangle(0, (self.selLine - 1) * self.lineHeight, self.columnWidth, self.lineHeight, self.selectionBGColor)
end

function MenuSystem:reset()
    self.lineI = 1
    self.lineY = self.lineHeight
end

function MenuSystem:start(onFocus)
    self.lineI = 1
    self.lineY = self.lineHeight

    --Handle scrolling if lineCount has been already computed previously
    if (self.selLineY > 0 and self.lineCount > 0) then
        --Check scroll boundaries
        local tempMtx = Gfx.getCurMatrix()
        local sb = Gfx.getCurScissorBox()

        --Check if the selected entry is above the current view
        local centerY = tempMtx[2][4] + self.selLineY - self.lineHeight

        if (self.scroll + centerY < sb.y + sb.height / 2) and self.scroll < 0 then
            self.scroll = sb.y + sb.height / 2 - centerY
        end

        if (self.scroll + centerY > sb.y + sb.height / 2) and self.scroll > -(self.lineCount * self.lineHeight - sb.height) then
            self.scroll = sb.y + sb.height / 2 - centerY
        end

        if self.scroll > 0 then
            self.scroll = 0
        end
    end

    --Set lineCount to zero to be counted for next loop
    self.lineCount = 0

    --Handle scrolling
    Gfx.pushMatrix()
    Gfx.translate(0, self.scroll)

    if self.selLineY > 0 then
        if onFocus then
            Gfx.drawRectangle(0, self.selLineY - self.lineHeight, self.columnWidth, self.lineHeight, self.selectionBGColorOnFocus)
        else
            Gfx.drawRectangle(0, self.selLineY - self.lineHeight, self.columnWidth, self.lineHeight, self.selectionBGColor)
        end
    end
end

function MenuSystem:finish()
    Gfx.popMatrix()
end

function MenuSystem:printLine(str, sel)
    local oldAlignment = Gfx.getFontVerticalAlignment(self.font)
    Gfx.setFontVerticalAlignment(self.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(self.font)
    if self.lineI == sel then
        self.selLineY = self.lineY
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0xFF))
    else
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    end
    Gfx.print(self.font, 2 * self.sideMargin, self.lineY - 0.5 * self.lineHeight, str)
    Gfx.drawLine(0, self.lineY, self.columnWidth, self.lineY, 1, self.lineColor)
    Gfx.setFontColor(self.font, oldColor)
    self.lineI = self.lineI + 1
    self.lineCount = self.lineCount + 1
    self.lineY = self.lineY + self.lineHeight
    Gfx.setFontVerticalAlignment(self.font, oldAlignment)
end

function MenuSystem:printInfoLine(str)
    local oldAlignment = Gfx.getFontVerticalAlignment(self.font)
    Gfx.setFontVerticalAlignment(self.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(self.font)
    Gfx.setFontColor(self.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    Gfx.print(self.font, 2 * self.sideMargin, self.lineY - 0.5 * self.lineHeight, str)
    Gfx.drawLine(0, self.lineY, self.columnWidth, self.lineY, 1, self.lineColor)
    Gfx.setFontColor(self.font, oldColor)
    self.lineCount = self.lineCount + 1
    self.lineY = self.lineY + self.lineHeight
    Gfx.setFontVerticalAlignment(self.font, oldAlignment)
end

function MenuSystem:printLineValue(str, changed)
    local oldAlignment = Gfx.getFontVerticalAlignment(self.font)
    Gfx.setFontVerticalAlignment(self.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(self.font)
    local tempLineY = self.lineY - self.lineHeight
    if changed then
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xFF, 0x00, 0x00, 0xFF))
    else
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    end
    Gfx.print(self.font, self.columnWidth - 2 * self.sideMargin - Gfx.getTextWidth(self.font, str), tempLineY - 0.5 * self.lineHeight, str)
    Gfx.setFontColor(self.font, oldColor)
    Gfx.setFontVerticalAlignment(self.font, oldAlignment)
end

function MenuSystem:addSpacer(str)
    local oldAlignment = Gfx.getFontVerticalAlignment(self.font)
    Gfx.setFontVerticalAlignment(self.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(self.font)
    Gfx.setFontColor(self.font, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0xFF))
    if str then
        Gfx.print(self.font, self.sideMargin, self.lineY - 0.5 * self.lineHeight, str)
    end
    Gfx.drawLine(0, self.lineY, self.columnWidth, self.lineY, 1, self.lineColor)
    Gfx.setFontColor(self.font, oldColor)
    self.lineY = self.lineY + self.lineHeight
    self.lineCount = self.lineCount + 1
    Gfx.setFontVerticalAlignment(self.font, oldAlignment)
end