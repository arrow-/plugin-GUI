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

#include "CyclopsCanvas.h"

namespace cyclops {

OwnedArray<CyclopsCanvas> CyclopsCanvas::canvasList;

const int CyclopsCanvas::BAUDRATES[12] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400};

CyclopsCanvas::CyclopsCanvas() : tabIndex(-1)
                               , realIndex(-1)
                               , dataWindow(nullptr)
                               , progress(0)
                               , in_a_test(false)
{
    // Add "port" list
    portCombo = new ComboBox();
    portCombo->addListener(this);
    portCombo->setTooltip("Select the serial port on which Cyclops is connected.");
    portCombo->addItemList(getDevices(), 1);
    addAndMakeVisible(portCombo);

    // Add refresh button
    refreshButton = new UtilityButton("R", Font("Default", 9, Font::plain));
    refreshButton->setRadius(3.0f);
    refreshButton->addListener(this);
    addAndMakeVisible(refreshButton);

    // Add baudrate list
    baudrateCombo = new ComboBox();
    baudrateCombo->addListener(this);
    baudrateCombo->setTooltip("Set the baud rate (115200 recommended).");

    Array<int> baudrates(getBaudrates());
    for (int i = 0; i < baudrates.size(); i++)
    {
        baudrateCombo->addItem(String(baudrates[i]), baudrates[i]);
    }
    baudrateCombo->setSelectedId(115200);
    addAndMakeVisible(baudrateCombo);

    // Add TEST buttons
    for (int i=0; i < 4; i++){
        testButtons.add(new UtilityButton(String("Test ") + String(i), Font("Default", 11, Font::bold)));
        testButtons[i]->addListener(this);
        addAndMakeVisible(testButtons[i]);
    }
    progressBar = new ProgressBar(progress);
    progressBar->setPercentageDisplay(false);
    addChildComponent(progressBar);
    pstep = 0.01;
    // communicate with teensy?
    
    // Add close Button
    closeButton = new UtilityButton("close", Font("Default", 9, Font::plain));
    closeButton->addListener(this);
    addChildComponent(closeButton);

    hookViewDisplay = new HookViewDisplay(this);
    hookViewport = new HookViewport(hookViewDisplay);
    addAndMakeVisible(hookViewport);

    /*bfbf = new HookViewDisplay(this);
    bfbf->setBounds(0, 0, 100, 100);
    addAndMakeVisible(bfbf);*/
}

void CyclopsCanvas::setRealIndex(int real_index)
{
    realIndex = real_index;
}

CyclopsCanvas::~CyclopsCanvas()
{
    //std::cout<<"deleting clcan"<<std::endl;
}

void CyclopsCanvas::beginAnimation()
{
    std::cout << "CyclopsCanvas beginning animation." << std::endl;

    startCallbacks();
}

void CyclopsCanvas::endAnimation()
{
    std::cout << "CyclopsCanvas ending animation." << std::endl;

    stopCallbacks();
}

void CyclopsCanvas::setParameter(int x, float f)
{

}

void CyclopsCanvas::setParameter(int a, int b, int c, float d)
{
}

void CyclopsCanvas::update()
{
    std::cout << "Updating CyclopsCanvas" << std::endl;
}


void CyclopsCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    repaint();
}

void CyclopsCanvas::resized()
{
    int width = getWidth(), height = getHeight();
    baudrateCombo->setBounds(width-75, 5, 70, 20);
    portCombo->setBounds(width-75-5-70, 5, 70, 20);
    refreshButton->setBounds(width-75-5-70-5-20, 5, 20, 20);
    for (int i=0; i < 4; i++){
        testButtons[i]->setBounds(jmax(100, width-53), jmax(45, (height/5))*(i+1)-(25/2), 50, 25);
    }
    progressBar->setBounds(2, height-16, width-4, 16);
    closeButton->setBounds(width/2-20, 5, 40, 20);

    hookViewport->setBounds(2, 50, width-100, height-50);
    hookViewDisplay->setBounds( 0
                              , 0
                              , width-100-hookViewport->getScrollBarThickness()
                              , 40+50*canvasEventListeners.size());
}

int CyclopsCanvas::getNumCanvas()
{
    return canvasList.size();
}

