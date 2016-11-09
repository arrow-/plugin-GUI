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

Image CyclopsEditor::normal;
Image CyclopsEditor::down;

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
        updateSerialIndicator(CanvasEvent::SERIAL);
    }
    jassert(connectedCanvas != nullptr);
    // listen to events from canvas
    connectedCanvas->addListener(this);
    connectedCanvas->addHook(nodeId);

    // add GUI elements    
    canvasCombo = new ComboBox();
    canvasCombo->addListener(this);
    canvasCombo->setTooltip("Select the Cyclops Board which would own this \"hook\".");
    prepareCanvasComboList(canvasCombo);
    canvasCombo->setSelectedId(1, dontSendNotification);
    canvasCombo->setBounds(142, 25, 90, 25);
    addAndMakeVisible(canvasCombo);

    comboText = new Label("combo label", "Select Device:");
    comboText->setBounds(17, 22, 200, 30);
    comboText->setFont(Font("Default", 16, Font::plain));
    comboText->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(comboText);

    cmapViewport = new Viewport();
    cmapViewport->setBounds(0, 0, 420, 300);
    cmapViewport->setScrollBarsShown(true, false);

    channelMapper = new ChannelMapperDisplay(this, connectedCanvas);
    cmapViewport->setViewedComponent(channelMapper, false);

    // Add button that opens the ChannelMapperWindow
    if (CyclopsEditor::normal.isNull()){
        CyclopsEditor::normal = Image(Image::PixelFormat::ARGB, 150, 20, true);
        CyclopsEditor::down = Image(Image::PixelFormat::ARGB, 150, 20, true);

        Graphics gn(CyclopsEditor::normal);
        gn.fillAll(Colour(0xff5e6619));
        gn.setColour(CyclopsColours::connected);
        gn.fillRect(0, 0, 148, 18);
        gn.setColour(Colours::black);
        gn.drawText("Map Channel->Slot", 0, 0, 150, 20, Justification::centred, false);
/*
        Graphics go(CyclopsEditor::over);
        go.fillAll(Colour(0xff994500));
        go.setColour(CyclopsColours::errorGenFlash);
        go.fillRect(0, 0, 148, 18);
        go.setColour(Colours::white);
        go.drawText("Map Channel->Slot", 0, 0, 150, 20, Justification::centred, false);
*/
        Graphics gd(CyclopsEditor::down);
        gd.fillAll(CyclopsColours::connected);
        gd.setColour(Colour(0xff5e6619));
        gd.fillRect(2, 2, 148, 18);
        gd.setColour(Colours::white);
        gd.drawText("Map Channel->Slot", 1, 1, 150, 20, Justification::centred, false);
    }
    mapperButton = new ImageButton("mapperButton");
    mapperButton->setImages(true, false, true,
                            CyclopsEditor::normal, 0.8, Colour(),
                            CyclopsEditor::normal, 1, Colour(),
                            CyclopsEditor::down, 1, Colour());
    mapperButton->setBounds(45, 60, 150, 20);
    mapperButton->addListener(this);
    addAndMakeVisible(mapperButton);

    /*
    mapperWindowLaunchOptions->dialogTitle = "Map Channels -> sub-plugin Slots";
    mapperWindowLaunchOptions->dialogBackgroundColour = Colours::yellow;
    mapperWindowLaunchOptions->componentToCentreAround = connectedCanvas;
    mapperWindowLaunchOptions->escapeKeyTriggersCloseButton = true;
    mapperWindowLaunchOptions->useNativeTitleBar = true;
    mapperWindowLaunchOptions->resizable = true;
    mapperWindowLaunchOptions->useBottomRightCornerResizer = true;
    mapperWindowLaunchOptions->content.setNonOwned(cmapViewport);
    */
    // Add LEDs
    serialLED->setBounds(169, 6, 12, 12);
    readinessLED->setBounds(183, 6, 12, 12);
    readinessLED->setTooltip("This editor has not been completely configured with a sub-plugin");
    addAndMakeVisible(serialLED);
    addAndMakeVisible(readinessLED);

    connectedCanvas->refresh();
    // communicate with teensy.
}

