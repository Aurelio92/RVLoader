require 'scripts/class'
require 'scripts/menuSystem'

SettingsMenu = class()

function SettingsMenu:init(font, lineHeight, columnWidth, sideMargin)
    self.menuSystem = MenuSystem()

    self.menuSystem.font = font
    self.menuSystem.lineHeight = lineHeight
    self.menuSystem.columnWidth = columnWidth
    self.menuSystem.sideMargin = sideMargin
end

function SettingsMenu:draw(onFocus)

end

function SettingsMenu:handleInputs(onFocus)
    handlingLeftColumn = true
end
