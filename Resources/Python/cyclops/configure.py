import json;
import os.path;
import os;
import subprocess;
import argparse;
def zenity(cfg):
	try:
		loc = subprocess.check_output(['zenity', '--file-selection', '--directory', '--title=\"Locate the Arduino extraction directory\"', '--filename=~'], stderr=subprocess.DEVNULL);
	except Exception as e:
		print(e)
		loc = b'';
		print('[FATAL]\tPath to Arduino not set.\nYou will not be able to program the cyclops device from the GUI.');
	cfg['arduino_path'] = loc.decode('utf-8').strip();

parser = argparse.ArgumentParser();
parser.add_argument('outdir', metavar='Cyclops sub-plugin build-directory',action='store', type=str);
args = parser.parse_args();
cfile = os.path.join(args.outdir, 'config.json');
config = {}
if os.path.exists(cfile):
	with open(cfile, 'r') as fin:
		try:
			config = json.loads(fin.read());
		except ValueError:
			config['arduino_path'] = '';
	if config['arduino_path'] != '':
		print('Path to Arduino extraction directory is:\n`%s`' % config['arduino_path']);
		choice = input('Do you want to change it? [n] | y');
	else:
		choice = 'y';
	if choice != '' and choice in 'yY':
		zenity(config);
else:
	print('Please locate the directory where Arduino IDE was extracted.')
	print('This is required by the OE GUI to automatically program the Cyclops Device.')
	config = {};
	zenity(config);
print('Setting `ARDUINO_PATH` to `%s`' % config['arduino_path']);

config['outdir'] = args.outdir
config['devdir'] = os.path.normpath(os.path.join(os.path.join(args.outdir, "../intermediate/device")))

with open(cfile, 'w') as fout:
	fout.write(json.dumps(config));