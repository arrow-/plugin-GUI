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

    devStatus = new IndicatorLED(CyclopsColours::disconnected, Colours::black);
    addChildComponent(devStatus);

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
    pstep = 0.01;
    // communicate with teensy?
    refreshPlugins();
}

void CyclopsCanvas::refreshPlugins()
{
    if (pluginManager->getNumPlugins() == 0){
        std::cout << "CPM> Making Cyclops-Plugin List" << std::endl;
        pluginManager->loadAllPlugins();
        std::cout << "CPM> Loaded " << pluginManager->getNumPlugins() << " cyclops plugin(s)." << std::endl;
    }
}

CyclopsCanvas::~CyclopsCanvas()
{
    
}

void CyclopsCanvas::beginAnimation()
{
    std::cout << "CyclopsCanvas beginning animation." << std::endl;
    disableAllInputWidgets();
    if (serialInfo.portName != ""){
        // serialInfo->Serial.writeByte(<launch>);
        devStatus->update(CyclopsColours::connected, "Device is in sync!");
        devStatus->setVisible(true);
        devStatus->repaint();
    }
    startTimer(40);
}

void CyclopsCanvas::endAnimation()
{
    std::cout << "CyclopsCanvas ending animation." << std::endl;
    enableAllInputWidgets();
    devStatus->setVisible(false);
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
    std::cout << "Updating CyclopsCanvas" << std::endl;
}


void CyclopsCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    repaint();
}

void CyclopsCanvas::resized()
{
    //std::cout << "resizing canvas" << std::endl;
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
    std::cout << "migrating " << node_id << " to Cyclops B" << dest->realIndex << std::endl;
    // update combo selection
    for (int i=0; i<CyclopsCanvas::canvasList.size(); i++){
        if (CyclopsCanvas::canvasList[i]->realIndex == dest->realIndex)
            break;
    }
    listener->changeCanvas(dest);
    listener->updateButtons(CanvasEvent::COMBO_BUTTON, true);

    src->removeListener(listener);
    HookView* hv = CyclopsCanvas::getHookView(node_id);
    jassert(hv != nullptr);
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
    listener->updateIndicators(CanvasEvent::TRANSFER_DROP);
    listener->changeCanvas(nullptr);
}

