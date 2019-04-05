#Heli-View

##Initialisation

1. `MainWindow.cpp` creates `UI/UIComponent`
2. `UIComponent` constructor creates `EditorViewport`, `DataViewport`, `PluginList`, `ControlPanel`, `ProcessorGraph` etc.
Calls [`ProcessorGraph.createDefaultNodes()`](#default-nodes)
Also calls `AccessClass::setUIComponent(this)` which sets up the `AccessClass`
3. When `EditorViewport::itemDropped()`, `ProcessorGraph::createNewProcessor()` creates the `Processor` node and adds it to the pipeline using `juce_AudioProcessorGraph::addNode()`, it then proceeds to create the Editor and returns a reference to `GenericEditor`, which is inserted into the `EditorViewport`.
4. The `GenericEditor` is told about the owner node, and that's how it sets up the `ParameterEditor` and the `ChannelDrawer`.

##Start / Play

The `AudioComponent` is in `Audio/AudioComponent`, interestingly, it is not derived from any JUCE class.
>Interfaces with system audio hardware.
Uses the audio card to generate the callbacks to run the ProcessorGraph
during data acquisition.
Sends output to the audio card for audio monitoring.
Determines the initial size of the sample buffer (crucial for
real-time feedback latency).

When an AudioComponent is created, an `AudioDeviceManager` is initialised.
`AudioComponent.graphPlayer` is an `AudioProcessorPlayer` which is given the `ProcessorGraph` as input.
>Remember, `ProcessorGraph` is itself a `Processor`.

`beginCallbacks()` does: `AudioDeviceManager.addAudioCallback(graphPlayer)`

4. When "play" is pressed, `ControlPanel::buttonClicked()` will
    * `ProcessorGraph::enableProcessors()`
        - `ProcessorGraph` connections are updated, cycle through all signal chains, all processor-nodes and connect them (shows on console).
        - Once all connections are made, the SignalChain cannot be edited.
    * Close `RecordEngine` config. window (if open)
    * `AudioComponent.beginCallbacks()`
2. When "play" is pressed again,
    * `AudioComponent.endCallbacks()`
    * `ProcessorGraph::disbleProcessors()`
        - all processor-nodes are `disable()`.
        - Once all connections are made, the SignalChain cannot be edited.
    * All editors are re-enabled.

#Default Nodes
There are 4 nodes in any graph.

###`on`, `AudioGraphIOProcessor` [JUCE object]
Used to *"play"* the graph.

###`recn`, `RecordNode`
This guy is responsible for all file writes.

###`an`, `AudioNode` [JUCE object]

>The default processor for sending output to the audio monitor. The `ProcessorGraph` has two default nodes: the `AudioNode` and the RecordNode.
Every channel of every processor (that's not a sink or a utility) is automatically connected to both of these nodes. The `AudioNode` is used to filter out channels to be sent to the audio output device, which can be selected by the user through the `AudioEditor` (located in the ControlPanel).

>Since the `AudioNode` exists no matter what, it doesn't appear in the `ProcessorList`. Instead, it's created by the `ProcessorGraph` at startup.

>Each processor has an "Audio" tab within its channel-selector drawer that determines which channels will be monitored. At the moment's there's no centralized way to control the channels going to the audio monitor; it all happens in a distributed way through the individual processors.

###msgCenter, `MessageCenter`

#Processor Functioning, Signal Passing

Signal passing is handled by AudioIO Callbacks implemented via AudioGraphPlayer. It magically passes buffers of continuous `AudioSampleBuffer` and `MIDIBuffer` to and from processors.
All processors derived from `GenericProcessor` have a `process(AudioSampleBuffer, MIDIBuffer)` thingy (which is actually `AudioProcessor::processBlock(...)`).

#Ask later

####`RHD2000Thread.cpp`
timerCallback() and startAcquisition() seemingly buggy code?

# Misc

[Arbitrary waveform generator](http://www.bkprecision.com/support/downloads/function-and-arbitrary-waveform-generator-guidebook.html)
