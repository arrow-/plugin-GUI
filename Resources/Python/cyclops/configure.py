import json;
import os.path;
import os;
import subprocess;
import argparse;

arduino_base_msg = "Locate the Arduino extraction directory."
arduino_lib_msg  = "Locate the Arduino library directory."

def zenity(cfg, msg, dest):
	try:
		loc = subprocess.check_output(['zenity', '--file-selection', '--directory', '--title=\"' + msg + '\"', '--filename=~'], stderr=subprocess.DEVNULL);
	except Exception as e:
		print(e)
		loc = b'';
		print('[FATAL]\tPath to Arduino and/or Libraries not set.\nYou will not be able to program the cyclops device from the GUI.');
	cfg[dest] = loc.decode('utf-8').strip();

parser = argparse.ArgumentParser();
parser.add_argument('outdir', metavar='Cyclops sub-plugin build-directory', action='store', type=str);
args = parser.parse_args();
cfile = os.path.join(args.outdir, 'config.json');
config = {}
if os.path.exists(cfile):
	with open(cfile, 'r') as fin:
		try:
			config = json.loads(fin.read());
		except ValueError:
			config['arduinoPath'] = '';
	if config['arduinoPath'] != '':
		print('Path to Arduino extraction directory is:\n`%s`' % config['arduinoPath']);
		choice = input('Do you want to change it? ([n] | y)\n> ');
	else:
		choice = 'y';
	if choice != '' and choice in 'yY':
		zenity(config, arduino_base_msg, 'arduinoPath');
else:
	print('Please locate the directory where Arduino IDE was extracted.')
	print('This is required by the OE GUI to automatically program the Cyclops Device.')
	config = {};
	zenity(config, arduino_base_msg, 'arduinoPath');
print('Setting `ARDUINO_PATH` to `%s`\n' % config['arduinoPath']);

if config.get('arduinoLibPath', None) is not None:
	print('Path to Arduino Library directory is:\n`%s`' % config['arduinoLibPath']);
	choice = input('Do you want to change it? [n] | y');
else:
	choice = 'y';
if choice != '' and choice in 'yY':
	zenity(config, arduino_lib_msg, 'arduinoLibPath');
print('Setting `ARDUINO_LIB_PATH` to `%s`\n' % config['arduinoLibPath']);

config['outDir'] = args.outdir
config['deviceDir'] = os.path.normpath(os.path.join(os.path.join(args.outdir, "../intermediate/device")))

with open(cfile, 'w') as fout:
	fout.write(json.dumps(config, indent=4));
