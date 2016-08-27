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

#ifndef CYCLOPS_STIM_CANVAS_H
#define CYCLOPS_STIM_CANVAS_H

#define CLSTIM_NUM_PARAMS  3
#define CLSTIM_MAP_CH      0
#define CLSTIM_MAP_SIG     1
#define CLSTIM_MAP_LED     2

#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>
#include <EditorHeaders.h>
#include <SerialLib.h>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <algorithm>

#include "plugin_manager/CLPluginManager.h"

namespace cyclops {
    namespace CyclopsColours{
    const Colour disconnected(0xffff3823);
    const Colour notResponding(0xffffa834);
    const Colour connected(0xffc1d045);
    const Colour notReady       = disconnected;
    const Colour Ready          = connected;
    }

enum class CanvasEvent{
    WINDOW_BUTTON,
    TAB_BUTTON,
    COMBO_BUTTON,
    SERIAL_LED,
    PLUGIN_SELECTED,
    TRANSFER_DROP,
    TRANSFER_MIGRATE,
    FREEZE,
    THAW,
};

struct cl_serial
{
    std::string portName;
    ScopedPointer<ofSerial> Serial;
    int baudRate;

    cl_serial()
    {
        portName = "";
        Serial = new ofSerial();
        baudRate = -1;
    }
};

class IndicatorLED;
class LEDChannelPort;

class HookInfo;
class HookConnector;
class HookView;
class HookViewDisplay;
class HookViewport;

class SignalButton;
class SignalView;
class SignalDisplay;
class SignalViewport;

/**
 * @brief      Holds UI widgets for Cyclops.
 */
class CyclopsCanvas : public Visualizer
                    , public Button::Listener
                    , public ComboBox::Listener
                    , public DragAndDropContainer
{
public:

    class Listener{
    public:
        virtual void updateIndicators(CanvasEvent LEDtype) = 0;
        virtual CyclopsPluginInfo* refreshPluginInfo() = 0;
        virtual bool channelMapStatus() = 0;
        virtual void changeCanvas(CyclopsCanvas* dest) = 0;
        virtual void updateButtons(CanvasEvent whichButton, bool state) = 0;
        virtual void setInteractivity(CanvasEvent interactivity) = 0;
        virtual int  getEditorId() = 0;
    };

    CyclopsCanvas();
    void refreshPlugins();
    ~CyclopsCanvas();

    /** Called when the component's tab becomes visible again.*/
    virtual void refreshState();

    /** Called when parameters of underlying data processor are changed.*/
    virtual void update();

    /** Called instead of "repaint" to avoid redrawing underlying components if not necessary.*/
    virtual void refresh();

    /** Disables all input widgets on the editor. */
    void disableAllInputWidgets();
    /** Enables all input widgets on the editor. */
    void enableAllInputWidgets();

    void paint(Graphics& g);

    /** Called when data acquisition is active.*/
    virtual void beginAnimation();

    /** Called when data acquisition ends.*/
    virtual void endAnimation();

    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, float);

    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, int, int, float);

    void resized();

    void buttonClicked(Button* button);
    void comboBoxChanged(ComboBox* comboBox);
    bool keyPressed(const KeyPress& key);
    /** Called whenever the timer is triggered. */
    void timerCallback();

    CyclopsPluginInfo* getPluginInfoById(int node_id);
    /** Setter, that allows you to set the serial device that will be used during acquisition */
    void setDevice(string device);

    /** Setter, that allows you to set the baudrate that will be used during acquisition */
    void setBaudrate(int baudrate);

    const cl_serial* getSerialInfo();

    void addListener(Listener* newListener);
    void removeListener(Listener* oldListener);

    /**
     * @brief      Adds a _new_ HookView object into the static hookViews array.
     *
     * @param[in]  node_id  The node identifier
     */
    void addHook(int node_id);

    /**
     * @brief      Removes the HookView from the static hookViews array
     *
     * @param[in]  node_id  The node identifier
     *
     * @return     ``true`` if an element was removed, ``false`` otherwise.
     */
    bool removeHook(int node_id);

    void getAllSummaries(std::vector<std::bitset<CLSTIM_NUM_PARAMS> >& summaries);
    bool getSummary(int node_id, bool& isPrimed);
    void getSummary(int node_id, std::bitset<CLSTIM_NUM_PARAMS>& summary);

    bool generateCode(int& genError);
    bool flashDevice(int& flashError);

    void removeLink(int ledChannel);
    void hideLink(int ledChannel);
    void redrawLinks();

    void broadcastButtonState(CanvasEvent whichButton, bool state);
    void broadcastEditorInteractivity(CanvasEvent interactivity);
    void unicastPluginIndicator(CanvasEvent pluginState, int node_id);
    void unicastUpdatePluginInfo(int node_id);
    bool unicastGetChannelMapStatus(int node_id);
    int  getNumListeners();

    /** Saves parameters as XML */
    virtual void saveVisualizerParameters(XmlElement* xml);

    /** Loads parameters from XML */
    virtual void loadVisualizerParameters(XmlElement* xml);

    static void broadcastNewCanvas();
    static int getNumCanvas();
    /**
     * @brief      Gets the editor identifiers.
     *
     * @param      cc            Canvas
     * @param      editorIdList  The editor identifier list
     */
    static void getEditorIds(CyclopsCanvas* cc, Array<int>& editorIdList);
    static HookView* getHookView(int node_id);
    static void dropEditor(CyclopsCanvas* closingCanvas, int node_id);
    static int migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, CyclopsCanvas::Listener* listener, bool refreshNow=true);
    static int migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, int nodeId);
    
    static ScopedPointer<CyclopsPluginManager> pluginManager;
    static OwnedArray<CyclopsCanvas> canvasList;
    static OwnedArray<HookView> hookViews;

    int tabIndex;  /** < This is used for VisualizerEditor, don't try to use this
                    * variable, unless absolutely sure that you understand it's
                    * purpose. This does not appear on the tab. */
    int realIndex; /** < This is the "real" name of the Canvas, which appears on the
                    * tab, dropwdowns, etc. */
    ScopedPointer<DataWindow> dataWindow; /** < The dataWindow is _owned_ by this canvas, unlike DataWindows of
                                           * other plugins. This is the only way editors can "share" it.
                                           * Editors create/destroy DataWindow when the SelectorButton is
                                           * pressed or the editor is deleted. When the DataWindow is owned by
                                           * an editor, how would it pass ownership when it is deleted, so
                                           * that window is not destroyed with it, to a sibling editor (if
                                           * any)? <br> We simply let the canvas own it. Unfortunately, the
                                           * window can only be created and destroyed by VisualizerEditor only (not
                                           * the Canvas), and for those operations we pass ownership around. */
    cl_serial serialInfo;   /** < Contains information on the serial port config, as well as the
                             * Serial Object. */

    ScopedPointer<LEDChannelPort> ledChannelPort;

    ScopedPointer<ProgressBar> progressBar;
    // Some state vars for "TEST" UI
    double progress, pstep;
    bool in_a_test;
    // LED links
    OwnedArray<Path> linkPaths;

    // Code Generation Decision Array
    std::map<int, bool> decisionMap;