/**
 * @brief      Destroys the object.
 */
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
    else if (button == mapperButton && connectedCanvas != nullptr){
        mapperWindow = new SimpleWindow("Map Channels -> sub-plugin Slots", cmapViewport);
        mapperWindow->setContentNonOwned(cmapViewport, true);
        mapperWindow->setVisible(true);
    }
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
    if (getCollapsedState() == false){
        Font old = g.getCurrentFont();
        g.setFont(Font(12, Font::FontStyleFlags::bold));
        g.setColour(Colours::white);
        g.addTransform(AffineTransform::rotation(-M_PI/2.0, 3, 47));
        g.fillRoundedRectangle(3, 47, 22, 16, 3.5);
        g.setColour(Colours::black);
        g.drawText (String(nodeId), 5, 49, 20, 12, Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI/2.0, 3, 47));
        g.setFont(old);
    }/*
    if (cmapViewport->isVisible()){
        g.setColour(Colours::black);
        Font old = g.getCurrentFont();
        g.setFont(10);
        g.addTransform(AffineTransform::rotation(-M_PI/2.0, 3, 117));
        g.drawText ("OE-GUI ch", 3, 117, 80, 12, Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI/2.0, 3, 117));
        
        g.addTransform(AffineTransform::rotation(-M_PI/2.0, 225, 117));
        g.drawText ("plugin ch", 225, 117, 80, 12, Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI/2.0, 225, 117));
        g.setFont(old);
    }*/
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

