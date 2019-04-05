# Module to Control Cyclops LED driver from Open-Ephys GUI

_GSoC 2016 Final Work Product Report_

| Name | Role | Contact |
| ----------- | -------- | --------- |
|**Ananya Bahadur** | GSoC Intern | ananya95@gmail.com |
|**Jonathan P. Newman** | Mentor | jpnewman@mit.edu |

>[Skip to Work Product][main-skip]

## About Open-Ephys
The [Open-Ephys][oe] project provides powerful extensible platforms to neuroscientists who have diverse requirements for conducting their research. One of the most popular tools, the Open Ephys (Plugin) GUI is the central product which connects and controls various hardware and software solutions.

The primary purpose of the GUI is to allow the user to design a _data flow_ for neural activity signals (acquired with the _[Open Ephys Aquisition Board][oe-board]_). Recently, the GUI was redesigned to use a plugin architecture. Since then, a lot of community developed plugins have been made that allow for event detection, visualisation to be baked into the _data flow_.

For more general information on the Open Ephys project, visit the Open-Ephys [Wiki][wiki], project on [GitHub][oe-git], and the [Public Mailing List][mail-list]. There's also a [slack channel][slack] for devs.

## Aim
Recently, more scientists are exploring the idea of closed-loop feedback neurons and neural circuits. In this scenario, neurons are not only be monitored but also _stimulated_ in a way that is contingent on their past activity. This is analogous to the feedback loops used in control systems. Fairly recently, _opengenetic_ techniques have allowed scientists to stimulate genetically defined sets of cell using different colors of light. 

**This project focused on creation of a plugin for the OE GUI that can talk to Cyclops during an experiment, and forward _"actions"_ based on events detected in the GUI.**
See my [abstract][abs] for more details.

## Goals and Deliverables
This project has two major goals:

1. Port the Cyclops project to the Teensy Family of micro-controllers which are based on ARM Cortex M4 architecture. <sup>_[(Jump)][skip-teensy]_</sup>
	- [Cyclops on Github][cl-gsoc]
2. Implement a plugin and RPC to program and communicate with the Cyclops (Teensy) <sup>_[(Jump)][skip-gui]_</sup>
	- [The Plugin on GitHub][gui-git]















### Hardware Programming _on_ [Teensy 3.2][teensy]
Cyclops [Wiki][cl-wiki] for users. The project is well documented for developers using Doxygen.

* [All commits][cl-commits] to [`jonnew/cyclops`][cl-git]::[`gsoc-dev`][cl-gsoc] (descriptive commit messages!)
* [All PRs][cl-pr] (have been merged to `gsoc-dev` branch)
* [All issues][cl-issues]
* My [clone][cl-mine]

This work will be merged into `jonnew/cyclops`::[`master`][cl-master] when Cyclpos Rev3.6 [which uses Teensy 3.2] is into `beta` (it is currently in `alpha`).  
A chronological account of the work done follows.

####_CL.1_ **Port Cyclops project to Teensy 3.2** :100:
Cyclops boards can be stacked (up to 4), and only a single Arduino / Teensy must be able to control up to 4 boards. See a very detailed [issue][cl-iss] in `jonnew/cyclops`.

####_CL.2_ **Implement an efficient waveform update scheduler.** :100:
The scheduler uses the look-ahead strategy to schedule timer interrupts for future, but performs the scheduling when not servicing interrupt.

* See related [bug][cl-bug] which is simple to resolve.
* [Full design docs][slack-arch] (architecture) _(will move this to wiki)_
* State: working, tested _@ [`c8351d1`][cl-wvf-commit]_

####_CL.3_ **Three kinds of `Source` Objects** :100:
`Source` objects provide the "signal" data which is _driven_ on the LED as an optical signal.
> **A potential "source" of confusion**
The equivalent of `Source` objects on the OE GUI is called "`Signal` object". This differentiation is intentional.

* `STORED` :100:% support
	- The signal points are stored completely in memory.
	- Also suited for representing PWM signals.