private:
    
    // GUI stuff
    ScopedPointer<IndicatorLED>  devStatus;
    ScopedPointer<UtilityButton> refreshButton; /**< Button that reloads device list */
    ScopedPointer<ComboBox> portCombo;          /**< List of all available dvices */
    ScopedPointer<ComboBox> baudrateCombo;      /**< List of all available baudrates. */

    ScopedPointer<Label> hookLabel;
    ScopedPointer<HookViewDisplay> hookViewDisplay;
    ScopedPointer<HookViewport> hookViewport;
    
    ScopedPointer<Label> sigLabel;
    ScopedPointer<SignalDisplay> signalDisplay;
    ScopedPointer<SignalViewport> signalViewport;

    ScopedPointer<UtilityButton> closeButton;   /**< Used to close a canvas */    

    // listeners
    ListenerList<Listener> canvasEventListeners;

    static const int BAUDRATES[12];
    static int numCanvases;   /**< Just used to provide a unique ``CyclopsCanvas::realIndex`` */

    /**
     * @brief      Filters only relevant serial ports (by name).
     *
     * @return     ``true`` if a Teensy or Arduino could be connected.
     */
    bool screenLikelyNames(const String& portName);
    /**
     * @brief      Returns a list of all serial devices that are available on
     *             the system. The list of available devices changes whenever
     *             devices are connected or removed.
     */
    StringArray getDevices();

    /**
     * @brief      Returns a list of all supported baudrates.
     */
    Array<int> getBaudrates();

    /**
     * @brief      Updates the "points" of the path linking the HookView with an
     *             LED port
     *
     * @param[in]  ledChannel  The LED ``channel``
     * @param[in]  src         source of path, right edge of HookView
     * @param[in]  dest        destination of path, LEDChannelPort.
     */
    void updateLink(int ledChannel, Point<int> src, Point<int> dest);
    
    static CyclopsCanvas::Listener* findListenerById(CyclopsCanvas* cc, int nodeId);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsCanvas);
};



















