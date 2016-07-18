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
    processor = (CyclopsProcessor*)parentNode;
    tabText = "Cyclops";

    if (processor->getProcessorCount() == 1){
        // surely no canvas has been created. So create one now:
        connectedCanvas = CyclopsCanvas::canvasList.add(new CyclopsCanvas());
    }
    else{
        connectedCanvas = CyclopsCanvas::canvasList.getFirst();
    }
    jassert(connectedCanvas != nullptr);
    connectedCanvas->addListener(this);

    canvasCombo = new ComboBox();
    canvasCombo->addListener(this);
    canvasCombo->setTooltip("Select the Cyclops Board which would own this \"hook\".");
    StringArray canvasOptions;
    prepareCanvasComboList(canvasOptions);
    canvasCombo->addItemList(canvasOptions, 1);
    canvasCombo->addSeparator();
    canvasCombo->addItem("New", -1);
    canvasCombo->setSelectedItemIndex(1);
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

    // communicate with teensy.
}

CyclopsEditor::~CyclopsEditor()
{
    //std::cout<<"deleting cledit" << CyclopsProcessor::getProcessorCount() <<std::endl;
    connectedCanvas->removeListener(this);
    if (CyclopsProcessor::getProcessorCount() == 0){
        CyclopsCanvas::canvasList.clear(true);
    }
    if (connectedCanvas->dataWindow != nullptr)
        VisualizerEditor::removeWindowListener(connectedCanvas->dataWindow, this);
}


Visualizer* CyclopsEditor::createNewCanvas()
{
    return new CyclopsCanvas();
}


// This method is used to open the visualizer in a tab or window; override with caution
void CyclopsEditor::buttonClicked(Button* button)
{
    // To handle default buttons, like the Channel Selector Drawer.
    GenericEditor::buttonClicked(button);

    // Handle the buttons to open the canvas in a tab or window
    jassert (connectedCanvas != nullptr);
    //std::cout << connectedCanvas->tabIndex << std::endl;
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
            connectedCanvas->tabIndex = addTab(tabText, connectedCanvas);
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
        int selection = canvasCombo->getSelectedId();
        if (selection > 0 && selection < canvasCombo->getNumItems()){
            connectedCanvas->removeListener(this);
            connectedCanvas = CyclopsCanvas::canvasList.getUnchecked(selection-1);
            connectedCanvas->addListener(this);
        }
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
    // Disable the whole gui
    canvasCombo->setEnabled(false);
}

void CyclopsEditor::enableAllInputWidgets()
{
    // Reenable the whole gui
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

void CyclopsEditor::prepareCanvasComboList(StringArray& canvasOptions)
{
    canvasOptions.clear();
    for (int i=0; i<CyclopsCanvas::canvasList.size(); i++)
        canvasOptions.add("Cyclops "+String(i));
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

void IndicatorLED::update(const Colour& fill, String& tooltip)
{
    fillColour = fill;
    setTooltip(tooltip);
}

void IndicatorLED::update(const Colour& fill, const Colour& line, String& tooltip)
{
    fillColour = fill;
    lineColour = line;
    setTooltip(tooltip);
}

} // NAMESPACE cyclops