void CyclopsCanvas::refresh()
{
    //std::cout << "refreshing hvd" << std::endl;
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

bool CyclopsCanvas::isReady()
{
    return hookViewDisplay->isReady() && serialInfo.portName != "";
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
    int response;
    while (serialInfo.Serial->available() > 0){
        response = serialInfo.Serial->readByte();
        switch (response){
        case 0:   //CL_RC_LAUNCH
        break;

        case 1:   //CL_RC_END
        break;

        case 8:   //CL_RC_SBDONE
        break;

        case 9:   //CL_RC_IDENTITY
        break;

        case 16:  //CL_RC_MBDONE
        break;

        case 240: //CL_RC_EA_FAIL
        case 241: //CL_RC_NEA_FAIL
            devStatus->update(CyclopsColours::notResponding, "Device sent an ErrorCode.");
            devStatus->repaint();
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
    if (serialInfo.portName != ""){
        serialInfo.Serial->setup(serialInfo.portName, serialInfo.baudRate);
        CyclopsRPC rpc;
        identify(&rpc);
        serialInfo.Serial->writeBytes(rpc.message, rpc.length);
        
        unsigned char identity[RPC_IDENTITY_SZ+1] = {0};
        while (serialInfo.Serial->available() < RPC_IDENTITY_SZ);
        serialInfo.Serial->readBytes(identity, RPC_IDENTITY_SZ);

        for (int i=0; i<RPC_IDENTITY_SZ-1-14; i++)
            std::cout << identity[i];
        std::cout << identity[52] << identity[55] << identity[58] << identity[61] << std::endl;
        /*
        if (identity[52] == '0')
        if (identity[55] == '0')
        if (identity[58] == '0')
        if (identity[61] == '0')
        */
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

void CyclopsCanvas::addHook(int node_id)
{
    HookView* hv = CyclopsCanvas::hookViews.add(new HookView(node_id));
    hookViewDisplay->addAndMakeVisible(hv);
    //std::cout << "added hook" << std::endl;
}

bool CyclopsCanvas::removeHook(int node_id)
{
    HookView* hv = getHookView(node_id);
    if (hv == nullptr)
        return false;
    hookViewDisplay->shownIds.removeFirstMatchingValue(node_id);
    CyclopsCanvas::hookViews.removeObject(hv, true); // deletes
    return true;
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

bool CyclopsCanvas::isReady(int node_id)
{
    HookView* hv = CyclopsCanvas::getHookView(node_id);
    return hv->isReady();
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

void CyclopsCanvas::unicastPluginIndicator(CanvasEvent pluginState, int node_id)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(this, node_id);
    listener->updateIndicators(CanvasEvent::PLUGIN_SELECTED);
}

void CyclopsCanvas::unicastUpdatePluginInfo(int node_id)
{
    CyclopsCanvas::Listener* listener = CyclopsCanvas::findListenerById(this, node_id);
    listener->refreshPluginInfo();
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

CyclopsCanvas::Listener* CyclopsCanvas::findListenerById(CyclopsCanvas* cc, int nodeId)
{
    auto& listener_list = cc->canvasEventListeners.getListeners();
    for (auto& listener : listener_list){
        if (nodeId == listener->getEditorId())
            return listener;
    }
    return nullptr;
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

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                               LED CHANNEL PORT                                |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/


LEDChannelPort::LEDChannelPort(CyclopsCanvas* parent) : canvas(parent)
                                                      , mouseOverIndex(-1)
                                                      , isDragging(false)
                                                      , dragShouldDraw(true)
{
    Image img(Image::ARGB, 21, 21, true);
    Graphics g(img);
    g.setColour(Colours::black);
    g.drawEllipse(1, 1, 16, 16, 2);
    g.drawEllipse(5, 5, 8, 8, 4);
    // Add buttons
    for (int i=0; i < 4; i++){
        testButtons.add(new UtilityButton(String("Test ") + String(i), Font("Default", 11, Font::bold)));
        testButtons[i]->addListener(this);
        addAndMakeVisible(testButtons[i]);
        
        ImageButton* imgButton = LEDButtons.add(new ImageButton());
        imgButton->setImages( true, true, true
                            , img, 0.9, Colours::transparentBlack
                            , img, 0.7, Colours::lightgrey
                            , img, 1, Colours::darkgrey);
        imgButton->addListener(this);
        addAndMakeVisible(imgButton);

        connections.add(-1);
    }
}

void LEDChannelPort::paint(Graphics &g)
{
    int heightBlock = jmax(70, getHeight()/5),
        width = getWidth();
    g.setColour(Colours::black);
    bool draw = false;
    for (int i=0; i<4; i++){
        if (connections[i] > -1){
            if (mouseOverIndex > -1 && mouseOverIndex == i){
                canvas->hideLink(mouseOverIndex);
            }
            draw = true;
        }
        else if (mouseOverIndex > -1 && mouseOverIndex == i){
            draw = true;
        }
        if (draw){
            g.setColour(Colours::black);
        }
        else{
            g.setColour(Colour(0xff6e6e6e));
        }
        g.fillRect(0, heightBlock*(i+1)-25, width/2.0+2, 4);
        g.setColour(Colours::black);
        draw = false;
    }
}

void LEDChannelPort::buttonClicked(Button* button)
{
    int test_index = -1;
    for (int i=0; i < 4; i++){
        if (button == testButtons[i]){
            test_index = i;
            break;
        }
    }
    if (test_index >= 0){
        canvas->disableAllInputWidgets();
        canvas->broadcastEditorInteractivity(CanvasEvent::FREEZE);
        std::cout << "Testing LED channel " << test_index << "\n";
        canvas->in_a_test = true;
        //serialInfo.Serial->testChannel(test_index);
        canvas->progressBar->setTextToDisplay("Testing LED channel" + String(test_index));
        canvas->progressBar->setVisible(true);
        startTimer(20);
        test_index = -1;
        for (int i=0; i<4; i++)
            testButtons[i]->setEnabled(false);
    }
    else{
        test_index = -1;
        for (int i=0; i < 4; i++){
            if (button == LEDButtons[i]){
                test_index = i;
                break;
            }
        }
        if (test_index >= 0 && connections[test_index] > -1){
            HookView* hv = CyclopsCanvas::getHookView(connections[test_index]);
            hv->hookInfo->LEDChannel = -1;
            connections.set(test_index, -1);
            canvas->removeLink(test_index);
            repaint();
            hv->repaint();
            canvas->redrawLinks();
        }
    }
}

void LEDChannelPort::timerCallback()
{
    if (canvas->in_a_test){
        canvas->progress += canvas->pstep;
        if (canvas->progress >= 1.0){
            canvas->progressBar->setVisible(false);
            canvas->progress = 0;
            canvas->in_a_test = false;
            stopTimer();
            for (int i=0; i<4; i++)
                testButtons[i]->setEnabled(true);
            canvas->enableAllInputWidgets();
            canvas->broadcastEditorInteractivity(CanvasEvent::THAW);
        }
    }
}

void LEDChannelPort::resized()
{
    int heightBlock = (getHeight()/5);
    for (int i=0; i < 4; i++){
        testButtons[i]->setBounds(10, jmax(70, heightBlock)*(i+1)-(25/2), 50, 25);
        LEDButtons[i]->setBounds(getWidth()/2.0-10, jmax(70, heightBlock)*(i+1)-32, 20, 20);
    }
}

int LEDChannelPort::getIndexfromXY(const Point<int>& pos)
{
    int height = getHeight(),
        x = pos.getX(),
        y = pos.getY(),
        index = -1;
    if (x > 20 && y > 5 && y < 4*height/5){
        // in Rect
        index = (y/(float)height)*5;
        // sanity check!
        jassert (index < 4);
    }
    return index;
}

bool LEDChannelPort::getLinkPathDest(int ledChannel, Point<int>& result)
{
    int h = jmax(getHeight()/5, 70)*(ledChannel+1) - 32+9;
    result.setX(0);
    result.setY(h);
    return true;
}

bool LEDChannelPort::isInterestedInDragSource(const SourceDetails& dragSouceDetails)
{
    dragDescription = dragSouceDetails.description.getArray();
    jassert(dragDescription != nullptr);
    if (dragDescription->getUnchecked(1).toString().startsWith("hookViewConnector"))
        return true;
    dragDescription = nullptr;
    return false;
}

void LEDChannelPort::itemDragEnter(const SourceDetails& dragSouceDetails)
{
    isDragging = true;
}

void LEDChannelPort::itemDragMove(const SourceDetails& dragSouceDetails)
{
    mouseOverIndex = getIndexfromXY(dragSouceDetails.localPosition);
    jassert(dragDescription != nullptr);
    if (mouseOverIndex > -1)
        dragShouldDraw = false;
    else
        dragShouldDraw = true;
    repaint();
}

void LEDChannelPort::itemDragExit(const SourceDetails& dragSouceDetails)
{
    isDragging = false;
    dragDescription = nullptr;
    dragShouldDraw = true;
    mouseOverIndex = -1;
    repaint();
}

void LEDChannelPort::itemDropped(const SourceDetails& dragSouceDetails)
{
    addConnection(dragSouceDetails.sourceComponent);
    isDragging = false;
    dragDescription = nullptr;
    dragShouldDraw = true;
    mouseOverIndex = -1;
    repaint();
}

bool LEDChannelPort::shouldDrawDragImageWhenOver()
{
    return dragShouldDraw;
}

void LEDChannelPort::addConnection(Component* dragSourceComponent)
{
    HookView* hv = dynamic_cast<HookView*>(dragSourceComponent);
    jassert(hv != nullptr);
    if (connections[mouseOverIndex] > -1){
        canvas->removeLink(mouseOverIndex);
        HookView* old_hv = CyclopsCanvas::getHookView(connections[mouseOverIndex]);
        jassert(old_hv != nullptr);
        old_hv->hookInfo->LEDChannel = -1;
    }
    hv->hookInfo->LEDChannel = mouseOverIndex;
    connections.set(mouseOverIndex, hv->nodeId);
    canvas->redrawLinks();
}

















































HookViewport::HookViewport(HookViewDisplay* display) : hvDisplay(display)
{
    setViewedComponent(hvDisplay, false);
    setScrollBarsShown(true, false);
}

bool HookViewport::getLinkPathSource(int nodeId, Point<int>& result)
{
    HookView* targetView = CyclopsCanvas::getHookView(nodeId);
    if (targetView == nullptr){
        // This HookView was just now deleted!, remove Links if any.
        return false;
    }
    jassert(hvDisplay->isParentOf(targetView));
    int top = getViewPositionY();
    int height = 5;
    for (int i=0; i<hvDisplay->shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(hvDisplay->shownIds[i]);
        height += hv->getHeight();
        if (hv == targetView)
            break;
        height += 5;
    }
    int resY = height - top - targetView->getHeight()/2;
    result.setX(getMaximumVisibleWidth()-1);
    if (resY < 0)
        result.setY(4);
    else if (resY > getViewHeight())
        result.setY(getViewHeight()-2);
    else
        result.setY(resY);
    return true;
}

void HookViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
    hvDisplay->canvas->redrawLinks();
}

void HookViewport::paint(Graphics& g)
{
}

















HookViewDisplay::HookViewDisplay(CyclopsCanvas* _canvas) : canvas(_canvas)
                                                         , height(45)
{
}

void HookViewDisplay::paint(Graphics& g)
{
    //std::cout << "painting hvd" << std::endl;
    g.fillAll(Colours::darkgrey);
}

void HookViewDisplay::refresh()
{
    height = 5;
    shownIds.clear();
    CyclopsCanvas::getEditorIds(canvas, shownIds);
    //std::cout << "Now showing " << shownIds.size();
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        jassert(hv != nullptr);
        jassert(isParentOf(hv));
        hv->setBounds(5, height, getWidth(), jmax(45, hv->getHeight()));
        height += jmax(45, hv->getHeight()) + 5;
        hv->repaint();
    }
    //std::cout << ". Height: " << height << std::endl;
    setSize(getWidth(), height);
    repaint();
}

void HookViewDisplay::resized()
{
    //std::cout << "resizing hvd to " << getHeight() << std::endl;
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        jassert(hv != nullptr);
        jassert(isParentOf(hv));
        hv->setSize(getWidth(), jmax(45, hv->getHeight()));
    }
}

bool HookViewDisplay::isReady()
{
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        if (!hv->isReady())
            return false;
    }
    return true;
}

void HookViewDisplay::disableAllInputWidgets()
{
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        hv->disableAllInputWidgets();
    }
}

void HookViewDisplay::enableAllInputWidgets()
{
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        hv->enableAllInputWidgets();
    }
}













HookInfo::HookInfo(int node_id) : nodeId(node_id)
                                , LEDChannel(-1)
                                , pluginInfo(nullptr)
{
    ;
}



/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                HOOK-CONNECTOR                                 |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

HookConnector::HookConnector(HookView* hv) : isDragging(false)
                                           , dragEnded(false)
                                           , hookView(hv)
{    
}

void HookConnector::resized()
{
    setBounds(jmax(235+5+150+2+123, hookView->getWidth()-40), 0, 40, hookView->getHeight());
}

void HookConnector::paint(Graphics &g)
{
    g.fillAll(Colours::darkgrey);
    int height = getHeight();
    if (isDragging || hookView->hookInfo->LEDChannel > -1){
        g.setColour(Colours::black);
        g.fillRect(18, height/2-1, 30, 4);
    }/*
    else if (dragEnded){
        if (hookView->hookInfo->LEDChannel < 0){
            g.setColour(Colours::green);
            g.fillRect(18, height/2.0-2, 30, 4);
        }
        dragEnded = false;
    }*/
    g.setColour(Colours::black);
    g.drawEllipse(20 - 8, height/2.0 - 8, 16, 16, 2);
    g.drawEllipse(20 - 4, height/2.0 - 4, 8, 8, 4);
}

void HookConnector::mouseDrag(const MouseEvent &event)
{
    if (hookView->hookInfo->LEDChannel > -1)
        return;
    Array<var> dragData;
    dragData.add(true); // user doing this live, false implies load from XML
    dragData.add("hookViewConnector");
    dragData.add(hookView->nodeId);

    Image img (Image::ARGB, 20, 20, true);
    Graphics g (img);
    g.setColour(Colours::red);
    g.fillEllipse(0, 0, 20, 20);
    CyclopsCanvas* canvas = hookView->getParentDisplay()->canvas;
    canvas->startDragging(dragData, hookView, img);
    isDragging = true;
}

void HookConnector::mouseUp(const MouseEvent &event)
{
    isDragging = false;
    dragEnded = true;
    repaint();
}




HookView::HookView(int node_id) : nodeId(node_id)
                                , dragShouldDraw(true)
                                , isDragging(false)
                                , offset(false)
                                , dragDescription(nullptr)
                                , signalRectStroke(new PathStrokeType(1))

{
    hookIdLabel = new Label("hook_id", String(nodeId));
    hookIdLabel->setFont(Font("Default", 16, Font::plain));
    hookIdLabel->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(hookIdLabel);

    hookConnector = new HookConnector(this);
    addAndMakeVisible(hookConnector);

    pluginSelect = new ComboBox();
    pluginSelect->setTooltip("Select the sub-plugin for this \"hook\".");
    StringArray nameList;
    CyclopsCanvas::pluginManager->getPluginNames(nameList);
    pluginSelect->addItemList(nameList, 1);
    pluginSelect->setTextWhenNothingSelected("Choose");
    pluginSelect->addListener(this);
    addAndMakeVisible(pluginSelect);

    hookInfo = new HookInfo(nodeId);
    setSize(80, 45);
}

void HookView::comboBoxChanged(ComboBox* cb)
{
    HookViewDisplay* parent = getParentDisplay();
    parent->canvas->unicastPluginIndicator(CanvasEvent::PLUGIN_SELECTED, nodeId);
    parent->canvas->unicastUpdatePluginInfo(nodeId);
    //std::cout << cb->getSelectedItemIndex() <<std::endl;
    String name = cb->getItemText(cb->getSelectedItemIndex());
    hookInfo->pluginInfo = CyclopsCanvas::pluginManager->getInfo(name.toStdString());

    // remove any selected Labels
    codeLabels.clear();
    signalLabels.clear();
    // resize the selectionMap, "zero" it in loop
    hookInfo->selectedSignals.resize(hookInfo->pluginInfo->sourceCount);
    // add sourceCodeLabels
    std::vector<std::string>* codeNames = &hookInfo->pluginInfo->sourceCodeNames;
    for (int i=0; i < hookInfo->pluginInfo->sourceCount; i++){
        Label* l = codeLabels.add(new Label("codelabel", String(codeNames->at(i))));
        l->setFont(Font("Default", 16, Font::plain));
        l->setColour(Label::textColourId, Colours::black);
        addAndMakeVisible(l);

        // pre-make the selectedSignalLabels, this vastly simplifies access to these labels
        l = signalLabels.add(new Label("siglabel", "."));
        l->setFont(Font("Default", 15, Font::plain));
        l->setColour(Label::textColourId, Colours::black);
        l->setBounds(240+145, 5+20*i, 130, 18);
        addChildComponent(l);

        hookInfo->selectedSignals[i] = -1;
    }    
    // set sizes
    setSize(parent->getWidth()-80, jmax(45, 20+20*codeLabels.size()));
    parent->refresh();
    hookConnector->resized();
    prepareForDrag();
}

void HookView::paint(Graphics& g)
{
    g.fillAll(Colours::lightgrey);
    int mouseIndex = -1;
    if (!dragShouldDraw)
        mouseIndex = getIndexfromXY(getMouseXYRelative());
    if (hookInfo->pluginInfo != nullptr){
        // flushing away drawing
        g.setFillType(FillType(Colours::lightgrey));
        g.fillRect(235, 0, 5+150+2+123, hookInfo->pluginInfo->sourceCount*20+10);

        for (int i=0; i < hookInfo->pluginInfo->sourceCount; i++){
            // drawing gradients
            switch (getCodeType(i)) {
                case 0:
                    g.setGradientFill(ColourGradient( Colour(0xFFf06292)
                                                    , 240, 0
                                                    , Colour(0x00f06292)
                                                    , 240+150, 0
                                                    , false));
                break;
                case 1:
                    g.setGradientFill(ColourGradient( Colour(0xFFffd54f)
                                                    , 240, 0
                                                    , Colour(0x00ffd54f)
                                                    , 240+150, 0
                                                    , false));
                break;
                case 2:
                    g.setGradientFill(ColourGradient( Colour(0xFFa7ffeb)
                                                    , 240, 0
                                                    , Colour(0x00a7ffeb)
                                                    , 240+150, 0
                                                    , false));
                break;
                default:
                    g.setFillType(FillType(Colours::lightgrey));
            }
            // the actual gradient rect
            g.fillRect(240, 5+20*i, 150, 18);

            switch (getCodeType(i)){
                case 0:
                    g.setFillType(FillType(Colour(0xFFe91e63)));
                    break;
                case 1:
                    g.setFillType(FillType(Colour(0xFFffc107)));
                    break;
                case 2:
                    g.setFillType(FillType(Colour(0xFF1de9b6)));
                    break;
                default:
                    g.setFillType(FillType(Colours::lightgrey));
            }
            if (i == mouseIndex){
                // hiding the label under the mouse
                signalLabels[i]->setVisible(false);
                g.fillRect(240+142, 7+20*i, 80, 16);
                Path p;
                p.addRectangle(240+142, 7+20*i, 80, 16);
                float dashOriginal[] = {5, 3, 1, 3};
                float dashOffset[] = {1, 3, 5, 3};
                if (offset)
                    signalRectStroke->createDashedStroke(p, p, dashOffset, 4);
                else
                    signalRectStroke->createDashedStroke(p, p, dashOriginal, 4);

                g.setFillType(FillType(Colours::black));
                g.fillPath(p);
                g.drawFittedText( dragDescription->getUnchecked(4).toString()
                                , 240+144, 5+20*i
                                , 78, 20
                                , Justification::verticallyCentred | Justification::left
                                , 1, 1.0);
            }
            else if (i != mouseIndex && hookInfo->selectedSignals[i] > -1){
                signalLabels[i]->setVisible(true);
                // draw background rectangle
                g.fillRoundedRectangle(240+142, 7+20*i, 133, 16, 3);
            }
        }
        if (isDragging){
            g.setFillType(FillType(Colours::black));
            g.fillPath(signalRect);
        }
    }
}

void HookView::resized()
{
    hookIdLabel->setBounds(5, 0, 35, 30);
    pluginSelect->setBounds(40, 2, 180, 30);
    hookConnector->resized();
    int index = 0;
    for (auto& label : codeLabels){
        label->setBounds(240, 5+20*(index++), 150, 20);
    }
}

void HookView::refresh()
{
    // parse all things in HookInfo
    if (hookInfo->pluginInfo != nullptr){
        prepareForDrag();
    }
    repaint();
}

bool HookView::isReady()
{
    if (hookInfo->pluginInfo == nullptr)
        return false;
    return true;
}

void HookView::timerCallback()
{
    offset = !offset;
    prepareForDrag(offset);
    repaint();
}

bool HookView::isInterestedInDragSource(const SourceDetails& dragSouceDetails)
{
    dragDescription = dragSouceDetails.description.getArray();
    jassert(dragDescription != nullptr);
    if (hookInfo->pluginInfo != nullptr && dragDescription->getUnchecked(1).toString().startsWith("signalButton"))
        return true;
    dragDescription = nullptr;
    dragShouldDraw = true;
    return false;
}

void HookView::itemDragEnter(const SourceDetails& dragSouceDetails)
{
    isDragging = true;
    startTimer(300);
}

void HookView::itemDragMove(const SourceDetails& dragSouceDetails)
{
    int index = getIndexfromXY(dragSouceDetails.localPosition);
    jassert(dragDescription != nullptr);
    if (index > -1 && getCodeType(index) == (int)dragDescription->getUnchecked(3))
        dragShouldDraw = false;
    else
        dragShouldDraw = true;
    repaint();
}

void HookView::itemDragExit(const SourceDetails& dragSouceDetails)
{
    isDragging = false;
    dragDescription = nullptr;
    dragShouldDraw = true;
    stopTimer();
    repaint();
}

void HookView::itemDropped(const SourceDetails& dragSouceDetails)
{
    addSignal(dragSouceDetails.localPosition);
    isDragging = false;
    dragDescription = nullptr;
    dragShouldDraw = true;
    stopTimer();
    repaint();
}

bool HookView::shouldDrawDragImageWhenOver()
{
    return dragShouldDraw;
}


void HookView::disableAllInputWidgets()
{
    pluginSelect->setEnabled(false);
}

void HookView::enableAllInputWidgets()
{
    pluginSelect->setEnabled(true);
}

void HookView::prepareForDrag(int offset /* = 0 */)
{
    signalRect.clear();
    signalRect.addRoundedRectangle(235, 2, 285, hookInfo->pluginInfo->sourceCount*20+6, 4);
    float dashOriginal[] = {6, 3, 2, 3};
    float dashOffset[] = {2, 3, 6, 3};
    if (offset)
        signalRectStroke->createDashedStroke(signalRect, signalRect, dashOffset, 4);
    else
        signalRectStroke->createDashedStroke(signalRect, signalRect, dashOriginal, 4);
}


int HookView::getIndexfromXY(const Point<int>& pos)
{
    int x = pos.getX(),
        y = pos.getY(),
        index = -1;
    if (x > 240 && y > 5 && y < hookInfo->pluginInfo->sourceCount*20+5){
        // in signalRect
        index = (y-5)/20;
        // sanity check!
        jassert (index < hookInfo->pluginInfo->sourceCount);
    }
    return index;
}

void HookView::addSignal(const Point<int>& pos)
{
    int index = getIndexfromXY(pos);
    if (index > -1 && !dragShouldDraw){
        hookInfo->selectedSignals[index] = (int)dragDescription->getUnchecked(2);
        Label* l = signalLabels[index];
        l->setText(dragDescription->getUnchecked(4).toString(), dontSendNotification);
        l->setVisible(true);
    }
}

int  HookView::getCodeType(int index)
{
    return (int) (hookInfo->pluginInfo->sourceCodeTypes[index]);
}

HookViewDisplay* HookView::getParentDisplay()
{
    return findParentComponentOfClass<HookViewDisplay>();
}











SignalButton::SignalButton(int index, SignalView* parent) : ShapeButton(String(index), Colours::black, Colours::black, Colours::black)
                                                          , signalIndex(index)
{
    roundedRect.addRoundedRectangle(0, 0, 150, 26, 2);
    setShape(roundedRect, true, false, false);
    CyclopsSignal* cs = CyclopsSignal::signals.getUnchecked(signalIndex);
    text = cs->name;
    parentView = parent;
    switch (cs->type){
        case 0: // STORED
        setColours( Colour(0xFFe91e63)
                  , Colour(0xFFf06292)
                  , Colour(0xFFad1457));
        break;
        case 1: // GENERATED
        setColours( Colour(0xFFffc107)
                  , Colour(0xFFffd54f)
                  , Colour(0xFFff8f00));
        break;
        case 2: // SQUARE
        setColours( Colour(0xFF1de9b6)
                  , Colour(0xFFa7ffeb)
                  , Colour(0xFF00bfa5));
        break;
    }
    setOutline(Colours::black, 1.0);
    setTooltip("Press to view details, Drag above to Hook Settings");
}

void SignalButton::mouseDrag(const MouseEvent& e)
{
    ShapeButton::mouseDrag(e);
    parentView->dragging(this);
}

void SignalButton::mouseUp(const MouseEvent& e)
{
    ShapeButton::mouseUp(e);
    parentView->dragDone(this);
}

void SignalButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    ShapeButton::paintButton(g, isMouseOverButton, isButtonDown);
    g.drawFittedText( text
                    , (isButtonDown)? 10 : 5
                    , 1
                    , 150 - ((isButtonDown)? 10 : 5)
                    , 24
                    , Justification::verticallyCentred | Justification::left
                    , 1
                    , 1.0);
}



SignalView::SignalView(int index, SignalDisplay *parent) : signalIndex(index)
                                                         , parentDisplay(parent)
{
    signalButton = new SignalButton(signalIndex, this);
    addAndMakeVisible(signalButton);
    signalButton->addListener(this);
    signalButton->setBounds(0, 0, 150, 26);
    setSize(300, 30);
}

void SignalView::buttonClicked(Button* btn)
{
    parentDisplay->showDetails(signalIndex);
}

void SignalView::mouseDrag(const MouseEvent& e)
{
    parentDisplay->dragging(signalButton);
}

void SignalView::mouseUp(const MouseEvent& e)
{
    parentDisplay->dragDone(signalButton);
}

void SignalView::paint(Graphics& g)
{
    //g.fillAll(Colours::white);
}

void SignalView::dragging(SignalButton* sb)
{
    parentDisplay->dragging(sb);
}

void SignalView::dragDone(SignalButton* sb)
{
    parentDisplay->dragDone(sb);
}

SignalDisplay::SignalDisplay(CyclopsCanvas *cc) : canvas(cc)
                                                , isDragging(false)
{
    File sigFile = getSignalsFile("cyclops_plugins/signals.yaml");
    std::cout << "Fecthing signals.yaml from `" << sigFile.getFullPathName() << "`\n";
    if (!sigFile.existsAsFile()){
        std::cout << "Signals File not found! Expected @ Builds/Linux/build/cyclops_plugins/signals.yaml";
        std::cout << "\nPerhaps you forgot to compile Cyclops (sub) Plugins?\n" << std::endl;
        jassert(false);
    }
    else{
        std::ifstream inFile(sigFile.getFullPathName().toStdString());
        if (inFile){
            CyclopsSignal::readSignals(inFile);
            std::cout << "Signals Collection created!\n\n";
        }
        else{
            std::cout << "Error in opening `Builds/Linux/build/cyclops_plugins/signals.yaml`\nCheck if you have permissions to this file.\n" << std::endl;
            jassert(false);
        }
        inFile.close();
    }
    for (int i=0; i<CyclopsSignal::signals.size(); i++){
        SignalView *sv = signalViews.add(new SignalView(i, this));
        sv->setBounds(25, 5+sv->getHeight()*i, sv->getWidth(), sv->getHeight());
        addAndMakeVisible(sv);
    }
    setSize(300, CyclopsSignal::signals.size()*30);
}

void SignalDisplay::showDetails(int index)
{
    CyclopsSignal* cs = CyclopsSignal::signals.getUnchecked(index);
    std::cout << std::endl << cs->name << "::T" << cs->type << " Points: " << cs->size << std::endl;
    int length=0;
    for (int i=0; i<cs->size; i++) length += cs->holdTime[i];
    std::cout << "Length (ms): " << length << std::endl;
}

void SignalDisplay::dragging(SignalButton* sb)
{
    if (!isDragging){
        //std::cout << "start" << std::endl;
        isDragging = true;
        int index = sb->signalIndex;
        CyclopsSignal* cs = CyclopsSignal::signals.getUnchecked(index);

        Array<var> dragData;
        dragData.add(true); // user doing this live, false implies load from XML
        dragData.add("signalButton");
        dragData.add(index);
        dragData.add(cs->type);
        dragData.add(String(cs->name));
        canvas->startDragging(dragData, sb);
    }
}

void SignalDisplay::dragDone(SignalButton* sb)
{
    //std::cout << "done" << std::endl;
    isDragging = false;
}

void SignalDisplay::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}

