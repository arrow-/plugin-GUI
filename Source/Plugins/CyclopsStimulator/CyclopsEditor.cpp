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
    }
    else{
        // connect this editor to the "first" canvas BY DEFAULT.
        // Editors must always be connected to a canvas.
        // BUT canvas may not have any listeners.
        connectedCanvas = CyclopsCanvas::canvasList.getFirst();
        updateSelectorButtons();
        updateIndicators(CanvasEvent::SERIAL_LED);
    }
    jassert(connectedCanvas != nullptr);
    // listen to events from canvas
    connectedCanvas->addListener(this);
    connectedCanvas->addHook(nodeId);

    // add GUI elements
    myID = new Label("my_id", String(nodeId));
    myID->setFont(Font("Default", 12, Font::plain));
    myID->setColour(Label::textColourId, Colours::black);
    myID->setBounds(3, 27, 60, 10);
    addAndMakeVisible(myID);
    
    canvasCombo = new ComboBox();
    canvasCombo->addListener(this);
    canvasCombo->setTooltip("Select the Cyclops Board which would own this \"hook\".");
    prepareCanvasComboList(canvasCombo);
    canvasCombo->setSelectedId(1, dontSendNotification);
    canvasCombo->setBounds((240-85)*5/6, 30+5, 90, 25);
    addAndMakeVisible(canvasCombo);

    comboText = new Label("combo label", "Select Device:");
    comboText->setBounds(3,27+5,200,30);
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
    //DBG ("deleting cl_editor\n");
    connectedCanvas->removeListener(this);
    connectedCanvas->removeHook(nodeId);
    connectedCanvas->refresh();
    if (connectedCanvas->getNumListeners() > 0){
        tabIndex = -1; // to avoid destruction of tab
    }
    else{
        tabIndex = connectedCanvas->tabIndex;
    }
    if (connectedCanvas->dataWindow != nullptr)
        VisualizerEditor::removeWindowListener(connectedCanvas->dataWindow, this);
    if (CyclopsProcessor::getProcessorCount() == 0){
        // This is the last CyclopsEditor to be removed, remove all canvases.
        // Needed for garbage collection.
        CyclopsCanvas::canvasList.clear(true);
    }
}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * @                                           @
 * @    THIS METHOD SHOULD NEVER BE CALLED!!   @
 * @                                           @
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */
// we never need this method.
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
            //DBG ("create new canvas\n");
            newCanvas = CyclopsCanvas::canvasList.add(new CyclopsCanvas());
            CyclopsCanvas::broadcastNewCanvas();
        }
        jassert (CyclopsCanvas::migrateEditor(newCanvas, oldCanvas, this) == 0);        
        // obviously,
        connectedCanvas = newCanvas;
        prepareCanvasComboList(canvasCombo);
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

void CyclopsEditor::isReadyForLaunch(bool& isOrphan, bool& isPrimed, int& genError, int& flashError)
/* Call Graph:
 * ProcessorGraph::enableProcessors() [loops through all processors]
 * CyclopsProcessor.isReady()
 * this()
 * CyclopsCanvas::getSummary(int, bool&)
 * |
 * +-- CyclopsCanvas::generateCode() [if getSummary() returns ``true``]
 *     |
 *     +-- CyclopsCanvas::flashDevice() [if generateCode() returns ``true``]
*/
{
    if (connectedCanvas == nullptr){
        isOrphan = true;
        isPrimed = false;
        genError = flashError = 0;
    }
    else{
        isOrphan = false;
        if (connectedCanvas->getSummary(nodeId, isPrimed)){
            if (connectedCanvas->generateCode(genError)){
                std::cout << "Generated\n";
                if (connectedCanvas->flashDevice(flashError)){
                    std::cout << "Flashed\n";
                }
                else{
                    std::cout << "Flash FAIL (code=" << flashError << ")\n";
                }
            }
            else{
                std::cout << "Generation FAIL (code=" << genError << ")\n";
                flashError = 0;
            }
        }
        else{
            genError = flashError = 0;
            DBG ("more editors waiting...\n");
        }
    }        
}

bool CyclopsEditor::isSerialConnected()
{
    const cl_serial* serial = connectedCanvas->getSerialInfo();
    return !(serial->portName == "");
}

void CyclopsEditor::startAcquisition()
{
    disableAllInputWidgets();
    connectedCanvas->beginAnimation();
    GenericEditor::startAcquisition();
}

void CyclopsEditor::stopAcquisition()
{
    enableAllInputWidgets();
    connectedCanvas->endAnimation();
    GenericEditor::stopAcquisition();
}

void CyclopsEditor::updateSettings()
{
    ;
}

void CyclopsEditor::updateIndicators(CanvasEvent event)
{
    // read connectedCanvas state
    if (event == CanvasEvent::TRANSFER_DROP){
        serialLED->update(Colours::black, "Invalid State");
        readinessLED->update(Colours::black, "Invalid State");
    }
    else if (event == CanvasEvent::SERIAL_LED){
        if (isSerialConnected())
            serialLED->update(CyclopsColours::connected, "Connected");
        else
            serialLED->update(CyclopsColours::disconnected, "Not Connected");
    }
    else if (event == CanvasEvent::PLUGIN_SELECTED){
        readinessLED->update(CyclopsColours::Ready, "Just fill the channel mux here.");
    }
    serialLED->repaint();
    readinessLED->repaint();
}

bool CyclopsEditor::channelMapStatus()
{
    return true;
}

CyclopsPluginInfo* CyclopsEditor::refreshPluginInfo()
{
    return connectedCanvas->getPluginInfoById(nodeId);
}

void CyclopsEditor::changeCanvas(CyclopsCanvas* dest)
{
    connectedCanvas = dest;
    if (connectedCanvas == nullptr){
        disableAllInputWidgets();
        tabSelector->setEnabled(false);
        windowSelector->setEnabled(false);
    }
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

    case CanvasEvent::COMBO_BUTTON:
        prepareCanvasComboList(canvasCombo);
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

cl_serial* CyclopsEditor::getSerial()
{
    return &(connectedCanvas->serialInfo);
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
    int num_canvases = CyclopsCanvas::canvasList.size(),
        selection = -1;
    for (int i=0; i<num_canvases; i++){
        canvasOptions.add("Cyclops "+String(CyclopsCanvas::canvasList[i]->realIndex));
        if (CyclopsCanvas::canvasList[i] == connectedCanvas)
            selection = i+1;
    }
    combobox->addItemList(canvasOptions, 1);
    combobox->addSeparator();
    combobox->addItem("New", -1);

    combobox->setSelectedId(selection, dontSendNotification);
    combobox->repaint();
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

} // NAMESPACE cyclops
