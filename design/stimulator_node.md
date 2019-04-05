[TOC]

# User interaction (`cyclops-stimulator` editor)

Editor refers to the editor of `cyclops-stimulator` node.  
Tab refers to the tabbed window of `cyclops-stimulator` node.  
`node` refers to `cyclops-stimulator` node.

**Remember**
> 1. _One_ `node` controls LEDs of only _one_ Cyclops (aka Teensy). `1 node -> 1 teensy`
> 2. _One_ Cyclops may be controlled by _many_ different `nodes`. `1 teensy -> many nodes`
> 3. A plugin may control k channels, `ch1, ch2, ..., chk` but **NO** other plugin (for the same Cyclops) must control any of `ch1, ch2, ..., chk`. `1 channel -> 1 plugin` and `1 plugin -> many channels`
> 4. And obviously, `1 node -> many plugins`

# Step-by-step run through

## 1. Creation of an editor
The list of serial ports is scanned and we attempt to discover a connected cyclops board (possible via `udev` rule or even guess-work).

* (1A) The editor tries to initialize the board, and queries statistics like num of cyclops "boards" connected.
    - Upon failure diagnostic message is shown but the node is created none-the-less. The pipeline would work even if board is not connected, as if the user wants to experiment with the pipeline.
    - There is a switch in the Editor which can virtually disconnect the cyclops, even if everything has been configured.
* (1B) New editor finds multiple boards or multiple `nodes`. Upon creation the **Channel Assignment** popup is opened.
* Waits for user to add `plugin` objects. Once added, a reference is added to a list of _pointers to_ `genericCyclopsPlugin`.
* User can add plugins for channels which are not yet connected to the Teensy, as if the user would do it later.
    - If acquisition/experiment is started without connecting, the `node` will not invoke `handleEvent` for such plugins. As a last measure, `CyclopsAPI` will drop any packets for disconnected channels.

## 2. Assigning channels
Made easy by the UI. There's also a button (which opens a popup) for the rare case when different channels of the same board are being controlled by different `nodes` -- You can exchange channels between `nodes`, and all.

## 3. Assigning `Signals` (aka `Sources`)
Piece of :cake:, drag-n-drop. Overwrites previous value, if any.

## 4. Pre launch (t minus 1) aka "Prepare Cyclops"
Call `setup()` of each `plugin`. Generate the `c++` code for Teensy, including all `signals`.

## 5. Launch
`Tab` cannot be edited.  
Channels can be muted.
The "virtual-disconnect" switch on the editor cannot be toggled. If it _was_ in connected state, events flow into the `plugin` triggering actions on the Teensy, otherwise nothing is sent to Cyclops.
Ideal for saving Cyclops events into a file! [See](#rpc-vents) how that is implemented.

## 6. Stop
Everything can be edited.

## 7. Re-launch
Same as Launch.

# Editing Signals
Pressing the edit button will launch a `matplotlib` window. The new waveform points can also be saved as a new `Signal`.

# Overall user journey
1. Devise experiment, decide how many and what kind of `siganls` are needed.
2. Open text editor, implement a derivation of `genericCyclopsPlugin`.
3. Launch OE GUI, connect Cyclops.
4. Make pipeline, add a `cyclops-stimulator` sink.
5. Import the plugin.
6. Assign channel(s) for the plugin.
7. Open waveform editor: `matplotlib`
8. Test connections.
9. Launch experiment.
10. Tweak plugin, recompile.
11. Do (5) onwards.

---------------------
# Implementation notes below

### RPC Events
To save events emitted by the `node`, a file must be specified or a default pathis used.
As I mentioned in a post long ago, I need to make something like a `spikeObject`. Specifically,

* `CyclopsRPC_Channel` -> `ChannelExtraData` Metadata about the channel.
* What will the `ChannelType` be for this processor? Options below:
    - `HEADSTAGE_CHANNEL`
    - `AUX_CHANNEL`
    - `ADC_CHANNEL`
    - `EVENT_CHANNEL` **<-**
    - `ELECTRODE_CHANNEL`
    - `MESSAGE_CHANNEL`
* I'll make something like `SpikeObject`, to hold the various attributes of the RPC message.
I think that's enough to make the GUI promptly record all events generated...

### The Serial Buffer Queues
This `shared` object is created by the first `node` and is destroyed if there is no `node` in the pipeline.
There is 1 queue for `rx`, another for `tx`. All `nodes` push into this (hopefully) thread-safe `tx` Queue and can read from the `rx` Queue, if they wish.
The queues used only during acquisition / experiment.