void SignalDisplay::resized()
{

}









SignalViewport::SignalViewport(SignalDisplay* sd) : signalDisplay(sd)
{
    setViewedComponent(signalDisplay, false);
    setScrollBarsShown(true, false);
}

void SignalViewport::paint(Graphics& g)
{
}









MigrateComponent::MigrateComponent(CyclopsCanvas* closing_canvas) : closingCanvas(closing_canvas)
{
    group = new GroupComponent ("group", "Select Hooks");
    addAndMakeVisible(group);

    canvasCombo = new ComboBox("target_canvas");

    allEditorsButton = new ToggleButton("All");
    addAndMakeVisible(allEditorsButton);
    allEditorsButton->addListener(this);

    int num_canvases = CyclopsCanvas::getNumCanvas();
    CyclopsCanvas::getEditorIds(closingCanvas, editorIdList);
    for (auto& editorId : editorIdList){
        ToggleButton* tb = editorButtonList.add(new ToggleButton(String(editorId)));
        //tb->setRadioGroupId(editorId);
        tb->setToggleState(false, dontSendNotification);
        addAndMakeVisible(tb);
        tb->addListener(this);
    }
    
    comboText = new Label("combo label", "Migrate to");
    comboText->setFont(Font("Default", 16, Font::plain));
    comboText->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(comboText);

    for (int i=0; i<num_canvases; i++){
        if (CyclopsCanvas::canvasList[i]->realIndex != closingCanvas->realIndex){
            canvasCombo->addItem("Cyclops" + String(CyclopsCanvas::canvasList[i]->realIndex), i+1);
        }
    }
    addAndMakeVisible(canvasCombo);
    canvasCombo->addListener(this);

    doneButton = new UtilityButton("DONE", Font("Default", 12, Font::plain));
    doneButton->addListener(this);
    doneButton->setEnabled(false);
    cancelButton = new UtilityButton("CANCEL", Font("Default", 12, Font::plain));
    cancelButton->addListener(this);
    addAndMakeVisible(doneButton);
    addAndMakeVisible(cancelButton);

    setSize(300, 60+30+40+30+25*editorButtonList.size());
}