class IndicatorLED : public Component
                   , public SettableTooltipClient
{
public:
    IndicatorLED (const Colour& fill, const Colour& line);
    void paint (Graphics& g);
    void update (const Colour& fill, String tooltip);
    void update (const Colour& fill, const Colour& line, String tooltip);
private:
    Colour fillColour, lineColour;
};


class LEDChannelPort : public Component
                     , public DragAndDropTarget
                     , public Timer
                     , public Button::Listener
{
public:
    LEDChannelPort(CyclopsCanvas* parent);
    void resized();
    void buttonClicked(Button* button);
    void timerCallback();
    void paint(Graphics &g);
    bool getLinkPathDest(int ledChannel, Point<int>& result);

    bool isInterestedInDragSource(const SourceDetails& dragSouceDetails);
    void itemDragEnter(const SourceDetails& dragSouceDetails);
    void itemDragMove(const SourceDetails& dragSouceDetails);
    void itemDragExit(const SourceDetails& dragSouceDetails);
    void itemDropped(const SourceDetails& dragSouceDetails);
    bool shouldDrawDragImageWhenOver();

    CyclopsCanvas* canvas;
    OwnedArray<UtilityButton> testButtons; /**< TEST Buttons */
    OwnedArray<ImageButton> LEDButtons;
    Array<int> connections; // <led-channel> -> <nodeId>
private:
    int getIndexfromXY(const Point<int>& pos);
    void addConnection(Component*);

    Array<var>* dragDescription;
    int mouseOverIndex;
    bool isDragging, dragShouldDraw;
};




















class HookViewport : public Viewport
{
public:
    HookViewport(HookViewDisplay* display);
    bool getLinkPathSource(int nodeId, Point<int>& result);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);
    void paint(Graphics& g);
private:
    HookViewDisplay* hvDisplay;
};






class HookViewDisplay : public Component
{
public:
    HookViewDisplay(CyclopsCanvas* _canvas);
    void addView(HookView* hv);
    void removeView(int node_id);
    void paint(Graphics& g);
    void refresh();
    void resized();
    void disableAllInputWidgets();
    void enableAllInputWidgets();

    Array<int> shownIds;
    CyclopsCanvas* canvas;
    int height;
};











class HookInfo{
public:
    int nodeId, LEDChannel;
    CyclopsPluginInfo* pluginInfo;
    std::vector<int> selectedSignals;
    HookInfo(int node_id);
};


class HookConnector : public Component
{
public:
    HookConnector(HookView* hv);
    void resized();
    void paint(Graphics &g);
    void mouseDrag(const MouseEvent &event);
    void mouseUp(const MouseEvent &event);

    bool isDragging, dragEnded;
private:
    HookView*      hookView;
};





