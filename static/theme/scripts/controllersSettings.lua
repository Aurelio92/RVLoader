function initButtonTester()
    buttonsTesterOutline_Img = Gfx.loadImage("buttonsTester/outline.png")
    buttonsTesterA_outline_Img = Gfx.loadImage("buttonsTester/A_outline.png")
    buttonsTesterA_pressed_Img = Gfx.loadImage("buttonsTester/A_pressed.png")
    buttonsTesterB_outline_Img = Gfx.loadImage("buttonsTester/B_outline.png")
    buttonsTesterB_pressed_Img = Gfx.loadImage("buttonsTester/B_pressed.png")
    buttonsTesterCStick_outline_Img = Gfx.loadImage("buttonsTester/CStick_outline.png")
    buttonsTesterCStick_pressed_Img = Gfx.loadImage("buttonsTester/CStick_pressed.png")
    buttonsTesterL_analog_Img = Gfx.loadImage("buttonsTester/L_analog.png")
    buttonsTesterL_outline_Img = Gfx.loadImage("buttonsTester/L_outline.png")
    buttonsTesterL_pressed_Img = Gfx.loadImage("buttonsTester/L_pressed.png")
    buttonsTesterR_analog_Img = Gfx.loadImage("buttonsTester/R_analog.png")
    buttonsTesterR_outline_Img = Gfx.loadImage("buttonsTester/R_outline.png")
    buttonsTesterR_pressed_Img = Gfx.loadImage("buttonsTester/R_pressed.png")
    buttonsTesterStart_outline_Img = Gfx.loadImage("buttonsTester/Start_outline.png")
    buttonsTesterStart_pressed_Img = Gfx.loadImage("buttonsTester/Start_pressed.png")
    buttonsTesterStick_outline_Img = Gfx.loadImage("buttonsTester/Stick_outline.png")
    buttonsTesterStick_pressed_Img = Gfx.loadImage("buttonsTester/Stick_pressed.png")
    buttonsTesterX_outline_Img = Gfx.loadImage("buttonsTester/X_outline.png")
    buttonsTesterX_pressed_Img = Gfx.loadImage("buttonsTester/X_pressed.png")
    buttonsTesterY_outline_Img = Gfx.loadImage("buttonsTester/Y_outline.png")
    buttonsTesterY_pressed_Img = Gfx.loadImage("buttonsTester/Y_pressed.png")
    buttonsTesterZ_outline_Img = Gfx.loadImage("buttonsTester/Z_outline.png")
    buttonsTesterZ_pressed_Img = Gfx.loadImage("buttonsTester/Z_pressed.png")
    buttonsTesterDpad_full_Img = Gfx.loadImage("buttonsTester/dpad_full.png")
    buttonsTesterDpad_outline_Img = Gfx.loadImage("buttonsTester/dpad_outline.png")
    buttonsTesterDpad_pressed_Img = Gfx.loadImage("buttonsTester/dpad_pressed.png")
end