void MigrateComponent::resized()
{
    int width = getWidth(), height = getHeight(), i=0;
    allEditorsButton->setBounds(width/2-200/2+10, 25, 60, 20);
    for (auto& tb : editorButtonList){
        tb->setBounds(width/2-200/2+10, 25+30+i*22, 60, 20);
        i++;
    }
    group->setBounds(width/2-200/2, 10, 200, 25+30*(i)+20);
    comboText->setBounds(width/2-100/2, height-100, 100, 25);
    canvasCombo->setBounds(width/2-90/2, height-70, 90, 25);
    doneButton->setBounds((width-80*2)/3, height-30, 80, 25);
    cancelButton->setBounds((width-80*2)*2/3+80, height-30, 80, 25);
}

void MigrateComponent::buttonClicked(Button* button)
{
    if (button == doneButton) {
        CyclopsCanvas *newCanvas = CyclopsCanvas::canvasList.getUnchecked(canvasCombo->getSelectedId()-1);
        for (int i=0; i<editorIdList.size(); i++){
            // this will changeCanvas and update SelectorButtons of src->listeners only
            if (editorButtonList[i]->getToggleState()){
                jassert(CyclopsCanvas::migrateEditor(newCanvas, closingCanvas, editorIdList[i]) == 0);
            }
            else{
                CyclopsCanvas::dropEditor(closingCanvas, editorIdList[i]);
            }
        }
        newCanvas->refresh();
        // remove tab if open
        // remove window if open
        // tell all that they need to update combo-list
        CyclopsCanvas::canvasList.removeObject(closingCanvas, true);
        for (auto& canvas : CyclopsCanvas::canvasList){
            canvas->broadcastButtonState(CanvasEvent::COMBO_BUTTON, true);
        }
        closeWindow();
    }
    else if (button == cancelButton) {
        closeWindow();
    }
    else if ( button == allEditorsButton) {
        for (auto& editorButton : editorButtonList){
            editorButton->setToggleState(true, dontSendNotification);
        }
    }
    // must be one in editorButtonList, do nothing for that (except maybe
    // 'highlight" in  EditorViewport?)
    else {
        allEditorsButton->setToggleState(false, dontSendNotification);
    }
    
}

void MigrateComponent::comboBoxChanged(ComboBox* cb)
{
    if (cb == canvasCombo){
        doneButton->setEnabled(true);
    }
}

void MigrateComponent::closeWindow()
{
    for (auto& canvas : CyclopsCanvas::canvasList){
        canvas->broadcastButtonState(CanvasEvent::COMBO_BUTTON, true);
        canvas->enableAllInputWidgets();
        canvas->broadcastEditorInteractivity(CanvasEvent::THAW);
    }
    
    if (DialogWindow* dw = findParentComponentOfClass<DialogWindow>())
        dw->exitModalState(0);
}

} // NAMESPACE cyclops
