# Generated by rpcgen.py at Tue Nov 29 11:01:00 2005

import rpc
import ag_vgwtypes
import ag_vgwconstants
import xdrlib
import numpy
import sys

try:
    # Assume that the python xdrlib comes first on the PYTHONPATH
    from xdrlib import Error as XDRError
except ImportError:
    # Never mind. I will the use pynfs one.
    from xdrlib import XDRError

class AG_VGWPacker(rpc.Packer):
    pack_short = rpc.Packer.pack_int

    pack_string = rpc.Packer.pack_string

    pack_opaque = rpc.Packer.pack_opaque

    pack_int = rpc.Packer.pack_int

    pack_double = rpc.Packer.pack_double

    pack_enum = rpc.Packer.pack_enum

    pack_unsigned = rpc.Packer.pack_uint

    pack_long = rpc.Packer.pack_int

    pack_char = rpc.Packer.pack_uint

    pack_unsigned_hyper = rpc.Packer.pack_uhyper

    pack_float = rpc.Packer.pack_float

    pack_hyper = rpc.Packer.pack_hyper

    pack_bool = rpc.Packer.pack_bool

    pack_quadruple = rpc.Packer.pack_double

    pack_unsigned_int = rpc.Packer.pack_uint

    pack_unsigned_short = rpc.Packer.pack_uint

    def pack_tgVform(self, data):
        if not isinstance(data, ag_vgwtypes.tgVform):
            raise TypeError, "pack_tgVform"
        if data.head is None:
            raise TypeError, "pack_tgVform: Member head has no value."
        if data.tv_sec is None:
            raise TypeError, "pack_tgVform: Member tv_sec has no value."
        if data.tv_usec is None:
            raise TypeError, "pack_tgVform: Member tv_usec has no value."
        if data.data is None:
            raise TypeError, "pack_tgVform: Member data has no value."
        self.pack_farray(60, data.head, self.pack_char)
        self.pack_long(data.tv_sec)
        self.pack_long(data.tv_usec)
        # This is VERRRRY SLOW -----
        #self.pack_array(data.data, self.pack_unsigned_short)
        # --------------------------
        # Replace with this (assumes data.data is numpy)
        array = data.data.astype('UInt32')
        if sys.byteorder == 'little':
            array.byteswap(True)
        buf = array.tostring()
        self.pack_string(buf)
        # --------------------------

    def mung(self, arr):
        newarr = []
        for c in arr:
            newarr.extend([0, 0, 0, c])
        return newarr

class AG_VGWUnpacker(rpc.Unpacker):
    unpack_short = rpc.Unpacker.unpack_int

    unpack_string = rpc.Unpacker.unpack_string

    unpack_opaque = rpc.Unpacker.unpack_opaque

    unpack_int = rpc.Unpacker.unpack_int

    unpack_double = rpc.Unpacker.unpack_double

    unpack_enum = rpc.Unpacker.unpack_enum

    unpack_unsigned = rpc.Unpacker.unpack_uint

    unpack_long = rpc.Unpacker.unpack_int

    unpack_char = rpc.Unpacker.unpack_uint

    unpack_unsigned_hyper = rpc.Unpacker.unpack_uhyper

    unpack_float = rpc.Unpacker.unpack_float

    unpack_hyper = rpc.Unpacker.unpack_hyper

    unpack_bool = rpc.Unpacker.unpack_bool

    unpack_quadruple = rpc.Unpacker.unpack_double

    unpack_unsigned_int = rpc.Unpacker.unpack_uint

    unpack_unsigned_short = rpc.Unpacker.unpack_uint
    
    #unpack_void = rpc.Unpacker.unpack_void
    
    def unpack_tgVform(self):
        data = ag_vgwtypes.tgVform()
        data.head = self.unpack_farray(60, self.unpack_char)
        data.tv_sec = self.unpack_long()
        data.tv_usec = self.unpack_long()
        # This is VERRRRY SLOW -----
        #data.data = self.unpack_array(self.unpack_unsigned_short)
        # --------------------------
        # Replace with this (assumes data.data will be numpy)
        length_seq = self.unpack_uint()
       
        seq = self.get_buffer()
        rest = seq[self.get_position():]
        if len(rest)//4 != length_seq:
            print "rest adv len=%d  buf len=%d" % (length_seq, len(rest)//4)
        #array = numpy.array(rest, 'UInt32')
        # array = numpy.fromstring(rest, dtype=numpy.uint32,
        #                          count=length_seq)
        array = numpy.fromstring(rest, dtype=numpy.uint32)
        if sys.byteorder == 'little':
            array.byteswap(True)
            
        self.set_position(len(seq))
        data.data = array
        # --------------------------
        return data
  
    

