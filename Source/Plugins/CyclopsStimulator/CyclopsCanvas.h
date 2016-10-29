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

#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>

#include "CyclopsCanvasUtils.h"
#include "plugin_manager/CLPluginManager.h"
#include "code_gen/Programs.h"

/**
 * @brief      Module to interface with and control the Cyclops LED Driver for
 *             automated Optogenetic (feedback) experiments.
 * @defgroup   ns-cyclops cyclops
 */
namespace cyclops {

class CyclopsProgram;

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
        virtual void updateSerialIndicator(CanvasEvent event) = 0;
        virtual void updateReadinessIndicator(CanvasEvent event, int attribute=0) = 0;
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
    /**
     * @brief      Sets the serial device that will be used during acquisition.
     *             It also tries communicating with the device, to determine
     *             it's identity and status.
     *
     * @param[in]  device  The device name. Follows the usual format:
     *                     * ``/dev/ttyXXX``
     *                     * ``/dev/COMY``
     */
    void setDevice(string device);

    /**
     * @brief      Gets the device identity.
     * @details    Sends a CyclopsAPI ``identify`` packet, waits 3 seconds for a
     *             reply. Opens a pop-up to ask what kind of device is
     *             connected, in case the device _could not be identified_.
     *
     * @return     ``true`` if identity can be verified.
     */
    bool getDeviceIdentity();
    
    /**
     * @brief      Sets the baudrate that will be used during acquisition. Calls
     *             ``setDevice``.
     *
     * @param[in]  baudrate  The baudrate
     * @sa         CyclopsCanvas::setDevice
     */
    void setBaudrate(int baudrate);

    /**
     * @brief      Gets the serial information.
     * @details    Exposes the private data as a constant struct.
     * @return     The serial information.
     */
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

    /**
     * @brief      Gets all hook-summaries and their info and places them in the
     *             provided containers.
     * @details    This is called just before code-generation, to
     *             find out which "hooks" are *primed* and if they are, what is
     *             their config.
     *
     * @param      hookInfoList  The hook-information list
     * @param      summaries     The list of corresponding summary bitsets
     */
    void getAllSummaries(std::vector<code::CyclopsHookConfig>& hookInfoList, Array<std::bitset<CLSTIM_NUM_PARAMS> >& summaries);
    
    /**
     * @brief      Gets the summary for a "hook" -- the options that have been
     *             configured by the user.
     * @attention  This is only called by CyclopsEditor::isReadyForLaunch,
     *             there's _another_ *static function* with same name!
     *
     * @param[in]  node_id   The CyclopsEditor Identifier
     * @param      isPrimed  Indicates if the hook is _completely primed
     *                       (configured)_, and ready to produce code
     *
     * @return     Returns ``true`` if we should start producing code for this
     *             canvas. Note that all "hooks" need to be verified by the
     *             ``ProcessorGraph::enableProcessors()``, to generate code. The
     *             actual summary is indicated by ``isPrimed``.
     * 
     * @sa         CyclopsEditor::isReadyForLaunch
     */
    bool getSummary(int node_id, bool& isPrimed);

    /**
     * @brief      Attempts to generate the code, and a unique hash-code for the
     *             current configuration.
     *
     * @param      genError  The error-code representing error during code
     *                       generation. ``0`` if no error occured, ``Positive``
     *                       otherwise.
     *
     * @return     ``true`` if we must move to the next step in
     *             "experiment-launch" (building), ``false`` otherwise.
     */
    bool generateCode(int& genError);

    /**
     * @brief      Attempts to invoke ``make`` to compile the generated code in
     *             a child process.
     *
     * @param      buildError  The error-code representing error during
     *                         compilation. ``0`` if no error  occured,
     *                         ``Positive`` otherwise.
     *
     * @return     ``true`` if we must move to the next step in
     *             "experiment-launch"" (flashing), ``false`` otherwise.
     */
    bool buildCode(int& buildError);

    /**
     * @brief      Reads the status of ``CyclopsCanvas::program``, and attempts
     *             flashing.
     *
     * @param      flashError  The error-code to represent flashing failure.
     *                         Positive if flashing failed.
     *
     * @return     ``true`` if we must move to the next step in the
     *             "experiment-launch" (launching), ``false`` otherwise.
     */
    bool flashDevice(int& flashError);

