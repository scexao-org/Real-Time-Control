# Generated by rpcgen.py at Tue Sep 16 18:36:06 2008

from DAQtkDatSndMemconstants import *
from DAQtkDatSndMempacker import *
try:
    # Assume that the python xdrlib comes first on the PYTHONPATH
    from xdrlib import Error as XDRError
except ImportError:
    # Never mind. I will use the pynfs one.
    from xdrlib import XDRError
    
import rpc

class BadDiscriminant(rpc.RPCException):
    def __init__(self, value, klass):
        self.value = value
        self.klass = klass

    def __str__(self):
        return "Bad Discriminant %s in %s" % (self.value, self.klass)

class DAQtkRpcFits:
    # XDR definition:
    # struct DAQtkRpcFits {
    #     char cframe<16>;
    #     opaque ph<>;
    #     opaque pd<>;
    #     opaque ah1<>;
    #     opaque ad1<>;
    #     opaque ah2<>;
    #     opaque ad2<>;
    #     opaque ah3<>;
    #     opaque ad3<>;
    # };
    def __init__(self, cframe=None, ph=None, pd=None, ah1=None, ad1=None, ah2=None, ad2=None, ah3=None, ad3=None):
        self.cframe = cframe
        self.ph = ph
        self.pd = pd
        self.ah1 = ah1
        self.ad1 = ad1
        self.ah2 = ah2
        self.ad2 = ad2
        self.ah3 = ah3
        self.ad3 = ad3

    def __repr__(self):
        s = " cframe=%s ph=%s pd=%s ah1=%s ad1=%s ah2=%s ad2=%s ah3=%s ad3=%s" % (str(self.cframe), str(self.ph), str(self.pd), str(self.ah1), str(self.ad1), str(self.ah2), str(self.ad2), str(self.ah3), str(self.ad3))
        if len(s) > 70: s = s[:70] + "..."
        return "<DAQtkRpcFits:%s>" % s