function initSticksWizard()
    buttonsTesterCStick_Img = Gfx.loadImage("buttonsTester/CStick.png")
    Gfx.resizeImage(buttonsTesterCStick_Img, 128, 128)
    buttonsTesterCStick_base_Img = Gfx.loadImage("buttonsTester/CStick_base.png")
    Gfx.resizeImage(buttonsTesterCStick_base_Img, 128, 128)
    buttonsTesterLeftStick_Img = Gfx.loadImage("buttonsTester/LeftStick.png")
    Gfx.resizeImage(buttonsTesterLeftStick_Img, 128, 128)
    buttonsTesterLeftStick_base_Img = Gfx.loadImage("buttonsTester/LeftStick_base.png")
    Gfx.resizeImage(buttonsTesterLeftStick_base_Img, 128, 128)

    sticksWizardAnimation = Anim.new()
    Anim.addStep(sticksWizardAnimation, 1000, 0, 0, 0, 0)
    Anim.addStep(sticksWizardAnimation, 100, 0, 0, 48, 0)
    Anim.addStep(sticksWizardAnimation, 1000, 48, 0, 48, 0)

    for i = 0, 98 do
        x1 = math.floor(48 * math.cos(i * 2 * math.pi / 100))
        y1 = math.floor(-48 * math.sin(i * 2 * math.pi / 100))
        x2 = math.floor(48 * math.cos((i + 1) * 2 * math.pi / 100))
        y2 = math.floor(-48 * math.sin((i + 1) * 2 * math.pi / 100))
        Anim.addStep(sticksWizardAnimation, 10, x1, y1, x2, y2)
    end
    Anim.addStep(sticksWizardAnimation, 300, 48, 0, 48, 0)
    Anim.addReturnToHomeStep(sticksWizardAnimation, 100)
    Anim.resume(sticksWizardAnimation)

    sticksWizardStateEnum = enum({"LS_WAIT_FOR_MOVE",
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

function resetSticksWizard()
    oldSticksRange = Gcp.getSticksRange()
    oldSticksInvert = Gcp.getSticksInvert()
    oldSticksChannel = Gcp.getSticksChannel()
    Gcp.loadDefaultSticksConfig()
    Gcp.rebuildLUT()
    sticksRange = Gcp.getSticksRange()
    sticksInvert = Gcp.getSticksInvert()
    sticksChannel = Gcp.getSticksChannel()

    sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_MOVE

    --Force out of boundary values for sticks range
    sticksMax = {}
    sticksMin = {}
    for i = 1, 4 do
        sticksMin[i] = 127
        sticksMax[i] = -128
    end
end

function initController()
    gcpUpdateTimeout = 2000
    gcpPrevUpdating = false
    gcpOldVersion = 0.0
    gcpNewVersion = 0.0
    gcpUpdateTime = Time.getms() - gcpUpdateTimeout

    runningButtonsTester = false
    runningSticksWizard = false
    oldControllerConnectedStatus = false
    isGCP2 = false
    controllerSelectedEnum = enum({"buttonsTester"})
    controllerSelected = controllerSelectedEnum[1]
    initButtonTester()
    initSticksWizard()
end

function drawButtonsTester()
    local held = Pad.held(0)
    local outlineW, outlineH = Gfx.getImageSize(buttonsTesterOutline_Img)
    local dpadW, dpadH = Gfx.getImageSize(buttonsTesterDpad_pressed_Img)
    local outlinePos = {
        x = (getDimensions()[1] - leftColumnWidth - outlineW) // 2,
        y = 32
    }
    Gfx.pushMatrix()
    Gfx.translate(outlinePos.x, outlinePos.y)
    Gfx.drawImage(buttonsTesterOutline_Img)

    if held.BUTTON_START then
        Gfx.drawImage(buttonsTesterStart_pressed_Img, 165, 80)
    else
        Gfx.drawImage(buttonsTesterStart_outline_Img, 165, 80)
    end
    if held.BUTTON_A then
        Gfx.drawImage(buttonsTesterA_pressed_Img, 259, 66)
    else
        Gfx.drawImage(buttonsTesterA_outline_Img, 259, 66)
    end
    if held.BUTTON_B then
        Gfx.drawImage(buttonsTesterB_pressed_Img, 228, 95)
    else
        Gfx.drawImage(buttonsTesterB_outline_Img, 228, 95)
    end
    if held.BUTTON_X then
        Gfx.drawImage(buttonsTesterX_pressed_Img, 308, 61)
    else
        Gfx.drawImage(buttonsTesterX_outline_Img, 308, 61)
    end
    if held.BUTTON_Y then
        Gfx.drawImage(buttonsTesterY_pressed_Img, 251, 39)
    else
        Gfx.drawImage(buttonsTesterY_outline_Img, 251, 39)
    end
    --DPad
    Gfx.pushMatrix()
    Gfx.translate(92, 142)
    Gfx.drawImage(buttonsTesterDpad_outline_Img)
    if held.BUTTON_UP then
        Gfx.drawImage(buttonsTesterDpad_pressed_Img)
    end
    if held.BUTTON_RIGHT then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 90)
        Gfx.drawImage(buttonsTesterDpad_pressed_Img)
        Gfx.popMatrix()
    end
    if held.BUTTON_DOWN then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 180)
        Gfx.drawImage(buttonsTesterDpad_pressed_Img)
        Gfx.popMatrix()
    end
    if held.BUTTON_LEFT then
        Gfx.pushMatrix()
        Gfx.rotate(dpadW / 2, dpadH / 2, 270)
        Gfx.drawImage(buttonsTesterDpad_pressed_Img)
        Gfx.popMatrix()
    end
    Gfx.popMatrix()

    if held.TRIGGER_L then
        Gfx.drawImage(buttonsTesterL_pressed_Img, 34, 1)
    elseif Pad.triggers(0).l > 0 then
        Gfx.drawImage(buttonsTesterL_analog_Img, 34, 1)
    else
        Gfx.drawImage(buttonsTesterL_outline_Img, 34, 1)
    end
    if held.TRIGGER_R then
        Gfx.drawImage(buttonsTesterR_pressed_Img, 259, 1)
    elseif Pad.triggers(0).r > 0 then
        Gfx.drawImage(buttonsTesterR_analog_Img, 259, 1)
    else
        Gfx.drawImage(buttonsTesterR_outline_Img, 259, 1)
    end
    if held.TRIGGER_Z then
        Gfx.drawImage(buttonsTesterZ_pressed_Img, 264, 9)
    else
        Gfx.drawImage(buttonsTesterZ_outline_Img, 264, 9)
    end

    if Pad.triggers(0).l > 0 then
        Gfx.print(fonts[12], 18, 1, string.format("%u", Pad.triggers(0).l))
    end
    if Pad.triggers(0).r > 0 then
        Gfx.print(fonts[12], 323, 1, string.format("%u", Pad.triggers(0).r))
    end

    Gfx.pushMatrix()
    Gfx.translate(44 + Pad.stick(0).x * 20 / 128, 67 - Pad.stick(0).y * 20 / 128)
    Gfx.drawImage(buttonsTesterStick_pressed_Img)
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(217 + Pad.subStick(0).x * 20 / 128, 153 - Pad.subStick(0).y * 20 / 128)
    Gfx.drawImage(buttonsTesterCStick_pressed_Img)
    Gfx.popMatrix()

    Gfx.print(fonts[20], 0, 280, "Hold A+B to activate rumble");
    Gfx.print(fonts[20], 0, 312, "Press L+R to go back");
    Gfx.popMatrix()