* `SQUARE` :100:% support
	- Compact representation for a Square wave, with supporting convenience methods.
* `GENERATED` `limited support`
	- Most compact representation. Instead of saving Signal points, the generating function is stored.
	- Signal is evaluated at runtime. [**Caution**: This is known to be costly, and can affect system resolution.]

####_CL.4_ **Provide support for Cyclops 3.5 boards**
The Arduino Leonardo (or equivalent) powered boards can be used to control up to 4 boards, but at a severe blow to the system's `resolution`. They can deliver a max resolution of 200usec (`check-figure`?), and with each additional board the `resolution` will drop.

* See [docs][arduino-limit] for full explanation.

####_CL.5_ **Teensy RPC for communication with OE-GUI** :100:
This step implemented a **Remote Procedure Call** interface on the Teensy and Arduino serial ports.  
Thanks to @PaulStoffregen awesome work on the Teensy USB, it was easy to setup a Task Queue.
* See the project documentation for all details.
* Completed @[`445dc63`][cl-rpc-commit], Improved @[`7456353`][cl-rpc-commit2]

####_CL.6_ **GUI to quickly test/debug Teensy RPC** :100:
Built a small `python` [GUI][teensy-gui-link] using `tkinter` to send and receive RPC packets from a connected Teensy.  
Completed @[`fd4b15d`][cl-rpc-gui-commit]

![teensy-gui-screenshot-here][teensy-gui-img]

####_CL.7_ **Documentation and Usage Guide**
Documentation coverage for Teensy is :100:, but is missing in many places for Arduino.
Usage guide is WIP on [Open-Ephys wiki][cl-wiki].











### Integration with Open Ephys GUI

