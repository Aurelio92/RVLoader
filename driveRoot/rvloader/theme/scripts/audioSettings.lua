require 'scripts/enum'
require 'scripts/class'
require 'scripts/topbarcmd'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'

audioSettings = class(SettingsMenu)

function audioSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.speakersVolume = UAMP.getSpeakersVolume()
    self.headphonesVolume = UAMP.getHeadphonesVolume()
    self.volumeControlSystem = UAMP.getVolumeControlSystem()

    self.AUTO_INCREASE_DELAY = 500
    self.AUTO_INCREASE_TIME = 100
    self.autoIncreaseTime = 0

    self.pms2Version = 0

    if PMS2.isConnected() then
        self.pms2Version = PMS2.getVer()
    end

    self.selectionEnum = enum({"spkVolume", "hpVolume", "volumeControlSystem"})
    self.selected = self.selectionEnum[1]
end

function audioSettings:draw(onFocus)
    self.menuSystem:start(onFocus)
    self.menuSystem:printLine("Speakers volume:", self.selected.id)
    self.menuSystem:printLineValue(self.speakersVolume, false)
    self.menuSystem:printLine("Headphones volume:", self.selected.id)
    self.menuSystem:printLineValue(self.headphonesVolume, false)
    self.menuSystem:printLine("Volume control system:", self.selected.id)
    if self.volumeControlSystem == UAMP.HUD_CTRL_GCPAD then
        self.menuSystem:printLineValue("GC controller", false)
    elseif self.volumeControlSystem == UAMP.HUD_CTRL_POT then
        self.menuSystem:printLineValue("Potentiometer", false)
    elseif self.volumeControlSystem == UAMP.HUD_CTRL_BUTTONS then
        self.menuSystem:printLineValue("Buttons", false)
    end
    self.menuSystem:finish()
end

function audioSettings:handleInputs(onFocus)
    local down = Pad.gendown(0)
    local held = Pad.genheld(0)

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    local curId = self.selected.id

    if down.BUTTON_DOWN and curId < self.selectionEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if down.BUTTON_A then
        if self.selected == self.selectionEnum.spkVolume then
            self.speakersVolume = self.speakersVolume + 1
            if self.speakersVolume > 32 then
                self.speakersVolume = 32
            end
            UAMP.setSpeakersVolume(self.speakersVolume)
        elseif self.selected == self.selectionEnum.hpVolume then
            self.headphonesVolume = self.headphonesVolume + 1
            if self.headphonesVolume > 32 then
                self.headphonesVolume = 32
            end
            UAMP.setHeadphonesVolume(self.headphonesVolume)
        elseif self.selected == self.selectionEnum.volumeControlSystem then
            if self.volumeControlSystem == UAMP.HUD_CTRL_GCPAD then
                if self.pms2Version >= 1.2 then
                    self.volumeControlSystem = UAMP.HUD_CTRL_POT
                else
                    self.volumeControlSystem = UAMP.HUD_CTRL_BUTTONS
                end
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_POT then
                self.volumeControlSystem = UAMP.HUD_CTRL_BUTTONS
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_BUTTONS then
                self.volumeControlSystem = UAMP.HUD_CTRL_GCPAD
            end
            UAMP.setVolumeControlSystem(self.volumeControlSystem)
        end
    elseif down.BUTTON_RIGHT or (held.BUTTON_RIGHT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.spkVolume then
            self.speakersVolume = self.speakersVolume + 1
            if self.speakersVolume > 32 then
                self.speakersVolume = 32
            end
            UAMP.setSpeakersVolume(self.speakersVolume)
        elseif self.selected == self.selectionEnum.hpVolume then
            self.headphonesVolume = self.headphonesVolume + 1
            if self.headphonesVolume > 32 then
                self.headphonesVolume = 32
            end
            UAMP.setHeadphonesVolume(self.headphonesVolume)
        elseif self.selected == self.selectionEnum.volumeControlSystem then
            if self.volumeControlSystem == UAMP.HUD_CTRL_GCPAD then
                if self.pms2Version >= 1.2 then
                    self.volumeControlSystem = UAMP.HUD_CTRL_POT
                else
                    self.volumeControlSystem = UAMP.HUD_CTRL_BUTTONS
                end
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_POT then
                self.volumeControlSystem = UAMP.HUD_CTRL_BUTTONS
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_BUTTONS then
                self.volumeControlSystem = UAMP.HUD_CTRL_GCPAD
            end
            UAMP.setVolumeControlSystem(self.volumeControlSystem)
        end
        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    elseif down.BUTTON_LEFT or (held.BUTTON_LEFT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.spkVolume then
            self.speakersVolume = self.speakersVolume - 1
            if self.speakersVolume < 0 then
                self.speakersVolume = 0
            end
            UAMP.setSpeakersVolume(self.speakersVolume)
        elseif self.selected == self.selectionEnum.hpVolume then
            self.headphonesVolume = self.headphonesVolume - 1
            if self.headphonesVolume < 0 then
                self.headphonesVolume = 0
            end
            UAMP.setHeadphonesVolume(self.headphonesVolume)
        elseif self.selected == self.selectionEnum.volumeControlSystem then
            if self.volumeControlSystem == UAMP.HUD_CTRL_GCPAD then
                self.volumeControlSystem = UAMP.HUD_CTRL_BUTTONS
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_POT then
                self.volumeControlSystem = UAMP.HUD_CTRL_GCPAD
            elseif self.volumeControlSystem == UAMP.HUD_CTRL_BUTTONS then
                self.volumeControlSystem = UAMP.HUD_CTRL_GCPAD
                if self.pms2Version >= 1.2 then
                    self.volumeControlSystem = UAMP.HUD_CTRL_POT
                else
                    self.volumeControlSystem = UAMP.HUD_CTRL_GCPAD
                end
            end
            UAMP.setVolumeControlSystem(self.volumeControlSystem)
        end

        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    end
    self.selected = self.selectionEnum[curId]
end
