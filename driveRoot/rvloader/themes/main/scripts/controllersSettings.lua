require 'scripts/enum'
require 'scripts/class'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'
require 'scripts/topbarcmd'

controllerSettings = class(SettingsMenu)
buttonsTester = class(SettingsMenu)
sticksWizard = class(SettingsMenu)

function controllerSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.updateMenuSystem = MenuSystem()
    self.updateMenuSystem.font = font
    self.updateMenuSystem.lineHeight = lineHeight
    self.updateMenuSystem.columnWidth = columnWidth
    self.updateMenuSystem.sideMargin = sideMargin

    self.gcpUpdateTimeout = 2000
    self.gcpPrevUpdating = false
    self.gcpOldVersion = 0.0
    self.gcpNewVersion = 0.0
    self.gcpUpdateTime = Time.getms() - self.gcpUpdateTimeout

    self.runningButtonsTester = false
    self.runningSticksWizard = false
    self.oldControllerConnectedStatus = false
    self.isGCP2 = false
    self.selectionEnum = enum({"buttonsTester"})
    self.selected = self.selectionEnum[1]
    self.buttonsTester = buttonsTester(font, lineHeight, columnWidth, sideMargin, self)
    self.sticksWizard = sticksWizard(font, lineHeight, columnWidth, sideMargin, self)
end

function controllerSettings:draw(onFocus)
    if Gcp.isUpdating() == false and self.gcpPrevUpdating then
        self.gcpUpdateTime = Time.getms()
        self.gcpNewVersion = Gcp.getFWVer()
    end
    self.gcpPrevUpdating = Gcp.isUpdating()

    if Gcp.isUpdating() or (Time.getms() - self.gcpUpdateTime) < self.gcpUpdateTimeout then
        self.updateMenuSystem:start(onFocus)
        self.updateMenuSystem:printLine("Old version:", 0)
        self.updateMenuSystem:printLineValue(string.format("%.1f", self.gcpOldVersion), false)
        self.updateMenuSystem:printLine("Update progress:", 0)
        self.updateMenuSystem:printLineValue(Gcp.getUpdateProgress() .. " %%", false)
        if not Gcp.isUpdating() then
            topBarEnableWheel()
            if Gcp.hasUpdateSucceeded then
                self.updateMenuSystem:printLine("New version:", 0)
                self.updateMenuSystem:printLineValue(string.format("%.1f", self.gcpNewVersion), false)
                self.updateMenuSystem:printLine("Update complete! Will now return", 0)
            else
                self.updateMenuSystem:printLine("Update failed", 0)
                self.updateMenuSystem:printLine("Make sure you have /GCPlus2.0Update.hex on your USB drive", 0)
            end
        else
            self.updateMenuSystem:printLine("Updating, don't power off!", 0)
        end
        self.updateMenuSystem:finish()
        return
    end

    if self.runningButtonsTester then
        self.buttonsTester:draw(onFocus)
        return
    end

    if self.runningSticksWizard then
        self.sticksWizard:draw(onFocus)
        return
    end

    local connected = Pad.isConnected(0)

    if connected ~= self.oldControllerConnectedStatus then
        self.oldControllerConnectedStatus = connected
        if connected then
            --Controller is connected, check if it's GC+2.0
            if Gcp.isV2() then
                self.isGCP2 = true
                self.deadzone = Gcp.getSticksDeadzoneRadius()
                self.deadzoneMode = Gcp.getSticksDeadzoneMode()
                self.triggerMode = Gcp.getTriggerMode()
                self.rumbleIntensity = Gcp.getRumbleIntensity()
                self.oldDeadzone = self.deadzone
                self.oldDeadzoneMode = self.deadzoneMode
                self.oldTriggerMode = self.triggerMode
                self.oldRumbleIntensity = self.rumbleIntensity
                self.selectionEnum = enum({"buttonsTester", "sticksWizard", "sticksDeadzone", "sticksDeadzoneMode", "triggersMode", "rumble", "saveConfig", "firmwareUpdate"})
                self.selected = self.selectionEnum[1]
            else
                self.isGCP2 = false
                self.selectionEnum = enum({"buttonsTester"})
                self.selected = self.selectionEnum[1]
            end
        else
            self.isGCP2 = false
            self.selectionEnum = enum({"buttonsTester"})
            self.selected = self.selectionEnum[1]
        end
    end

    self.menuSystem:start(onFocus)
    self.menuSystem:printLine("Buttons tester", self.selected.id)
    if self.isGCP2 then
        self.menuSystem:printLine("Sticks wizard", self.selected.id)
        self.menuSystem:printLine("Sticks deadzone:", self.selected.id)
        self.menuSystem:printLineValue(self.deadzone, self.deadzone ~= self.oldDeadzone)
        self.menuSystem:printLine("Sticks deadzone mode:", self.selected.id)
        if self.deadzoneMode == Gcp.DEADZONE_RADIAL then
            self.menuSystem:printLineValue("Radial", self.deadzoneMode ~= self.oldDeadzoneMode)
        else
            self.menuSystem:printLineValue("Scaled radial", self.deadzoneMode ~= self.oldDeadzoneMode)
        end
        self.menuSystem:printLine("Triggers mode:", self.selected.id)
        if self.triggerMode == Gcp.TRIGGER_DIGITAL then
            self.menuSystem:printLineValue("Digital", self.triggerMode ~= self.oldTriggerMode)
        else
            self.menuSystem:printLineValue("Analog", self.triggerMode ~= self.oldTriggerMode)
        end
        self.menuSystem:printLine("Rumble intensity:", self.selected.id)
        self.menuSystem:printLineValue(self.rumbleIntensity, self.rumbleIntensity ~= self.oldRumbleIntensity)
        self.menuSystem:printLine("Save config", self.selected.id)
        self.menuSystem:printLine("Firmware update", self.selected.id)
    end
    self.menuSystem:finish()
