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
OwnedArray<HookView>      CyclopsCanvas::hookViews;
int                       CyclopsCanvas::numCanvases = 0;

ScopedPointer<CyclopsPluginManager> CyclopsCanvas::pluginManager = new CyclopsPluginManager();

const int CyclopsCanvas::BAUDRATES[12] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400};

CyclopsCanvas::CyclopsCanvas() : tabIndex(-1)
                               , realIndex(-1)
                               , dataWindow(nullptr)
                               , serialIsVerified(false)
                               , progress(0)
                               , in_a_test(false)
{
    realIndex = CyclopsCanvas::numCanvases++; // 0 onwards
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
    
    // Add close Button
    closeButton = new UtilityButton("close", Font("Default", 9, Font::plain));
    closeButton->addListener(this);
    addChildComponent(closeButton);

    hookLabel = new Label("HookLabel", "Connected Hooks");
    hookLabel->setFont(Font("Default", 20, Font::plain));
    hookLabel->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(hookLabel);
    hookViewDisplay = new HookViewDisplay(this);
    hookViewport = new HookViewport(hookViewDisplay);
    addAndMakeVisible(hookViewport);

    sigLabel = new Label("SigLabel", "Signal Collection");
    sigLabel->setFont(Font("Default", 20, Font::plain));
    sigLabel->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(sigLabel);

    signalDisplay = new SignalDisplay(this);
    signalViewport = new SignalViewport(signalDisplay);
    addAndMakeVisible(signalViewport);

    ledChannelPort = new LEDChannelPort(this);
    addAndMakeVisible(ledChannelPort);
    for (int i=0; i<4; i++)
        linkPaths.add(new Path());

    progressBar = new ProgressBar(progress);
    progressBar->setPercentageDisplay(false);
    addChildComponent(progressBar);
    pstep = 0.00565; // +40ms
    refreshPlugins();

    //BEWARE
    program = nullptr;
    CLDevInfo = nullptr;
}

void CyclopsCanvas::refreshPlugins()
{
    if (pluginManager->getNumPlugins() == 0){
        std::cout << "*CL:sPM* Making Cyclops-Plugin List" << std::endl;
        pluginManager->loadAllPlugins();
        std::cout << "*CL:sPM* Loaded " << pluginManager->getNumPlugins() << " cyclops plugin(s).\n" << std::endl;
    }
}

CyclopsCanvas::~CyclopsCanvas()
{
    
}

void CyclopsCanvas::beginAnimation()
{
    //DBG ("CyclopsCanvas beginning animation.\n");
    disableAllInputWidgets();
    if (serialInfo.portName != ""){
        ;
    }
    //startTimer(40);
}

void CyclopsCanvas::endAnimation()
{
    //DBG ("CyclopsCanvas ending animation.\n");
    enableAllInputWidgets();
    stopTimer();
}

void CyclopsCanvas::setParameter(int x, float f)
{

}

void CyclopsCanvas::setParameter(int a, int b, int c, float d)
{
}

void CyclopsCanvas::update()
{
    DBG ("Updating CyclopsCanvas\n");
}


void CyclopsCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    repaint();
}

void CyclopsCanvas::resized()
{
    //DBG ("resizing canvas\n");
    int width = getWidth(), height = getHeight();
    baudrateCombo->setBounds(width-75, 5, 70, 20);
    portCombo->setBounds(width-75-5-70, 5, 70, 20);
    refreshButton->setBounds(width-75-5-70-5-20, 5, 20, 20);
    
    ledChannelPort->setBounds(jmax(100, width-73), 25, 70, height-25);
    progressBar->setBounds(2, height-16, width-4, 16);
    closeButton->setBounds(width/2-20, 5, 40, 20);

    hookLabel->setBounds(20, 5, jmax(200, width-200), 24);
    hookViewport->setBounds(2, 30, width-140, height/2-30);
    hookViewDisplay->setBounds( 0
                              , 0
                              , width-140-hookViewport->getScrollBarThickness()
                              , hookViewDisplay->height);

    sigLabel->setBounds(20, 30+5+height/2-30, jmax(200, width-200), 24);
    signalViewport->setBounds(2, 30+30+height/2-30, width/2-50, height/2-30);
    signalDisplay->setBounds( 0
                            , 0
                            , width/2-50-signalViewport->getScrollBarThickness()
                            , 10+30*CyclopsSignal::signals.size());
}

