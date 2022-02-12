require 'scripts/enum'
require 'scripts/class'
require 'scripts/topbarcmd'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'

function fanSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.AUTO_INCREASE_DELAY = 500
    self.AUTO_INCREASE_TIME = 100
    self.autoIncreaseTime = 0

    self.calibrationRunning = false
    self.calibrationTimer = 0
    self.CALIBRATION_TIME = 2 * 60000
    self.calibrationLoop = 0
    self.CALIBRATION_LOOPS = 3
    self.calibrationStateEnum = enum({"heating", "stablizing", "done"})
    self.calibrationState = self.calibrationStateEnum[1]

    self.fankP = PMS2.getFanPIDkP()
    self.fankI = PMS2.getFanPIDkI()
    self.fankD = PMS2.getFanPIDkD()
    self.PIDTarget = PMS2.getFanPIDTarget()
    self.fanRange = PMS2.getFanRange()

    self.selectionEnum = enum({"fanMin", "fanMax", "PIDTarget", "PIDkP", "PIDkI", "PIDkD", "saveConfig", "runCalibration"})
    self.selected = self.selectionEnum[1]

    self.fanOldRange = PMS2.getFanRange()
    self.oldkP = self.fankP
    self.oldkI = self.fankI
    self.oldkD = self.fankD
    self.oldPIDTarget = self.PIDTarget
end

function drawFan(onFocus)
    if self.calibrationRunning then
        drawFanCalibration(onFocus)
        return
    end

    menuSystem:reset()
    --menuSystem:printLine("Speed:", self.selected.id)
    --menuSystem:printLineValue(string.format("%u", fanSpeed), fanSpeed ~= fanOldSpeed)
    menuSystem:printLine("Fan speed min:", self.selected.id)
    menuSystem:printLineValue(string.format("%u", self.fanRange.min), self.fanRange.min ~= self.fanOldRange.min)
    menuSystem:printLine("Fan speed max:", self.selected.id)
    menuSystem:printLineValue(string.format("%u", self.fanRange.max), self.fanRange.max ~= self.fanOldRange.max)
    menuSystem:printLine("Target temperature:", self.selected.id)
    menuSystem:printLineValue(string.format("%f", self.PIDTarget) .. " °C", self.PIDTarget ~= self.oldPIDTarget)
    menuSystem:printLine("PID kP:", self.selected.id)
    menuSystem:printLineValue(string.format("%.5f", self.fankP), self.fankP ~= self.oldkP)
    menuSystem:printLine("PID kI:", self.selected.id)
    menuSystem:printLineValue(string.format("%.5f", self.fankI), self.fankI ~= self.oldkI)
    menuSystem:printLine("PID kD:", self.selected.id)
    menuSystem:printLineValue(string.format("%.5f", self.fankD), self.fankD ~= self.oldkD)
    menuSystem:printLine("Save configuration", self.selected.id)
    menuSystem:printLine("Run calibration", self.selected.id)

    menuSystem:printLine("Fan speed:", -1)
    menuSystem:printLineValue(string.format("%u", PMS2.getFanSpeed()), false)
end