end

function controllerSettings:handleInputs(onFocus)
    if Gcp.isUpdating() or (Time.getms() - self.gcpUpdateTime) < self.gcpUpdateTimeout then
        return
    end

    if self.runningButtonsTester then
        topBarDisableWheel()
        self.buttonsTester:handleInputs(onFocus)
        return
    end

    if self.runningSticksWizard then
        topBarDisableWheel()
        self.sticksWizard:handleInputs(onFocus)
        return
    end

    topBarEnableWheel()

    local down = Pad.gendown(0)
    local curId = self.selected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < self.selectionEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if self.selected == self.selectionEnum.buttonsTester then
        if down.BUTTON_A then
            self.runningButtonsTester = true
        end
    elseif self.selected == self.selectionEnum.sticksWizard then
        if down.BUTTON_A then
            self.sticksWizard:reset()
            self.runningSticksWizard = true
        end
    elseif self.selected == self.selectionEnum.sticksDeadzone then
        if down.BUTTON_LEFT and self.deadzone > 0 then
            self.deadzone = self.deadzone - 1
        end
        if down.BUTTON_RIGHT and self.deadzone < 60 then
            self.deadzone = self.deadzone + 1
        end
    elseif self.selected == self.selectionEnum.sticksDeadzoneMode then
        if down.BUTTON_LEFT or down.BUTTON_RIGHT then
            self.deadzoneMode = 1 - self.deadzoneMode
        end
    elseif self.selected == self.selectionEnum.triggersMode then
        if down.BUTTON_LEFT or down.BUTTON_RIGHT then
            self.triggerMode = 1 - self.triggerMode
        end
    elseif self.selected == self.selectionEnum.rumble then
        if down.BUTTON_LEFT and self.rumbleIntensity > 0 then
            self.rumbleIntensity = self.rumbleIntensity - 1
        end
        if down.BUTTON_RIGHT and self.rumbleIntensity < 127 then
            self.rumbleIntensity = self.rumbleIntensity + 1
        end
    elseif self.selected == self.selectionEnum.saveConfig then
        if down.BUTTON_A then
            Gcp.setSticksDeadzoneRadius(self.deadzone)
            Gcp.setSticksDeadzoneMode(self.deadzoneMode)
            Gcp.setRumbleIntensity(self.rumbleIntensity)
            Gcp.setTriggerMode(self.triggerMode)
            self.oldDeadzone = self.deadzone
            self.oldDeadzoneMode = self.deadzoneMode
            self.oldRumbleIntensity = self.rumbleIntensity
            self.oldTriggerMode = self.triggerMode
        end
    elseif self.selected == self.selectionEnum.firmwareUpdate then
        if down.BUTTON_A then
            topBarDisableWheel()
            self.gcpOldVersion = Gcp.getFWVer()
            Gcp.startUpdate("/GCPlus2.0Update.hex")
        end
    end

    self.selected = self.selectionEnum[curId]