    /**
     * @brief      Removes an existing link from the "hook" to _the_ LED channel.
     *
     * @param[in]  ledChannel  The led channel
     */
    void removeLink(int ledChannel);

    /**
     * @brief      Hides the (existing) link from the "hook" to _the_ LED channel.
     *
     * @param[in]  ledChannel  The led channel
     */
    void hideLink(int ledChannel);

    /**
     * @brief      Helper that redraws all links to the LED channels.
     */
    void redrawLinks();

    void broadcastButtonState(CanvasEvent whichButton, bool state);
    void broadcastEditorInteractivity(CanvasEvent interactivity);
    void broadcastIndicatorLED(int LEDtype, CanvasEvent event, int attribute=0);
    void unicastPluginSelected(CanvasEvent pluginState, int node_id);
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

    /**
     * @brief      Gets the hook view, searching for it by the CyclopsEditor ID.
     *
     * @param[in]  node_id  The CyclopsEditor (node)node identifier
     *
     * @return     The hook view.
     */
    static HookView* getHookView(int node_id);

    /**
     * @brief      Disconnects ``closingCanvas`` from the CyclopsEditor with ID
     *             = ``node_id``
     *
     * @param      closingCanvas  The (pointer to) closing canvas
     * @param[in]  node_id        The CyclopsEditor (node) identifier
     */
    static void dropEditor(CyclopsCanvas* closingCanvas, int node_id);
    
    /**
     * @brief      Migrates the CyclopsEditor (identified by the ``listener``)
     *             from ``src`` canvas to ``dest``.
     *
     * @param      dest        The destination
     * @param      src         The source
     * @param      listener    The listener, identifies the CyclopsEditor.
     * @param[in]  refreshNow  whether to refresh the UI elements _ASAP_ or not.
     *
     * @return     ``0`` always.
     * @todo       Handle errors here. Convert ``jasserts`` to actual error checks.
     */
    static int migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, CyclopsCanvas::Listener* listener, bool refreshNow=true);
    
    /**
     * @brief      Migrates the CyclopsEditor (identified by the ``listener``)
     *             from ``src`` canvas to ``dest``.
     * @details    Does not flush the UI elements _ASAP_, allowing you to
     *             migrate many, and then refresh just once.
     *
     * @param      dest    The destination
     * @param      src     The source
     * @param[in]  nodeId  The CyclopsEditor (node) identifier that needs to be
     *                     transferred.
     *
     * @return     { description_of_the_return_value }
     */
    static int migrateEditor(CyclopsCanvas* dest, CyclopsCanvas* src, int nodeId);
    
    /**
     * Provides access to Cyclops Sub Plugins.
     */
    static ScopedPointer<CyclopsPluginManager> pluginManager;

    /**
     * _Owns_ all CyclopsCanvas instances, allows canvases to communicate.
     */
    static OwnedArray<CyclopsCanvas> canvasList;

    /**
     * Hooks are not owned by the canvases in which they exist -- they are just
     * connected to the canvases. This greatly smplifies migration.
     */
    static OwnedArray<HookView> hookViews;

    /** This is used for VisualizerEditor, don't try to use this variable,
     * unless absolutely sure that you understand it's purpose. This does not
     * appear on the tab. */
    int tabIndex;

    /** This is the "real" name of the Canvas, which appears on the tab,
     * dropwdowns, etc. */
    int realIndex;
    /** The dataWindow is _owned_ by this canvas, unlike DataWindows of other
     * plugins. This is the only way editors can "share" it. Editors
     * create/destroy DataWindow when the SelectorButton is pressed or the
     * editor is deleted. When the DataWindow is owned by an editor, how would
     * it pass ownership when it is deleted, so that window is not destroyed
     * with it, to a sibling editor (if any)? <br> We simply let the canvas own
     * it. Unfortunately, the window can only be created and destroyed by
     * VisualizerEditor only (not the Canvas), and for those operations we pass
     * ownership around.
     */
    ScopedPointer<DataWindow> dataWindow;
    
    /** Contains information on the serial port config, as well as the Serial
     * Object. */
    cl_serial serialInfo;
    /** Contains information about the Cyclops device which was found on the
    selected serial port. */
    ScopedPointer<CyclopsDeviceInfo> CLDevInfo;
    /**
     * This is set to ``true`` if the device on the selected serial port is
     * sucessfully "identified". Set to ``flase`` otherwise.
     * @sa CyclopsCanvas::setDevice CyclopsCanvas::flashDevice CyclopsCanvas::buttonClicked
     */
    bool serialIsVerified;
    
    ScopedPointer<LEDChannelPort> ledChannelPort;
    /**
     * @brief      Shows the progress of a long operation.
     * @todo       Currently used only during "TEST", expand it's use.
     */
    ScopedPointer<ProgressBar> progressBar;
    // Some state vars for "TEST" UI
    double progress, pstep;
    bool in_a_test;
    
    /**
     * Owns the links graphic that connects "hooks" to LED %Channels.
     */
    OwnedArray<Path> linkPaths;
    
    ScopedPointer<code::CyclopsProgram> program;
    // Code Generation Decision Array
    std::map<int, bool> decisionMap;
