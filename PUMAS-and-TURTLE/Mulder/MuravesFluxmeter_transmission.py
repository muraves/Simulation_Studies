#!/usr/bin/env python3
"""
Mulder 0.3.7

Calculates:
- Open sky MCEq flux
- Rock transmitted flux
- Transmission = Phi_rock / Phi_open
- Monte Carlo uncertainty from discrete transport

"""

import os
import argparse
import numpy as np
import mulder


# ==============================================================
# Arguments
# ==============================================================

parser = argparse.ArgumentParser()

parser.add_argument("--phi-min", type=float, required=True)
parser.add_argument("--phi-max", type=float, required=True)
parser.add_argument("--tag", type=str, required=True)

parser.add_argument("--rho", type=float, default=2.65E+03)

parser.add_argument("--el-min", type=float, required=True)
parser.add_argument("--el-max", type=float, required=True)

parser.add_argument("--n-E", type=int, default=10000)
parser.add_argument("--nEvents", type=int, default=100)

parser.add_argument("--d-phi", type=float, default=1.)
parser.add_argument("--d-el", type=float, default=1.)

parser.add_argument("--output-path",
                    type=str,
                    default="./mulder_output")
parser.add_argument("--input-path", type=str, default="./")

args = parser.parse_args()


# ==============================================================
# Geometry
# ==============================================================

'''geometry = mulder.EarthGeometry(
    mulder.Layer(
        args.input_path + "vesuvius_latlon.asc",
        density=args.rho,
        material="Rock",
    ),
)'''

geometry = mulder.EarthGeometry(
    mulder.Layer(
        mulder.Grid(args.input_path + "vesuvio_5m_cut.asc", crs=32633),
        density=args.rho,
        material="Rock",
    ),
)


# ==============================================================
# Fluxmeter
# ==============================================================

fluxmeter = mulder.Fluxmeter(
    geometry=geometry
)

fluxmeter.mode = "discrete"

fluxmeter.reference = args.input_path + "flux-mceq-yfm-gsf-usstd.table"

# ==============================================================
# Observation point
# ==============================================================

rock = geometry.layers[0]

latitude = 40.810251
longitude = 14.411708


topo = rock.altitude(
    latitude,
    longitude
)
print("Detector altitude:", topo)


print(f"Observation altitude: {topo:.2f} m")


# ==============================================================
# Energy grid
# ==============================================================

E_min = 0.9
E_max = 3000.0
n_E = args.n_E


energy = np.logspace(
    np.log10(E_min),
    np.log10(E_max),
    n_E
)


# ==============================================================
# Angular grid
# ==============================================================

#az_vals = np.arange(args.phi_min, args.phi_max, args.d_phi)
#el_vals = np.arange(args.el_min, args.el_max, args.d_el)

# use linspace when working with float steps
n_steps_phi = round((args.phi_max - args.phi_min) / args.d_phi)
az_vals = np.linspace(args.phi_min, args.phi_min + n_steps_phi * args.d_phi, n_steps_phi, endpoint=False)

n_steps_el = round((args.el_max - args.el_min) / args.d_el)
el_vals = np.linspace(args.el_min, args.el_min + n_steps_el * args.d_el, n_steps_el, endpoint=False)

n_az = len(az_vals)
n_el = len(el_vals)


print(f"Running azimuth range: {args.phi_min} to {args.phi_max}", flush=True)
print(f"Number of azimuth bins: {n_az}", flush=True)
print(f"Number of elevation bins: {n_el}", flush=True)
print(f"Total angular bins in this job: {n_az * n_el}", flush=True)


# ==============================================================
# Output
# ==============================================================

os.makedirs(
    args.output_path,
    exist_ok=True
)


outfile = os.path.join(
    args.output_path,
    f"flux_5m_{args.d_phi}bin_rho{args.rho}_discrete_100T_{args.tag}.txt"
)


N_EVENTS = args.nEvents


# ==============================================================
# Main calculation
# ==============================================================

with open(outfile, "w") as ffile:

    #ffile.write("# azimuth elevation " "phi_rock phi_rock_err " "phi_open transmission transmission_err " "N_events\n")
    for i, az in enumerate(az_vals):

        for j, el in enumerate(el_vals):


            # --------------------------------------------------
            # Observation state
            # --------------------------------------------------

            s_obs = mulder.GeographicStates(
                latitude=latitude,
                longitude=longitude,
                altitude=topo,
                azimuth=az,
                elevation=el,
                energy=energy,
            )


            # --------------------------------------------------
            # OPEN SKY FLUX
            # No geometry
            # --------------------------------------------------

            # Let us recall that, by default, the fluxmeter is configured in continuous
            # mode, which we will use for this example. This is usually a good approximation
            # for opensky geometries, owing to the low density of the atmosphere.
            flux_open_E = fluxmeter.reference.flux(s_obs)


            flux_open = np.trapezoid(flux_open_E, energy)


            # --------------------------------------------------
            # ROCK TRANSMISSION
            # --------------------------------------------------

            s_ref = fluxmeter.transport(s_obs, events=N_EVENTS) # topology enters here

            flux_ref = np.asarray(fluxmeter.reference.flux(s_ref))

            weight = np.asarray(s_ref.weight)

            values = flux_ref * weight
   
            values = values.reshape(n_E, N_EVENTS)


            # integrate over energy
            flux_int = np.trapezoid(values, energy, axis=0)

            flux_rock = np.mean(flux_int)

            # MC error
            if N_EVENTS > 1:
                flux_rock_err = np.std(flux_int, ddof=1) / np.sqrt(N_EVENTS)

            else:
                flux_rock_err = 0.0

            # --------------------------------------------------
            # Transmission
            # --------------------------------------------------

            transmission = flux_rock / flux_open
    
            transmission_err = flux_rock_err / flux_open # no error on flux_open

            # --------------------------------------------------
            # Save
            # --------------------------------------------------

            ffile.write(
                f"{az + 136:.2f} "
                f"{el:.2f} "
                f"{flux_rock:.6e} "
                f"{flux_rock_err:.6e} "
                f"{flux_open:.6e} "
                f"{transmission:.6e} "
                f"{transmission_err:.6e} "
                f"{N_EVENTS}\n"
            )

            print(
                f"[{args.tag}] "
                f"rho={args.rho} "
                f"az={az:.1f} "
                f"el={el:.1f} "
                f"T={transmission:.4e} "
                f"{i*n_el+j+1}/{n_az*n_el}",
                flush=True
            )

print(
    f"Finished. Output written to: {outfile}",
    flush=True
)