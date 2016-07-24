/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CyclopsEditor.h"

namespace cyclops {

CyclopsEditor::CyclopsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : VisualizerEditor   (parentNode, 240, useDefaultParameterEditors)
    //, progress(0, 1.0, 1000)
    , serialLED(new IndicatorLED(CyclopsColours::disconnected, Colours::black))
    , readinessLED(new IndicatorLED(CyclopsColours::notReady, Colours::black))
{
    processor = static_cast<CyclopsProcessor*>(parentNode);
    if (processor->getProcessorCount() == 1){
        // surely no canvas has been created. So create one now:
        connectedCanvas = CyclopsCanvas::canvasList.add(new CyclopsCanvas());
        connectedCanvas->setRealIndex(0); // must be called whenever new Canvas is made
    }
    else{
        // connect this editor to the "first" canvas BY DEFAULT.
        // Editors must always be connected to a canvas.
        // BUT canvas may not have any listeners.
        connectedCanvas = CyclopsCanvas::canvasList.getFirst();
        updateSelectorButtons();
    }
    jassert(connectedCanvas != nullptr);
    // listen to events from canvas
    connectedCanvas->addListener(this);

    // add GUI elements
    canvasCombo = new ComboBox();
    canvasCombo->addListener(this);
    canvasCombo->setTooltip("Select the Cyclops Board which would own this \"hook\".");
    prepareCanvasComboList(canvasCombo);
    canvasCombo->setSelectedId(1, dontSendNotification);
    canvasCombo->setBounds((240-85)*5/6, 30, 90, 25);
    addAndMakeVisible(canvasCombo);

    comboText = new Label("combo label", "Select Device:");
    comboText->setBounds(3,27,200,30);
    comboText->setFont(Font("Default", 16, Font::plain));
    comboText->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(comboText);

    // Add LEDs
    serialLED->setBounds(169, 6, 12, 12);
    readinessLED->setBounds(183, 6, 12, 12);
    readinessLED->setTooltip("This editor has not been completely configured with a sub-plugin");
    addAndMakeVisible(serialLED);
    addAndMakeVisible(readinessLED);

    connectedCanvas->refresh();
    // communicate with teensy.
}

CyclopsEditor::~CyclopsEditor()
{
    //std::cout<<"deleting cl_editor" << std::endl;
    connectedCanvas->removeListener(this);
    connectedCanvas->refresh();
    if (connectedCanvas->getNumListeners() > 0){
        tabIndex = -1; // to avoid destruction of tab
    }
    else{
        tabIndex = connectedCanvas->tabIndex;
    }
    if (CyclopsProcessor::getProcessorCount() == 0){
        // This is the last CyclopsEditor to be removed, remove all canvases.
        // Needed for garbage collection.
        CyclopsCanvas::canvasList.clear(true);
    }
    if (connectedCanvas->dataWindow != nullptr)
        VisualizerEditor::removeWindowListener(connectedCanvas->dataWindow, this);
}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * @                                           @
 * @    THIS METHOD SHOULD NEVER BE CALLED!!   @
 * @                                           @
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */
// we never need this method. That's why there's no setRealIndex() method around
Visualizer* CyclopsEditor::createNewCanvas()
{
    // VisualizerEditor calls this in buttonClicked() If someone changes
    // VisualizerEditor, such that this is called from elsewhere, then Cyclops
    // Plugin WILL FAIL
    //
    // (Failure is desired)
    //
    // Is this a sign of very bad coupling? It's not if VisualizerEditor is not
    // edited often.
    CyclopsCanvas* cc = nullptr;
    return cc;
}


// This method is used to open the visualizer in a tab or window; override with caution
void CyclopsEditor::buttonClicked(Button* button)
{
    // To handle default buttons, like the Channel Selector Drawer.
    GenericEditor::buttonClicked(button);

    // Handle the buttons to open the canvas in a tab or window
    jassert (connectedCanvas != nullptr);
    if (button == windowSelector) {
        //std::cout << "window:" << std::flush;
        if (connectedCanvas->tabIndex > -1){
            //std::cout << "tab open--" << std::flush;
            //AccessClass::getDataViewport()->destroyTab(tabIndex);
            removeTab(connectedCanvas->tabIndex);
            connectedCanvas->tabIndex = -1;
            //std::cout << "removed tab--" << std::flush;
        }
        // have we created a window already?
        if (connectedCanvas->dataWindow == nullptr) {
            //std::cout << "no win-exists--" << std::flush;
            makeNewWindow();
            // now pass ownership -- very ugly line...
            connectedCanvas->dataWindow = dataWindow;
            //std::cout << "made win--" << std::flush;
            connectedCanvas->dataWindow->setContentNonOwned(connectedCanvas, false);
            connectedCanvas->dataWindow->setName("Cyclops B" + String(connectedCanvas->realIndex));
            connectedCanvas->dataWindow->setVisible(true);
            connectedCanvas->broadcastButtonState(CanvasEvent::WINDOW_BUTTON, true);
            //std::cout << "show" << std::flush;
        }
        else {
            if (connectedCanvas->dataWindow->isVisible()){
                //std::cout << "win is open--" << std::flush;
                connectedCanvas->dataWindow->setVisible(false);
                connectedCanvas->dataWindow->setContentNonOwned(0, false);
                //std::cout << "hide, strip" << std::flush;
                connectedCanvas->broadcastButtonState(CanvasEvent::WINDOW_BUTTON, false);
            }
            else {
                //std::cout << "win exists--" << std::flush;
                connectedCanvas->dataWindow->setContentNonOwned(connectedCanvas, false);
                //std::cout << "re-set content--" << std::flush;
                connectedCanvas->setBounds(0,0,connectedCanvas->getParentWidth(), connectedCanvas->getParentHeight());
                connectedCanvas->dataWindow->setVisible(true);
                connectedCanvas->broadcastButtonState(CanvasEvent::WINDOW_BUTTON, true);
                //std::cout << "show" << std::flush;
            }
        }
        // listen to DataWindow::Listener::windowClosed
        VisualizerEditor::addWindowListener(connectedCanvas->dataWindow, this);
        connectedCanvas->broadcastButtonState(CanvasEvent::TAB_BUTTON, false);
    }
    else if (button == tabSelector) {
        //std::cout << "tab:" << std::flush;
        int canvas_tab_index = connectedCanvas->tabIndex;
        if (connectedCanvas->dataWindow != nullptr && connectedCanvas->dataWindow->isVisible()){
            //std::cout << "win is open--" << std::flush;
            connectedCanvas->dataWindow->setVisible(false);
            connectedCanvas->dataWindow->setContentNonOwned(0, false);
            //std::cout << "hide, strip--" << std::flush;
        }
        if (connectedCanvas->tabIndex > -1){
            //std::cout << "tab open--" << std::flush;
            if (connectedCanvas == getActiveTabContentComponent()){
                //AccessClass::getDataViewport()->destroyTab(tabIndex);
                removeTab(connectedCanvas->tabIndex);
                connectedCanvas->tabIndex = -1;
                connectedCanvas->broadcastButtonState(CanvasEvent::TAB_BUTTON, false);
                //std::cout << "removed tab" << std::flush;
            }
            else{
                // Tab is open but not visible, just switch to it
                setActiveTabId(canvas_tab_index);
                connectedCanvas->broadcastButtonState(CanvasEvent::TAB_BUTTON, true);
                //std::cout << "switching" << std::flush;
            }
        }
        else{
            //tabIndex = AccessClass::getDataViewport()->addTabToDataViewport(tabText, connectedCanvas, this);
            //std::cout << "add new tab--" << std::flush;
            connectedCanvas->tabIndex = addTab("Cyclops B"+String(connectedCanvas->realIndex), connectedCanvas);
            connectedCanvas->broadcastButtonState(CanvasEvent::TAB_BUTTON, true);
            //std::cout << "added!" << std::flush;
        }
        connectedCanvas->broadcastButtonState(CanvasEvent::WINDOW_BUTTON, false);
    }
    //std::cout << std::endl;
    //std::cout << connectedCanvas->tabIndex << std::endl;
    // Pass the button event along to "this" class.
    buttonEvent(button);
}

void CyclopsEditor::windowClosed()
{
    connectedCanvas->broadcastButtonState(CanvasEvent::WINDOW_BUTTON, false);
}

/**
The listener methods that reacts to the button click. The same method is called for all buttons
on the editor, so the button variable, which cointains a pointer to the button that called the method
has to be checked to know which function to perform.
*/
void CyclopsEditor::buttonEvent(Button* button)
{
    ;
}

void CyclopsEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == canvasCombo){
        CyclopsCanvas *oldCanvas = connectedCanvas, *newCanvas;
        int selection = canvasCombo->getSelectedId();

        if (selection > 0 && selection < canvasCombo->getNumItems()){
            // switch to another canvas, with all settings
            newCanvas = CyclopsCanvas::canvasList.getUnchecked(selection-1);
        }
        else{
            //std::cout << "create new canvas" <<std::endl;
            newCanvas = CyclopsCanvas::canvasList.add(new CyclopsCanvas());
            int num_canvases = prepareCanvasComboList(canvasCombo);
            newCanvas->setRealIndex(num_canvases-1); // must be called whenever new Canvas is made
            
            canvasCombo->setSelectedId(num_canvases, dontSendNotification);
            canvasCombo->repaint();
        }
        jassert (CyclopsCanvas::migrateEditor(newCanvas, oldCanvas, this) == 0);
        oldCanvas->removeListener(this);
        newCanvas->addListener(this);
        
        // obviously,
        connectedCanvas = newCanvas;
        updateSelectorButtons();
    }
}

