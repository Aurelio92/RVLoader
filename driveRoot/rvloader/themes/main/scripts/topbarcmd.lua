TOPBAR_CMD_SETTEXT      = 1
TOPBAR_CMD_DISABLEWHEEL = 2
TOPBAR_CMD_ENABLEWHEEL  = 3

function topBarSetText(txt)
    local msg = {cmd = TOPBAR_CMD_SETTEXT, text = txt}
    Theme.sendMessage("TopBar", msg)
end

function topBarDisableWheel()
    local msg = {cmd = TOPBAR_CMD_DISABLEWHEEL}
    Theme.sendMessage("TopBar", msg)
end

function topBarEnableWheel()
    local msg = {cmd = TOPBAR_CMD_ENABLEWHEEL}
    Theme.sendMessage("TopBar", msg)
end