private:
    
    // GUI stuff
    /** Button that reloads device list
     */
    ScopedPointer<UtilityButton> refreshButton;
    /** List of all available dvices 
     */
    ScopedPointer<ComboBox> portCombo;

    /** List of all available baudrates.
     */
    ScopedPointer<ComboBox> baudrateCombo;

    ScopedPointer<Label> hookLabel;
    ScopedPointer<HookViewDisplay> hookViewDisplay;
    ScopedPointer<HookViewport> hookViewport;
    
    ScopedPointer<Label> sigLabel;
    ScopedPointer<SignalDisplay> signalDisplay;
    ScopedPointer<SignalViewport> signalViewport;

    /**< Used to close a canvas 
     */
    ScopedPointer<UtilityButton> closeButton;    

    /** Listeners of events on the Canvas, includes all the CyclopsEditors that
     *  are _connected_ to this canvas.
     */
    ListenerList<Listener> canvasEventListeners;

    static const int BAUDRATES[12];
    
    /** Just used to provide a unique ``CyclopsCanvas::realIndex``
     */
    static int numCanvases;

    /**
     * @brief      Filters only relevant serial ports (by name).
     *
     * @param[in]  portName  The port name
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
    
    /**
     * @brief      Gets the summary of the "hook", *as well as* the "HookInfo"
     *             identified by the CyclopsEditor (node) Identifier.
     *
     * @param[in]  node_id  The CyclopsEditor (node) identifier
     * @param      summary  The summary (is placed in this container)
     *
     * @return     The HookInfo contents.
     */
    HookInfo* getSummary(int node_id, std::bitset<CLSTIM_NUM_PARAMS>& summary);

    static CyclopsCanvas::Listener* findListenerById(CyclopsCanvas* cc, int nodeId);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsCanvas);
};


/*
  +###########################################################################+
  ||                             LED CHANNEL PORT                            ||
  +###########################################################################+
*/
/**
 * @brief      Holds the LED %Channel UI elements:
 *             * Test Buttons
 *             * LED link buttons
 * @details    Allows user to route any sub-plugin's output to any channel on
 *             the cyclops device.
 */
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
    OwnedArray<UtilityButton> testButtons; /**< TEST Buttons @todo Change the disabled state colour. */
    OwnedArray<ImageButton> LEDButtons;
    Array<int> connections; /**< Maps led-channel to nodeID */
private:
    int getIndexfromXY(const Point<int>& pos);
    void addConnection(Component*);

    Array<var>* dragDescription;
    int mouseOverIndex; /**< The LED channel which is under the mouse. */
    bool isDragging, dragShouldDraw;
};


/*
  +###########################################################################+
  ||                              HOOK VIEW PORT                             ||
  +###########################################################################+
*/
/**
 * @brief      Holds the HookView objects in a scrollable port in a
 *             HookViewDisplay component.
 */
class HookViewport : public Viewport
{
public:
    HookViewport(HookViewDisplay* display);
    bool getLinkPathSource(int nodeId, Point<int>& result);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);
    void paint(Graphics& g);
private:
    HookViewDisplay* hvDisplay; /**< This component is displayed in side the Viewport. */
};

