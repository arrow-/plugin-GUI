#!/usr/bin/python
import signalIO
import numpy as np

args = signalIO.parser.parse_args()

print('\n'+'~'*20, 'Welcome to the Cyclops Signal Editor!', '~'*21)

import signalEditor
se = signalEditor.Interactive(args)
if se.verbose:
	print("* Verbosity can be changed at any time be setting `se.verbose = <boolean>`")
print('+'+'-'*78+'+')
print('|  The `se` object (signalEditor.Interactive) has already been prepared        |')
print('|  for your use.                                                               |')
print('+'+'-'*78+'+')
print('                 ** Use the `se` object for all your tasks. **')
