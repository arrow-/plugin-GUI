#!/usr/bin/env python

import os, sys, yaml, argparse
import numpy as np

class Signal(yaml.YAMLObject):
    MIN_HOLD_TIME = 50
    def __init__(self, name="TemplateSquare", sourceType=2, size=2, holdTime=[500, 500], voltage=[0, 2047]):
        self.name = name
        self.sourceType = sourceType
        self.size = size
        self.holdTime = holdTime
        self.voltage = voltage

    def minVoltage():
        min(self.voltage)

    def maxVoltage():
        max(self.voltage)

    def __repr__(self):
        return "%s(name=%s, sourceType=%i, size=%i, holdTime=%r, voltage=%r)" % (self.__class__.__name__, self.name, self.sourceType, self.size, self.holdTime, self.voltage)

    def getYAML(self):
        # d = {'name' : self.name,
        #      'sourceType' : self.sourceType,
        #      'size' : self.size,
        #      'holdTime' : self.holdTime.tolist(),
        #      'voltage' : self.voltage.tolist()}
        return self.__dict__


class TemporarySignal:
    """The signalEditor creates these objects to plot, and easily manipulate Signal objects.
    
    Read Only properties (set only during construction):
    name        -- Name of signal
    sourceType  -- Support 0 -> (STORED) and 2 -> (SQUARE)
    size        -- Number of points
    dirty       -- Whether a change was made since last `save`

    Setter Methods:
    setData         -- set `holdTime` and `voltage` together.
    scaleVoltage    -- Multiplies `voltage` with `factor`
    scaleTime       -- Scales `holdTime` with `factor`
    offsetVoltage   -- Adds `offset` to `voltage`
    invert          -- Inverts `voltage`

    Inspection Methods:
    getPeriod (gp)      -- get Time Period of the signal
    getFrequency (gf)   -- get frequency of the signal

    Generation Methods:
    To create template signals. These methods return (holdTime, voltage) pair of `numpy.ndarray`s
    These are methods of the `signalEditor.Interactive` class.
    se.square
    se.triangle
    se.sine
    """
    def __init__(self, manager, orig_signal):
        """Constructs object, each one requires a manager (for all callbacks to work)

        Automatically clips out of bound values into [0, 4095]
        """
        self._name = orig_signal.name
        self._sourceType = orig_signal.sourceType
        self._size = orig_signal.size

        self._holdTime = np.array(orig_signal.holdTime)
        self._voltage = np.clip(orig_signal.voltage, 0, 4095)
        self._tData = np.insert(np.cumsum(self.holdTime), 0, 0)
        self._vData = np.insert(self.voltage, 0, self.voltage[-1])
        
        self._manager = manager
        self._dirty = False

    @property
    def holdTime(self):
        return self._holdTime
    @property
    def voltage(self):
        return self._voltage    
    @property
    def name(self):
        return self._name
    @property
    def sourceType(self):
        return self._sourceType
    @property
    def size(self):
        return self._size
    # @property
    # def tData(self):
    #     return self._tData
    # @property
    # def vData(self):
    #     return self._vData
    @property
    def dirty(self):
        return self._dirty

    def getPeriod(self):
        """Get Time Period of the signal"""
        return self._tData[-1]
    def gp(self):
        """Get Time Period of the signal"""
        return self._tData[-1]

    def getFrequency(self):
        """Get frequency of the signal"""
        return 1e6/float(self._tData[-1])
    def gf(self):
        """Get frequency of the signal"""
        return 1e6/float(self._tData[-1])

    def setData(self, ht, v):
        """Checks if length of both arrays is same, and then sets as `holdTime` and `voltage` resp.

        Immediately updates plot.
        """
        if isinstance(ht, (np.ndarray, list)) and isinstance(v, (np.ndarray, list)):
            if len(ht) == len(v):
                self._holdTime = np.array(ht)
                self._voltage = np.clip(v, 0, 4095)
                self._tData = np.insert(np.cumsum(self.holdTime), 0, 0)
                self._vData = np.insert(self.voltage, 0, self.voltage[-1])
                self._size = len(ht)
                self._manager.onSelect(self)
                self._dirty = True
                if self._manager.verbose:
                    print('[Hint] Any changes you make to this (temporary) signal object will not be saved!')
                    print('       Use `se.save()`')
                return True
            else:
                print("Size (aka. first dimension) of both arrays (aka lists) must match!")
                return False
        else:
            print("Both args must be of type: [`numpy.ndarray` or `list`]")
            return False

    def scaleVoltage(self, factor):
        """Multiplies `voltage` with `factor`, but clips out-of-bounds `voltage` values.

        Immediately updates plot.
        """
        np.clip(self._voltage*factor, 0, 4095, out=self._voltage)
        np.clip(self._vData*factor, 0, 4095, out=self._vData)
        self._manager.onSelect(self)
        return True

    def scaleTime(self, factor):
        """Scales `holdTime` with `factor`, which must be positive (and hence non-zero too).

        Will abort if any holdTime value drops below `Signal.MIN_HOLD_TIME` (because of hardware
        resolution limit)
        Immediately updates plot (if needed).
        """
        assert(factor > 0)
        newT = self._holdTime*factor
        m = np.amin(newT)
        if (m < Signal.MIN_HOLD_TIME):
            print("This makes smallest holdTime =", m, "usec. But, it must remain >", Signal.MIN_HOLD_TIME)
            return False
        if self._manager.verbose:
            print("Smallest holdTime is now", m, "usec.")
        self._manager.onSelect(self)
        return True

    def offsetVoltage(self, offset):
        """Adds `offset` to `voltage`, but clips out-of-bounds voltage values.

        Immediately updates plot.
        """
        np.clip(self._voltage+offset, 0, 4095, out=self._voltage)
        np.clip(self._vData+offset, 0, 4095, out=self._vData)
        self._manager.onSelect(self)
        return True

    def invert(self):
        """Inverts `voltage`, but clips out-of-bounds voltage values.

        Immediately updates plot. THIS OPERATION IS POTENTIALLY LOSSY!
        """
        np.clip(-1*self._voltage, 0, 4095, out=self._voltage)
        np.clip(-1*self._vData, 0, 4095, out=self._vData)
        self._manager.onSelect(self)
        return True

