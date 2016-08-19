#!/usr/bin/env python

import os, sys, yaml, argparse

class Signal(yaml.YAMLObject):
    def __init__(self, name="TemplateSquare", sourceType=2, size=2, holdTime=[500, 500], voltage=[0, 2047]):
        self.name = name
        self.sourceType = sourceType
        self.size = size
        self.holdTime = holdTime
        self.voltage = voltage
        self.np_holdTime = None
        self.np_voltage = None
        self.holdTime_min = None
        self.holdTime_max = None

    def __repr__(self):
        return "%s(name=%s, sourceType=%i, size=%i, holdTime=%r, voltage=%r)" % (self.__class__.__name__, self.name, self.sourceType, self.size, self.holdTime, self.voltage)

    def getYAML(self):
        return self.__dict__

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

def parse(file_handle):
    sig_list = []
    # type name size holdTime[size] voltage[size]
    lines = file_handle.readlines()
    line = 0
    while (line+4 < len(lines)):
        t = int(lines[line].strip())
        name = lines[line+1].strip()
        sz = int(lines[line+2].strip())
        ht = [int(x) for x in lines[line+3].strip().split(' ')]
        v = [int(x) for x in lines[line+4].strip().split(' ')]
        sig_list.append(Signal(name, t, sz, ht, v))
        line += 5
    return sig_list

def parseYAML(file_handle):
    sig_list = []
    for x in yaml.load_all(file_handle):
        sig_list.append(Signal(**x))
    return sig_list

def getSignalDatabase(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as signal_file:
            print("Read Signals Database from: `" + file_path + '`')
            return parseYAML(signal_file)
    else:
        signal_file = open(file_path, 'w')
        signal_file.close()
        print("Could not find the SignalsDatabase here!\n(but we did expect it to be here, just like you did...)")
        print("Made a new Signals Database! (signals.yaml), and also added a template signal.")
        print("\nYou can make new signals, and save. Heck! You can even edit the file by hand!")
        print("Not happy? Go ahead and replace this file, look in the OpenEphys GUI repo for a cool replacement!")
        return list()

parser = argparse.ArgumentParser(description="Launches the Cyclops Signal Editor, which \
                                 can be used to Read, Add and Delete signals from a \"\
                                 signals.yaml\"\n\
                                 Without any arguments the program will try to fetch the\
                                 file from `Source/..../CyclopsStimulator/plugins/`")
parser.add_argument('--location', dest='signal_file',
                    action='store', type=str,
                    default=None,
                    help="Use the \"signals.yaml\" from specified path.")
parser.add_argument('-B', '--builds', dest='inBuildsDir',
                    action='store_true',
                    default=False,
                    help="Opens the signals.yaml inside the `Builds/..../builds/cyclops_plugins/` directory.")

if __name__ == '__main__':
    args = parser.parse_args()
    pathTable = makePathTable(args)

    signalList = getSignalDatabase(pathTable['sig_file'])
    print(signalList)