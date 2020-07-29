#!/usr/bin/env python

import numpy as np
import pyfits

vmax = 300.0 # maximum supported voltage

v0 = 100.0
# ------------------------------------------------
#           death star pattern
# ------------------------------------------------
vmap=np.zeros((32,32))

for i in range( 0, 14, 3): vmap[i:i+2,:] =v0
for i in range(18, 32, 2): vmap[i,:] = v0

vmap[:,16] = 0.0
vmap[:, 20] = 0.0
vmap[:, 24] = 0.0
vmap[:, 28] = 0.0
vmap[16:,8] = 0.0

vmap[:,:] = v0
# ------------------------------------------------
#           death star pattern
# ------------------------------------------------
#vmap[:,:] = 0.0
#vmap[15:17,:] = v0
#vmap[:,15:17] = v0

#vmap[:,9] = v0
#vmap[:,4] = v0

hdu1 = pyfits.PrimaryHDU(vmap)
hdulist=pyfits.HDUList([hdu1])
hdulist.writeto('dmv_cst_100.fits', clobber=True)

f = open('dmv_parallel.txt', 'w')
for i in range(32):
    for j in range(32):
        f.write("%04X\n" % int(vmap[i,j]*65536.0/vmax))

# fill up the file until 4096 with zeroes.

for i in range(3072):
    f.write("%04X\n" % int(0.0))

f.close()


