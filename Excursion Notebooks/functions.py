# %%
import numpy as np
import warnings
def ang2vec(intensity, inc, dec):
    """
    Convert intensity, inclination and  declination to a 3-component vector

    .. note:: Coordinate system is assumed to be x->North, y->East, z->Down.
        Inclination is positive down and declination is measured with respect
        to x (North).

    Parameter:

    * intensity : float or array
        The intensity (norm) of the vector
    * inc : float
        The inclination of the vector (in degrees)
    * dec : float
        The declination of the vector (in degrees)

    Returns:

    * vec : array = [x, y, z]
        The vector

    Examples::

        >>> import np
        >>> print ang2vec(3, 45, 45)
        [ 1.5         1.5         2.12132034]
        >>> print ang2vec(np.arange(4), 45, 45)
        [[ 0.          0.          0.        ]
         [ 0.5         0.5         0.70710678]
         [ 1.          1.          1.41421356]
         [ 1.5         1.5         2.12132034]]

    """
    return np.transpose([intensity * i for i in dircos(inc, dec)])
def dircos(inc, dec):
    """
    Returns the 3 coordinates of a unit vector given its inclination and
    declination.

    .. note:: Coordinate system is assumed to be x->North, y->East, z->Down.
        Inclination is positive down and declination is measured with respect
        to x (North).

    Parameter:

    * inc : float
        The inclination of the vector (in degrees)
    * dec : float
        The declination of the vector (in degrees)

    Returns:

    * vect : list = [x, y, z]
        The unit vector

    """
    d2r = np.pi / 180.
    vect = [np.cos(d2r * inc) * np.cos(d2r * dec),
            np.cos(d2r * inc) * np.sin(d2r * dec),
            np.sin(d2r * inc)]
    return vect
def _fftfreqs(x, y, shape, padshape):
    """
    Get two 2D-arrays with the wave numbers in the x and y directions.
    """
    nx, ny = shape
    dx = (x.max() - x.min())/(nx - 1)
    fx = 2*np.pi*np.fft.fftfreq(padshape[0], dx)
    dy = (y.max() - y.min())/(ny - 1)
    fy = 2*np.pi*np.fft.fftfreq(padshape[1], dy)
    return np.meshgrid(fy, fx)[::-1]
def reduce_to_pole(x, y, data, shape, inc, dec, sinc, sdec):
    r"""
    Reduce total field magnetic anomaly data to the pole.

    The reduction to the pole if a phase transformation that can be applied to
    total field magnetic anomaly data. It "simulates" how the data would be if
    **both** the Geomagnetic field and the magnetization of the source were
    vertical (:math:`90^\circ` inclination) (Blakely, 1996).

    This functions performs the reduction in the frequency domain (using the
    FFT). The transform filter is (in the frequency domain):

    .. math::

        RTP(k_x, k_y) = \frac{|k|}{
            a_1 k_x^2 + a_2 k_y^2 + a_3 k_x k_y +
            i|k|(b_1 k_x + b_2 k_y)}

    in which :math:`k_x` and :math:`k_y` are the wave-numbers in the x and y
    directions and

    .. math::

        |k| = \sqrt{k_x^2 + k_y^2} \\
        a_1 = m_z f_z - m_x f_x \\
        a_2 = m_z f_z - m_y f_y \\
        a_3 = -m_y f_x - m_x f_y \\
        b_1 = m_x f_z + m_z f_x \\
        b_2 = m_y f_z + m_z f_y

    :math:`\mathbf{m} = (m_x, m_y, m_z)` is the unit-vector of the total
    magnetization of the source and
    :math:`\mathbf{f} = (f_x, f_y, f_z)` is the unit-vector of the Geomagnetic
    field.

    .. note:: Requires gridded data.

    .. warning::

        The magnetization direction of the anomaly source is crucial to the
        reduction-to-the-pole.
        **Wrong values of *sinc* and *sdec* will lead to a wrong reduction.**

    Parameters:

    * x, y : 1d-arrays
        The x, y, z coordinates of each data point.
    * data : 1d-array
        The total field anomaly data at each point.
    * shape : tuple = (nx, ny)
        The shape of the data grid
    * inc, dec : floats
        The inclination and declination of the inducing Geomagnetic field
    * sinc, sdec : floats
        The inclination and declination of the total magnetization of the
        anomaly source. The total magnetization is the vector sum of the
        induced and remanent magnetization. If there is only induced
        magnetization, use the *inc* and *dec* of the Geomagnetic field.

    Returns:

    * rtp : 1d-array
        The data reduced to the pole.

    References:

    Blakely, R. J. (1996), Potential Theory in Gravity and Magnetic
    Applications, Cambridge University Press.

    """
    fx, fy, fz = ang2vec(1, inc, dec)
    if sinc is None or sdec is None:
        mx, my, mz = fx, fy, fz
    else:
        mx, my, mz = ang2vec(1, sinc, sdec)
    kx, ky = [k for k in _fftfreqs(x, y, shape, shape)]
    kz_sqr = kx**2 + ky**2
    a1 = mz*fz - mx*fx
    a2 = mz*fz - my*fy
    a3 = -my*fx - mx*fy
    b1 = mx*fz + mz*fx
    b2 = my*fz + mz*fy
    # The division gives a RuntimeWarning because of the zero frequency term.
    # This suppresses the warning.
    with np.errstate(divide='ignore', invalid='ignore'):
        rtp = (kz_sqr)/(a1*kx**2 + a2*ky**2 + a3*kx*ky +
                        1j*np.sqrt(kz_sqr)*(b1*kx + b2*ky))
    rtp[0, 0] = 0
    ft_pole = rtp*np.fft.fft2(np.reshape(data, shape))
    return np.real(np.fft.ifft2(ft_pole)).ravel()