end

function buttonsTester:init(font, lineHeight, columnWidth, sideMargin, parent)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.controllerSettings = parent

    self.Outline_Img = Gfx.loadImage("buttonsTester/outline.png")
    self.A_outline_Img = Gfx.loadImage("buttonsTester/A_outline.png")
    self.A_pressed_Img = Gfx.loadImage("buttonsTester/A_pressed.png")
    self.B_outline_Img = Gfx.loadImage("buttonsTester/B_outline.png")
    self.B_pressed_Img = Gfx.loadImage("buttonsTester/B_pressed.png")
    self.CStick_outline_Img = Gfx.loadImage("buttonsTester/CStick_outline.png")
    self.CStick_pressed_Img = Gfx.loadImage("buttonsTester/CStick_pressed.png")
    self.L_analog_Img = Gfx.loadImage("buttonsTester/L_analog.png")
    self.L_outline_Img = Gfx.loadImage("buttonsTester/L_outline.png")
    self.L_pressed_Img = Gfx.loadImage("buttonsTester/L_pressed.png")
    self.R_analog_Img = Gfx.loadImage("buttonsTester/R_analog.png")
    self.R_outline_Img = Gfx.loadImage("buttonsTester/R_outline.png")
    self.R_pressed_Img = Gfx.loadImage("buttonsTester/R_pressed.png")
    self.Start_outline_Img = Gfx.loadImage("buttonsTester/Start_outline.png")
    self.Start_pressed_Img = Gfx.loadImage("buttonsTester/Start_pressed.png")
    self.Stick_outline_Img = Gfx.loadImage("buttonsTester/Stick_outline.png")
    self.Stick_pressed_Img = Gfx.loadImage("buttonsTester/Stick_pressed.png")
    self.X_outline_Img = Gfx.loadImage("buttonsTester/X_outline.png")
    self.X_pressed_Img = Gfx.loadImage("buttonsTester/X_pressed.png")
    self.Y_outline_Img = Gfx.loadImage("buttonsTester/Y_outline.png")
    self.Y_pressed_Img = Gfx.loadImage("buttonsTester/Y_pressed.png")
    self.Z_outline_Img = Gfx.loadImage("buttonsTester/Z_outline.png")
    self.Z_pressed_Img = Gfx.loadImage("buttonsTester/Z_pressed.png")
    self.Dpad_full_Img = Gfx.loadImage("buttonsTester/dpad_full.png")
    self.Dpad_outline_Img = Gfx.loadImage("buttonsTester/dpad_outline.png")
    self.Dpad_pressed_Img = Gfx.loadImage("buttonsTester/dpad_pressed.png")
end

