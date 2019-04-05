#Regarding the Interactiive Waveform editor
The challenge is not only making intuitive user interactions for editiong of common waveforms like pulse trains, but also in pan, zoom and drawing accurate `axes` on the borders. Drawing of `axes` is known to be especially hard.

JUCE provides lots of UI widgets and `Canvas` but no 2D plotting API. Outsourceing the plotting to plotting projects is hence desireable.
There are very few GUI toolkits for 2D plotting in C++. A comprehensive list :)

* Vtk
    - Big! Well suited for 3D visualisation and camera interactions. `vtKChart`, `vtkPlot` provide the _plotting functions_, and `axes`, etc. 
    - Unfortunately, does not seem(?) to work with JUCE. [VTK docs](http://www.vtk.org/features-interaction-and-gui-support/):
    >VTK works with  ​Qt, FLTK, wxWindows, Tcl/Tk, Python/Tk, Java, X11, Motif, Windows, Cocoa, and Carbon​.
    - [This](https://forum.juce.com/t/vtk-juce-and-openglcomponent/3607) (on JUCE forum) seems to be unanswered since forever.
* MathGL
    - Old, seems to be active though. Plots don't look really nice.
    - NO UI widgets..
* Qt
    - Obviously not useable.

The most promising options are the plotting tollkits which can be embedded into any C++ program, but are built on a scripting language like `python`. In particular I recommend using [`matplotlib`](www.matplotlib.org).
It not only makes beautiful plots, but also provides standard GUI widgets and user interactions.. and yeah, I have quite some experience with it, so quick development is possible.
It can be embedded into C++ by 2 methods:

* By linking against `libpython2.7` or `libpython3` via cython.
    - Advanced method, seamlessly pass data to and from the python interpreter.
    - Not thread safe (because the interpreter is not :( )
    - But.. but.. we don't have high-bandwidth or even real-time data-flow between matplotlib script and OE!
* By invoking a `matplotlib` script as a subprocess _(so not really "embedding")_
    - Can use standard redirection technique for data passing.
    - Send original waveform (to py::stdin)
    - Wait till it dies, all UI interaction handled by the matplotlib GUI.
    - Read (py::stdout)

##Future ready?

Well, matplotlib _can do_ 3D plots (and interactions). It is easier to work with, used a lot by scientific community. If you have some complicated 3D visualisation planned ahead, why not use matplotlib instead of JUCE::Canvas? or OpenGL.
>matplotlib does not use OpenGL. Check out `vispy` instead.