int CyclopsCanvas::getNumCanvas()
{
    return CyclopsCanvas::canvasList.size();
}

void CyclopsCanvas::getEditorIds(CyclopsCanvas* cc, Array<int>& editorIdList)
{
    for (int i=0; i<cc->canvasEventListeners.size(); i++){
        editorIdList.add(cc->canvasEventListeners.getListeners()[i]->getEditorId());
    }
}

int CyclopsCanvas::migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, CyclopsCanvas::Listener* listener, bool refreshNow /*=true*/)
{
    int node_id = listener->getEditorId();
    std::cout << "*CL* Migrating " << node_id << " to Cyclops B" << dest->realIndex << "from Cyclops B" << src->realIndex << std::endl;
    // update combo selection
    for (int i=0; i<CyclopsCanvas::canvasList.size(); i++){
        if (CyclopsCanvas::canvasList[i]->realIndex == dest->realIndex)
            break;
    }
    listener->changeCanvas(dest);
    listener->updateButtons(CanvasEvent::COMBO_BUTTON, true);

    // code generation decision map
    src->decisionMap.erase(node_id);
    dest->decisionMap[node_id] = false;

    src->removeListener(listener);
    HookView* hv = CyclopsCanvas::getHookView(node_id);
    jassert(hv != nullptr);
    // hook LED connection reset
    hv->hookInfo->LEDChannel = -1;
    src->hookViewDisplay->removeChildComponent(hv);
    dest->addListener(listener);
    dest->hookViewDisplay->addAndMakeVisible(hv);
    
    if (refreshNow) {
        dest->refresh();
        src->refresh();
    }
    return 0;
}

int CyclopsCanvas::migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, int nodeId)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(src, nodeId);
    jassert (listener != nullptr);
    return CyclopsCanvas::migrateEditor(dest, src, listener, false);
}

void CyclopsCanvas::dropEditor(CyclopsCanvas* closingCanvas, int node_id)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(closingCanvas, node_id);
    jassert (listener != nullptr);
    listener->updateReadinessIndicator(CanvasEvent::TRANSFER_DROP);
    listener->updateSerialIndicator(CanvasEvent::TRANSFER_DROP);
    listener->changeCanvas(nullptr);
}

void CyclopsCanvas::refresh()
{
    //DBG ("refreshing hvd\n");
    hookViewDisplay->refresh();
    resized();
}

void CyclopsCanvas::disableAllInputWidgets()
{
    // Disable the whole gui
    portCombo->setEnabled(false);
    baudrateCombo->setEnabled(false);
    refreshButton->setEnabled(false);
    hookViewDisplay->disableAllInputWidgets();
}

void CyclopsCanvas::enableAllInputWidgets()
{
    // Reenable the whole gui
    portCombo->setEnabled(true);
    baudrateCombo->setEnabled(true);
    refreshButton->setEnabled(true);
    hookViewDisplay->enableAllInputWidgets();
}

void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colour(0xff6e6e6e));
    if (!in_a_test)
        progressBar->setVisible(false);
    if (canvasList.size() > 1)
        closeButton->setVisible(true);
    g.setColour(Colours::black);
    for (auto& p : linkPaths){
        g.strokePath(*p, PathStrokeType(3));
    }
}

void CyclopsCanvas::buttonClicked(Button* button)
{
    if (button == refreshButton)
    {
        // Refresh list of devices
        portCombo->clear();
        portCombo->addItemList(getDevices(), 1);
        serialIsVerified = false;
        CLDevInfo = nullptr;
        program = nullptr;
        for (int i=0; i<4; i++){
            ledChannelPort->testButtons[i]->setEnabled(false);
        }
        if (serialInfo.portName != ""){
            serialInfo.Serial->flush();
            serialInfo.Serial->close();
            serialInfo.portName = "";
        }
    }
    else if (button == closeButton)
    {
        // close this canvas!
        int choice = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon, "Migrate or Drop?", "Choose 'Migrate' if you want to preserve the configuration of hooks and sub-plugins,\nand move (some / all of) the hooks to another Cyclops\nOR\nChoose 'Drop' to orphan all the hooks.", "Migrate some", "Drop all", "Cancel");
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
    if (comboBox == portCombo){
        setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateCombo){
        setBaudrate(comboBox->getSelectedId());
    }
}

bool CyclopsCanvas::keyPressed(const KeyPress& key)
{
    return true;
}

