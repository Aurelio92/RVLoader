dofile("scripts/topbarcmd.lua")

function init()
    fonts = {}
    fonts[16] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 16)
    fonts[20] = Gfx.loadFont("assets/NotoSansJP-Regular.otf", 20)

    screensize = Gui.getScreenSize()
    if screensize.x == 640 then
        hbPerRow = 4
    else
        hbPerRow = 5
    end

    rowsScroll = 0
    rowsPerPage = 4
    coverWidth = 128
    coverHeight = 48
    selectorBorder = 3
    gridGap = 10
    selectedHB = 0
    prevSelectedHB = 0
    textScroll = 0
    scrollTextGap = 32
    scrollTextTimeRef = 0
    scrollTextTime = 50
    HBView.setCoverSize(coverWidth, coverHeight)

    hbCount = HBView.getHBCount()

    rowsCount = (hbCount + hbPerRow - 1) // hbPerRow

    button_a_icon = Gfx.loadImage("assets/button_a.png")

    SETTING_FONT_SIZE = 20
    SETTING_EL_HEIGHT = 32
    SETTINGS_SIDE_MARGIN = 16
end

function draw(onFocus)
    topBarSetText("Wii homebrews")

    Gfx.pushMatrix()
    local winWidth = hbPerRow * coverWidth + (hbPerRow - 1) * gridGap + 2 * selectorBorder
    local winHeight = rowsPerPage * (coverHeight + 28) + (rowsPerPage - 1) * gridGap + 2 * selectorBorder
    Gfx.translate((getDimensions()[1] - winWidth) / 2, 0)
    Gfx.pushScissorBox(winWidth, winHeight)
    Gfx.translate(0, -rowsScroll * (coverHeight + gridGap + 28))
    Gfx.translate(selectorBorder, selectorBorder)
    for i = 0, (hbCount - 1) do
        local x = (i % hbPerRow) * (coverWidth + gridGap)
        local y = (i // hbPerRow) * (coverHeight + gridGap + 28)
        if i == selectedHB and onFocus then
            Gfx.drawRectangle(x - selectorBorder, y - selectorBorder, coverWidth + 2 * selectorBorder, coverHeight + 2 * selectorBorder, Gfx.RGBA8(0x25, 0xcb, 0xf9, 0xff))
        end
        HBView.drawHBCover(x, y, i)

        if selectedHB ~= prevSelectedHB then
            scrollTextTimeRef = Time.getms()
            prevSelectedHB = selectedHB
            textScroll = 0
        end

        Gfx.pushMatrix()
        Gfx.translate(x, y + coverHeight + 8)
        Gfx.pushScissorBox(coverWidth, 20)
        if i == selectedHB and onFocus then
            Gfx.translate(-textScroll, 0)
            local textWidth = Gfx.print(fonts[16], 0, 0, HBView.getHBName(i))
            if textWidth > coverWidth then
                Gfx.print(fonts[16], textWidth + scrollTextGap, 0, HBView.getHBName(i))
                textScroll = (Time.getms() - scrollTextTimeRef) // scrollTextTime
                if textScroll > textWidth + scrollTextGap then
                    textScroll = textScroll - (textWidth + scrollTextGap)
                    scrollTextTimeRef = Time.getms() - textScroll * scrollTextTime
                end
            end
        else
            Gfx.print(fonts[16], 0, 0, HBView.getHBName(i))
        end
        Gfx.popScissorBox()
        Gfx.popMatrix()
    end
    Gfx.popScissorBox()
    Gfx.popMatrix()

    Gfx.pushMatrix()
    Gfx.translate(SETTINGS_SIDE_MARGIN, getDimensions()[2] - 32 - SETTINGS_SIDE_MARGIN)
    Gfx.pushScissorBox(getDimensions()[1] - SETTINGS_SIDE_MARGIN, 32)
    local oldAlignment = Gfx.getFontVerticalAlignment(fonts[SETTING_FONT_SIZE])
    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], Gfx.CENTER_ALIGN)
    local tempX = 0
    Gfx.drawImage(button_a_icon, tempX, 0)
    tempX = tempX + 40
    tempX = tempX + Gfx.print(fonts[SETTING_FONT_SIZE], tempX, SETTING_EL_HEIGHT / 2, "Boot homebrew")
    tempX = tempX + 16

    Gfx.setFontVerticalAlignment(fonts[SETTING_FONT_SIZE], oldAlignment)
    Gfx.popScissorBox()
    Gfx.popMatrix()
end

function handleInputs()
    local down = Pad.gendown(0)

    if down.BUTTON_RIGHT then
        selectedHB = selectedHB + 1
    elseif down.BUTTON_LEFT then
        selectedHB = selectedHB - 1
    elseif down.BUTTON_UP then
        selectedHB = selectedHB - hbPerRow
    elseif down.BUTTON_DOWN then
        selectedHB = selectedHB + hbPerRow
    end

    --Boundary checks
    if selectedHB < 0 then
        selectedHB = 0
    elseif selectedHB >= hbCount then
        selectedHB = hbCount - 1;
    end

    --Scroll
    if rowsCount > rowsPerPage then
        local curRow = (selectedHB // hbPerRow) - rowsScroll
        if curRow < (rowsPerPage // 2 - 1) and rowsScroll > 0 and down.BUTTON_UP then
            rowsScroll = rowsScroll - 1
        elseif curRow > (rowsPerPage // 2) and (rowsScroll + rowsPerPage) < rowsCount and down.BUTTON_DOWN then
            rowsScroll = rowsScroll + 1
        end
    end

    if down.BUTTON_A and hbCount > 0 then
        HBView.bootHB(selectedHB)
    end
end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y - 40
    return {w, h}
end