void CyclopsEditor::timerCallback()
{
}

void CyclopsEditor::paint(Graphics& g)
{
    GenericEditor::paint(g);
}

void CyclopsEditor::disableAllInputWidgets()
{
    // Disable the whole editor
    canvasCombo->setEnabled(false);
}

void CyclopsEditor::enableAllInputWidgets()
{
    // Reenable the whole editor
    canvasCombo->setEnabled(true);
}

bool CyclopsEditor::isReady()
{
    return connectedCanvas->isReady();
}

void CyclopsEditor::startAcquisition()
{
    disableAllInputWidgets();
    GenericEditor::startAcquisition();
}

void CyclopsEditor::stopAcquisition()
{
    enableAllInputWidgets();
    GenericEditor::stopAcquisition();
}

void CyclopsEditor::updateSettings()
{
    ;
}

void CyclopsEditor::updateIndicators(CanvasEvent LEDtype)
{
    // read connectedCanvas state
    const cl_serial* serial = connectedCanvas->getSerialInfo();
    if (serial->portName == "")
        serialLED->update(CyclopsColours::disconnected, "Not Connected");
    else
        serialLED->update(CyclopsColours::connected, "Connected");
    serialLED->repaint();
    // update the LED
}

void CyclopsEditor::canvasClosing(CyclopsCanvas* newParentCanvas, CanvasEvent transferMode)
{
    connectedCanvas = newParentCanvas;
    connectedCanvas->addListener(this);
    switch (transferMode)
    {
        case CanvasEvent::TRANSFER_DROP:
        break;

        case CanvasEvent::TRANSFER_MIGRATE:
        break;

        default:
        break;
    }
}

