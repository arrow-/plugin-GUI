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

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>
#include <EditorHeaders.h>
#include <SerialLib.h>
#include <string>

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
class HookInfo;
class HookView;
class HookViewDisplay;
class HookViewport;
class SignalDisplay;
class SignalViewport;

/**
 * @brief      Holds UI widgets for Cyclops.
 */
class CyclopsCanvas : public Visualizer
                    , public Button::Listener
                    , public ComboBox::Listener
{
public:

    class Listener{
    public:
        virtual void updateIndicators(CanvasEvent LEDtype) = 0;
        virtual CyclopsPluginInfo* refreshPluginInfo() = 0;
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
    
    bool isReady();

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

    void broadcastButtonState(CanvasEvent whichButton, bool state);
    void broadcastEditorInteractivity(CanvasEvent interactivity);
    void unicastPluginIndicator(CanvasEvent pluginState, int node_id);
    void unicastUpdatePluginInfo(int node_id);
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
    static bool isReady(int node_id);
    
    static ScopedPointer<CyclopsPluginManager> pluginManager;
    static OwnedArray<CyclopsCanvas> canvasList;
    static OwnedArray<HookView> hookViews;

    int tabIndex;  /** < This is used for VisualizerEditor, don't try to use this
                    * variable, unless absolutely sure that you understand it's
                    * purpose. This does not appear on the tab. */
    int realIndex; /** < This is the "real" name of the Canvas, which appears on the
                    * tab, dropwdowns, etc. */
    ScopedPointer<DataWindow> dataWindow;
    // serial object
    cl_serial serialInfo;

private:
    
    // GUI stuff
    ScopedPointer<IndicatorLED>  devStatus;
    ScopedPointer<UtilityButton> refreshButton; /**< Button that reloads device list */
    ScopedPointer<ComboBox> portCombo;          /**< List of all available dvices */
    ScopedPointer<ComboBox> baudrateCombo;      /**< List of all available baudrates. */
    OwnedArray<UtilityButton> testButtons;      /**< TEST Buttons */

    ScopedPointer<HookViewDisplay> hookViewDisplay;
    ScopedPointer<HookViewport> hookViewport;

    ScopedPointer<SignalDisplay> signalDisplay;
    ScopedPointer<SignalViewport> signalViewport;

    ScopedPointer<ProgressBar> progressBar;
    ScopedPointer<UtilityButton> closeButton;   /**< Used to close a canvas */
    // Some state vars for "TEST" UI
    double progress, pstep;
    bool in_a_test;

    // listeners
    ListenerList<Listener> canvasEventListeners;

    static const int BAUDRATES[12];
    static int numCanvases;

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








class HookViewport : public Viewport
{
public:
    HookViewport(HookViewDisplay* display);
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
    bool isReady();
    void disableAllInputWidgets();
    void enableAllInputWidgets();

    Array<int> shownIds;
    CyclopsCanvas* canvas;
};











class HookInfo{
public:
    int nodeId;
    CyclopsPluginInfo* pluginInfo;
    HookInfo(int node_id);
};







class HookView : public Component
               , public ComboBox::Listener
{
public:
    int nodeId;
    ScopedPointer<Label> hookIdLabel;
    ScopedPointer<ComboBox> pluginSelect;
    ScopedPointer<HookInfo> hookInfo;

    HookView(int node_id);
    void comboBoxChanged(ComboBox* cb);
    void refresh();
    void paint(Graphics& g);
    void resized();
    bool isReady();
    void disableAllInputWidgets();
    void enableAllInputWidgets();
};












class SignalDisplay : public Component
{
public:
    SignalDisplay(CyclopsCanvas *cc);
    void resized();
    void paint(Graphics& g);

    CyclopsCanvas *canvas;
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