>If you are not familiar with the Open Ephys GUI and its workflow, I suggest you read this [excellent article][oe-wiki-guide] to form a better context for the following section.  
You can also [skip](#implementing-the-cyclops-plugin) "Introduction" and "Usage".

This was the most challenging part of the project. The Cyclops Plugin is very different from other plugins, and its design pushes the plugin-architecture of the OE-GUI project to its limits. The architecture,

* does not allow instances of plugins to (easily) hold shared resources/references,
* does not provide a general interface for inter (and intra) plugin communication, and
* is very strict about ownership of _core_ objects.

Significant time was devoted on developing workarounds for these during the project. But, I must also note that such features are (and will be) required only by few plugins and that investing time in a general interface for all these would be overkill.

Another cool feature is the concept of Cyclops `sub-plugins`. So, the dynamically loaded Cyclops _Plugin_ can dynamically load _more (user) `sub-plugins`_. I took a lot of inspiration (and code!) from the OE GUI's `Plugin Manager` to make the `Cyclops Plugin Manager`

#### Usage

>Succinctly, the Cyclops Plugin provides a way to ___grab events___ from the Data Graph and transform them into ___actions___ for a Cyclops device.  

Other plugins process the data channels _(aka data streams)_ during _"aquisition"_, whereas the Cyclops Plugin does not do **any kind of processing** -- this apparently _dumb_ behaviour is an intended feature! During acquisition (ie, when the experiment is running), the plugin just passes

* **(event) data** from `GUI` -> `sub-plugin` and
* **actions (RPC)** from `sub-plugin` -> `Cyclops device` (via USB).

[How is _being dumb_ an intended feature?][keeping-it-dumb]









### Implementing the Cyclops Plugin
[Wiki][plug-wiki] for users. The project is well documented for developers using Doxygen.

* [All commits][plug-commits] to [`arrow-/plugin-GUI`][plug-dev]::[`cyclops`][plug-gsoc] (descriptive commit messages!)
* [All PRs][plug-pr] (some have been merged to `development` branch)
* [All issues][plug-issues]
* My [clone][plug-dev]

This work will be merged into `development` pending review and completion. It will eventually be merged into `master` when the next version of the plugin-GUI is released.  
A chronological account of the work done follows.

####_OE.1_ **Share the `Visualizer` Canvas among `Editors`**
>In OE GUI jargon, `Editors` represent _plugin instances_ and you see them on the bottom (Editor Viewport). If the plugin needs to plot something on the screen, it plots on a `Visualizer` which appears on the top right.

See [motivation][sh-canvas-motiv] for a **shared canvas**.
The shared canvas has better UI interactions, that should be moved to the base `Visualizer` class.

* Changes, [older][viz-edit-commit-1] and [recent][viz-edit-commit-2], were made to `VisualizerEditor` and `Visualizer` to make "canvas sharing" possible and to refactor it [(see forum)][forum-viz-edit].
* Also see related [PR][viz-edit-pr]. My thanks to @sept-en for suggesting the use of [_Observer Pattern_][obs-pat] (`Listener` interface).

####_OE.2_ **Provide a `Cyclops API`**
The API depends on [openFrameworks](http://www.openframeworks.cc) and uses the existing OE code for access to serial ports. The API exposes Cyclops RPC commands.

* See project documentation for detailed documentation.
* Completed @[b06baba](https://github.com/arrow-/plugin-GUI/commit/263a2b22426fc09f0d696dd02acbbb585b06baba), Tested @[68e89f3](https://github.com/arrow-/plugin-GUI/commit/bc67a922674d173eac33e6a1c2acf672068e89f3).

####_OE.3_ **Built Cyclops `sub-plugin` Manager**
This was based on code from OE GUI's `PluginManager`, greatly simplified though.

* Completed @[16cc0c6](https://github.com/arrow-/plugin-GUI/commit/85970eb442d12995a04f69c9351c91d5216cc0c6)

####_OE.4_ **Make `CyclopsPlugin` Base Class**
Users extend this base class to implement their "custom" plugins. The clearly defines the capabilities and responsibilities of any Cyclops `sub-plugin`.  
Please see project documentation and the [User Guide][cl-plugin-guide].

* The CyclopsPlugin has an built in Timer, which can be used to perform open-loop control using periodic tasks _(`beta` feature: see [660332](https://github.com/arrow-/plugin-GUI/commit/7725208fdcac209a2cd3cc1d3b86e24d27660332))_
* There is no need to hardcode the signal data, which is to deployed via the Cyclops device, into the `sub-plugin` code.
* The Cyclops `sub-plugins` are compiled using a separate `Makefile.cyclops`. They are not built with the OE GUI plugins.

####_OE.5_ **Refactor and decouple `CyclopsEditor` and `CyclopsCanvas`**
As I kept adding more and more functionality, the code for these two classes started smelling and become increasingly coupled. Once again, a solution inspired by _Observer Pattern_ was implemented.  
Relevant [commit](https://github.com/arrow-/plugin-GUI/commit/512ba041eee726b39a804f33aa9abecce29b487c).

####_OE.6_ **Allow _"migration"_ of ___hooks___ among _Canvases_**
This was needed to support experiments which might require more than 4 Cyclops Boards. The UI interaction helps to:

* Quickly to migrate a Cyclops `Editor` (aka ___hook___) from one Canvas to another, with all of its configuration intact. [2 clicks]
* Quickly Migrate all ___hooks___ if canvas is "closed". [between 2 to N clicks]

Even though it is not complete, the current implementation is built with soon-to-be-added features in mind.

####_OE.7_ **Added various UI widgets and interactions**
This happened throughout the project. Especially interesting are the _drag-and-drop_ `SignalViews` and the Migration UI.  
During acquisition, migration and Cyclops-Testing, all UI widgets become "inactive", this is because these three operations can execute only in mutual exclusion (as they change state/core settings).

####_OE.8_ **A _very_ user friendly `SignalEditor`**
The Cyclops device is used to drive different `signals` on the LED.
>Currently only `STORED` and `SQUARE` wave signals can be deployed from the GUI. See [why?][beta-gen-signal]

Users will spend considerable time to define signals. This nifty package that leverages "ease-of-use" from [`IPython`][ipy], allows users to edit, view and save signals quickly using popular scientific python tools like `matplotlib`, `numpy` and `scipy`.

* Save in `YAML` for OE GUI :100:
* Excellent documentation available from inside `IPython`. :100:
* Future Work:
	- Save in plain-text for `gnuplot`, `matlab`, `json`
	- Launch from within OE GUI, and update OE GUI's signal database "live". This will also help others to make plugin utilities in python.

### Future Work for GUI Integration
These items are required for the completion of the project (as per me). None of these could be delivered by the GSoC 2016 deadline date (Aug 23 UTC) :sad:

* Canvas Upgrades:
	- Connect the `HookView` with the LED by drag-n-drop.
* Core:
	- Automatically generate code for Teensy 3.2, Arduino Due (and equivalent).
	- Flash the generated code to Teensy, without using Arduino IDE or the Teensy Loader (can be done using simple Makefiles that are available in upstream repos).
	- Launch and communicate with _any_ subprocess using redirection.
	- Use the subprocess communications to launch the SignalEditor from the OE GUI.
* `Editor` Upgrades:
	- Make channel-mapper like object. Very challenging given the size constraints.






















# Appendix
**Here you'll find long<i>-ish</i> explanations for _some_ design decisions.**

## Motivation for ___shared canvases___
* Each _canvas_ represents a single Cyclops device. Almost all configuration for the device and `sub-plugin` is done here.
* Each `Editor` represents a ___hook___ __in the data flow__, that _fishes out_ (grabs) events from that point in the data flow and forwards them to _a_ Cyclops `sub-plugin`.
* Each Cyclops `sub-plugin` can control ___only 1 Cyclops Board___.

Since each Cyclops device can control up to 4 Cyclops Boards, a single _canvas_ must be responsible for more than 1 `Editor`.

## Keeping it... _dumb_
It is impossible to anticipate how ___events___ will or can be ___transformed___ into ___actions___ -- the answer to that will be found when more research is (hopefully :wink:)  -- that this system enables! This is especially true when designing experiments around an object as complex as the mammalian brain. Therefore, it was very important to provide the user with extreme flexibility in determining how stimulus patterns would be generated based upon incoming neural events. Because this system allows unprecedented performance and customizability in the fairly new domain of _Optogenetic Stimulation_, all while being fairly easy to use, it may enable research that narrows down exactly what stimulus patterns are most effective at evoking neural activity and, consequently, what features of neural activity are most important for circuit function.  

The ___transformation___ form neural events to optical output happens inside the cyclops `sub-plugins`. These are normal C++ programs which implement function(s) that, well... _perform the transformation_ and invoke Remote Procedure Calls on the Cyclops device (via an API over USB).  By passing the buck to the user, to make the `sub-plugin` program intelligent, the system is flexible enough to suit literally any kind of research in this domain. Not to mention the expressive power, popularity and infinite capabilities of the C++ language, which makes this `sub-plugin` interface an ideal and elegant solution.

Also see [future work][future-w].

## Why is `GENERATED` in `beta`?
Computation of arbitrary expressions is a costly operation. Even seemingly innocent floating point arithmetic can take many CPU cycles on a Teensy 3.2, because it lacks a FPU.
> [Teensy 3.6][teensy-36] will have an FPU. The CPU clock is also up to `2x` faster. Additionally, teensy 3.6 is nearly pin-for-pin compatible with the 3.2, which was targeted by this project. Therefore this upgrade will likely come for free :smile:

By `beta` we mean that it cannot be used from the OE GUI, `GENERATED` ___can be used now___ if you are programming the Cyclops directly (which is entirely possible, btw!).  
`GENERATED` will stay in `beta` till we can confirm that it does not make a significant affect to the resolution in usual scenarios.







[main-skip]:#goals-and-deliverables
[skip-teensy]:#hardware-programming-on-teensy-32
[skip-gui]:#implementing-the-cyclops-plugin

[oe]:https://www.open-ephys.org
[oe-board]:http://www.open-ephys.org/acq-board/
[oe-git]:https://github.com/open-ephys/
[wiki]:https://open-ephys.atlassian.net/wiki
[slack]:https://open-ephys.slack.com
[mail-list]:https://groups.google.com/forum/?utm_source=digest&utm_medium=email#!forum/open-ephys/topics
[clweb]:http://www.open-ephys.org/cyclops/
[abs]:https://summerofcode.withgoogle.com/dashboard/project/6082848201113600/overview/
[cl-gsoc]:https://github.com/jonnew/cyclops/tree/gsoc-dev
[gui-git]:https://github.com/arrow-/plugin-GUI/tree/cyclops
[teensy]:https://www.pjrc.com/teensy/teensy31.html

[cl-wiki]:https://open-ephys.atlassian.net/wiki/display/OEW/Cyclops+LED+Driver
[cl-commits]:https://github.com/jonnew/cyclops/commits/gsoc-dev?author=arrow-
[cl-pr]:https://github.com/jonnew/cyclops/pulls?&q=author%3Aarrow-
[cl-issues]:https://github.com/jonnew/cyclops/issues?&q=is%3Aissue+author%3Aarrow-
[cl-mine]:https://github.com/arrow-/cyclops/tree/gsoc-dev
[cl-master]:https://github.com/jonnew/cyclops/tree/master
[cl-iss]:https://github.com/jonnew/cyclops/issues/5

[cl-git]:https://github.com/jonnew/cyclops
[cl-wvf-commit]:https://github.com/jonnew/cyclops/commit/0dabb89d60e547d398cde8f02852f4134c8351d1
[cl-rpc-commit]:https://github.com/jonnew/cyclops/commit/005e7f506e0019adafb599177e640336d445dc63
[cl-rpc-commit2]:https://github.com/jonnew/cyclops/commit/e02bf56d33223216b253f21b7246de62a7456353
[cl-rpc-gui-commit]:https://github.com/jonnew/cyclops/commit/14d0eb379512d30bcbf3bae26a7d83a5cfd4b15d
[cl-bug]:https://github.com/jonnew/cyclops/issues/16
[slack-arch]:https://slack-files.com/T0QDMBVFD-F11BG6WSY-bdeae21840

[arduino-limit]:

[teensy-gui-img]:
[teensy-gui-link]:https://github.com/jonnew/cyclops/tree/gsoc-dev/src/cyclops-ctrl-gui

[plug-wiki]:
[plug-commits]:https://github.com/arrow-/plugin-GUI/commits/cyclops?author=arrow-
[plug-pr]:https://github.com/open-ephys/plugin-GUI/pulls?&q=author%3Aarrow-
[plug-issues]:https://github.com/open-ephys/plugin-GUI/issues?&q=is%3Aissue+author%3Aarrow-
[plug-gsoc]:https://github.com/arrow-/plugin-GUI/tree/cyclops
[plug-dev]:https://github.com/open-ephys/plugin-GUI/tree/development
[plug-iss]:https://github.com/open-ephys/plugin-GUI/issues?&q=is%3Aissue+author%3Aarrow-
[keeping-it-dumb]:#keeping-it-dumb

[viz-edit-commit-1]:https://github.com/arrow-/plugin-GUI/commit/2deb793304481a04928ce910805704f39d45a4c8
[viz-edit-commit-2]:https://github.com/open-ephys/plugin-GUI/pull/72/commits/dc7395dd703c8c301290ae51abd57d17a61d724a
[viz-edit-pr]:https://github.com/open-ephys/plugin-GUI/pull/72
[obs-pat]:https://msdn.microsoft.com/en-us/library/ee817669.aspx
[sh-canvas-motiv]:motivation-for-shared-canvases

[cl-plugin-guide]:
[beta-gen-signal]:#why-is-generated-in-beta
[ipy]:http://www.ipython.org/

[forum-viz-edit]:https://groups.google.com/forum/#!topic/open-ephys/JlPdxwMWQHg
[teensy-36]:https://www.kickstarter.com/projects/paulstoffregen/teensy-35-and-36
[future-w]:#future-work-for-gui-integration