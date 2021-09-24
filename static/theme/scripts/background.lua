function init()
    loaderBackgrounds = Theme.getBackgrounds()
    loaderCurBackground = Theme.getLoadedBackground()
    loaderCurBackgroundId = 1

    for i = 1, #loaderBackgrounds do
        Sys.debug("Background: " .. loaderBackgrounds[i])
        if loaderBackgrounds[i] == loaderCurBackground then
            loaderCurBackgroundId = i
        end
    end

    background = Gfx.loadImage("/rvloader/backgrounds/" .. loaderBackgrounds[loaderCurBackgroundId])
    Gfx.setImageSize(background, getDimensions()[1], getDimensions()[2])
end

function draw(onFocus)
    if background then
      Gfx.drawImage(background)
    end
end

function handleInputs(onFocus)

end

function getDimensions()
    local w = Gui.getScreenSize().x
    local h = Gui.getScreenSize().y
    return {w, h}
end
