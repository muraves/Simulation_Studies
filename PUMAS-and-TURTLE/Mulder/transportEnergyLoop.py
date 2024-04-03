#! /usr/bin/env python3
"""
The Fluxmeter.transport method takes as input an observation mulder.State and
returns a conjugated state, transported to the reference's height where the
corresponding muon flux is known. The returned conjugated State carries an
additional transport weight resulting from two multiplicative factors:

- A Jacobian factor, expressing the conservation of the total number of muons
  during the transport, in the absence of sources and sinks.
  

- An attenuation factor representing muon decays. Note that for reverse
  transport this term is actually a regeneration factor, i.e. it is larger than
  one.

Depending on the Fluxmeter.mode flag, different transport algorithms are used.
Possible values for the mode flag are "continuous", "discrete" or "mixed". The
corresponding algorithms are discussed below, together with examples of
application.

Note:
  This example uses data produced by the `basic/layer.py` and
  `basic/reference.py` examples. Please, run these examples first.
"""

import matplotlib.pyplot as plot
from mulder import Fluxmeter, Reference, State
import numpy


# =============================================================================
# As a preamble, let us define a geometry and let us instanciate thecd
# corresponding fluxmeter. Following the `basic/fluxmeter.py` example, this is
# is done in a single step (using brief syntax) as

fluxmeter = Fluxmeter(
    Rock  = "../../mulder/data/GMRT.png",
)

# The observation state
rock = fluxmeter.geometry.layers[0]
xMURAVES = 14.411708
yMURAVES = 40.810251
latitudeMURAVES, longitudeMURAVES, heightMURAVES = rock.position(xMURAVES, yMURAVES)

energy_min = 0.8
energy_max = 3000.


ffile = open("Flux.txt", 'a')

for DefineAZIMUTH in range (0, 361): # deg (i.e. looking towards East)
  for DefineELEVATION in range (0, 91):  # deg (i.e. 30 deg below Zenith)
    flux_oneAngle = []
    kf_values = []
    for i in range (20000):
      kf = numpy.exp(numpy.random.uniform(numpy.log(energy_min), numpy.log(energy_max)))
      kf_values.append(kf)
      
      s_obs = State(
        position = rock.position(xMURAVES, yMURAVES),
        azimuth = DefineAZIMUTH,
        elevation = DefineELEVATION,
        energy = kf
      )
      # Let us point out that, by default mulder.States have aweight of one.
      assert(s_obs.weight == 1)
      
      fluxmeter.geometry.geomagnet = True

      fluxmeter.mode = "discrete"
      s_ref = fluxmeter.transport(s_obs)
  
      flux = s_ref.flux(fluxmeter.reference)
      flux_oneAngle.append(flux.value)
      #print(flux_oneAngle[:10])
    flux_mean = numpy.mean(flux_oneAngle)
    ffile.write(f"""{DefineAZIMUTH} {DefineELEVATION} {flux_mean}\n""")
    print(f"{DefineAZIMUTH} {DefineELEVATION}")

ffile.close()