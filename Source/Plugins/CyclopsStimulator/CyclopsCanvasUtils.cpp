#include "CyclopsCanvasUtils.h"
#include "CyclopsCanvas.h"

namespace cyclops{
/*
  +###########################################################################+
  ||                             LED CHANNEL PORT                            ||
  +###########################################################################+
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
        testButtons[i]->setEnabled(false);
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
        api::CyclopsRPC rpc;
        if (api::test(&rpc, test_index)){
            canvas->disableAllInputWidgets();
            canvas->broadcastEditorInteractivity(CanvasEvent::FREEZE);
            for (int i=0; i<4; i++)
                testButtons[i]->setEnabled(false);
            CoreServices::sendStatusMessage("Testing LED channel " + String(test_index) + "...");
            canvas->in_a_test = true;            
            canvas->progressBar->setTextToDisplay("Testing LED channel" + String(test_index));
            canvas->progressBar->setVisible(true);
            startTimer(20);
            canvas->serialInfo.Serial->writeBytes(rpc.message, rpc.length);
            test_index = -1;
        }
        else{
            CoreServices::sendStatusMessage("CyclopsAPI::test [Error] Invalid channel index");
        }
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
        if (canvas->progress >= 1){
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
    if (x > 20 && y > 5){
        // in Rect
        index = jmin(3.0f, y*5/(float)height);
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
    if(mouseOverIndex >= 0)
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

/*
  +###########################################################################+
  ||                               HOOK VIEWPORT                             ||
  +###########################################################################+
*/

HookViewport::HookViewport(HookViewDisplay* display) : hvDisplay(display)
{
    setViewedComponent(hvDisplay, false);
    setScrollBarsShown(true, false);
}

bool HookViewport::getLinkPathSource(int nodeId, Point<int>& result)
{
    HookView* targetView = CyclopsCanvas::getHookView(nodeId);
    if (targetView == nullptr || !(hvDisplay->isParentOf(targetView))) {
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

/*
  +###########################################################################+
  ||                             HOOK VIEW DISPLAY                           ||
  +###########################################################################+
*/

HookViewDisplay::HookViewDisplay(CyclopsCanvas* _canvas) : canvas(_canvas)
                                                         , height(45)
{
}

void HookViewDisplay::paint(Graphics& g)
{
    //DBG ("painting hvd\n");
    g.fillAll(Colours::darkgrey);
}

void HookViewDisplay::refresh()
{
    height = 5;
    shownIds.clear();
    CyclopsCanvas::getEditorIds(canvas, shownIds);
    //DBG ("Now showing " << shownIds.size());
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        jassert(hv != nullptr);
        jassert(isParentOf(hv));
        hv->setBounds(5, height, getWidth(), jmax(45, hv->getHeight()));
        height += jmax(45, hv->getHeight()) + 5;
        hv->repaint();
    }
    //DBG (". Height: " << height\n");
    setSize(getWidth(), height);
    repaint();
}

void HookViewDisplay::resized()
{
    //DBG ("resizing hvd to " << getHeight() << "\n");
    for (int i=0; i<shownIds.size(); i++){
        HookView* hv = CyclopsCanvas::getHookView(shownIds[i]);
        jassert(hv != nullptr);
        jassert(isParentOf(hv));
        hv->setSize(getWidth(), jmax(45, hv->getHeight()));
    }
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

/*
  +###########################################################################+
  ||                                  HOOK INFO                              ||
  +###########################################################################+
*/

HookInfo::HookInfo(int node_id) : nodeId(node_id)
                                , LEDChannel(-1)
                                , pluginInfo(nullptr)
{
    ;
}

/*
  +###########################################################################+
  ||                                HOOK CONNECTOR                           ||
  +###########################################################################+
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
    if (hookView->hookInfo->pluginInfo == nullptr || hookView->hookInfo->LEDChannel > -1)
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

/*
  +###########################################################################+
  ||                                  HOOK VIEW                              ||
  +###########################################################################+
*/
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
    parent->canvas->unicastPluginSelected(CanvasEvent::PLUGIN_SELECTED, nodeId);
    parent->canvas->unicastUpdatePluginInfo(nodeId);
    //DBG (cb->getSelectedItemIndex() << "\n");
    String name = cb->getItemText(cb->getSelectedItemIndex());
    hookInfo->pluginInfo = CyclopsCanvas::pluginManager->getInfo(name.toStdString());

    // remove any selected Labels
    codeLabels.clear();
    signalLabels.clear();
    // resize the selectionMap, "zero" it in loop
    hookInfo->selectedSignals.resize(hookInfo->pluginInfo->signalCount);
    // add sourceCodeLabels
    std::vector<std::string>* codeNames = &hookInfo->pluginInfo->signalCodeNames;
    for (int i=0; i < hookInfo->pluginInfo->signalCount; i++){
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
        g.fillRect(235, 0, 5+150+2+123, hookInfo->pluginInfo->signalCount*20+10);

        for (int i=0; i < hookInfo->pluginInfo->signalCount; i++){
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

void HookView::makeSummary(std::bitset<CLSTIM_NUM_PARAMS>& summary)
{
    if (hookInfo->pluginInfo == nullptr)
        return;
    // LED output port is connected?
    if (hookInfo->LEDChannel > -1){
        summary.set(CLSTIM_MAP_LED);
    }
    // sanity check assertion, ignore.
    jassert(hookInfo->pluginInfo->signalCount == (int)hookInfo->selectedSignals.size());
    // SignalMap completely configured?
    if (! (std::find(hookInfo->selectedSignals.begin(), hookInfo->selectedSignals.end(), -1) != hookInfo->selectedSignals.end()) ){
        summary.set(CLSTIM_MAP_SIG);
    }
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
    signalRect.addRoundedRectangle(235, 2, 285, hookInfo->pluginInfo->signalCount*20+6, 4);
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
    if (x > 240 && y > 5 && y < hookInfo->pluginInfo->signalCount*20+5){
        // in signalRect
        index = (y-5)/20;
        // sanity check!
        jassert (index < hookInfo->pluginInfo->signalCount);
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

/*
  +###########################################################################+
  ||                               SIGNAL BUTTON                             ||
  +###########################################################################+
*/

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

/*
  +###########################################################################+
  ||                                 SIGNAL VIEW                             ||
  +###########################################################################+
*/

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

/*
  +###########################################################################+
  ||                              SIGNAL DISPLAY                             ||
  +###########################################################################+
*/

SignalDisplay::SignalDisplay(CyclopsCanvas *cc) : canvas(cc)
                                                , isDragging(false)
{
    File sigFile = getSignalsFile("cyclops_plugins/signals.yaml");
    std::cout << "*CL* Fetching signals.yaml from `" << sigFile.getFullPathName() << "`\n";
    if (!sigFile.existsAsFile()){
        std::cout << "*CL* Signals File not found! Expected @ Builds/Linux/build/cyclops_plugins/signals.yaml\n";
        std::cout << "*CL* Perhaps you forgot to compile Cyclops (sub) Plugins?\n" << std::endl;
        jassert(false);
    }
    else{
        std::ifstream inFile(sigFile.getFullPathName().toStdString());
        if (inFile){
            CyclopsSignal::readSignals(inFile);
            std::cout << "*CL* Signals Collection created!\n\n";
        }
        else{
            std::cout << "*CL* Error in opening `Builds/Linux/build/cyclops_plugins/signals.yaml`\nCheck if you have permissions to this file.\n" << std::endl;
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
    std::cout << "*CL* <Signal Details>\n";
    std::cout << cs->name << "::T" << cs->type << " Points: " << cs->size << std::endl;
    int length=0;
    for (int i=0; i<cs->size; i++) length += cs->holdTime[i];
    std::cout << "Length (ms): " << length << std::endl;
}

void SignalDisplay::dragging(SignalButton* sb)
{
    if (!isDragging){
        //DBG ("start\n");
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
    //DBG ("done\n");
    isDragging = false;
}

void SignalDisplay::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}

void SignalDisplay::resized()
{

}

/*
  +###########################################################################+
  ||                              SIGNAL VIEWPORT                            ||
  +###########################################################################+
*/

SignalViewport::SignalViewport(SignalDisplay* sd) : signalDisplay(sd)
{
    setViewedComponent(signalDisplay, false);
    setScrollBarsShown(true, false);
}

void SignalViewport::paint(Graphics& g)
{
}

/*
  +###########################################################################+
  ||                            MIGRATE COMPONENT                            ||
  +###########################################################################+
*/

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