void CyclopsCanvas::getEditorIds(CyclopsCanvas* cc, Array<int>& editorIdList)
{
    for (int i=0; i<cc->canvasEventListeners.size(); i++){
        editorIdList.add(cc->canvasEventListeners.getListeners()[i]->getEditorId());
    }
}

int CyclopsCanvas::migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, CyclopsCanvas::Listener* listener, bool refreshNow)
{
    std::cout << "migrating " << listener->getEditorId() << " to Cyclops B" << dest->realIndex << std::endl;
    // update combo selection
    listener->changeCanvas(dest);
    src->removeListener(listener);
    dest->addListener(listener);
    if (refreshNow) {
        dest->refresh();
        src->refresh();
    }
    return 0;
}

// this DOES NOT REFRESH CANVAS!!
int CyclopsCanvas::migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, int nodeId)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findById(src, nodeId);
    jassert (listener != nullptr);
    return CyclopsCanvas::migrateEditor(dest, src, listener, false);
}

void CyclopsCanvas::refresh()
{
    hookViewDisplay->refresh();
    resized();
}

void CyclopsCanvas::disableAllInputWidgets()
{
    // Disable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(false);
    portCombo->setEnabled(false);
    baudrateCombo->setEnabled(false);
    refreshButton->setEnabled(false);
}

void CyclopsCanvas::enableAllInputWidgets()
{
    // Reenable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(true);
    portCombo->setEnabled(true);
    baudrateCombo->setEnabled(true);
    refreshButton->setEnabled(true);
}

bool CyclopsCanvas::isReady()
{
    return false;
}

void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    if (!in_a_test)
        progressBar->setVisible(false);
    if (canvasList.size() > 1)
        closeButton->setVisible(true);
}

void CyclopsCanvas::buttonClicked(Button* button)
{  
    int test_index = -1;
    for (int i=0; i < 4; i++){
        if (button == testButtons[i]){
            test_index = i;
            break;
        }
    }
    if (test_index >= 0){
        disableAllInputWidgets();
        std::cout << "Testing LED channel " << test_index << "\n";
        in_a_test = true;
        //serialInfo.Serial->testChannel(test_index);
        progressBar->setTextToDisplay("Testing LED channel" + String(test_index));
        progressBar->setVisible(true);
        startTimer(20);
        test_index = -1;
        //for (auto& )
    }
    if (button == refreshButton)
    {
        // Refresh list of devices
        portCombo->clear();
        portCombo->addItemList(getDevices(), 1);
    }
    else if (button == closeButton)
    {
        // close this canvas!
        int choice = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon, "Migrate or Drop?", "Choose 'Migrate' if you want to preserve the configuration of hooks and sub-plugins,\nand move (some / all of) the hooks to another Cyclops\nOR\nChoose 'Drop' to orphan the hooks.", "Migrate", "Drop", "Cancel");
        switch (choice){
        case 0: // cancel, do nothing
        break;

        case 1: // migrate
            DialogWindow::LaunchOptions dw;
            dw.dialogTitle                  = "Select hooks and target board";
            dw.dialogBackgroundColour       = Colours::grey;
            dw.componentToCentreAround      = this;
            dw.escapeKeyTriggersCloseButton = true;
            dw.useNativeTitleBar            = false;
            dw.resizable                    = true;
            dw.useBottomRightCornerResizer  = false;
            MigrateComponent* mw            = new MigrateComponent(this);
            dw.content.set(mw, true);

            // freeze all cyclops elements
            for (auto& canvas : CyclopsCanvas::canvasList){
                canvas->disableAllInputWidgets();
                canvas->broadcastEditorInteractivity(CanvasEvent::FREEZE);
            }
            dw.launchAsync();
        break;
        }
    }
}

void CyclopsCanvas::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == portCombo)
    {
        setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateCombo)
    {
        setBaudrate(comboBox->getSelectedId());
    }
}

bool CyclopsCanvas::keyPressed(const KeyPress& key)
{
    return true;
}

void CyclopsCanvas::timerCallback()
{
    if (in_a_test){
        progress += pstep;
        if (progress >= 1.0){
            progressBar->setVisible(false);
            progress = 0;
            in_a_test = false;
            stopTimer();
            enableAllInputWidgets();
        }
    }
}

bool CyclopsCanvas::screenLikelyNames(const String& port_name)
{
    #ifdef TARGET_OSX
        return port_name.contains("cu.") || port_name.contains("tty.");
    #endif
    #ifdef TARGET_LINUX
        return port_name.contains("ttyUSB") || port_name.contains("ttyA");
    #endif
    return true; // for TARGET_WIN32
}

