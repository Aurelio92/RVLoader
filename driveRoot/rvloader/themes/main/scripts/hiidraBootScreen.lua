require 'scripts/enum'
require 'scripts/class'
require 'scripts/topbarcmd'
require 'scripts/menuSystem'

menuSystem = MenuSystem()

function init()
    fonts = {}
    fonts[16] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 16)
    fonts[20] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 20)

    hiidraLogo = Gfx.loadImage("assets/hiidraLogo.png")
    hiidraLogoWidth, hiidraLogoHeight = Gfx.getImageSize(hiidraLogo)
end

function draw(onFocus)
    local lineHeight = 20
    local tempY = 0
    topBarSetText("Booting Hiidra")
    Gfx.drawImage(hiidraLogo, getDimensions()[1] - hiidraLogoWidth - 16, (getDimensions()[2] - hiidraLogoHeight) // 2)
    Gfx.pushMatrix()
    logLines = Hiidra.getLogLines()

    --Scroll if needed
    if lineHeight * (#logLines) > getDimensions()[2] then
        Gfx.translate(0, getDimensions()[2] - lineHeight * (#logLines))
    end

    for i = 1, #logLines do
        Gfx.print(fonts[16], 16, tempY, logLines[i])
        tempY = tempY + lineHeight
    end
    Gfx.popMatrix()
end

function handleInputs(onFocus)

end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y - 40
    return {w, h}
end