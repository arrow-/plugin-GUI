import numpy as np, os, yaml

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

def parseYAML(file_handle):
    sig_map = {}
    orig_list = yaml.load(file_handle)
    orig_ordered_name_list = []
    for sig in orig_list:
        sig_map[sig['name']] = Signal(**sig)
        orig_ordered_name_list.append(sig['name'])
    return orig_ordered_name_list, sig_map

def getBase(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as signal_file:
            print("* Read Signals Database from:\n> `" + file_path + '`')
            print('~'*80)
            return parseYAML(signal_file)

def saveAll(file_handle, order, sigMap):
    return yaml.dump([sigMap[name].getYAML() for name in order], file_handle, width=80, indent=4, explicit_start=True, explicit_end=True)

ol, ma = getBase('config.yaml')
for s in ol:
    x = ma[s]
    print(s)
    x.voltage = np.array(x.voltage, dtype=int).tolist()
    x.holdTime = np.array(x.holdTime, dtype=int).tolist()

with open('check.yaml', 'w') as signal_file:
    saveAll(signal_file, ol, ma)