/*
  +###########################################################################+
  ||                             HOOK VIEW DISPLAY                           ||
  +###########################################################################+
*/
/**
 * @brief      This component just displays HookView Components, without owning
 *             them.
 * @details    The HookViews are statically owned by (all) CyclopsCanvases. This
 *             makes it super-easy to migrate them from one display to another.
 *             
 *             It keeps track of the "displayed" HookViews using just an integer
 *             array: ``shownIDs``.  
 *             _Care is taken to ensure a HookView is displayed in only one display!_
 */
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

    Array<int> shownIds; /**< Keeps track of the HookViews currently displayed by the display. */
    CyclopsCanvas* canvas;
    int height;
};

/*
  +###########################################################################+
  ||                                HOOK INFO                                ||
  +###########################################################################+
*/
/**
 * @brief      The actual data that all GUI elements are displaying/obtaining
 *             from user eventually comes to the HookInfo.
 */
class HookInfo{
public:
    int nodeId, /**< The CyclopsEditor (node) Identifier. */
        LEDChannel; /**< The LED %Channel that this sub-plugin controls. */
    CyclopsPluginInfo* pluginInfo; /**< Provided from CyclopsPluginManager */
    /**
     * The program reads a list of CyclopsSignals from the computer
     * (signals.yaml), into the CyclopsSignals::signals list.
     * 
     * As the user _chooses_ the kᵗʰ signal from CyclopsSignals::signals and 
     * drops it in the mᵗʰ position of the HookView,
     * ```
     * selectedSignals[m] = k;
     * ```
     */
    std::vector<int> selectedSignals;
    HookInfo(int node_id);
};

/*
  +###########################################################################+
  ||                             HOOK CONNECTOR                              ||
  +###########################################################################+
*/
/**
 * @brief      The draggable wire-reel which is used to connect a HookView to an
 *             LED %Channel.
 */
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

/*
  +###########################################################################+
  ||                                 HOOK VIEW                               ||
  +###########################################################################+
*/
/**
 * @brief      The HookView bridges CyclopsSignals and the CyclopsPluginManager and
 *             allows the user to configure all aspects of the sub-plugin.
 * @details    Feature list:
 *             * Update CyclopsEditor LED indicators when a plugin is selected.
 *             * Allow user to drop a compatible signal into signal slots.
 *             * Connect hook to an LED channel.
 * @sa         CyclopsSignal, CyclopsPluginManager
 */
class HookView : public Component
               , public ComboBox::Listener
               , public DragAndDropTarget
               , public Timer
{
public:
    int nodeId;
    /**
     * To display the Cyclops Editor (node) Identifier.
     */
    ScopedPointer<Label> hookIdLabel;
    /**
     * Dropdown to choose a sub-plugin. Poulated by the CyclopsPluginManager.
     */
    ScopedPointer<ComboBox> pluginSelect;
    /**
     * Holds all configuration info of this sub-plugin.
     */
    ScopedPointer<HookInfo> hookInfo;
    /**
     * The draggable wire-reel, to connect HookView and LED %Channel.
     */
    ScopedPointer<HookConnector> hookConnector;
    OwnedArray<Label> codeLabels;
    OwnedArray<Label> signalLabels;

    /**
     * @brief      { function_description }
     *
     * @param[in]  node_id  The node identifier
     */
    HookView(int node_id);
    /**
     * @brief      Catches the "setting" of a sub-plugin to update the LED
     *             Indicator on the Hook.
     *
     * @param      cb    The ComboBox that was manipualted.
     */
    void comboBoxChanged(ComboBox* cb);
    void refresh();
    void paint(Graphics& g);
    void resized();
    /**
     * @brief      Makes a "summary" of the configuration of this Hook.
     * @details    This is used to verify if the user has completely configured
     *             the Hook or not. The user needs to perform 3 actions:
     *             1. Choose all input event channels on the CyclopsEditor.
     *             2. Select the (all) "Signals" (ie drag-n-drop them).
     *             3. Connect the HookView to an LED %Channel.
     *
     *             ``makeSummary`` checks if (2) and (3) are done, and sets
     *             accordingly, the 1ˢᵗ or the 2ⁿᵈ bit in the ``bitset``.
     *             The 3ʳᵈ bit is set by CyclopsEditor::channelMapStatus.
     * @sa         CyclopsEditor::channelMapStatus
     *
     * @param      summary  This ``bitset`` is _filled_.
     */
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
    Path signalRect; // A dashed rectangle appears while dragging.
    ScopedPointer<PathStrokeType> signalRectStroke;

    void prepareForDrag(int offset = 0);
    int  getIndexfromXY(const Point<int>& pos);
    void addSignal(const Point<int>& pos);
    int  getCodeType(int index);
};