void CyclopsCanvas::timerCallback()
{
    returnCode response;
    while (serialInfo.Serial->available() > 0){
        response = (returnCode) serialInfo.Serial->readByte();
        switch (response){
        case CL_RC_LAUNCH:
        break;

        case CL_RC_END:
        break;

        case CL_RC_TEST:
        break;

        case CL_RC_SBDONE:
        break;

        case CL_RC_IDENTITY:
        break;

        case CL_RC_MBDONE:
        break;

        case CL_RC_EA_FAIL:
        case CL_RC_NEA_FAIL:
        break;
        case CL_RC_UNKNOWN:
        break;
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

CyclopsPluginInfo* CyclopsCanvas::getPluginInfoById(int node_id)
{
    HookView* hv = getHookView(node_id);
    if (hv != nullptr){
        return hv->hookInfo->pluginInfo;
    }
    return nullptr;
}

void CyclopsCanvas::setDevice(string port)
{
    serialInfo.portName = port;
    if (serialInfo.portName != "")
    {
        if (serialInfo.Serial->setup(serialInfo.portName, serialInfo.baudRate))
        {
            serialInfo.Serial->flush();
            if (getDeviceIdentity()){
                // determine the kind of device
                if (CLDevInfo->isTeensy){ // Teensy32
                    program = new code::ProgramTeensy32();
                }
                /*
                else if (CLDevInfo->isArduino) // Arduino
                    program = new code::ProgramArduino();
                */

                // Finally,
                for (int i=0; i<4; i++){
                    // enable the test buttons
                    if (CLDevInfo->channelState[i] == 1){
                        ledChannelPort->testButtons[i]->setEnabled(true);
                    }
                }
                DBG ("*CL* [success] Device identified and prepared. [" << CLDevInfo->toString() << ")]");
                CoreServices::sendStatusMessage("Cyclops Device identified and prepared.");
                serialIsVerified = true;
            }
            else{
                // (Serial Error) OR (port accessed but ((device not recognised) OR (device not responding) OR (device not pre-programmed))
                CoreServices::sendStatusMessage("Unidentifiable device. See project FAQ for details!");
                std::cout << "*CL* [warning] Device is not responding or has not been pre-programmed or is not a Cyclops Device!" << std::endl;
                if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, "Choose Controller", "What is controlling the Cyclops Device?", "Teensy", "Arduino"))
                    program = new code::ProgramTeensy32();
                else
                    program = nullptr;//new code::ProgramArduino();
                serialIsVerified = false;
            }
        }
        else{
            CoreServices::sendStatusMessage("Error: Serial Port cannot be accesed");
            std::cout << "*CL* [failure] Serial Port could not be accessed." << std::endl;
            serialIsVerified = false;
        }
    }
    canvasEventListeners.call(&CyclopsCanvas::Listener::updateSerialIndicator, CanvasEvent::SERIAL);
}

bool CyclopsCanvas::getDeviceIdentity()
{
    api::CyclopsRPC rpc;
    api::identify(&rpc);
    serialInfo.Serial->writeBytes(rpc.message, rpc.length);
    
    unsigned char identity[RPC_IDENTITY_SUCCESSCODE_SZ] = {0};
    DBG ("*CL* Contacting Cyclops device...");
    // Wait upto 0.5 sec for Cyclops Device response.
    int64 ctime = Time::currentTimeMillis();
    while (ctime + 500 > Time::currentTimeMillis()){
        if (serialInfo.Serial->available() >= RPC_IDENTITY_SUCCESSCODE_SZ)
            break;
    }
    int readBytes = serialInfo.Serial->readBytes(identity, RPC_IDENTITY_SUCCESSCODE_SZ);
    //DBG (readBytes);
    if (readBytes == RPC_IDENTITY_SUCCESSCODE_SZ && identity[RPC_IDENTITY_SUCCESSCODE_SZ-1] == CL_RC_IDENTITY){
        CLDevInfo = new CyclopsDeviceInfo(identity);
        return true;
    }
    return false;
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

void CyclopsCanvas::addHook(int node_id)
{
    HookView* hv = CyclopsCanvas::hookViews.add(new HookView(node_id));
    hookViewDisplay->addAndMakeVisible(hv);
    decisionMap[node_id] = false;
    //DBG ("added hook\n");
}

bool CyclopsCanvas::removeHook(int node_id)
{
    HookView* hv = getHookView(node_id);
    if (hv == nullptr)
        return false;
    // Keep shownIds up-to-date (solves deletion-of-editor mem-leak bug)
    // Just by updating showIds, we cleanly remove any LED Link of this HookView
    hookViewDisplay->shownIds.removeFirstMatchingValue(node_id);
    CyclopsCanvas::hookViews.removeObject(hv, true); // deletes
    decisionMap.erase(node_id);
    return true;
}

void CyclopsCanvas::getAllSummaries(std::vector<code::CyclopsHookConfig>& hookInfoList, Array<std::bitset<CLSTIM_NUM_PARAMS> >& summaries)
{
    summaries.clear();
    hookInfoList.clear();
    for (auto& listener : canvasEventListeners.getListeners()){
        std::bitset<CLSTIM_NUM_PARAMS> summary;
        HookInfo* hi = getSummary(listener->getEditorId(), summary);

        // Making the CyclopsHookConfig instance
        code::CyclopsHookConfig chc(hi->nodeId, hi->LEDChannel, hi->pluginInfo, hi->selectedSignals);
        hookInfoList.push_back(chc);
        summaries.add(summary);
    }
}

// This is only called by CyclopsEditor::isReadyForLaunch()
// there's another static function with same name!
bool CyclopsCanvas::getSummary(int node_id, bool& isPrimed)
{
    std::bitset<CLSTIM_NUM_PARAMS> summary;
    getSummary(node_id, summary);
    if (decisionMap[node_id] == false){
        decisionMap[node_id] = true;
    }
    isPrimed = summary.all();

    // check decisionMap
    std::map<int, bool>::iterator it;
    /*
    for (it = decisionMap.begin(); it != decisionMap.end(); it++)
        DBG (it->first << " : " << it->second);
    */
    for (it = decisionMap.begin(); it != decisionMap.end(); it++){
        if (it->second == false)
            break;
    }
    if (it == decisionMap.end()){
        // successfully checked all listeners
        for (it = decisionMap.begin(); it != decisionMap.end(); it++)
            it->second = false;
        return true; // continue with code gen
    }
    return false;
}

bool CyclopsCanvas::generateCode(int& genError)
{
    if (program != nullptr){
        Array<std::bitset<CLSTIM_NUM_PARAMS> > summaries;
        std::vector<code::CyclopsHookConfig> hookInfoList;
        getAllSummaries(hookInfoList, summaries);

        code::CyclopsConfig config(hookInfoList, summaries);
        return program->create(config, genError);
    }
    genError = 15; // program object not created!
    return false;
}

bool CyclopsCanvas::buildCode(int& buildError)
{
    if (program != nullptr){
        if (program->currentHash != 0)
            return program->build(buildError);
        else{
            buildError = 1;
            return false;
        }
    }
    buildError = 15; // program object not created!
    return false;
}

bool CyclopsCanvas::flashDevice(int& flashError)
{
    if (program != nullptr){
        serialInfo.Serial->flush();
        serialInfo.Serial->close();
        if (program->currentHash != 0){
            if (program->flash(flashError, realIndex)){
                DBG ("\nWaiting " << program->reconDuration << " msec for device to reboot...");
                // Wait for some time, give teensy time to reconnect after flashing.
                int64 ctime = Time::currentTimeMillis();
                while (ctime + program->reconDuration > Time::currentTimeMillis());
                std::cout << "\n";
                if (serialInfo.Serial->setup(serialInfo.portName, serialInfo.baudRate)){
                    if (getDeviceIdentity()){
                        for (int i=0; i<4; i++){
                            // enable the test buttons
                            if (CLDevInfo->channelState[i] == 1){
                                ledChannelPort->testButtons[i]->setEnabled(true);
                            }
                        }
                        serialIsVerified = true;
                        return true;
                    }
                    else{
                        serialIsVerified = false;
                        canvasEventListeners.call(&CyclopsCanvas::Listener::updateSerialIndicator, CanvasEvent::SERIAL);
                        flashError = 14; // flashed but no response!
                        CoreServices::sendStatusMessage("[Error] Cyclops Device did not respond. Try refreshing Serial Port.");
                        DBG ("*CL* [Error] Cyclops Device did not respond. Try refreshing Serial Port.");
                        return false;
                    }
                }
                else{
                    serialIsVerified = false;
                    canvasEventListeners.call(&CyclopsCanvas::Listener::updateSerialIndicator, CanvasEvent::SERIAL);
                    flashError = 13; // flashed but couldn't reconnect!
                    CoreServices::sendStatusMessage("[Error] Cyclops Device did not respond [WORMHOLE]");
                    DBG ("*CL* [Error] Cyclops Device did not respond. This might be the WORMHOLE condition.");
                    return false;
                }
            }
        }
        else{
            flashError = 1;
            return false;
        }
    }
    flashError = 15; // program object not created!
    return false;
}



HookView* CyclopsCanvas::getHookView(int node_id)
{
    HookView* res = nullptr;
    for (auto& hv : CyclopsCanvas::hookViews){
        if (hv->nodeId == node_id){
            res = hv;
            break;
        }
    }
    return res;
}

void CyclopsCanvas::updateLink(int ledChannel, Point<int> src, Point<int> dest)
{
    float x1 = src.getX(), y1 = src.getY(),
          x2 = dest.getX(), y2 = dest.getY();
    Path* p = linkPaths[ledChannel];
    p->clear();
    p->startNewSubPath(x1, y1);
    p->cubicTo(x1 + (x2 - x1) * 0.8f, y1 + (y2 - y1) * 0.05f,
               x2 - (x2 - x1) * 0.8f, y2 - (y2 - y1) * 0.05f,
               x2, y2);
}

void CyclopsCanvas::removeLink(int ledChannel)
{
    linkPaths[ledChannel]->clear();
}

void CyclopsCanvas::hideLink(int ledChannel)
{

}

void CyclopsCanvas::redrawLinks()
{
    int hookID;
    for (int i=0; i<4; i++){
        hookID = ledChannelPort->connections[i];
        if (hookID > -1){
            Point<int> src, dest;
            if (! hookViewport->getLinkPathSource(hookID, src)){
                // This HookView no longer exists ('twas deleted)
                ledChannelPort->connections.set(i, -1);
                removeLink(i);
            }
            else{
                ledChannelPort->getLinkPathDest(i, dest); // always returns true
                updateLink (i,
                            getLocalPoint(hookViewport, src),
                            getLocalPoint(ledChannelPort, dest));
            }
        }
    }
    repaint();
}



void CyclopsCanvas::broadcastButtonState(CanvasEvent whichButton, bool state)
{
    canvasEventListeners.call(&CyclopsCanvas::Listener::updateButtons, whichButton, state);
}

void CyclopsCanvas::broadcastEditorInteractivity(CanvasEvent interactivity)
{
    canvasEventListeners.call(&CyclopsCanvas::Listener::setInteractivity, interactivity);
}

void CyclopsCanvas::broadcastIndicatorLED(int LEDtype, CanvasEvent event, int attribute)
{
    if (LEDtype == 0)
        canvasEventListeners.call(&CyclopsCanvas::Listener::updateReadinessIndicator, event, attribute);
    else if (LEDtype == 1)
        canvasEventListeners.call(&CyclopsCanvas::Listener::updateSerialIndicator, event);
}

void CyclopsCanvas::unicastPluginSelected(CanvasEvent pluginState, int node_id)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(this, node_id);
    // also updates the channel mapper display
    listener->updateReadinessIndicator(CanvasEvent::PLUGIN_SELECTED);
    listener->refreshPluginInfo();
}

bool CyclopsCanvas::unicastGetChannelMapStatus(int node_id)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(this, node_id);
    return listener->channelMapStatus();
}

void CyclopsCanvas::broadcastNewCanvas()
{
    for (auto& c : CyclopsCanvas::canvasList){
        c->broadcastButtonState(CanvasEvent::COMBO_BUTTON, true);
    }
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

HookInfo* CyclopsCanvas::getSummary(int node_id, std::bitset<CLSTIM_NUM_PARAMS>& summary)
{
    HookView* hv = CyclopsCanvas::getHookView(node_id);
    jassert(hv != nullptr);
    hv->makeSummary(summary);
    summary.set(CLSTIM_MAP_CH, unicastGetChannelMapStatus(node_id));
    return hv->hookInfo.get();
}

CyclopsCanvas::Listener* CyclopsCanvas::findListenerById(CyclopsCanvas* cc, int nodeId)
{
    auto& listener_list = cc->canvasEventListeners.getListeners();
    for (auto& listener : listener_list){
        if (nodeId == listener->getEditorId())
            return listener;
    }
    return nullptr;
}

} // NAMESPACE cyclops