function buttonsTester:draw(onFocus)
    local held = Pad.held(0)
    local outlineW, outlineH = Gfx.getImageSize(self.Outline_Img)
    local dpadW, dpadH = Gfx.getImageSize(self.Dpad_pressed_Img)
    local outlinePos = {
        x = (getDimensions()[1] - leftColumnWidth - outlineW) // 2,
        y = 32
    }
    Gfx.pushMatrix()
    Gfx.translate(outlinePos.x, outlinePos.y)
    Gfx.drawImage(self.Outline_Img)

    if held.BUTTON_START then
        Gfx.drawImage(self.Start_pressed_Img, 165, 80)
    else
        Gfx.drawImage(self.Start_outline_Img, 165, 80)
    end
    if held.BUTTON_A then
        Gfx.drawImage(self.A_pressed_Img, 259, 66)
    else
        Gfx.drawImage(self.A_outline_Img, 259, 66)
    end
    if held.BUTTON_B then
        Gfx.drawImage(self.B_pressed_Img, 228, 95)
    else
        Gfx.drawImage(self.B_outline_Img, 228, 95)
    end
    if held.BUTTON_X then
        Gfx.drawImage(self.X_pressed_Img, 308, 61)
    else
        Gfx.drawImage(self.X_outline_Img, 308, 61)
    end
    if held.BUTTON_Y then
        Gfx.drawImage(self.Y_pressed_Img, 251, 39)
    else
        Gfx.drawImage(self.Y_outline_Img, 251, 39)
    end
    --DPad
    Gfx.pushMatrix()
    Gfx.translate(92, 142)
    Gfx.drawImage(self.Dpad_outline_Img)
    if held.BUTTON_UP then
        Gfx.drawImage(self.Dpad_pressed_Img)
    end
    if held.BUTTON_RIGHT then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 90)
        Gfx.drawImage(self.Dpad_pressed_Img)
        Gfx.popMatrix()
    end
    if held.BUTTON_DOWN then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 180)
        Gfx.drawImage(self.Dpad_pressed_Img)
        Gfx.popMatrix()
    end
    if held.BUTTON_LEFT then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 270)
        Gfx.drawImage(self.Dpad_pressed_Img)
        Gfx.popMatrix()
    end
    Gfx.popMatrix()

    if held.TRIGGER_L then
        Gfx.drawImage(self.L_pressed_Img, 34, 1)
    elseif Pad.triggers(0).l > 0 then
        Gfx.drawImage(self.L_analog_Img, 34, 1)
    else
        Gfx.drawImage(self.L_outline_Img, 34, 1)
    end
    if held.TRIGGER_R then
        Gfx.drawImage(self.R_pressed_Img, 259, 1)
    elseif Pad.triggers(0).r > 0 then
        Gfx.drawImage(self.R_analog_Img, 259, 1)
    else
        Gfx.drawImage(self.R_outline_Img, 259, 1)
    end
    if held.TRIGGER_Z then
        Gfx.drawImage(self.Z_pressed_Img, 264, 9)
    else
        Gfx.drawImage(self.Z_outline_Img, 264, 9)
    end

    if Pad.triggers(0).l > 0 then
        Gfx.print(fonts[12], 18, 1, string.format("%u", Pad.triggers(0).l))
    end
    if Pad.triggers(0).r > 0 then
        Gfx.print(fonts[12], 323, 1, string.format("%u", Pad.triggers(0).r))
    end

    Gfx.pushMatrix()
    Gfx.translate(44 + Pad.stick(0).x * 20 / 128, 67 - Pad.stick(0).y * 20 / 128)
    Gfx.drawImage(self.Stick_pressed_Img)
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(217 + Pad.subStick(0).x * 20 / 128, 153 - Pad.subStick(0).y * 20 / 128)
    Gfx.drawImage(self.CStick_pressed_Img)
    Gfx.popMatrix()

    Gfx.print(self.menuSystem.font, 0, 280, "Hold A+B to activate rumble");
    Gfx.print(self.menuSystem.font, 0, 312, "Press L+R to go back");
    Gfx.popMatrix()
end