end

function drawSticksWizard()
    local stickW, stickH = Gfx.getImageSize(buttonsTesterLeftStick_base_Img)
    local sticksWizardAnimationPos = Anim.getPosition(sticksWizardAnimation)

    Gfx.pushMatrix()
    Gfx.translate((getDimensions()[1] - leftColumnWidth) // 2 - stickW - 64, 64)
    Gfx.drawImage(buttonsTesterLeftStick_base_Img)
    if sticksWizardState.id <= sticksWizardStateEnum.LS_WAIT_FOR_RELEASE.id then
        Gfx.drawImage(buttonsTesterLeftStick_Img, sticksWizardAnimationPos.x, sticksWizardAnimationPos.y)
    elseif sticksWizardState.id == sticksWizardStateEnum.STICKS_TEST.id then
        Gfx.drawImage(buttonsTesterLeftStick_Img, 48 * Pad.stick(0).x / 127, -48 * Pad.stick(0).y / 127)
    else
        Gfx.drawImage(buttonsTesterLeftStick_Img)
    end
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate((getDimensions()[1] - leftColumnWidth) // 2 + 64, 64)
    Gfx.drawImage(buttonsTesterCStick_base_Img)
    if sticksWizardState.id < sticksWizardStateEnum.CS_WAIT_FOR_MOVE.id then
        Gfx.drawImage(buttonsTesterCStick_Img)
    elseif sticksWizardState.id < sticksWizardStateEnum.STICKS_TEST.id then
        Gfx.drawImage(buttonsTesterCStick_Img, sticksWizardAnimationPos.x, sticksWizardAnimationPos.y)
    else
        Gfx.drawImage(buttonsTesterCStick_Img, 48 * Pad.subStick(0).x / 127, -48 * Pad.subStick(0).y / 127)
    end
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(leftMargin, 128 + stickH)
    if sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_MOVE then
        Gfx.print(fonts[20], 0, 0, "Move the left stick to the far right.");
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT1 or
           sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT2 or
           sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT3 then
        Gfx.print(fonts[20], 0, 0, "Move the left stick in anti-clockwise direction.");
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT4 then
        Gfx.print(fonts[20], 0, 0, "Keep turning");
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_RELEASE then
        Gfx.print(fonts[20], 0, 0, "Release the left stick.");
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_MOVE then
        Gfx.print(fonts[20], 0, 0, "Move the C-stick to the far right.");
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT1 then
        Gfx.print(fonts[20], 0, 0, "Move the C-stick in anti-clockwise direction.");
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT2 or
           sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT3 or
           sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT4 then
        Gfx.print(fonts[20], 0, 0, "Keep turning.");
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_RELEASE then
        Gfx.print(fonts[20], 0, 0, "Release the C-stick.");
    elseif sticksWizardState == sticksWizardStateEnum.STICKS_TEST then
        Gfx.print(fonts[20], 0, 0, "Test your sticks and make sure they work properly.");
    end

    if sticksWizardState == sticksWizardStateEnum.STICKS_TEST then
        Gfx.print(fonts[20], 0, 32, "Press A to save or B to cancel.");
    else
        Gfx.print(fonts[20], 0, 32, "Press B to cancel.");
    end

    Gfx.popMatrix()
end

function drawController(onFocus)
    if Gcp.isUpdating() == false and gcpPrevUpdating then
        gcpUpdateTime = Time.getms()
        gcpNewVersion = Gcp.getFWVer()
    end
    gcpPrevUpdating = Gcp.isUpdating()

    if Gcp.isUpdating() or (Time.getms() - gcpUpdateTime) < gcpUpdateTimeout then
        menuSystem.reset()
        menuSystem.printLine("Old version:", 0)
        menuSystem.printLineValue(string.format("%.1f", gcpOldVersion), false)
        menuSystem.printLine("Update progress:", 0)
        menuSystem.printLineValue(Gcp.getUpdateProgress() .. " %%", false)
        if not Gcp.isUpdating() then
            topBarEnableWheel()
            if Gcp.hasUpdateSucceeded then
                menuSystem.printLine("New version:", 0)
                menuSystem.printLineValue(string.format("%.1f", gcpNewVersion), false)
                menuSystem.printLine("Update complete! Will now return", 0)
            else
                menuSystem.printLine("Update failed", 0)
                menuSystem.printLine("Make sure you have /GCPlus2.0Update.hex on your USB drive", 0)
            end
        else
            menuSystem.printLine("Updating, don't power off!", 0)
        end
        return
    end

    if runningButtonsTester then
        drawButtonsTester()
        return
    end

    if runningSticksWizard then
        drawSticksWizard()
        return
    end

    local connected = Pad.isConnected(0)

    if connected ~= oldControllerConnectedStatus then
        oldControllerConnectedStatus = connected
        if connected then
            --Controller is connected, check if it's GC+2.0
            if Gcp.isV2() then
                isGCP2 = true
                controllerDeadzone = Gcp.getSticksDeadzoneRadius()
                controllerDeadzoneMode = Gcp.getSticksDeadzoneMode()
                controllerTriggerMode = Gcp.getTriggerMode()
                controllerRumbleIntensity = Gcp.getRumbleIntensity()
                controllerOldDeadzone = controllerDeadzone
                controllerOldDeadzoneMode = controllerDeadzoneMode
                controllerOldTriggerMode = controllerTriggerMode
                controllerOldRumbleIntensity = controllerRumbleIntensity
                controllerSelectedEnum = enum({"buttonsTester", "sticksWizard", "sticksDeadzone", "sticksDeadzoneMode", "triggersMode", "rumble", "saveConfig", "firmwareUpdate"})
                controllerSelected = controllerSelectedEnum[1]
            else
                isGCP2 = false
                controllerSelectedEnum = enum({"buttonsTester"})
                controllerSelected = controllerSelectedEnum[1]
            end
        else
            isGCP2 = false
            controllerSelectedEnum = enum({"buttonsTester"})
            controllerSelected = controllerSelectedEnum[1]
        end
    end

    local colWidth = getDimensions()[1] - leftColumnWidth
    if onFocus then
        Gfx.drawRectangle(0, (controllerSelected.id - 1) * lineHeight, colWidth, lineHeight, Gfx.RGBA8(0x1F, 0x22, 0x27, 0xFF));
    end
    menuSystem.reset()
    menuSystem.printLine("Buttons tester", controllerSelected.id)
    if isGCP2 then
        menuSystem.printLine("Sticks wizard", controllerSelected.id)
        menuSystem.printLine("Sticks deadzone:", controllerSelected.id)
        menuSystem.printLineValue(controllerDeadzone, controllerDeadzone ~= controllerOldDeadzone)
        menuSystem.printLine("Sticks deadzone mode:", controllerSelected.id)
        if controllerDeadzoneMode == Gcp.DEADZONE_RADIAL then
            menuSystem.printLineValue("Radial", controllerDeadzoneMode ~= controllerOldDeadzoneMode)
        else
            menuSystem.printLineValue("Scaled radial", controllerDeadzoneMode ~= controllerOldDeadzoneMode)
        end
        menuSystem.printLine("Triggers mode:", controllerSelected.id)
        if controllerTriggerMode == Gcp.TRIGGER_DIGITAL then
            menuSystem.printLineValue("Digital", controllerTriggerMode ~= controllerOldTriggerMode)
        else
            menuSystem.printLineValue("Analog", controllerTriggerMode ~= controllerOldTriggerMode)
        end
        menuSystem.printLine("Rumble intensity:", controllerSelected.id)
        menuSystem.printLineValue(controllerRumbleIntensity, controllerRumbleIntensity ~= controllerOldRumbleIntensity)
        menuSystem.printLine("Save config", controllerSelected.id)
        menuSystem.printLine("Firmware update", controllerSelected.id)
    end
end

function handleButtonsTester()
    local held = Pad.genheld(0)

    if held.TRIGGER_R and held.TRIGGER_L then
        runningButtonsTester = false
    end
end

function handleSticksWizard()
    local STICKS_THRESHOLD = 30
    local sticks = {Pad.stick(0).x, Pad.stick(0).y, Pad.subStick(0).x, Pad.subStick(0).y}
    local down = Pad.gendown(0)

    if down.BUTTON_B then
        --Restore old sticks config
        Gcp.setSticksRange(oldSticksRange)
        Gcp.setSticksInvert(oldSticksInvert)
        Gcp.setSticksChannel(oldSticksChannel)
        Gcp.rebuildLUT()
        runningSticksWizard = false
    end

    --Check ranges
    for i = 1, 4 do
        if sticks[i] > sticksMax[i] then
            sticksMax[i] = sticks[i]
        end
        if sticks[i] < sticksMin[i] then
            sticksMin[i] = sticks[i]
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

    if sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_MOVE then
        --Check if a stick has moved
        if math.abs(maxStick) > STICKS_THRESHOLD then
            sticksChannel.leftX = maxIdx
            if maxStick < 0 then
                sticksInvert.leftX = true
            end
            sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_ROT1
        end
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT1 then
        --Check if the stick has been rotated
        local angle = 0
        if sticksInvert.leftX then
            angle = math.atan(maxStick, -sticks[sticksChannel.leftX])
        else
            angle = math.atan(maxStick, sticks[sticksChannel.leftX])
        end
        if ((math.abs(angle) > math.pi / 3) and maxIdx ~= sticksChannel.leftX) then
            sticksChannel.leftY = maxIdx
            if angle < 0 then
                sticksInvert.leftY = true
            end
            sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_ROT2
        end
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT2 then
        --Wait until the stick is on the left
        local tempX = 0
        local tempY = 0
        if sticksInvert.leftX then
            tempX = -sticks[sticksChannel.leftX]
        else
            tempX = sticks[sticksChannel.leftX]
        end
        if sticksInvert.leftY then
            tempY = -sticks[sticksChannel.leftY]
        else
            tempY = sticks[sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) > 5 * math.pi / 6 then
            sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_ROT3
        end
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT3 then
        --Wait until the stick is on the bottom left
        local tempX = 0
        local tempY = 0
        if sticksInvert.leftX then
            tempX = -sticks[sticksChannel.leftX]
        else
            tempX = sticks[sticksChannel.leftX]
        end
        if sticksInvert.leftY then
            tempY = -sticks[sticksChannel.leftY]
        else
            tempY = sticks[sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if angle < - math.pi / 3 then
            sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_ROT4
        end
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_ROT4 then
        --Wait until the stick is on the right
        local tempX = 0
        local tempY = 0
        if sticksInvert.leftX then
            tempX = -sticks[sticksChannel.leftX]
        else
            tempX = sticks[sticksChannel.leftX]
        end
        if sticksInvert.leftY then
            tempY = -sticks[sticksChannel.leftY]
        else
            tempY = sticks[sticksChannel.leftY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) < 0.1 then
            sticksWizardState = sticksWizardStateEnum.LS_WAIT_FOR_RELEASE
        end
    elseif sticksWizardState == sticksWizardStateEnum.LS_WAIT_FOR_RELEASE then
        --Wait for the stick to be released
        if math.sqrt(sticks[sticksChannel.leftX] * sticks[sticksChannel.leftX] + sticks[sticksChannel.leftY] * sticks[sticksChannel.leftY]) < STICKS_THRESHOLD then
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_MOVE
            Anim.reset(sticksWizardAnimation)
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_MOVE then
        --Check if a stick has moved
        if math.abs(maxStick) > STICKS_THRESHOLD then
            sticksChannel.rightX = maxIdx
            if maxStick < 0 then
                sticksInvert.rightX = true
            end
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_ROT1
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT1 then
        --Check if the stick has been rotated
        local angle = 0
        if sticksInvert.rightX then
            angle = math.atan(maxStick, -sticks[sticksChannel.rightX])
        else
            angle = math.atan(maxStick, sticks[sticksChannel.rightX])
        end
        if ((math.abs(angle) > math.pi / 3) and maxIdx ~= sticksChannel.rightX) then
            sticksChannel.rightY = maxIdx
            if angle < 0 then
                sticksInvert.rightY = true
            end
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_ROT2
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT2 then
        --Wait until the stick is on the left
        local tempX = 0
        local tempY = 0
        if sticksInvert.rightX then
            tempX = -sticks[sticksChannel.rightX]
        else
            tempX = sticks[sticksChannel.rightX]
        end
        if sticksInvert.rightY then
            tempY = -sticks[sticksChannel.rightY]
        else
            tempY = sticks[sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) > 5 * math.pi / 6 then
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_ROT3
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT3 then
        --Wait until the stick is on the bottom left
        local tempX = 0
        local tempY = 0
        if sticksInvert.rightX then
            tempX = -sticks[sticksChannel.rightX]
        else
            tempX = sticks[sticksChannel.rightX]
        end
        if sticksInvert.rightY then
            tempY = -sticks[sticksChannel.rightY]
        else
            tempY = sticks[sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if angle < - math.pi / 3 then
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_ROT4
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_ROT4 then
        --Wait until the stick is on the right
        local tempX = 0
        local tempY = 0
        if sticksInvert.rightX then
            tempX = -sticks[sticksChannel.rightX]
        else
            tempX = sticks[sticksChannel.rightX]
        end
        if sticksInvert.rightY then
            tempY = -sticks[sticksChannel.rightY]
        else
            tempY = sticks[sticksChannel.rightY]
        end
        local angle = math.atan(tempX, tempY)

        if math.abs(angle) < 0.1 then
            sticksWizardState = sticksWizardStateEnum.CS_WAIT_FOR_RELEASE
        end
    elseif sticksWizardState == sticksWizardStateEnum.CS_WAIT_FOR_RELEASE then
        --Wait for the stick to be released
        if math.sqrt(sticks[sticksChannel.rightX] * sticks[sticksChannel.rightX] + sticks[sticksChannel.rightY] * sticks[sticksChannel.rightY]) < STICKS_THRESHOLD then
            sticksWizardState = sticksWizardStateEnum.STICKS_TEST
            if sticksInvert.leftX then
                local temp = -sticksMax[sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMinX = math.floor(temp + 0.5)
                temp = -sticksMin[sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMaxX = math.floor(temp + 0.5)
            else
                local temp = sticksMin[sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMinX = math.floor(temp + 0.5)
                temp = sticksMax[sticksChannel.leftX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMaxX = math.floor(temp + 0.5)
            end
            if sticksInvert.leftY then
                local temp = -sticksMax[sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMinY = math.floor(temp + 0.5)
                temp = -sticksMin[sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMaxY = math.floor(temp + 0.5)
            else
                local temp = sticksMin[sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMinY = math.floor(temp + 0.5)
                temp = sticksMax[sticksChannel.leftY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.leftMaxY = math.floor(temp + 0.5)
            end
            if sticksInvert.rightX then
                local temp = -sticksMax[sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMinX = math.floor(temp + 0.5)
                temp = -sticksMin[sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMaxX = math.floor(temp + 0.5)
            else
                local temp = sticksMin[sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMinX = math.floor(temp + 0.5)
                temp = sticksMax[sticksChannel.rightX] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMaxX = math.floor(temp + 0.5)
            end
            if sticksInvert.rightY then
                local temp = -sticksMax[sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMinY = math.floor(temp + 0.5)
                temp = -sticksMin[sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMaxY = math.floor(temp + 0.5)
            else
                local temp = sticksMin[sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMinY = math.floor(temp + 0.5)
                temp = sticksMax[sticksChannel.rightY] + 128
                if temp < 0 then
                    temp = 0
                elseif temp > 255 then
                    temp = 255
                end
                sticksRange.rightMaxY = math.floor(temp + 0.5)
            end
            Gcp.setSticksRange(sticksRange)
            Gcp.setSticksInvert(sticksInvert)
            Gcp.setSticksChannel(sticksChannel)
            Gcp.rebuildLUT()
            Anim.reset(sticksWizardAnimation)
        end
    elseif sticksWizardState == sticksWizardStateEnum.STICKS_TEST then
        if down.BUTTON_A then
            --Go back
            runningSticksWizard = false
        end
    end
end

function handleController(onFocus)
    if Gcp.isUpdating() or (Time.getms() - gcpUpdateTime) < gcpUpdateTimeout then
        return
    end

    if runningButtonsTester then
        topBarDisableWheel()
        handleButtonsTester()
        return
    end

    if runningSticksWizard then
        topBarDisableWheel()
        handleSticksWizard()
        return
    end

    topBarEnableWheel()

    local down = Pad.gendown(0)
    local curId = controllerSelected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < controllerSelectedEnum.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if controllerSelected == controllerSelectedEnum.buttonsTester then
        if down.BUTTON_A then
            runningButtonsTester = true
        end
    elseif controllerSelected == controllerSelectedEnum.sticksWizard then
        if down.BUTTON_A then
            resetSticksWizard()
            runningSticksWizard = true
        end
    elseif controllerSelected == controllerSelectedEnum.sticksDeadzone then
        if down.BUTTON_LEFT and controllerDeadzone > 0 then
            controllerDeadzone = controllerDeadzone - 1
        end
        if down.BUTTON_RIGHT and controllerDeadzone < 60 then
            controllerDeadzone = controllerDeadzone + 1
        end
    elseif controllerSelected == controllerSelectedEnum.sticksDeadzoneMode then
        if down.BUTTON_LEFT or down.BUTTON_RIGHT then
            controllerDeadzoneMode = 1 - controllerDeadzoneMode
        end
    elseif controllerSelected == controllerSelectedEnum.triggersMode then
        if down.BUTTON_LEFT or down.BUTTON_RIGHT then
            controllerTriggerMode = 1 - controllerTriggerMode
        end
    elseif controllerSelected == controllerSelectedEnum.rumble then
        if down.BUTTON_LEFT and controllerRumbleIntensity > 0 then
            controllerRumbleIntensity = controllerRumbleIntensity - 1
        end
        if down.BUTTON_RIGHT and controllerRumbleIntensity < 127 then
            controllerRumbleIntensity = controllerRumbleIntensity + 1
        end
    elseif controllerSelected == controllerSelectedEnum.saveConfig then
        if down.BUTTON_A then
            Gcp.setSticksDeadzoneRadius(controllerDeadzone)
            Gcp.setSticksDeadzoneMode(controllerDeadzoneMode)
            Gcp.setRumbleIntensity(controllerRumbleIntensity)
            Gcp.setTriggerMode(controllerTriggerMode)
            controllerOldDeadzone = controllerDeadzone
            controllerOldDeadzoneMode = controllerDeadzoneMode
            controllerOldRumbleIntensity = controllerRumbleIntensity
            controllerOldTriggerMode = controllerTriggerMode
        end
    elseif controllerSelected == controllerSelectedEnum.firmwareUpdate then
        if down.BUTTON_A then
            topBarDisableWheel()
            gcpOldVersion = Gcp.getFWVer()
            Gcp.startUpdate("/GCPlus2.0Update.hex")
        end
    end

    controllerSelected = controllerSelectedEnum[curId]
end
