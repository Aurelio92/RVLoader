require 'scripts/class'
require 'scripts/utils'

MenuSystem = class()

function MenuSystem:init()
    self.font = 0
    self.lineHeight = 32
    self.lineWidth = 1000
    self.lineScroll = 0
    self.lineScrollTextGap = 32
    self.lineScrollTextTimeRef = 0
    self.lineScrollTextTime = 50
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
    
    self.selLine = 1

    self.entries = {}
    self.entriesIndex = {}
end

function MenuSystem:reset()
    self.selLine = 1
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
    local strWidth = Gfx.getTextWidth(self.font, str)
    Gfx.setFontVerticalAlignment(self.font, Gfx.CENTER_ALIGN)
    local oldColor = Gfx.getFontColor(self.font)
    if self.lineI == sel then
        self.selLineY = self.lineY
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xFF, 0xFF, 0xFF, 0xFF))
    else
        Gfx.setFontColor(self.font, Gfx.RGBA8(0xA0, 0xA0, 0xA0, 0xFF))
    end
    Gfx.pushMatrix()
    Gfx.translate(2 * self.sideMargin, self.lineY - self.lineHeight)
    Gfx.pushScissorBox(self.lineWidth, self.lineHeight)
    if self.lineI == sel then
        Gfx.translate(-self.lineScroll, 0)
        Gfx.print(self.font, 0, 0.5 * self.lineHeight, str)
        if strWidth > self.lineWidth then
            Gfx.print(self.font, strWidth + self.lineScrollTextGap, 0.5 * self.lineHeight, str)
            self.lineScroll = (Time.getms() - self.lineScrollTextTimeRef) // self.lineScrollTextTime
            if self.lineScroll > strWidth + self.lineScrollTextGap then
                self.lineScroll = self.lineScroll - (strWidth + self.lineScrollTextGap)
                self.lineScrollTextTimeRef = Time.getms() - self.lineScroll * self.lineScrollTextTime
            end
        end
    else
        Gfx.print(self.font, 0, 0.5 * self.lineHeight, str)
    end
    Gfx.popScissorBox()
    Gfx.popMatrix()
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

function MenuSystem:clearEntries()
    self.entries = {}
    self.entriesIndex = {}
end

function MenuSystem:increaseEntryValue(index)
    if index >= 1 and index <= #self.entries then
        if #self.entries[index].optionsValue > 0 then
            self.entries[index].curOptionIndex = 1 + (self.entries[index].curOptionIndex % #self.entries[index].optionsValue)
        end
    end
end

function MenuSystem:decreaseEntryValue(index)
    if index >= 1 and index <= #self.entries then
        if #self.entries[index].optionsValue > 0 then
            self.entries[index].curOptionIndex = 1 + ((self.entries[index].curOptionIndex + #self.entries[index].optionsValue - 2) % #self.entries[index].optionsValue)
        end
    end
end

function MenuSystem:addEntry(id, showChanges)
    table.insert(self.entries, {id = id, index = #self.entries + 1, showChanges = showChanges, setOptionIndex = 1, curOptionIndex = 1, optionsName = {}, optionsValue = {}, selectAction = nil, increaseAction = nil, decreaseAction = nil})
    self.entriesIndex[id] = #self.entries
end

function MenuSystem:addEntryOption(name, value)
    table.insert(self.entries[#self.entries].optionsName, name)
    table.insert(self.entries[#self.entries].optionsValue, value)
end

function MenuSystem:setEntrySelectAction(action)
    self.entries[#self.entries].selectAction = action
end

function MenuSystem:setEntryIncreaseAction(action)
    self.entries[#self.entries].increaseAction = action
end

function MenuSystem:setEntryDecreaseAction(action)
    self.entries[#self.entries].decreaseAction = action
end

function MenuSystem:getEntriesWithOptions()
    entriesIdList = {}
    for i = 1, #self.entries do
        if #self.entries[i].optionsValue > 0 then
            table.insert(entriesIdList, self.entries[i].id)
        end
    end
    
    return entriesIdList
end

function MenuSystem:addYesNoEntry(id, showChanges, yesValue, noValue)
    self:addEntry(id, showChanges, false)
    self:addEntryOption("Yes", yesValue)
    self:addEntryOption("No", noValue)
    self:setEntrySelectAction(self.increaseEntryValue)
    self:setEntryIncreaseAction(self.increaseEntryValue)
    self:setEntryDecreaseAction(self.decreaseEntryValue)
end

--Very crude system, but gets the job done for nintendont's video width for now
function MenuSystem:addRangeToOptions(min, max, step)
    for i = min, max, step do
        self:addEntryOption(tostring(i), i)
    end
end

function MenuSystem:setEntryValue(id, value)
    entryIndex = self.entriesIndex[id]
    valueIndex = nil

    if entryIndex ~= nil then
        valueIndex = table.contains(self.entries[entryIndex].optionsValue, value)
    end

    if valueIndex ~= nil then
        self.entries[entryIndex].curOptionIndex = valueIndex
        self.entries[entryIndex].setOptionIndex = valueIndex
    end
end

function MenuSystem:getEntryValue(id)
    entryIndex = self.entriesIndex[id]

    if entryIndex ~= nil then
        return self.entries[entryIndex].optionsValue[self.entries[entryIndex].curOptionIndex]
    end
end

function MenuSystem:printMenu(onFocus)
    self:start(onFocus)
    for i = 1, #self.entries do
        self:printLine(self.entries[i].id, self.selLine)
        if #self.entries[i].optionsValue > 0 then
            self:printLineValue(self.entries[i].optionsName[self.entries[i].curOptionIndex], showChanges and (self.entries[i].curOptionIndex ~= self.entries[i].setOptionIndex))
        end
    end
    self:finish()
end

function MenuSystem:handleInputs()
    local down = Pad.gendown(0)

    if down.BUTTON_DOWN and self.selLine < #self.entries then
        self.selLine = self.selLine + 1
        self.lineScroll = 0
        self.lineScrollTextTimeRef = Time.getms()
    elseif down.BUTTON_UP and self.selLine > 1 then
        self.selLine = self.selLine - 1
        self.lineScroll = 0
        self.lineScrollTextTimeRef = Time.getms()
    end

    self.selLineY = self.selLine * self.lineHeight

    if down.BUTTON_A and self.entries[self.selLine].selectAction ~= nil then
        self.entries[self.selLine].selectAction(self, self.selLine)
    end

    if down.BUTTON_RIGHT and self.entries[self.selLine].increaseAction ~= nil then
        self.entries[self.selLine].increaseAction(self, self.selLine)
    end

    if down.BUTTON_LEFT and self.entries[self.selLine].decreaseAction ~= nil then
        self.entries[self.selLine].decreaseAction(self, self.selLine)
    end
end