function buttonsTester:handleInputs(onFocus)
    local held = Pad.genheld(0)

    if held.BUTTON_A and held.BUTTON_B then
        Pad.setRumble(0, true)
    else
        Pad.setRumble(0, false)
    end

    if held.TRIGGER_R and held.TRIGGER_L then
        self.controllerSettings.runningButtonsTester = false
        Pad.setRumble(0, false)
    end
end

function sticksWizard:init(font, lineHeight, columnWidth, sideMargin, parent)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.controllerSettings = parent

    self.CStick_Img = Gfx.loadImage("buttonsTester/CStick.png")
    Gfx.resizeImage(self.CStick_Img, 128, 128)
    self.CStick_base_Img = Gfx.loadImage("buttonsTester/CStick_base.png")
    Gfx.resizeImage(self.CStick_base_Img, 128, 128)
    self.TesterLeftStick_Img = Gfx.loadImage("buttonsTester/LeftStick.png")
    Gfx.resizeImage(self.TesterLeftStick_Img, 128, 128)
    self.LeftStick_base_Img = Gfx.loadImage("buttonsTester/LeftStick_base.png")
    Gfx.resizeImage(self.LeftStick_base_Img, 128, 128)

    self.stickAnimation = Anim.new()
    Anim.addStep(self.stickAnimation, 1000, 0, 0, 0, 0)
    Anim.addStep(self.stickAnimation, 100, 0, 0, 48, 0)
    Anim.addStep(self.stickAnimation, 1000, 48, 0, 48, 0)

    for i = 0, 98 do
        x1 = math.floor(48 * math.cos(i * 2 * math.pi / 100))
        y1 = math.floor(-48 * math.sin(i * 2 * math.pi / 100))
        x2 = math.floor(48 * math.cos((i + 1) * 2 * math.pi / 100))
        y2 = math.floor(-48 * math.sin((i + 1) * 2 * math.pi / 100))
        Anim.addStep(self.stickAnimation, 10, x1, y1, x2, y2)
    end
    Anim.addStep(self.stickAnimation, 300, 48, 0, 48, 0)
    Anim.addReturnToHomeStep(self.stickAnimation, 100)
    Anim.resume(self.stickAnimation)

    self.stateEnum = enum({"LS_WAIT_FOR_MOVE",
            "LS_WAIT_FOR_ROT1", --60 degrees
            "LS_WAIT_FOR_ROT2", --180
            "LS_WAIT_FOR_ROT3", --240
            "LS_WAIT_FOR_ROT4", --360
            "LS_WAIT_FOR_RELEASE",
            "CS_WAIT_FOR_MOVE",
            "CS_WAIT_FOR_ROT1", --60 degrees
            "CS_WAIT_FOR_ROT2", --180
            "CS_WAIT_FOR_ROT3", --240
            "CS_WAIT_FOR_ROT4", --360
            "CS_WAIT_FOR_RELEASE",
            "STICKS_TEST"})
end

function sticksWizard:reset()
    self.oldSticksRange = Gcp.getSticksRange()
    self.oldSticksInvert = Gcp.getSticksInvert()
    self.oldSticksChannel = Gcp.getSticksChannel()
    Gcp.loadDefaultSticksConfig()
    Gcp.rebuildLUT()
    self.sticksRange = Gcp.getSticksRange()
    self.sticksInvert = Gcp.getSticksInvert()
    self.sticksChannel = Gcp.getSticksChannel()

    self.wizardState = self.stateEnum.LS_WAIT_FOR_MOVE

    --Force out of boundary values for sticks range
    self.sticksMax = {}
    self.sticksMin = {}
    for i = 1, 4 do
        self.sticksMin[i] = 127
        self.sticksMax[i] = -128
    end
end