class HookView : public Component
               , public ComboBox::Listener
               , public DragAndDropTarget
               , public Timer
{
public:
    int nodeId;
    ScopedPointer<Label> hookIdLabel;
    ScopedPointer<ComboBox> pluginSelect;
    ScopedPointer<HookInfo> hookInfo;
    ScopedPointer<HookConnector> hookConnector;
    OwnedArray<Label> codeLabels;
    OwnedArray<Label> signalLabels;

    HookView(int node_id);
    void comboBoxChanged(ComboBox* cb);
    void refresh();
    void paint(Graphics& g);
    void resized();
    void makeSummary(std::bitset<CLSTIM_NUM_PARAMS>& summary);

    void timerCallback();

    bool isInterestedInDragSource(const SourceDetails& dragSouceDetails);
    void itemDragEnter(const SourceDetails& dragSouceDetails);
    void itemDragMove(const SourceDetails& dragSouceDetails);
    void itemDragExit(const SourceDetails& dragSouceDetails);
    void itemDropped(const SourceDetails& dragSouceDetails);
    bool shouldDrawDragImageWhenOver();

    void disableAllInputWidgets();
    void enableAllInputWidgets();
    HookViewDisplay* getParentDisplay();
private:
    bool dragShouldDraw,
         isDragging,
         offset;
    Array<var>* dragDescription;
    Path signalRect;
    ScopedPointer<PathStrokeType> signalRectStroke;

    void prepareForDrag(int offset = 0);
    int  getIndexfromXY(const Point<int>& pos);
    void addSignal(const Point<int>& pos);
    int  getCodeType(int index);
};











class SignalButton : public ShapeButton
{
public:
    String text;
    int signalIndex;
    SignalButton(int index, SignalView* parent);
    /**
     * @brief      Called when the button is dragged by the mouse.
     *
     * @param[in]  e     Describes the event
     */
    void mouseDrag(const MouseEvent& e);
    void mouseUp(const MouseEvent& e);
    void paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown);
private:
    Path roundedRect;
    SignalView* parentView;
};



class SignalView : public Component
                 , public Button::Listener
{
public:
    ScopedPointer<SignalButton> signalButton;
    int signalIndex;
    SignalView(int index, SignalDisplay *parent);
    /**
     * @brief      Called when mouse drags the SignalView (which is slightly
     *             bigger than the SignalButton).
     *
     * @param[in]  e     Describes the event
     */
    void mouseDrag(const MouseEvent& e);
    void mouseUp(const MouseEvent& e);
    void buttonClicked(Button *btn);
    void paint(Graphics& g);
    void dragging(SignalButton* sb);
    void dragDone(SignalButton* sb);
private:
    SignalDisplay *parentDisplay;
};

class SignalDisplay : public Component
{
public:
    SignalDisplay(CyclopsCanvas *cc);
    void showDetails(int index);
    void resized();
    void paint(Graphics& g);
    void dragging(SignalButton* sb);
    void dragDone(SignalButton* sb);

    OwnedArray<SignalView> signalViews;
    CyclopsCanvas *canvas;
private:
    bool isDragging;
    static inline File getSignalsFile(const String& pathFromExeDir) {
    // SUPPORT APPLE LATER
    /*
    #if defined(__APPLE__)
        File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
        if (!dir.isDirectory()) {
            dir.createDirectory();
        }
        return std::move(dir);
    #else
        return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
    #endif
    */
        return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory().getChildFile(pathFromExeDir);
    }
};



class SignalViewport : public Viewport
{
public:
    SignalViewport(SignalDisplay* sd);
    void paint(Graphics& g);
private:
    SignalDisplay* signalDisplay;
};













class MigrateComponent : public Component
                       , public Button::Listener
                       , public ComboBox::Listener
{
public:
    MigrateComponent(CyclopsCanvas* closing_canvas);
    void resized();
    void buttonClicked(Button* button);
    void comboBoxChanged(ComboBox* cb);

private:
    CyclopsCanvas* closingCanvas;
    Array<int> editorIdList;
    ScopedPointer<GroupComponent> group;
    ScopedPointer<ToggleButton> allEditorsButton;
    OwnedArray<ToggleButton> editorButtonList;
    ScopedPointer<Label> comboText;
    ScopedPointer<ComboBox> canvasCombo;
    ScopedPointer<UtilityButton> doneButton;
    ScopedPointer<UtilityButton> cancelButton;

    void closeWindow();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MigrateComponent);
};

} // NAMESPACE cyclops

#endif