void CyclopsEditor::isReadyForLaunch(bool& isOrphan, bool& isPrimed, int& genError, int & buildError, int& flashError)
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
                connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::CODE_GEN, 0);
                if (connectedCanvas->buildCode(buildError)){
                    std::cout << "Compiled\n";
                    connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::BUILD, 0);
                    if (connectedCanvas->flashDevice(flashError)){
                        std::cout << "Flashed\n";
                        connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::FLASH, 0);
                    }
                    else{
                        std::cout << "\n\t*CL:Code* Flash FAIL (code=" << flashError << ")\n";
                        connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::FLASH, 1);
                    }
                }
                else{
                    std::cout << "\n\t*CL:Code* Compilation FAIL (code=" << buildError <<")\n";
                    flashError = 0;
                    connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::BUILD, 1);
                }
            }
            else{
                std::cout << "\n\t*CL:Code* Generation FAIL (code=" << genError << ")\n";
                buildError = 0;
                flashError = 0;
                connectedCanvas->broadcastIndicatorLED(0, CanvasEvent::CODE_GEN, 1);
            }
        }
        else{
            genError = buildError = flashError = 0;
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

void CyclopsEditor::updateSerialIndicator(CanvasEvent event)
{
    // read connectedCanvas state
    if (event == CanvasEvent::TRANSFER_DROP){
        serialLED->update(Colours::black, "Invalid State");
        readinessLED->update(Colours::black, "Invalid State");
    }
    else if (event == CanvasEvent::SERIAL){
        if (isSerialConnected())
            if (connectedCanvas->serialIsVerified)
                serialLED->update(CyclopsColours::connected, "Connected");
            else
                serialLED->update(CyclopsColours::notVerified, "Connected, but not verified");
        else
            serialLED->update(CyclopsColours::disconnected, "Not Connected");
    }
    serialLED->repaint();
}

void CyclopsEditor::updateReadinessIndicator(CanvasEvent event, int attribute=0)
{
    if (event == CanvasEvent::CODE_GEN){
        if (attribute == 0)
            readinessLED->update(CyclopsColours::Ready, "Code Generation successful!");
        else
            readinessLED->update(CyclopsColours::errorGenFlash, "Code Generation failed!");
    }
    else if (event == CanvasEvent::BUILD){
        if (attribute == 0)
            readinessLED->update(CyclopsColours::Ready, "Code Generation and Compilation successful!");
        else
            readinessLED->update(CyclopsColours::errorGenFlash, "Compilation failed!");
    }
    else if (event == CanvasEvent::FLASH){
        if (attribute == 0)
            readinessLED->update(CyclopsColours::Ready, "Code Generation and Flashing  successful!");
        else
            readinessLED->update(CyclopsColours::errorGenFlash, "Flashing failed!");
    }
    else if (event == CanvasEvent::PLUGIN_SELECTED){
        // sub-plugin selected
        readinessLED->update(CyclopsColours::pluginSelected, "Plugin Selected, now configure it.");
    }
    readinessLED->repaint();
}

bool CyclopsEditor::channelMapStatus()
{
    return channelMapper->status();
}

Array<int> CyclopsEditor::getChannelMap()
{
    return channelMapper->channelMap;
}

CyclopsPluginInfo* CyclopsEditor::getPluginInfo()
{
    return connectedCanvas->getPluginInfoById(nodeId);
}

void CyclopsEditor::refreshPluginInfo()
{
    channelMapper->update(getPluginInfo());
    cmapViewport->setVisible(true);

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


/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                         CHANNEL-MAPPER-DISPLAY                           |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */
ChannelMapperDisplay::ChannelMapperDisplay(CyclopsEditor* editor, CyclopsCanvas* canvas):
    editor(editor),
    canvas(canvas)
{}

void ChannelMapperDisplay::update(CyclopsPluginInfo* pluginInfo)
{
    int delta = pluginInfo->channelCount - selectors.size();
    int width = getParentWidth();
    if (delta < 0){
        selectors.removeLast(delta);
        channelMap.removeLast(delta);
    }
    else if (delta > 0){
        for (int i=0; i<delta; i++){
            Slider* s = new Slider();
            s->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
            s->setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxLeft, false, 24, 18);
            s->setVelocityBasedMode(true);
            s->setVelocityModeParameters();
            //s->setIncDecButtonsMode(IncDecButtonMode::incDecButtonsDraggable_AutoDirection);
            s->setChangeNotificationOnlyOnRelease(true);
            s->addListener(this);
            s->setBounds(10, i*(20+5)+5, width-100, 20);
            addAndMakeVisible(s);
            selectors.add(s);
            channelMap.add(-1);
        }
    }
    for (int i=0; i<pluginInfo->channelCount; i++){
        selectors[i]->setValue(-1);
        selectors[i]->setRange(0, editor->getProcessor()->getNumInputs(), 1.0);
        channelMap.set(i, 0);
    }
    setBounds(0, 0, width, 25*selectors.size()+20);
    editor->repaint();
}

bool ChannelMapperDisplay::status()
{
    // do any kind of validation of slider selections...
    // like all sliders unique
    // like all sliders moved atleast once or whatever.
    //
    // No validation is a great choice too. As far as capability of the GUI is
    // considered, every mapping is valid because the mapping from OE->plugin
    // channels can be one-to-many.
    return true;
}

void ChannelMapperDisplay::paint(Graphics& g){
    g.fillAll(Colour(0x805E5E5E));
    int width = getParentWidth();
    for (int i=0; i<selectors.size(); i++){
        g.drawText(String(i), width-100+5, i*(20+5)+5, 24, 20, Justification::Flags::left);
    }
}

void ChannelMapperDisplay::sliderValueChanged(Slider* s)
{
    std::cout << s->getValue() << ":" << selectors.indexOf(s) << std::endl;
}

void ChannelMapperDisplay::sliderDragStarted(Slider* s) {}
void ChannelMapperDisplay::sliderDragEnded(Slider* s) {}

/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                               SIMPLE-WINDOW                              |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */

SimpleWindow::SimpleWindow(const String& title, Component* content, const void (*closeCallback_fn)(DocumentWindow*))
: DocumentWindow(title, Colours::lightgrey, DocumentWindow::TitleBarButtons::allButtons),
  content(content),
  closeCallback(closeCallback_fn)
{
    setVisible(false);
    setContentNonOwned(content, true);
    setUsingNativeTitleBar(true);
    setResizable(true, true);
}

bool SimpleWindow::keyPressed(const KeyPress& key)
{
    if (key.isKeyCode(KeyPress::escapeKey)){
        closeButtonPressed();
    }
    DocumentWindow::keyPressed(key);
    return false;
}

void SimpleWindow::closeButtonPressed()
{
    if (closeCallback != nullptr)
        closeCallback(this);
    setVisible(false);
}

} // NAMESPACE cyclops