function sticksWizard:draw(onFocus)
    local stickW, stickH = Gfx.getImageSize(self.LeftStick_base_Img)
    local stickAnimationPos = Anim.getPosition(self.stickAnimation)

    Gfx.pushMatrix()
    Gfx.translate((getDimensions()[1] - leftColumnWidth) // 2 - stickW - 64, 64)
    Gfx.drawImage(self.LeftStick_base_Img)
    if self.wizardState.id <= self.stateEnum.LS_WAIT_FOR_RELEASE.id then
        Gfx.drawImage(self.TesterLeftStick_Img, stickAnimationPos.x, stickAnimationPos.y)
    elseif self.wizardState.id == self.stateEnum.STICKS_TEST.id then
        Gfx.drawImage(self.TesterLeftStick_Img, 48 * Pad.stick(0).x / 127, -48 * Pad.stick(0).y / 127)
    else
        Gfx.drawImage(self.TesterLeftStick_Img)
    end
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate((getDimensions()[1] - leftColumnWidth) // 2 + 64, 64)
    Gfx.drawImage(self.CStick_base_Img)
    if self.wizardState.id < self.stateEnum.CS_WAIT_FOR_MOVE.id then
        Gfx.drawImage(self.CStick_Img)
    elseif self.wizardState.id < self.stateEnum.STICKS_TEST.id then
        Gfx.drawImage(self.CStick_Img, stickAnimationPos.x, stickAnimationPos.y)
    else
        Gfx.drawImage(self.CStick_Img, 48 * Pad.subStick(0).x / 127, -48 * Pad.subStick(0).y / 127)
    end
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(leftMargin, 128 + stickH)
    if self.wizardState == self.stateEnum.LS_WAIT_FOR_MOVE then
        Gfx.print(fonts[20], 0, 0, "Move the left stick to the far right.");
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT1 or
           self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT2 or
           self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT3 then
        Gfx.print(fonts[20], 0, 0, "Move the left stick in anti-clockwise direction.");
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT4 then
        Gfx.print(fonts[20], 0, 0, "Keep turning");
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_RELEASE then
        Gfx.print(fonts[20], 0, 0, "Release the left stick.");
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_MOVE then
        Gfx.print(fonts[20], 0, 0, "Move the C-stick to the far right.");
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT1 then
        Gfx.print(fonts[20], 0, 0, "Move the C-stick in anti-clockwise direction.");
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT2 or
           self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT3 or
           self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT4 then
        Gfx.print(fonts[20], 0, 0, "Keep turning.");
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_RELEASE then
        Gfx.print(fonts[20], 0, 0, "Release the C-stick.");
    elseif self.wizardState == self.stateEnum.STICKS_TEST then
        Gfx.print(fonts[20], 0, 0, "Test your sticks and make sure they work properly.");
    end

    if self.wizardState == self.stateEnum.STICKS_TEST then
        Gfx.print(fonts[20], 0, 32, "Press A to save or B to cancel.");
    else
        Gfx.print(fonts[20], 0, 32, "Press B to cancel.");
    end

    Gfx.popMatrix()
end

function sticksWizard:handleInputs(onFocus)
    local STICKS_THRESHOLD = 30
    local sticks = {Pad.stick(0).x, Pad.stick(0).y, Pad.subStick(0).x, Pad.subStick(0).y}
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        --Restore old sticks config
        Gcp.setSticksRange(self.oldSticksRange)
        Gcp.setSticksInvert(self.oldSticksInvert)
        Gcp.setSticksChannel(self.oldSticksChannel)
        Gcp.rebuildLUT()
        self.controllerSettings.runningSticksWizard = false
    end

    --Check ranges
    for i = 1, 4 do
        if sticks[i] > self.sticksMax[i] then
            self.sticksMax[i] = sticks[i]
        end
        if sticks[i] < self.sticksMin[i] then
            self.sticksMin[i] = sticks[i]
        end
    end

    --Check which stick has the max absoulute value
    local maxStick = sticks[1]
    local maxIdx = 1
    for i = 2, 4 do
        if (math.abs(sticks[i]) > math.abs(maxStick)) then
            maxStick = sticks[i]
            maxIdx = i
        end
    end

    if self.wizardState == self.stateEnum.LS_WAIT_FOR_MOVE then
        --Check if a stick has moved
        if math.abs(maxStick) > STICKS_THRESHOLD then
            self.sticksChannel.leftX = maxIdx
            if maxStick < 0 then
                self.sticksInvert.leftX = true
            end
            self.wizardState = self.stateEnum.LS_WAIT_FOR_ROT1
        end
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT1 then
        --Check if the stick has been rotated
        local angle = 0
        if self.sticksInvert.leftX then
            angle = math.atan(maxStick, -sticks[self.sticksChannel.leftX])
        else
            angle = math.atan(maxStick, sticks[self.sticksChannel.leftX])
        end
        if ((math.abs(angle) > math.pi / 3) and maxIdx ~= self.sticksChannel.leftX) then
            self.sticksChannel.leftY = maxIdx
            if angle < 0 then
                self.sticksInvert.leftY = true
            end
            self.wizardState = self.stateEnum.LS_WAIT_FOR_ROT2
        end
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT2 then
        --Wait until the stick is on the left
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.leftX then
            tempX = -sticks[self.sticksChannel.leftX]
        else
            tempX = sticks[self.sticksChannel.leftX]
        end
        if self.sticksInvert.leftY then
            tempY = -sticks[self.sticksChannel.leftY]
        else
            tempY = sticks[self.sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) > 5 * math.pi / 6 then
            self.wizardState = self.stateEnum.LS_WAIT_FOR_ROT3
        end
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT3 then
        --Wait until the stick is on the bottom left
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.leftX then
            tempX = -sticks[self.sticksChannel.leftX]
        else
            tempX = sticks[self.sticksChannel.leftX]
        end
        if self.sticksInvert.leftY then
            tempY = -sticks[self.sticksChannel.leftY]
        else
            tempY = sticks[self.sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if angle < - math.pi / 3 then
            self.wizardState = self.stateEnum.LS_WAIT_FOR_ROT4
        end
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_ROT4 then
        --Wait until the stick is on the right
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.leftX then
            tempX = -sticks[self.sticksChannel.leftX]
        else
            tempX = sticks[self.sticksChannel.leftX]
        end
        if self.sticksInvert.leftY then
            tempY = -sticks[self.sticksChannel.leftY]
        else
            tempY = sticks[self.sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) < 0.1 then
            self.wizardState = self.stateEnum.LS_WAIT_FOR_RELEASE
        end
    elseif self.wizardState == self.stateEnum.LS_WAIT_FOR_RELEASE then
        --Wait for the stick to be released
        if math.sqrt(sticks[self.sticksChannel.leftX] * sticks[self.sticksChannel.leftX] + sticks[self.sticksChannel.leftY] * sticks[self.sticksChannel.leftY]) < STICKS_THRESHOLD then
            self.wizardState = self.stateEnum.CS_WAIT_FOR_MOVE
            Anim.reset(self.stickAnimation)
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_MOVE then
        --Check if a stick has moved
        if math.abs(maxStick) > STICKS_THRESHOLD then
            self.sticksChannel.rightX = maxIdx
            if maxStick < 0 then
                self.sticksInvert.rightX = true
            end
            self.wizardState = self.stateEnum.CS_WAIT_FOR_ROT1
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT1 then
        --Check if the stick has been rotated
        local angle = 0
        if self.sticksInvert.rightX then
            angle = math.atan(maxStick, -sticks[self.sticksChannel.rightX])
        else
            angle = math.atan(maxStick, sticks[self.sticksChannel.rightX])
        end
        if ((math.abs(angle) > math.pi / 3) and maxIdx ~= self.sticksChannel.rightX) then
            self.sticksChannel.rightY = maxIdx
            if angle < 0 then
                self.sticksInvert.rightY = true
            end
            self.wizardState = self.stateEnum.CS_WAIT_FOR_ROT2
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT2 then
        --Wait until the stick is on the left
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.rightX then
            tempX = -sticks[self.sticksChannel.rightX]
        else
            tempX = sticks[self.sticksChannel.rightX]
        end
        if self.sticksInvert.rightY then
            tempY = -sticks[self.sticksChannel.rightY]
        else
            tempY = sticks[self.sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) > 5 * math.pi / 6 then
            self.wizardState = self.stateEnum.CS_WAIT_FOR_ROT3
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT3 then
        --Wait until the stick is on the bottom left
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.rightX then
            tempX = -sticks[self.sticksChannel.rightX]
        else
            tempX = sticks[self.sticksChannel.rightX]
        end
        if self.sticksInvert.rightY then
            tempY = -sticks[self.sticksChannel.rightY]
        else
            tempY = sticks[self.sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if angle < - math.pi / 3 then
            self.wizardState = self.stateEnum.CS_WAIT_FOR_ROT4
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_ROT4 then
        --Wait until the stick is on the right
        local tempX = 0
        local tempY = 0
        if self.sticksInvert.rightX then
            tempX = -sticks[self.sticksChannel.rightX]
        else
            tempX = sticks[self.sticksChannel.rightX]
        end
        if self.sticksInvert.rightY then
            tempY = -sticks[self.sticksChannel.rightY]
        else
            tempY = sticks[self.sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) < 0.1 then
            self.wizardState = self.stateEnum.CS_WAIT_FOR_RELEASE
        end
    elseif self.wizardState == self.stateEnum.CS_WAIT_FOR_RELEASE then
        --Wait for the stick to be released
        if math.sqrt(sticks[self.sticksChannel.rightX] * sticks[self.sticksChannel.rightX] + sticks[self.sticksChannel.rightY] * sticks[self.sticksChannel.rightY]) < STICKS_THRESHOLD then
            self.wizardState = self.stateEnum.STICKS_TEST
            if self.sticksInvert.leftX then
                local temp = -self.sticksMax[self.sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMinX = math.floor(temp + 0.5)
                temp = -self.sticksMin[self.sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMaxX = math.floor(temp + 0.5)
            else
                local temp = self.sticksMin[self.sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMinX = math.floor(temp + 0.5)
                temp = self.sticksMax[self.sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMaxX = math.floor(temp + 0.5)
            end
            if self.sticksInvert.leftY then
                local temp = -self.sticksMax[self.sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMinY = math.floor(temp + 0.5)
                temp = -self.sticksMin[self.sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMaxY = math.floor(temp + 0.5)
            else
                local temp = self.sticksMin[self.sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMinY = math.floor(temp + 0.5)
                temp = self.sticksMax[self.sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.leftMaxY = math.floor(temp + 0.5)
            end
            if self.sticksInvert.rightX then
                local temp = -self.sticksMax[self.sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMinX = math.floor(temp + 0.5)
                temp = -self.sticksMin[self.sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMaxX = math.floor(temp + 0.5)
            else
                local temp = self.sticksMin[self.sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMinX = math.floor(temp + 0.5)
                temp = self.sticksMax[self.sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMaxX = math.floor(temp + 0.5)
            end
            if self.sticksInvert.rightY then
                local temp = -self.sticksMax[self.sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMinY = math.floor(temp + 0.5)
                temp = -self.sticksMin[self.sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMaxY = math.floor(temp + 0.5)
            else
                local temp = self.sticksMin[self.sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMinY = math.floor(temp + 0.5)
                temp = self.sticksMax[self.sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                self.sticksRange.rightMaxY = math.floor(temp + 0.5)
            end
            Gcp.setSticksRange(self.sticksRange)
            Gcp.setSticksInvert(self.sticksInvert)
            Gcp.setSticksChannel(self.sticksChannel)
            Gcp.rebuildLUT()
            Anim.reset(self.stickAnimation)
        end
    elseif self.wizardState == self.stateEnum.STICKS_TEST then
        if down.BUTTON_A then
            --Go back
            self.controllerSettings.runningSticksWizard = false
        end
    end
end