/*
  +###########################################################################+
  ||                               SIGNAL BUTTON                             ||
  +###########################################################################+
*/
/**
 * @brief      Each SignalButton represents a Signal that was read from the
 *             ``signals.yaml`` file.
 * @details    Clicking the button will display/present information/graph of the
 *             Signal.
 */
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

/*
  +###########################################################################+
  ||                                SIGNAL VIEW                              ||
  +###########################################################################+
*/
/**
 * @brief      Each SignalView is basically just a SignalButton.
 * @details    More UI elements will be added to SignalView, like SignalGraph,
 *             edit buttons, etc.
 */
class SignalView : public Component
                 , public Button::Listener
{
public:
    ScopedPointer<SignalButton> signalButton; /**< The button. */
    int signalIndex;
    /**
     * @brief      Just creates and places the SignalButton.
     *
     * @param[in]  index   The index of the SignalButton in
     *                     CyclopsSignal::signals.
     * @param      parent  The parent SignalDisplay component.
     */
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

/*
  +###########################################################################+
  ||                              SIGNAL DISPLAY                             ||
  +###########################################################################+
*/
/**
 * @brief      Holds the list of SignalViews.
 */
class SignalDisplay : public Component
{
public:
    /**
     * @brief      Constructs the CyclopsSignals::signals by reading
     *             ``signals.yaml``, and invoking CyclopsSignal::readSignals.
     *
     * @param      cc    The parent CyclopsCanvas.
     */
    SignalDisplay(CyclopsCanvas *cc);
    /**
     * @brief      Helper function that pretty prints the CyclopsSignal
     *             information.
     *
     * @param[in]  index  The index of the CyclopsSignal in the
     *                    CyclopsSignal::signals list.
     */
    void showDetails(int index);
    void resized();
    void paint(Graphics& g);
    void dragging(SignalButton* sb);
    void dragDone(SignalButton* sb);

    OwnedArray<SignalView> signalViews; /**< Holds all the SignalViews. */
    CyclopsCanvas *canvas; /** < The parent Canvas. */
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

/*
  +###########################################################################+
  ||                                SIGNAL VIEWPORT                          ||
  +###########################################################################+
*/
/**
 * @brief      Just makes the SignalDisplay scrollable.
 */
class SignalViewport : public Viewport
{
public:
    /**
     * @brief      Constructor.
     * @details    Turns on vertical scrollbar by default.
     * @param      sd    The SignalDisplay that is to be displayed.
     */
    SignalViewport(SignalDisplay* sd);
    void paint(Graphics& g);
private:
    SignalDisplay* signalDisplay;
};

/*
  +###########################################################################+
  ||                             MIGRATE COMPONENT                           ||
  +###########################################################################+
*/
/**
 * @brief      Migrate Component shows itself in a AlertWindow, when a Canvas is
 *             closed by the user, to help him/her _migrate_ or _drop_ the
 *             HookViews in the canvas.
 */
class MigrateComponent : public Component
                       , public Button::Listener
                       , public ComboBox::Listener
{
public:
    MigrateComponent(CyclopsCanvas* closing_canvas);
    void resized();
    /**
     * @brief      Responds to any Button press on the AlertWindow.
     * @details    When "DONE" is pressed,
     *             - Destroy all HookViews which are to be dropped.
     *             - _Move_ to-be-migrated HookViews to the destination Canvas,
     *               by just manipulating ``HookViewDisplay::shownIDs``.
     *             - Refresh both Canvas.
     *             - Destroy source Canvas.
     *             
     * @sa         HookViewDisplay
     * @param      button  The button which was pressed.
     */
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