StringArray CyclopsCanvas::getDevices()
{
    vector<ofSerialDeviceInfo> allDeviceInfos = serialInfo.Serial->getDeviceList();
    StringArray allDevices;
    String port_name;
    for (unsigned int i = 0; i < allDeviceInfos.size(); i++)
    {
        port_name = allDeviceInfos[i].getDeviceName();
        if (screenLikelyNames(port_name))
        {
            allDevices.add(port_name);
        }
    }
    return allDevices;
}

Array<int> CyclopsCanvas::getBaudrates()
{
    Array<int> allBaudrates(BAUDRATES, 12);
    return allBaudrates;
}

void CyclopsCanvas::setDevice(string port)
{
    serialInfo.portName = port;
    if (serialInfo.portName != ""){
        serialInfo.Serial->setup(serialInfo.portName, serialInfo.baudRate);
    }
    canvasEventListeners.call(&CyclopsCanvas::Listener::updateIndicators, CanvasEvent::SERIAL_LED);
}

void CyclopsCanvas::setBaudrate(int baudrate)
{
    if (serialInfo.baudRate != baudrate){
        serialInfo.baudRate = baudrate;
        setDevice(serialInfo.portName);
    }
}

const cl_serial* CyclopsCanvas::getSerialInfo()
{
    return &serialInfo;
}

void CyclopsCanvas::addListener(CyclopsCanvas::Listener* const newListener)
{
    canvasEventListeners.add(newListener);
    // and other info
}

void CyclopsCanvas::removeListener(CyclopsCanvas::Listener* const oldListener)
{
    canvasEventListeners.remove(oldListener);
    // and other cleanup!!
}

void CyclopsCanvas::broadcastButtonState(CanvasEvent whichButton, bool state)
{
    canvasEventListeners.call(&CyclopsCanvas::Listener::updateButtons, whichButton, state);
}

void CyclopsCanvas::broadcastEditorInteractivity(CanvasEvent interactivity)
{
    canvasEventListeners.call(&CyclopsCanvas::Listener::setInteractivity, interactivity);
}

int  CyclopsCanvas::getNumListeners()
{
    return canvasEventListeners.size();
}

void CyclopsCanvas::saveVisualizerParameters(XmlElement* xml)
{
    XmlElement* parameters = xml->createNewChildElement("PARAMETERS");
    parameters->setAttribute("device", portCombo->getText().toStdString());
    parameters->setAttribute("baudrate", baudrateCombo->getSelectedId());
}

void CyclopsCanvas::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, subNode) {
        if (subNode->hasTagName("PARAMETERS")) {
            portCombo->setText(subNode->getStringAttribute("device", ""));
            baudrateCombo->setSelectedId(subNode->getIntAttribute("baudrate"));
        }
    }
}

CyclopsCanvas::Listener* CyclopsCanvas::findById(CyclopsCanvas* cc, int nodeId)
{
    auto& listener_list = cc->canvasEventListeners.getListeners();
    for (auto& listener : listener_list){
        if (nodeId == listener->getEditorId())
            return listener;
    }
    return nullptr;
}


















HookViewport::HookViewport(HookViewDisplay* display) : hvDisplay(display)
{
    setViewedComponent(hvDisplay, false);
    setScrollBarsShown(true, false);
}

void HookViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{

}

void HookViewport::paint(Graphics& g)
{
}

















HookViewDisplay::HookViewDisplay(CyclopsCanvas* _canvas) : canvas(_canvas)
{
}

void HookViewDisplay::paint(Graphics& g)
{
    g.fillAll(Colours::orange);
}

void HookViewDisplay::refresh()
{
    Array<int> newIds;
    CyclopsCanvas::getEditorIds(canvas, newIds);
    // find first index where the 2 arrays deviate.
    int newSize = newIds.size(),
        oldSize = shownIds.size(),
        runLength = min(newSize, oldSize),
        deviant;
    Rectangle<int> dirty;
    if (newSize < oldSize){
        // find deleted EditorId
        for (deviant=0; deviant<runLength; deviant++)
            if (newIds[deviant] != shownIds[deviant])
                break;
        shownIds.remove(deviant);
        hookViews.remove(deviant);
        dirty.setBounds(5, 50+deviant*45, getWidth()-80, 45*(oldSize-deviant));
    }
    else if (newSize > oldSize){
        for (deviant=0; deviant<runLength; deviant++)
            if (newIds[deviant] != shownIds[deviant])
                break;
        shownIds.insert(deviant, newIds[deviant]);
        HookView *hv = hookViews.insert(deviant, new HookView(newIds[deviant]));
        addAndMakeVisible(hv);
        dirty.setBounds(5, 50+deviant*45, getWidth()-80, 45*(newSize-deviant));
    }
    //repaint();
    repaint(dirty);
}