void CyclopsEditor::changeCanvas(CyclopsCanvas* dest)
{
    connectedCanvas = dest;
    canvasCombo->setSelectedId(dest->realIndex+1, dontSendNotification); // don't invoke callback, because migration has been completed, the
                                                                         // the callback would re-migrate!
    updateSelectorButtons();
}

void CyclopsEditor::refreshPluginInfo()
{
    // connectedCanvas->getPluginInfoForHookID(hook_id);
}

void CyclopsEditor::updateButtons(CanvasEvent whichButton, bool state)
{
    switch (whichButton)
    {
    case CanvasEvent::WINDOW_BUTTON :
        windowSelector->setToggleState(state, dontSendNotification);
    break;

    case CanvasEvent::TAB_BUTTON:
        tabSelector->setToggleState(state, dontSendNotification);
    break;

    default:
    break;
    }
}

void CyclopsEditor::setInteractivity(CanvasEvent interactivity)
{
    switch (interactivity)
    {
    case CanvasEvent::FREEZE :
        disableAllInputWidgets();
    break;

    case CanvasEvent::THAW :
        enableAllInputWidgets();
    break;
    
    default:
    break;
    }
}

int CyclopsEditor::getEditorId()
{
    return nodeId;
}

void CyclopsEditor::saveEditorParameters(XmlElement* xmlNode)
{
    XmlElement* parameters = xmlNode->createNewChildElement("PARAMETERS");
    parameters->setAttribute("canvas_id", "Cyclops " + String(canvasCombo->getSelectedItemIndex()));
}

void CyclopsEditor::loadEditorParameters(XmlElement* xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode) {
        if (subNode->hasTagName("PARAMETERS")) {
            canvasCombo->setText(subNode->getStringAttribute("canvas_id", ""));
        }
    }
}

int CyclopsEditor::prepareCanvasComboList(ComboBox* combobox)
{
    combobox->clear(dontSendNotification);
    StringArray canvasOptions;
    int num_canvases = CyclopsCanvas::canvasList.size();
    for (int i=0; i<num_canvases; i++)
        canvasOptions.add("Cyclops "+String(i));
    combobox->addItemList(canvasOptions, 1);
    combobox->addSeparator();
    combobox->addItem("New", -1);
    return num_canvases;
}


void CyclopsEditor::updateSelectorButtons()
{
    if (connectedCanvas->tabIndex > 0)
        tabSelector->setToggleState(true, dontSendNotification);
    else
        tabSelector->setToggleState(false, dontSendNotification);
    if (connectedCanvas->dataWindow != nullptr && connectedCanvas->dataWindow->isVisible())
        windowSelector->setToggleState(true, dontSendNotification);
    else
        windowSelector->setToggleState(false, dontSendNotification);
}
/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 INDICATOR LEDS                                |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

IndicatorLED::IndicatorLED(const Colour& fill, const Colour& line)
{
    fillColour = fill;
    lineColour = line;
}

void IndicatorLED::paint(Graphics& g)
{
    g.setColour(fillColour);
    g.fillEllipse(1, 1, getWidth()-2, getHeight()-2);
    g.setColour(lineColour);
    g.drawEllipse(1, 1, getWidth()-2, getHeight()-2, 1.2);
}

void IndicatorLED::update(const Colour& fill, String tooltip)
{
    fillColour = fill;
    setTooltip(tooltip);
}

void IndicatorLED::update(const Colour& fill, const Colour& line, String tooltip)
{
    fillColour = fill;
    lineColour = line;
    setTooltip(tooltip);
}

} // NAMESPACE cyclops