def upcontinue(x, y, data, shape, height):
    r"""
    Upward continuation of potential field data.

    Calculates the continuation through the Fast Fourier Transform in the
    wavenumber domain (Blakely, 1996):

    .. math::

        F\{h_{up}\} = F\{h\} e^{-\Delta z |k|}

    and then transformed back to the space domain. :math:`h_{up}` is the upward
    continue data, :math:`\Delta z` is the height increase, :math:`F` denotes
    the Fourier Transform,  and :math:`|k|` is the wavenumber modulus.

    .. note:: Requires gridded data.

    .. note:: x, y, z and height should be in meters.

    .. note::

        It is not possible to get the FFT of a masked grid. The default
        :func:`fatiando.gridder.interp` call using minimum curvature will not
        be suitable.  Use ``extrapolate=True`` or ``algorithm='nearest'`` to
        get an unmasked grid.

    Parameters:

    * x, y : 1D-arrays
        The x and y coordinates of the grid points
    * data : 1D-array
        The potential field at the grid points
    * shape : tuple = (nx, ny)
        The shape of the grid
    * height : float
        The height increase (delta z) in meters.

    Returns:

    * cont : array
        The upward continued data

    References:

    Blakely, R. J. (1996), Potential Theory in Gravity and Magnetic
    Applications, Cambridge University Press.

    """
    assert x.shape == y.shape, \
        "x and y arrays must have same shape"
    if height <= 0:
        warnings.warn("Using 'height' <= 0 means downward continuation, " +
                      "which is known to be unstable.")
    nx, ny = shape
    # Pad the array with the edge values to avoid instability
    padded, padx, pady = _pad_data(data, shape)
    kx, ky = _fftfreqs(x, y, shape, padded.shape)
    kz = np.sqrt(kx**2 + ky**2)
    upcont_ft = np.fft.fft2(padded)*np.exp(-height*kz)
    cont = np.real(np.fft.ifft2(upcont_ft))
    # Remove padding
    cont = cont[padx: padx + nx, pady: pady + ny].ravel()
    return cont
def _pad_data(data, shape):
    n = _nextpow2(np.max(shape))
    nx, ny = shape
    padx = (n - nx)//2
    pady = (n - ny)//2
    padded = np.pad(data.reshape(shape), ((padx, padx), (pady, pady)),
                        mode='edge')
    return padded, padx, pady
def _nextpow2(i):
    buf = np.ceil(np.log(i)/np.log(2))
    return int(2**buf)



# %%