void HookViewDisplay::resized()
{
    int index = 0;
    for (auto& hv : hookViews){
        hv->setBounds(5, 50+index*45, getWidth()-80, 40);
        //std::cout << "rs " << index << " " << hv->nodeId<<std::endl;
        index++;
    }
}
















HookView::HookView(int node_id) : nodeId(node_id)
{
    hookIdLabel = new Label("hook_id", String(nodeId));
    hookIdLabel->setFont(Font("Default", 16, Font::plain));
    hookIdLabel->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(hookIdLabel);
}

void HookView::paint(Graphics& g)
{
    g.fillAll(Colours::lightgrey);
}

void HookView::resized()
{
    hookIdLabel->setBounds(5, 0, 80, 30);
}













MigrateComponent::MigrateComponent(CyclopsCanvas* closing_canvas) : closingCanvas(closing_canvas)
{
    group = new GroupComponent ("group", "Select Hooks");
    addAndMakeVisible(group);

    canvasCombo = new ComboBox("target_canvas");

    int num_canvases = CyclopsCanvas::getNumCanvas();
    CyclopsCanvas::getEditorIds(closingCanvas, editorIdList);
    for (auto& editorId : editorIdList){
        ToggleButton* tb = editorButtonList.add(new ToggleButton(String(editorId)));
        //tb->setRadioGroupId(editorId);
        addAndMakeVisible(tb);
    }
    
    comboText = new Label("combo label", "Migrate to");
    comboText->setFont(Font("Default", 16, Font::plain));
    comboText->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(comboText);

    for (int i=0; i<num_canvases; i++){
        if (i != closingCanvas->realIndex){
            canvasCombo->addItem("Cyclops" + String(i), i+1);
        }
    }
    addAndMakeVisible(canvasCombo);

    doneButton = new UtilityButton("DONE", Font("Default", 12, Font::plain));
    doneButton->addListener(this);
    cancelButton = new UtilityButton("CANCEL", Font("Default", 12, Font::plain));
    cancelButton->addListener(this);
    addAndMakeVisible(doneButton);
    addAndMakeVisible(cancelButton);

    setSize(300, 60+30+40+25*editorButtonList.size());
}

void MigrateComponent::resized()
{
    int width = getWidth(), height = getHeight(), i=0;
    for (auto& tb : editorButtonList){
        tb->setBounds(width/2-200/2+10, 25+i*22, 60, 20);
        i++;
    }
    group->setBounds(width/2-200/2, 10, 200, 25*(i)+20);
    comboText->setBounds(width/2-100/2, height-100, 100, 25);
    canvasCombo->setBounds(width/2-90/2, height-70, 90, 25);
    doneButton->setBounds((width-80*2)/3, height-30, 80, 25);
    cancelButton->setBounds((width-80*2)*2/3+80, height-30, 80, 25);
}

void MigrateComponent::buttonClicked(Button* button)
{
    if (button == doneButton){
        CyclopsCanvas *newCanvas = CyclopsCanvas::canvasList.getUnchecked(canvasCombo->getSelectedId()-1);
        for (int& editorId : editorIdList){
            // this will changeCanvas and update SelectorButtons of src->listeners only
            jassert (CyclopsCanvas::migrateEditor(newCanvas, closingCanvas, editorId) == 0);
        }
        newCanvas->refresh();
        closingCanvas->refresh();
    }
    for (auto& canvas : CyclopsCanvas::canvasList){
        canvas->enableAllInputWidgets();
        canvas->broadcastEditorInteractivity(CanvasEvent::THAW);
    }
    // now update all comboBoxes -- ALL!
    // ??
    
    if (DialogWindow* dw = findParentComponentOfClass<DialogWindow>())
        dw->exitModalState(0);
}

} // NAMESPACE cyclops