def getChoice(msg, prompt, default=True):
    if default:
        opt = '([Y]|n)'
        notTarget = 'nN'
    else:
        opt = '([N]|y)'
        notTarget = 'yY'
    if msg != '':
        res = input('\n'+msg+'\n'+prompt+' '+opt+' ')
    else:
        res = input(prompt+' '+opt+' ')
    if len(res) > 0 and res[0] in notTarget:
        return False
    return True

def validate_env():
    cwd = os.getcwd()
    return os.path.basename(cwd) == "Python" and os.path.basename(os.path.dirname(cwd)) == "Resources"

def makePathTable(args):
    path_table = {'cwd' : os.getcwd()}
    if (args.signal_file != None):
        sig_file = os.path.abspath(os.path.relpath(args.signal_file))
        if not os.path.exists(sig_file) or not os.path.isfile(sig_file):
            raise Exception("\"signals.yaml\" not found here: %s" % sig_file)
        path_table['sig_file'] = sig_file
        path_table['root'] = None
        path_table['source_dir'] = None
        path_table['builds_dir'] = None
    else:
        if not validate_env():
            raise Exception("Please call from /Resources/Python\nAborting!")
        path_table['root'] = os.path.abspath(os.path.relpath(os.path.join(os.path.pardir, os.path.pardir)))
        path_table['source_dir'] = os.path.join(path_table['root'], "Source", "Plugins", "CyclopsStimulator", "plugins")
        path_table['builds_dir'] = os.path.join(path_table['root'], "Builds", "Linux", "build", "cyclops_plugins")
        if args.inBuildsDir:
            path_table['sig_file'] = os.path.join(path_table['builds_dir'], "signals.yaml")
        else:
            path_table['sig_file'] = os.path.join(path_table['source_dir'], "signals.yaml")
    return path_table

def parseYAML(file_handle):
    sig_map = {}
    orig_list = yaml.load(file_handle)
    orig_ordered_name_list = []
    for sig in orig_list:
        sig_map[sig['name']] = Signal(**sig)
        orig_ordered_name_list.append(sig['name'])
    return orig_ordered_name_list, sig_map

def saveAll(file_handle, order, sigMap):
    return yaml.dump([sigMap[name].getYAML() for name in order], file_handle, width=80, indent=4, explicit_start=True, explicit_end=True)

def getSignalDatabase(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as signal_file:
            print("* Read Signals Database from:\n> `" + file_path + '`')
            print('~'*80)
            return parseYAML(signal_file)
    else:
        signal_file = open(file_path, 'w')
        signal_file.close()
        print("(;ﾟ Дﾟ;) Could not find the SignalsDatabase here!\n(but we did expect it to be here, just like you did...)")
        print("* Made a new Signals Database! (signals.yaml), and also added a template signal.")
        print('-'*80,"\nYou can make new signals, and save. Heck! You can even edit the file by hand!")
        print("Not happy? Go ahead and replace this file, look in the OpenEphys GUI repo for a cool replacement!")
        s = signalIO.Signal()
        print('~'*80)
        return [s.name], {s.name : s}

def saveSignalDatabase(file_path, order, sigMap):
    if os.path.exists(file_path):
        if not getChoice('', "Overwrite file?", default=True):
            print("Cancelling...")
            return
    with open(file_path, 'w') as signal_file:
        print("Writing Signals Database to: `" + file_path + '`')
        saveAll(signal_file, order, sigMap)

parser = argparse.ArgumentParser(description="Launches the Cyclops Signal Editor, which \
                                 can be used to Read, Add and Delete signals from a \"\
                                 signals.yaml\"\n\
                                 Without any arguments the program will try to fetch the\
                                 file from `Source/..../CyclopsStimulator/plugins/`")
parser.add_argument('--location', dest='signal_file',
                    action='store', type=str,
                    default=None,
                    help="Use the \"signals.yaml\" from specified path.")
parser.add_argument('-v', dest='verbose',
                    action='store_true',
                    default=False,
                    help="Show verbose output.")
parser.add_argument('-B', '--builds', dest='inBuildsDir',
                    action='store_true',
                    default=False,
                    help="Opens the signals.yaml inside the `Builds/..../builds/cyclops_plugins/` directory.")

if __name__ == '__main__':
    args = parser.parse_args()
    pathTable = makePathTable(args)

    origNameOrder, signalMap = getSignalDatabase(pathTable['sig_file'])
    for name in signalMap:
        print(name, ":", signalMap[name])
    #saveSignalDatabase(pathTable['sig_file']+'.sss', signalMap)