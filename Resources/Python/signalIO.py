#!/usr/bin/env python

import os, sys, yaml, argparse
import numpy as np

class Signal(yaml.YAMLObject):
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
    def __init__(self, manager, orig_signal):
        self._name = orig_signal.name
        self._sourceType = orig_signal.sourceType
        self._size = orig_signal.size

        self._holdTime = np.array(orig_signal.holdTime)
        self._voltage = np.array(orig_signal.voltage)
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

    def setData(self, ht, v):
        if isinstance(ht, (np.ndarray, list)) and isinstance(v, (np.ndarray, list)):
            if len(ht) == len(v):
                self._holdTime = np.array(ht)
                self._voltage = np.array(v)
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

def parseTxt(file_handle):
    sig_map = {}
    orig_ordered_name_list = []
    # type name size holdTime[size] voltage[size]
    lines = file_handle.readlines()
    line = 0
    while (line+4 < len(lines)):
        t = int(lines[line].strip())
        name = lines[line+1].strip()
        sz = int(lines[line+2].strip())
        ht = [int(x) for x in lines[line+3].strip().split(' ')]
        v = [int(x) for x in lines[line+4].strip().split(' ')]
        sig_map[name] = Signal(name, t, sz, ht, v)
        line += 5
        orig_ordered_name_list.append(name)
    return sig_map

def parseYAML(file_handle):
    sig_map = {}
    orig_ordered_name_list = []
    for x in yaml.load_all(file_handle):
        sig_map[x['name']] = Signal(**x)
        orig_ordered_name_list.append(x['name'])
    return orig_ordered_name_list, sig_map

def saveYAML(file_handle, signal):
    return yaml.dump(signal.getYAML(), file_handle, width=80, indent=4, explicit_start=True, explicit_end=True)

def saveAll(file_handle, order, sigMap):
    for name in order:
        saveYAML(file_handle, sigMap[name])

def getSignalDatabase(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as signal_file:
            print("\nRead Signals Database from:\n`" + file_path + '`')
            return parseYAML(signal_file)
    else:
        signal_file = open(file_path, 'w')
        signal_file.close()
        print("Could not find the SignalsDatabase here!\n(but we did expect it to be here, just like you did...)")
        print("Made a new Signals Database! (signals.yaml), and also added a template signal.")
        print("\nYou can make new signals, and save. Heck! You can even edit the file by hand!")
        print("Not happy? Go ahead and replace this file, look in the OpenEphys GUI repo for a cool replacement!")
        s = signalIO.Signal()
        return [s.name], {s.name : s}

def saveSignalDatabase(file_path, order, sigMap):
    if os.path.exists(file_path):
        if not getChoice('', "Overwrite file?", default=True):
            print("Cancelling...")
            return
    with open(file_path, 'w') as signal_file:
        print("Writing Signals Database to: `" + file_path + '`')
        saveAll(signal_file, oreder, sigMap)

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