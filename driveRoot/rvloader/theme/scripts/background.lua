function init()
    loaderBackgrounds = Theme.getBackgrounds()
    loaderCurBackground = Theme.getLoadedBackground()
    loaderCurBackgroundId = 1

    --Return if no backgrounds were found
    if #loaderBackgrounds == 0 then
        return
    end

    if loaderCurBackground == "None" then
        return
    else
        for i = 1, #loaderBackgrounds do
            Sys.debug("Background: " .. loaderBackgrounds[i] .. "\n")
            if loaderBackgrounds[i] == loaderCurBackground then
                loaderCurBackgroundId = i
            end
        end
    end

    background = Gfx.loadImage("/rvloader/backgrounds/" .. loaderBackgrounds[loaderCurBackgroundId])
    Gfx.resizeImage(background, getDimensions()[1], getDimensions()[2])
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
