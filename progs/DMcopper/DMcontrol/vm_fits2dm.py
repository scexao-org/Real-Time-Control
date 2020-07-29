#!/usr/bin/env python

import numpy as np, pyfits, sys, pdb


args = sys.argv[1:]

if np.size(args) == 0:
    print "---------------------------"
    print "Use of the command:"
    print "vm_fits2dm myFitsMap.fits"
    print ""
    print "creates a txt file that can"
    print "be loaded by the dm control"
    print "program"
    print "---------------------------"
    exit(0)

print("Loading vmap %s", args[0])
hdu=pyfits.open(args[0])

vmax = 300.0 # maximum supported voltage
vmap=hdu[0].data


f = open('dmv.txt', 'w')
for i in range(32):
    for j in range(32):
        f.write("%04X\n" % int(vmap[i,j]*65536.0/vmax))

# fill up the file until 4096 with zeroes.

for i in range(3072):
    f.write("%04X\n" % int(0.0))

f.close()


