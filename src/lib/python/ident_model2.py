#!/usr/bin/env python

import os, sys
import numpy as np
import matplotlib.pyplot as plt
import array
import pyfits as pf
import numpy.fft as nfft
from numpy.linalg import inv
import math as m
import time
import copy as cp

home = os.getenv('HOME')
sys.path.append(home+'/src/lib/python/')
from   scexao_shm   import shm

#-------------------------------------------------------
def make_ramp(*args):
    # make_ramp - Creates a n points ramp between a and b.
    # PURPOSE:
    # This function computes and returns a n point ramp between a and b, b can
    # be excluded or included, with a linear or logarithmic step.
    # INPUTS:
    #    a: left limit, included
    #    b: right limit, excluded or included
    #    n: numer of points in the ramp
    #  leq: OPTIONAL boolean to include b, default is b is excluded
    # loga: OPTIONAL boolean to use a log frequency axis, default is linear.
    # OUPUTS:
    # ramp: ramp
    
    ni = len(args)
    
    a = args[0]
    b = args[1]
    n = args[2]
    if ni == 3:
        leq = 0
        loga = 0
    elif ni == 4:
        leq = args[3]
        loga = 0
    else:
        leq = args[3]
        loga = args[4]
        if leq == []:
            leq = 0
    
    if leq == 1:
        n2 = n-1
    else:
        n2 = n
    
    if loga == 1:
        ramptemp = np.array(map(float, range(n)))*(m.log10(b)-m.log10(a))/n2+m.log10(a)
        ramp = np.power(10,ramptemp)
    else:
        ramp = np.array(map(float, range(n)))*(b-a)/n2+a
    
    return ramp


#-------------------------------------------------------------
def ts_to_ar(X, p):
    # ts_to_ar - ARn regression from a time sequence
    # PURPOSE:
    # This function computes the coefficients used in a Pth order
    # autoregressive time-series forecasting/backcasting model. The coef is a
    # p-element vector whose type is identical to X.
    # INPUTS:
    #    X: An n-element vector of type float or double containing time-series 
    #       samples.
    #    p: A scalar of type integer or long integer that specifies the number 
    #       of coefficients to be computed.
    # OUTPUTS:
    # coef: A P-elemnt containing the coefficients.
    #  mse: Use this keyword to specify a named variable which returns the
    #       mean square error of the Pth order autoregressive model.
    
    n = len(X)
    R = np.zeros([n-p, p], dtype=complex)
    for k in range(p):
        R[:, k] = X[p-k-1:n-k-1]
    
    yp = X[p:n]
    coeftemp = inv((R.T).dot(R))
    coef = np.real((coeftemp.T).dot(R.T).dot(yp))
    mse = np.var(yp-R.dot(coef))

    return coef, mse

#-----------------------------------------------------------------------
def kfs_to_ar(k, fv, fsamp):
    # kfs_to_ar - Compute a1 a2 corresponding to the discretization of a
    # (k,fmax) continuous second order signal.
    # PURPOSE:
    # This function computes and returns the a1 a2 corresponding to the
    # discretization of a (k,fmax) continuous second order signal, with fsamp
    # the sampling frequency.
    # INPUTS:
    #        k: dampening coefficient (can be 1D array)
    #     fmax: resonance frequency
    #    fsamp: sampling frequency
    # OUTPUTS:
    #       ai: the ai coefficients.
    
    term = np.cos(2*m.pi*np.sqrt(1-k**2+0J)*fv/fsamp).real
    
    a1 = 2*term*np.exp(-k*2*m.pi*fv/fsamp)
    a2 = -np.exp(-k*4*m.pi*fv/fsamp)
    if np.array(k).ndim == 0:
        ai = np.array([a1, a2])
    else:
        a1 = np.expand_dims(a1,axis=0)
        a2 = np.expand_dims(a2,axis=0)
        ai = np.concatenate((a1,a2))
    
    return ai


#-----------------------------------------------------------------------
class Returnarcoeff(object):
    def __init__(self, psd, freq, vecp):
        self.psd = psd
        self.freq = freq
        self.vecp = vecp

