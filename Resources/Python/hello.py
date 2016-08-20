#!/usr/bin/python
import signalIO
import numpy as np

args = signalIO.parser.parse_args()

print('\n'+'~'*10, 'Welcome to the Cyclops Signal Editor!', '~'*10)
print('The `se` object (signalEditor.Interactive) has already been prepared for your use.\n')
print('** Use the `se` object for all your tasks. **')
print('** Do not tinker with signalEditor.OrigOrderList and signalEditor.SignalMap **')

import signalEditor
se = signalEditor.Interactive(args)
if se.verbose:
	print("\nVerbosity can be changed at any time be setting `se.verbose = <boolean>`\n")