function handleFan(onFocus)
    if self.calibrationRunning then
        handleInputsFanCalibration(onFocus)
        return
    end

    local down = Pad.gendown(0)
    local held = Pad.genheld(0)

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

    if down.BUTTON_A then
        if self.selected == self.selectionEnum.saveConfig then
            PMS2.setFanPIDkP(self.fankP)
            PMS2.setFanPIDkI(self.fankI)
            PMS2.setFanPIDkD(self.fankD)
            PMS2.setFanRange(self.fanRange)
            PMS2.setFanPIDTarget(self.PIDTarget)

            self.fanOldRange = PMS2.getFanRange()
            self.oldkP = self.fankP
            self.oldkI = self.fankI
            self.oldkD = self.fankD
            self.oldPIDTarget = self.PIDTarget

            PMS2.flashConfig()
        elseif self.selected == self.selectionEnum.runCalibration then
            initFanCalibration()
        end
    elseif down.BUTTON_RIGHT or (held.BUTTON_RIGHT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.fanMin then
            self.fanRange.min = self.fanRange.min + 5
            if self.fanRange.min > 255 then
                self.fanRange.min = 255
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.fanMax then
            self.fanRange.max = self.fanRange.max + 5
            if self.fanRange.max > 255 then
                self.fanRange.max = 255
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.PIDTarget then
            self.PIDTarget = self.PIDTarget + 1
            if self.PIDTarget > 50 then
                self.PIDTarget = 50
            end
        elseif self.selected == self.selectionEnum.PIDkP then
            self.fankP = self.fankP + 0.2
            if self.fankP > 100 then
                self.fankP = 100
            end
        elseif self.selected == self.selectionEnum.PIDkI then
            self.fankI = self.fankI + 0.02
            if self.fankI > 100 then
                self.fankI = 100
            end
        elseif self.selected == self.selectionEnum.PIDkD then
            self.fankD = self.fankD + 0.02
            if self.fankD > 100 then
                self.fankD = 100
            end
        end
        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    elseif down.BUTTON_LEFT or (held.BUTTON_LEFT and Time.getms() > self.autoIncreaseTime) then
        if self.selected == self.selectionEnum.fanMin then
            self.fanRange.min = self.fanRange.min - 5
            if self.fanRange.min < 0 then
                self.fanRange.min = 0
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.fanMax then
            self.fanRange.max = self.fanRange.max - 5
            if self.fanRange.max < 130 then
                self.fanRange.max = 130
            end
            if self.fanRange.min > self.fanRange.max then
                self.fanRange.min = self.fanRange.max
            end
        elseif self.selected == self.selectionEnum.PIDTarget then
            self.PIDTarget = self.PIDTarget - 1
            if self.PIDTarget < 35 then
                self.PIDTarget = 35
            end
        elseif self.selected == self.selectionEnum.PIDkP then
            self.fankP = self.fankP - 0.2
            if self.fankP < 0 then
                self.fankP = 0
            end
        elseif self.selected == self.selectionEnum.PIDkI then
            self.fankI = self.fankI - 0.02
            if self.fankI < 0 then
                self.fankI = 0
            end
        elseif self.selected == self.selectionEnum.PIDkD then
            self.fankD = self.fankD - 0.02
            if self.fankD < 0 then
                self.fankD = 0
            end
        end

        if down.BUTTON_RIGHT then
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_DELAY
        else
            self.autoIncreaseTime = Time.getms() + self.AUTO_INCREASE_TIME
        end
    end

    self.selected = self.selectionEnum[curId]
end

function initFanCalibration()
    self.calibrationRunning = true
    self.calibrationState = self.calibrationStateEnum[1]
    PMS2.setFanSpeed(0)
    topBarDisableWheel()
end

function drawFanCalibration(onFocus)
    menuSystem:reset()
    menuSystem:printLine("Performing fan calibration", -1)
    menuSystem:printLine("This might take several minutes", -1)
    menuSystem:printLine("Fan might stop spinning multiple times", -1)
    menuSystem:printLine("Press B to cancel at any time", -1)
    menuSystem:printLine("Temperature:", -1)
    menuSystem:printLineValue(string.format("%.1f °C", PMS2.getNTC()), false)
    menuSystem:printLine("Fan speed:", -1)
    menuSystem:printLineValue(string.format("%u", PMS2.getFanSpeed()), false)
    menuSystem:printLine("Cur state:", -1)
    menuSystem:printLineValue(self.calibrationState.name, false)
end

function handleInputsFanCalibration(onFocus)
    local down = Pad.gendown(0)

    if self.calibrationState == self.calibrationStateEnum.heating then
        if PMS2.getNTC() > (self.PIDTarget + 5) then
            fanCalibrationMinTemp = self.PIDTarget
            fanCalibrationMaxTemp = 70

            local kP = (self.fanRange.max - self.fanRange.min) / ((fanCalibrationMaxTemp + fanCalibrationMinTemp) / 2 - self.PIDTarget)
            PMS2.setFanPIDkP(kP)
            PMS2.setFanPIDkI(0)
            PMS2.setFanPIDkD(0)
            PMS2.setFanRange(self.fanRange)
            PMS2.setFanPIDTarget(self.PIDTarget)
            PMS2.freeFan()
            self.calibrationTimer = Time.getms()
            self.calibrationLoop = 0
            self.calibrationState = self.calibrationStateEnum.stablizing
        end
    elseif self.calibrationState == self.calibrationStateEnum.stablizing then
        if (Time.getms() - self.calibrationTimer) > self.CALIBRATION_TIME then
            if PMS2.getNTC() < self.PIDTarget then --fan too fast
                fanCalibrationMinTemp = (fanCalibrationMaxTemp + fanCalibrationMinTemp) / 2
            else --fan not fast enough
                fanCalibrationMaxTemp = (fanCalibrationMaxTemp + fanCalibrationMinTemp) / 2
            end

            local kP = (self.fanRange.max - self.fanRange.min) / ((fanCalibrationMaxTemp + fanCalibrationMinTemp) / 2 - self.PIDTarget)
            PMS2.setFanPIDkP(kP)

            if self.calibrationLoop < self.CALIBRATION_LOOPS then
                self.calibrationLoop = self.calibrationLoop + 1
                self.calibrationTimer = Time.getms()
            else
                self.fanOldRange = PMS2.getFanRange()
                self.fankP = kP
                self.fankI = 0
                self.fankD = 0
                self.oldkP = self.fankP
                self.oldkI = self.fankI
                self.oldkD = self.fankD
                self.oldPIDTarget = self.PIDTarget

                PMS2.flashConfig()

                self.calibrationTimer = Time.getms()
                self.calibrationState = self.calibrationStateEnum.done
            end
        end
    elseif self.calibrationState == self.calibrationStateEnum.done then
        if (Time.getms() - self.calibrationTimer) > 1000 then
            self.calibrationRunning = false
            topBarEnableWheel()
        end
    end

    if down.BUTTON_B then
        self.calibrationRunning = false
        PMS2.setFanRange(self.fanOldRange)
        PMS2.setFanPIDkP(self.oldkP)
        PMS2.setFanPIDkI(self.oldkI)
        PMS2.setFanPIDkD(self.oldkD)
        PMS2.setFanPIDTarget(self.oldPIDTarget)
        PMS2.freeFan()
        topBarEnableWheel()
    end
end