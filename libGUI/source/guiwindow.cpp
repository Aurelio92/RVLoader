#include <string>
#include "libgui.h"
#include "gfx.h"

GuiWindow::~GuiWindow() {
    elements.clear();
    elementsOrder.clear();
    elementsTable.clear();
}

void GuiWindow::draw(bool onFocus) {
    Gfx::pushMatrix();
    Gfx::pushScissorBox(width, height);
    Gfx::translate(scroll.x, scroll.y);
    drawRectangle(0, 0, width, height, color);
    for (auto& it : elementsOrder) {
        WinElement* el = it.second;
        if (el->active) {
            Vector2 dim = el->element->getDimensions();
            Gfx::pushMatrix();
            Gfx::translate(el->posX, el->posY);
            Gfx::pushScissorBox(dim.x, dim.y);
            el->element->draw(onFocus && (elementOnFocus == el));
            Gfx::popScissorBox();
            Gfx::popMatrix();
        }
    }
    Gfx::popScissorBox();
    Gfx::popMatrix();
}

void GuiWindow::handleInputs(bool onFocus) {
    for (auto& it : elements) {
        if (it.second.active)
            it.first->handleInputs((onFocus && elementOnFocus == &it.second) || it.second.superFocus);
    }
}

void GuiWindow::loseFocus(Vector2 dir) {
    WinElement* closestElement = NULL;
    int minDistance = 0;

    if (elementOnFocus == NULL)
        return;

    //Apply some transformation useful later
    //Right: (1, 0) -> (2, 1)
    //Left: (-1, 0) -> (0, 1)
    //Up: (0, -1) -> (1, 0)
    //Down: (0, 1) -> (1, 2)
    Vector2 focusDir(dir.x + 1, dir.y + 1);
    Vector2 invDir(2 - focusDir.x, 2 - focusDir.y);

    Vector2 focusRefPoint = Vector2(elementOnFocus->posX, elementOnFocus->posY) + focusDir * elementOnFocus->element->getDimensions() / 2;

    for (auto& it : elementsOrder) {
        WinElement* el = it.second;
        if (el == elementOnFocus)
            continue;
        if (!el->active)
            continue;
        if (el->superFocus)
            continue;

        //Vector2 targetRefPoint = Vector2(el->posX, el->posY) + invDir * el->element->getDimensions() / 2;
        Vector2 targetRefPoint = Vector2(el->posX, el->posY) + el->element->getDimensions() / 2; //Center point

        //Check that the element is actually
        //on the 'dir' side of the currently focused element
        Vector2 diffVec = (targetRefPoint - focusRefPoint) * dir;
        if (diffVec.x < 0 || diffVec.y < 0)
            continue;

        int tempDistance = diffVec.sqrMagnitude();
        if ((closestElement == NULL) || (tempDistance < minDistance)) {
            closestElement = el;
            minDistance = tempDistance;
        }
    }

    if (closestElement != NULL)
        elementOnFocus = closestElement;
}

void GuiWindow::addElement(GuiElement* el, int posX, int posY) {
    addElement(el, posX, posY, true, "");
}

void GuiWindow::addElement(GuiElement* el, int posX, int posY, bool active) {
    addElement(el, posX, posY, active, "");
}

void GuiWindow::addElement(GuiElement* el, int posX, int posY, bool active, std::string id) {
    if (el != NULL) {
        //Set element parent window
        el->parentElement = this;

        elements.insert(std::pair<GuiElement*, WinElement>(el, WinElement(el, posX, posY, active)));
        elementsOrder.insert(std::pair<u32, WinElement*>(elements.size(), &(elements[el])));

        if (id.length() > 0) {
            elementsTable.insert(std::pair<std::string, WinElement*>(id, &(elements[el])));
        }
    }
}

void GuiWindow::setElementPosition(GuiElement* el, int posX, int posY) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        it->second.posX = posX;
        it->second.posY = posY;
    }
}

void GuiWindow::setElementActive(GuiElement* el, bool active) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        it->second.active = active;
        if (active) {
            el->onActiveEvent();
        } else {
            el->onInactiveEvent();
        }
    }
}

void GuiWindow::setElementSuperFocus(GuiElement* el, bool superFocus) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        it->second.superFocus = superFocus;
    }
}

void GuiWindow::setElementSwitchable(GuiElement* el, bool switchable) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        it->second.switchable = switchable;
    }
}