def arcoeff_to_psd(*args):
    # arcoeff_to_psd - Computes the psd of an ARn filter from its coefficients
    # PURPOSE:
    # This function computes and returns the psd of an ARn filter of
    # coefficients a and noise variance var, with a sampling frequency fsamp
    # and a number of points n.
    # INPUTS:
    #        a: FLOAT ARRAY of ARn coefficients (a=[a1,a2,a3,...,an])
    #      var: noise variance (FLOAT)
    #        n: INT desired number of points for the psd.
    #    fsamp: sampling frequency.
    #      log: OPTIONAL boolean to use a log frequency axis, default is linear.
    #     full: OPTIONAL full PSD or half the frequencies, default is half.
    #     indw: OPTIONAL indexes for the psd
    # OUPUTS:
    #  psd: PSD of the ARn filter.
    # freq: FLOAT ARRAY of the n frequency values for which the psd is
    #       computed.
    
    ni = len(args)
    
    a = args[0]
    var = args[1]
    n = args[2]
    fsamp = args[3]
    if ni == 4:
        log = 0
        full = 0
    else:
        log = args[4]
        if ni == 5:
            full = 0
        else:
            full = args[5]
            if ni == 7:
                indw = args[6]
    
    if full:
        npts = n
        freq = make_ramp(fsamp/n, fsamp, npts, [], log)
    else:
        npts = n//2
        freq = make_ramp(fsamp/n, fsamp/2, npts, [], log)
    if ni == 7:
        freq = freq[indw]
        npts = freq.size
    vecp = np.exp(-2*m.pi*1j*freq/fsamp)
    if full == 1:
        vecp = vecp[0:n//2]
    
    na = np.array(a.shape)
    p = na[0]
    if a.ndim > 1:
        na1 = na[1]
    else:
        na1 = np.array(1)
    if a.ndim > 2:
        na2 = na[2]
    else:
        na2 = np.array(1)
    a = a.reshape(p,na1,na2)
    nvar = np.size(var)
    
    xf,xk,xi,xj = np.meshgrid(freq, np.arange(p), np.arange(na1), np.arange(na2), indexing='ij')
    xa = np.tile(a, (npts,1,1,1))
    b = xa*np.exp(-(xk+1)*2*m.pi*1j*xf/fsamp)
    psd_temp = (np.abs(1.0/(1+np.sum(b,1))))**2/n
    xpsd = np.tile(psd_temp.reshape((npts,na1,na2,1)), (1,1,1,nvar))
    xvar = np.tile(var, (npts, na1, na2, 1))
    psd = xvar*xpsd
    psd = psd.squeeze()

    return Returnarcoeff(psd, freq, vecp)

#-----------------------------------------------------------------------
class Returnminarray(object):
    def __init__(self, minimum, indices):
        self.minimum = minimum
        self.indices = indices

def min_array(array):
    # min_array - find the minimum of an array, and return the value and the
    # coordinates
    # PURPOSE:
    # This function finds the minimum value in an array of any dimensions, and
    # returns its value and the coordinates in that array. If there are
    # different points at the minimum, the program returns only the first one.
    # INPUTS:
    #    array: n-dimensional array
    # OUTPUTS:
    #  minimum: value of the minimum point
    #  indices: coordinates of the point in the array.
    
    minimum = np.nanmin(array)
    indtemp = np.where(array == minimum)
    indices = np.array(indtemp)
    if np.shape(indices)[1] > 0:
        indices = np.squeeze(indices[:,0])
    
    return Returnminarray(minimum, indices)


#-----------------------------------------------------------------------
class Returnar2(object):
    def __init__(self, aiturb, cvturb, kf):
        self.aiturb = aiturb
        self.cvturb = cvturb
        self.kf = kf

def arn_to_ar2(ai_arn, cv_arn, noise, npoints, fsamp, fw, condition):
    # arn_to_ar2 - Fits a noisy AR2 on a noisy ARN
    # PURPOSE:
    # This function computes and returns the ar2 parameters (a1, a2, sigma2) of
    # the AR2 closest to the ARn noisy model, i.e. minimizing:
    # J=total((alog(psd_model(k,fv,sigma2)+noise)-alog(psd_arn))^2) 
    # with psd_arn computed from arcoeff_to_psd(ai_arn,cv_arn)+noise
    # result is a structure with the following fields:
    # aiturb    DOUBLE    Array[2]:  [a1,a2]
    # cvturb    DOUBLE    float :  sigma2
    # INPUTS:
    #       ai_arn: ai coefficient of the ARn to fit
    #       cv_arn: noise variance of the ARn to fit
    #        noise: value of the additive measurement noise
    #      npoints: number of points
    #        fsamp: sampling frequency
    #    condition: boolean to constrain a1 and a2 to a1+a2=1
    # OUTPUTS:
    #   result: the ar2 parameters
    
    loga = 1
    res = arcoeff_to_psd(-ai_arn, cv_arn, npoints, fsamp, loga)
    psd = res.psd
    freq = res.freq
    npts = 8
    
    psd_arn = psd+noise
    psd_arn2 = np.tile(psd_arn.reshape((npoints/2,1,1,1)), (1,npts,npts,npts))
    
    # in what follows, we search for k, fv and sigma2 minimizing the criterion
    # J=total((alog(psd_model(k,fv,sigma2)+noise)-alog(psd_arn))^2) 
    # this is done by computing the criterion for a set of (k,fv,sigma2) on a
    # 3D grid. It is first done on a coarse grid. Then a second and a third
    # explorations are performed on a subspace, to improve the accuracy.
    
    #%%%%%%%%%% coarse gridding %%%%%%%%%%
    krange = np.array([1e-1, 1e3])
    fvrange = np.array([1e-1, fsamp])
    cvrange = np.array([1e-5, 1e3])#([1e-5, 1e1])
    tk = make_ramp(krange[0], krange[1], npts, 1, 1)
    tfv = make_ramp(fvrange[0], fvrange[1], npts, 1, 1)
    tcv = make_ramp(cvrange[0], cvrange[1], npts, 1, 1)
    
    Xk, Xfv = np.meshgrid(tk, tfv, indexing='ij')
    Xk = Xk.astype("float")
    Xfv = Xfv.astype("float")

    term = np.cos(2*m.pi*np.sqrt(1-Xk**2+0J)*Xfv/fsamp).real
    
    a1 = 2*term*np.exp(-Xk*2*m.pi*Xfv/fsamp)
    if condition == 1:
        a2 = 1-a1
    else:
        a2 = -np.exp(-Xk*4*m.pi*Xfv/fsamp)
    a1b = np.expand_dims(a1, axis=0)
    a2b = np.expand_dims(a2, axis=0)
    
    res = arcoeff_to_psd(np.concatenate((-a1b, -a2b)), tcv, npoints, fsamp, loga)
    psd_model = res.psd
    criterion = np.sum((np.log(psd_model+noise)-np.log(psd_arn2))**2, axis=0)
    
    res = min_array(criterion)
    ind = res.indices
    print 'arn2ar2 1/2 completed'
    
    #%%%%%%%%%% fine gridding %%%%%%%%%%
    krange = np.array([tk[max(ind[0]-1,0)], tk[min(ind[0]+1,npts-1)]])
    fvrange = np.array([tfv[max(ind[1]-1,0)], tfv[min(ind[1]+1,npts-1)]])
    cvrange = np.array([tcv[max(ind[2]-1,0)], tcv[min(ind[2]+1,npts-1)]])
    tk = make_ramp(krange[0], krange[1], npts, 1, 1)
    tfv = make_ramp(fvrange[0], fvrange[1], npts, 1, 1)
    tcv = make_ramp(cvrange[0], cvrange[1], npts, 1, 1)
    
    Xk, Xfv = np.meshgrid(tk, tfv, indexing='ij')\

    term = np.cos(2*m.pi*np.sqrt(1-Xk**2+0J)*Xfv/fsamp).real
    
    a1 = 2*term*np.exp(-Xk*2*m.pi*Xfv/fsamp)
    if condition == 1:
        a2 = 1-a1
    else:
        a2 = -np.exp(-Xk*4*m.pi*Xfv/fsamp)
    a1b = np.expand_dims(a1, axis=0)
    a2b = np.expand_dims(a2, axis=0)
    
    res = arcoeff_to_psd(np.concatenate((-a1b, -a2b)), tcv, npoints, fsamp, loga)
    psd_model = res.psd
    criterion = np.sum((np.log(psd_model+noise)-np.log(psd_arn2))**2, axis=0)
    
    res = min_array(criterion)
    ind = res.indices
    print 'arn2ar2 2/2 completed'

    '''    
    #%%%%%%%%%% xfine gridding %%%%%%%%%%
    krange = np.array([tk[max(ind[0]-1,0)], tk[min(ind[0]+1,npts-1)]])
    fvrange = np.array([tfv[max(ind[1]-1,0)], tfv[min(ind[1]+1,npts-1)]])
    cvrange = np.array([tcv[max(ind[2]-1,0)], tcv[min(ind[2]+1,npts-1)]])
    tk = make_ramp(krange[0], krange[1], npts, 1, 1)
    tfv = make_ramp(fvrange[0], fvrange[1], npts, 1, 1)
    tcv = make_ramp(cvrange[0], cvrange[1], npts, 1, 1)
    
    Xk, Xfv = np.meshgrid(tk, tfv, indexing='ij')\

    term = np.cos(2*m.pi*np.sqrt(1-Xk**2+0J)*Xfv/fsamp).real
    
    a1 = 2*term*np.exp(-Xk*2*m.pi*Xfv/fsamp)
    if condition == 1:
        a2 = 1-a1
    else:
        a2 = -np.exp(-Xk*4*m.pi*Xfv/fsamp)
    a1b = np.expand_dims(a1, axis=0)
    a2b = np.expand_dims(a2, axis=0)
    
    res = arcoeff_to_psd(np.concatenate((-a1b, -a2b)), tcv, npoints, fsamp, loga)
    psd_model = res.psd
    criterion = np.sum((np.log(psd_model+noise)-np.log(psd_arn2))**2, axis=0)
    
    res = min_array(criterion)
    ind = res.indices
    print 'arn2ar2 3/3 completed'
    '''
    #%%%%%%%%%% output computation %%%%%%%%%%
    a1 = a1[ind[0],ind[1]]
    a2 = a2[ind[0],ind[1]]
    aiturb = np.array([a1, a2])
    cvturb = tcv[ind[2]]
    kf = np.array([tk[ind[0]], tfv[ind[1]]])
    print tk[ind[0]], tfv[ind[1]], tcv[ind[2]]
    return Returnar2(aiturb, cvturb, kf)


#-----------------------------------------------------------------
class Returnvibs(object):
    def __init__(self, vib_fv, vib_k, vib_sigma2, vib_np, vib_fsamp, vib_psd):
        self.vib_fv = vib_fv
        self.vib_k = vib_k
        self.vib_sigma2 = vib_sigma2
        self.vib_np = vib_np
        self.vib_fsamp = vib_fsamp
        self.vib_psd = vib_psd

def find_vibs(data, tabfreq, indw):
    # find_vibs - Determins the parameters of an AR2 fit of the most energetic
    # peak
    # PURPOSE:
    # This function computes and returns the parameters of an AR2 fit of the
    # next most energetic peak in the data . It is done by minimizing
    # J(k,fv,sigma2)= total( |psdres(f)-psdv(k,fv,sigma2)|^2/psdfit(f)^2 ),
    # with sdv(k,fv,sigma2)= arcoeff_to_psd(-ai, 1, npoints, fsamp, ff = ff)
    # and ai = kfs_to_ar(tk[indi], fmax, fsamp)
    # INPUTS:
    #     data: structure made of the following fields:
    #           psdres:     residual psd (between f=0  and f=fsamp/2)  
    #           psdtot:     total psd (between f=0 and f=fsamp/2)  
    #           psdfit:     fitted psd 
    #           npoints:    number of points
    #           fsamp:      sampling frequency
    #  tabfreq: array of frequencies, in Hertz, corresponding to the various
    #           PSD stored in data
    #     indw: Set this keyword to the frequency region of interest in which
    #           you search for a vibration peak.
    # OUTPUTS:
    #       vib_fv: freqency of the vibration
    #        vib_k: damping coefficient of the vibration
    #   vib_sigma2: amplitude of the vibration
    #       vib_np: number of points used
    #    vib_fsamp: sampling frequency used
    #   est_psdvib: The PSD of the identified vibration peak.
    
    npoints = data.npoints
    fsamp = data.fsamp
    npts = 10
    
    # central frequency
    #temp1 = np.convolve(data.psdres/data.psdfit, np.ones((20,))/20)
    temp1 = data.psdres/data.psdfit
    indmax = np.where(temp1[indw] == max(temp1[indw]))
    fmax = tabfreq[indw]
    if indmax[0] < 1 or indmax[0] > len(temp1)-4:
        fmax = np.squeeze(fmax[indmax[0]+1])
    else:
        fmax = temp1[indmax[0]+range(3)].dot(fmax[indmax[0]+range(3)])/np.sum(temp1[indmax[0]+range(3)])
    # fmax = fmax[indmax[0]+1]
    # fmax = max_vib(tabfreq, temp, indw)
    psd_res = data.psdres[indw]
    npoints2 = psd_res.size
    psd_fit = data.psdfit[indw]
    psd_res = np.tile(psd_res.reshape((npoints2,1,1)), (1,npts,npts))
    psd_fit = np.tile(psd_fit.reshape((npoints2,1,1)), (1,npts,npts))

    # In what follows, we search for k and sigma2 minimizing the criterion
    # total((((data.psdres-psdvib)^2)/data.psdfit^2)[indw])
    # this is done by computing the criterion for a set of (k,sigma^2) on a 2D
    # grid. It is first done on a coarse grid. Then a second exploration is
    # performed on a subspace, to improve the accuracy.
    
    #%%%%%%%%%% coarse gridding %%%%%%%%%%
    krange = np.array([1e-4, 1e-2])
    powrange = np.array([1e-6, 1e1])
    
    tk = make_ramp(krange[0], krange[1], npts, 1, 1)
    tsig = make_ramp(powrange[0], powrange[1], npts, 1, 1)
    
    ai = kfs_to_ar(tk, fmax, fsamp)
    res = arcoeff_to_psd(-ai, tsig, npoints, fsamp, 0, 0, indw)
    psdvib = res.psd
    temp = (psd_res-psdvib)**2/psd_fit**2
    criterion = np.sum(temp, axis=0)
    
    res = min_array(criterion)
    indmin = res.indices
    nk = tk[indmin[0]]
    nsig = tsig[indmin[1]]
    
    #%%%%%%%%%% fine gridding %%%%%%%%%%
    #krange = np.array([.1, 10.])*nk
    #powrange = np.array([.1, 10.])*nsig
    krange = np.array([tk[max(indmin[0]-1,0)], tk[min(indmin[0]+1,npts-1)]])
    powrange = np.array([tsig[max(indmin[1]-1,0)], tsig[min(indmin[1]+1,npts-1)]])
    
    tk = make_ramp(krange[0], krange[1], npts, 1, 1)
    tsig = make_ramp(powrange[0], powrange[1], npts, 1, 1)
    criterion = np.zeros([npts, npts])

    ai = kfs_to_ar(tk, fmax, fsamp)
    res = arcoeff_to_psd(-ai, tsig, npoints, fsamp, 0, 0, indw)
    psdvib = res.psd
    temp = (psd_res-psdvib)**2/psd_fit**2
    criterion = np.sum(temp, axis=0)
    
    res = min_array(criterion)
    indmin = res.indices

    #%%%%%%%%%% outputs %%%%%%%%%%
    vib_fv = np.array([fmax])
    vib_k = np.array([tk[indmin[0]]])
    vib_sigma2 = np.array([tsig[indmin[1]]])
    vib_np = np.array([npoints])
    vib_fsamp = np.array([fsamp])
    ai = ai[:,indmin[0]]
    res = arcoeff_to_psd(-ai, vib_sigma2, npoints, fsamp)
    vib_psd = res.psd
    print vib_fv, vib_k, vib_sigma2
    return Returnvibs(vib_fv, vib_k, vib_sigma2, vib_np, vib_fsamp, vib_psd)


#------------------------------------------------------------------
class Returnpsd(object):
    def __init__(self, psd, cumint):
        self.psd = psd
        self.cumint = cumint

def calc_psd(x):
    # calc_psd - calculate the PSD and the cumulative integral of a set of data.
    # PURPOSE:
    # This function computes the PSD of a set of data.
    # INPUTS:
    #        x: Time sequence to analyze.
    # OUTPUTS:
    #      psd: PSD of the time sequence.
    #   cumint: cumulative integral of the time sequence.
    
    s = np.shape(x)
    npoints = s[0]
    if len(s) == 2:
        ndim = s[1]
    else:
        x = np.reshape(x, [npoints, 1])
        ndim = 1
    psd = np.zeros([npoints//2, ndim])
    cumint = np.zeros([npoints//2, ndim])
    #window = np.hanning(npoints)
    #window /= np.mean(window)
    for i in range(ndim):
        fftxx = nfft.fft(np.squeeze(x[:, i]))/npoints#*window)/npoints
        psdxx = abs(fftxx)**2
        psd[:, i] = psdxx[0:npoints//2]
        cumint[1:npoints//2, i] = 2*np.cumsum(np.squeeze(psd[1:npoints//2, i]))
    
    psd = np.squeeze(psd)
    cumint = np.squeeze(cumint)
    
    return Returnpsd(psd, cumint)


#------------------------------------------------------------------
class Datapsd(object):
    def __init__(self, psdres, psdtot, psdfit, npoints, fsamp):
        self.psdres = psdres
        self.psdtot = psdtot
        self.psdfit = psdfit
        self.npoints = npoints
        self.fsamp = fsamp

class Returnmodel(object):
    def __init__(self, aiturb, cvturb, kf, nvib, vib_ai, vib_sigma2, vib_fv, vib_k, psdsim, noise_level):
        self.aiturb = aiturb
        self.cvturb = cvturb
        self.kf = kf
        self.nvib = nvib
        self.vib_ai = vib_ai
        self.vib_sigma2 = vib_sigma2
        self.vib_fv = vib_fv
        self.vib_k = vib_k
        self.psdsim = psdsim
        self.noise_level = noise_level

def ident_model(x, fsamp, maxpeaks, sharp_threshold, vibrange, freq_ex, novib, condition, display, mk, it2):
    # ident_model - turbulence and vibration identification procedure.
    # PURPOSE:
    # This function computes and returns models for turbulence and vibration
    # peaks made of one AR2 + measurement noise for turbulence, and a sum of n
    # AR2 signals for vibrations
    # INPUTS:
    #                x: Time sequence to analyse.
    #            fsamp: sampling frequency
    #         maxpeaks: maximum number of identified vibrations
    #  sharp_threshold: minimum sharpness of the vibrations
    #         vibrange: frequency range for the vibration identification
    #          freq_ex: Frequency ranges to exclude from the identification
    #            novib: boolean telling if there are no vibrations to identify
    #        condition: boolean to constrain a1 and a2 to a1+a2=1
    #          display: boolean to display plots along the estimation process.
    #             ifig: figure number for the display
    # OUTPUTS:
    #    model: identified vibration model. Structure with the following
    #           fields:
    #                 nvib: number of identified sharp components.
    #                aivib: array containing the a1 and a2 components for each
    #                       identified sharp component.
    #                cvvib: array containing the sigma for each identified
    #                       sharp component.
    #           vibrations: Structure made of the  the following fields:
    #                               fv: central frequency of the identified
    #                                   component.
    #                                k: dampening coefficient of the identified
    #                                   component.
    #                        sigma2vib: energy of the identified component.  
    #                          npoints: number of points of the time sequence
    #                            fsamp: sampling frequency.    
    #    modelturb: identified turbulence model
    #  noise_level: identified noise level
    
    npoints = len(x)
    
    #%%%%%%%%%% A. Turbulence+noise identification %%%%%%%%%%
    
    #%%%%%%%%%% 1. time sequence study %%%%%%%%%%
    
    x -= np.mean(x)
    window = np.hanning(npoints)
    window /= np.mean(window)
    #x *= window
    fftx = nfft.fft(x)/npoints
    res = calc_psd(x)
    psdx = res.psd
    tabfreq = make_ramp(0, fsamp/2, npoints//2)
    xr = np.array([fsamp/npoints, fsamp/2])
    
    concplot = np.zeros((9,npoints//2))

    # noise level: average value of psdx for frequencies between vibrange(3)
    # and fsamp/2
    indnoise = np.where(tabfreq >= vibrange[2])
    noise_level = np.mean(psdx[indnoise])
    
    # AR100 fit of x
    print 'starting ar20 fit'
    arcoef_x, mse_x = ts_to_ar(x, 20)
    
    # corresponding PSD
    res = arcoeff_to_psd(-arcoef_x, mse_x, npoints, fsamp, 0, 1)
    est_psd_x = res.psd
    
    #%%%%%%%%%% 2. Vibrating peaks clipping %%%%%%%%%%
    
    # indices of tabfreq corresponding to medium frequencies, ie between
    # vibrange(1) and vibrange (2)
    indvib = np.where((tabfreq >= vibrange[0]) & (tabfreq <= vibrange[1]))
    rangevib = np.array([np.min(indvib), np.max(indvib)])
    
    val1 = est_psd_x[rangevib[0]]
    val2 = est_psd_x[rangevib[1]]

    # slope in log scale
    a = np.log(val2/val1)/m.log(rangevib[1]/rangevib[0])
    # offset
    b = np.log(val2)-a*m.log(rangevib[1])
    # linear (in log log) spectrum model in the central zone
    prolong = np.exp(b)*indvib**a

    spectrum_lin = cp.deepcopy(psdx)
    spectrum_lin[indvib] = np.abs(prolong+noise_level-val2)
    spectrum_lin[indnoise] = noise_level
    est_psd_lin = np.concatenate((spectrum_lin, spectrum_lin[::-1]))
    
    # est_psd_lin is the linear gauge spectrum, We consider that the part of
    # psdx higher than 2*est_psd_lin correspond to vibrations so we clip it.
    # The clipping is performed on fftx instead of its squared modulus:
    absfft_novib = np.minimum(np.abs(fftx), np.sqrt(4*est_psd_lin))
    absfft_novib = absfft_novib-m.sqrt(noise_level)
    # the phaser is:
    fftx_phaser = fftx/abs(fftx)
    # we gather the two:
    fft_novib = fftx_phaser*absfft_novib
    # the corresponding time sequence is obtained by:
    serie_novib = nfft.ifft(fft_novib)*npoints
    serie_novib = serie_novib[1:npoints-1]
    
    #%%%%%%%%%% 3. AR20 fit on the clipped time sequence %%%%%%%%%%
    
    arcoeff_novib, mse_novib = ts_to_ar(serie_novib, 20)

    # corresponding PSD
    res = arcoeff_to_psd(-arcoeff_novib, mse_novib, npoints, fsamp, 0, 1)
    est_psd_novib = res.psd
    
    #%%%%%%%%%% 4. Fitting an AR2+noise to this AR20+noise %%%%%%%%%%
    print 'starting arn_to_ar2'
    
    modelturb = arn_to_ar2(arcoeff_novib, mse_novib, noise_level, npoints, fsamp, vibrange[2], condition)
    
    print 'Turbulence model: AR coefficients and noise level'
    print modelturb.aiturb
    print noise_level
    
    #%%%%%%%%%% B. Vibration identification %%%%%%%%%%
    
    if novib == 0:
        # PSD corresponding to modelturb+noise
        res = arcoeff_to_psd(-modelturb.aiturb, modelturb.cvturb, npoints, fsamp)
        
        psdfit = res.psd+noise_level
        psdsim = psdfit[:]
        # residual
        psdres = abs(psdx-psdfit)
        data  = Datapsd(psdres, psdx, psdfit, npoints, fsamp)
        
        # initialization
        indw = indvib[:]
        ctabfreq = tabfreq[:]
        cdata = cp.deepcopy(data)

        #if display == 1:
        #    plt.ion()
        #    plt.figure()
        #    plt.loglog(tabfreq[1:], cdata.psdtot[1:], label='Data')
        #    plt.plot(tabfreq[1:], est_psd_x[1:npoints//2], label='AR100')
        #    plt.plot(tabfreq[1:], est_psd_lin[1:npoints//2], label='lin')
        #    plt.plot(tabfreq[1:], (np.abs(fft_novib[1:npoints//2]))**2, label = 'novib')
        #    plt.plot(tabfreq[1:], est_psd_novib[1:npoints//2], label='AR20')
        #    plt.plot(tabfreq[1:], psdfit[1:npoints//2], label='AR2')
        #    #axis([xr, 1, 2], 'auto y')
        #    plt.xlabel('frequency [Hz]')
        #    plt.ylabel('PSD [um^2/Hz]')
        
        i = 0
        docontinue = 1
        countsharp = 0
        n_ex = len(freq_ex)
        
        # estimation loop
        while docontinue == 1:
            cest = find_vibs(cdata, ctabfreq, indw)
            if abs(sum(abs(cdata.psdres)/abs(psdsim)/npoints*2)-1) <= 1e-6:
                docontinue = 0
                print 'Model error:'
                print sum(cest.vib_psd/psdfit)
            else:
                if countsharp == maxpeaks:
                    docontinue = 0
                    #'itmax'
                else:
                    if i == 0:
                        tab_est = cp.deepcopy(cest)
                    else:
                        print tab_est.vib_fv.shape
                        print cest.vib_fv.shape
                        tab_est.vib_fv = np.concatenate((tab_est.vib_fv, cest.vib_fv))
                        tab_est.vib_k = np.concatenate((tab_est.vib_k, cest.vib_k))
                        tab_est.vib_sigma2 = np.concatenate((tab_est.vib_sigma2, cest.vib_sigma2))
                        tab_est.vib_np = np.concatenate((tab_est.vib_np, cest.vib_np))
                        tab_est.vib_fsamp = np.concatenate((tab_est.vib_fsamp, cest.vib_fsamp))
                        if len(tab_est.vib_psd.shape) == 1:
                            tab_est.vib_psd = tab_est.vib_psd.reshape((npoints//2,1))
                        
                        tab_est.vib_psd = np.concatenate((tab_est.vib_psd, cest.vib_psd.reshape((npoints//2,1))), axis=1)
                    
                    if n_ex > 0:
                        indfreq = np.where((tab_est.vib_fv < freq_ex[0]) | (tab_est.vib_fv > freq_ex[n_ex-1]))
                        countfreq = len(indfreq)
                        if n_ex >= 4:
                            for i in range(n_ex/2-1):
                                indfreqtemp = np.where((tab_est.vib_fv > freq_ex[2*i+1]) & (tab_est.vib_fv < freq_ex[2*i+2]))
                                counttemp = len(indfreqtemp)
                                if countfreq == 0:
                                    if counttemp == 0:
                                        indfreq = -1
                                    else:
                                        indfreq = indfreqtemp[:]
                                else:
                                    if counttemp != 0:
                                        indfreq = np.concactenate((indfreq, indfreqtemp))
                                countfreq += counttemp
                        if countfreq == 0:
                            indsmooth = -1
                            count = 0
                            indsharp = -1
                            countsharp = 0
                        else:
                            indsmooth = np.where(tab_est.vib_k >= sharp_threshold)
                            count = len(indsmooth)
                            indsharptemp = np.where(tab_est.vib_k[indfreq] < sharp_threshold)
                            countsharp = len(indsharptemp)
                            indsharp = indfreq[indsharptemp]
                    else:
                        indsmooth = np.where(tab_est.vib_k >= sharp_threshold)
                        count = len(indsmooth)
                        indsharp = np.squeeze(np.where(tab_est.vib_k < sharp_threshold))
                        countsharp = np.size(indsharp)
                        
                    if display == 1:
                        if i == 0:
                            #plt.ion()
                            #fig  = plt.figure(1)
                            #graph = fig.add_subplot(211+mk)
                            #graph.clear()
                            #graph.loglog(tabfreq[1:], cdata.psdtot[1:], label='Data')
                            #if mk:
                            #    plt.xlabel('Frequency [Hz]')
                            #plt.ylabel('PSD [mas^2/Hz]')
                            if countsharp >= 1:
                                psdsim = psdfit+tab_est.vib_psd
                                #plotsim, = plt.plot(tabfreq, psdsim, color='r', label='Model w/ turbulence and vibrations')
                            #graph.plot(tabfreq, psdfit, color='c', label = 'Model w/ turbulence')
                            #plt.legend(loc=3)
                        else:
                            if countsharp == 1:
                                psdsim = psdfit+tab_est.vib_psd[:, indsharp]
                                #plotsim, = graph.plot(tabfreq, psdsim, color='r', label='Model w/ turbulence and vibrations')
                            elif countsharp > 1:
                                psdsim = psdfit+np.sum(tab_est.vib_psd[:, indsharp], axis=1)
                                #plotsim.set_ydata(psdsim)
                                #plt.draw()
                        #if mk == 1:
                            #plt.savefig("./Figures/identmodelPSD_"+str(it2)+".eps", clobber=True)
                        if i ==0:
                            if not os.path.isfile("/tmp/LQG_identplot.im.shm"):
                                os.system("creashmim LQG_identplot %d %d" % (npoints//2,9))
                            concplotshm = shm("/tmp/LQG_identplot.im.shm", verbose = False)
                            concplot = concplotshm.get_data()
                        concplot[0,:] = tabfreq
                        concplot[4*mk+1:4*mk+5,:] = np.vstack((cdata.psdtot, est_psd_novib[0:npoints//2], psdfit, psdsim))
                        concplotshm.set_data(concplot.astype(np.float32))
                    cdata = Datapsd(abs(cdata.psdres-cest.vib_psd), cdata.psdtot, abs(cdata.psdfit+cest.vib_psd), npoints, fsamp)

                    print 'Estimated residual and vib. parameters (f, k):'
                    print [sum(psdx/psdsim*noise_level), cest.vib_fv[0], cest.vib_k[0]]
                    i += 1
        
        # Output computation
        
        #if count > 0:
            #'smooth components:'
            #temp = struct2cell(tab_est_vib);
            #size(temp)
            #temp(indsmooth, :)
        
        if countsharp > 0:
            # 'sharp components:'
            vib_ai = np.zeros([countsharp, 2])
            vib_sigma2 = tab_est.vib_sigma2[indsharp]
            vib_fv = tab_est.vib_fv[indsharp]
            vib_k = tab_est.vib_k[indsharp]
            print 'vib. frequencies:'
            print vib_fv
            print 'vib. damping coefficient:'
            print vib_k
            print 'vib. variance:'
            print vib_sigma2
            if countsharp == 1:
                vib_ai[0, :] = kfs_to_ar(tab_est.vib_k[indsharp], tab_est.vib_fv[indsharp], tab_est.vib_fsamp[indsharp])
            else:
                for j in range(countsharp):
                    #tab_est_vib(indsharp(j))
                    vib_ai[j, :] = kfs_to_ar(tab_est.vib_k[indsharp[j]], tab_est.vib_fv[indsharp[j]], tab_est.vib_fsamp[indsharp[j]])
            nvib = countsharp
        else:
            nvib = 0
            vib_ai = 0
            vib_sigma2 = 0
            vib_fv = 0
            vib_k = 0
        
        aiturb = modelturb.aiturb
        cvturb = modelturb.cvturb
        kf = modelturb.kf

    return Returnmodel(aiturb, cvturb, kf, nvib, vib_ai, vib_sigma2, vib_fv, vib_k, psdsim, noise_level)