bool GuiWindow::switchToElement(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        if (!it->second.switchable)
            return false;

        //Deactivate all the other switchable elements
        for (auto& it : elementsOrder) {
            WinElement* el = it.second;
            if (el->active && el->switchable) {
                el->active = false;
                el->element->onInactiveEvent();
            }
        }

        //Activate new element
        it->second.active = true;
        el->onActiveEvent();

        //Give focus to new element
        if (elementOnFocus)
            elementOnFocus->element->onDefocusEvent();
        elementOnFocus = &it->second;
        el->onFocusEvent();

        return true;
    }

    return false;
}

bool GuiWindow::switchToElement(std::string id) {
    std::unordered_map<std::string, WinElement*>::iterator it;
    it = elementsTable.find(id);

    if (it != elementsTable.end()) {
        return switchToElement(it->second->element);
    }

    return false;
}

void GuiWindow::focusOnElement(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        if (elementOnFocus)
            elementOnFocus->element->onDefocusEvent();
        elementOnFocus = &it->second;
        el->onFocusEvent();
    }
}

void GuiWindow::bringElementForward(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        //Search for WinElement
        for (auto& itOrder : elementsOrder) {
            if (itOrder.second == &(it->second)) { //Element found
                if (itOrder.first < elementsOrder.size() - 1) { //Make sure it's not the top element already
                    //Do the swap
                    WinElement* winElTemp = itOrder.second;
                    elementsOrder[itOrder.first] = elementsOrder[itOrder.first + 1];
                    elementsOrder[itOrder.first + 1] = winElTemp;
                }
                break;
            }
        }
    }
}

void GuiWindow::bringElementToFront(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        //Search for WinElement
        for (auto& itOrder : elementsOrder) {
            if (itOrder.second == &(it->second)) { //Element found
                if (itOrder.first < elementsOrder.size() - 1) { //Make sure it's not the top element already
                    //Do the swap
                    WinElement* winElTemp = itOrder.second;
                    for (u32 i = itOrder.first; i < elementsOrder.size() - 1; i++)
                        elementsOrder[i] = elementsOrder[i + 1];
                    elementsOrder[elementsOrder.size() - 1] = winElTemp;
                }
                break;
            }
        }
    }
}

void GuiWindow::pushElementBackward(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        //Search for WinElement
        for (auto& itOrder : elementsOrder) {
            if (itOrder.second == &(it->second)) { //Element found
                if (itOrder.first > 0) { //Make sure it's not the back element already
                    //Do the swap
                    WinElement* winElTemp = itOrder.second;
                    elementsOrder[itOrder.first] = elementsOrder[itOrder.first - 1];
                    elementsOrder[itOrder.first - 1] = winElTemp;
                }
                break;
            }
        }
    }
}

void GuiWindow::pushElementToBack(GuiElement* el) {
    std::map<GuiElement*, WinElement>::iterator it;
    it = elements.find(el);

    if (it != elements.end()) {
        //Search for WinElement
        for (auto& itOrder : elementsOrder) {
            if (itOrder.second == &(it->second)) { //Element found
                if (itOrder.first > 0) { //Make sure it's not the back element already
                    //Do the swap
                    WinElement* winElTemp = itOrder.second;
                    for (u32 i = itOrder.first; i > 0; i++)
                        elementsOrder[i] = elementsOrder[i - 1];
                    elementsOrder[0] = winElTemp;
                }
                break;
            }
        }
    }
}

GuiWindow& GuiWindow::operator = (const GuiWindow& l) {
    if (this == &l) { //Copying itself?
        return *this;
    }

    this->elements = l.elements;
    this->elementsOrder = l.elementsOrder;

    return *this;
}

void GuiWindow::onFocusEvent() {
    //Propagate the event
    if (elementOnFocus)
        elementOnFocus->element->onFocusEvent();
}

void GuiWindow::onDefocusEvent() {
    //Propagate the event
    if (elementOnFocus)
        elementOnFocus->element->onFocusEvent();
}

void GuiWindow::onActiveEvent() {
    //Propagate the event
    for (auto& it : elementsOrder) {
        WinElement* el = it.second;
        if (el->active) {
            el->element->onActiveEvent();
        }
    }
}

void GuiWindow::onInactiveEvent() {
    //Propagate the event
    for (auto& it : elementsOrder) {
        WinElement* el = it.second;
        if (el->active) {
            el->element->onInactiveEvent();
        }
    }
}

void GuiWindow::sendSignal(std::string receipientId, u32 signal) {
    //Propagate the signal
    std::unordered_map<std::string, WinElement*>::iterator it;
    it = elementsTable.find(receipientId);

    if (it != elementsTable.end()) {
        it->second->element->signalHandler(signal);
    }
}
