#!/usr/bin/env python3
"""
Diagnose a flux kink vs a smooth rock-thickness profile, at fixed azimuth.

For one azimuth column it:
  1. Reads the Mulder flux .txt output (flux_rock, flux_open, etc.)
  2. Re-scans rock thickness with Mulder's geometry for the SAME az/el grid
     (so the two are guaranteed to be sampled identically)
  3. Estimates the minimum surface muon energy E_min(theta) needed to
     survive that slant thickness, using the standard continuous energy
     loss approximation  dE/dx = -(a + b*E)  for standard rock:
         a ~ 2.60e-3 GeV cm^2/g   (ionization term)
         b ~ 3.10e-6 cm^2/g       (radiative term)
     Closed form:  E_min = (a/b) * (exp(b*X) - 1)
     with X = slant grammage in g/cm^2 = thickness[m] * 100 * rho[g/cm3]
  4. Plots flux, thickness and E_min vs elevation on stacked panels for
     elevations 8-16 deg, so you can see whether the flux kink lines up
     with a specific E_min (e.g. a table boundary or regime switch in
     your energy-loss model), while thickness itself stays smooth.

Usage:
    python diagnose_flux_kink.py flux_5m_1.0bin_rho3500.0_..._TAG.txt \
        --input-path ./ \
        --az 180 \
        --el-min 8 --el-max 16 \
        --rho 3.5e3
"""

import argparse
import numpy as np
import matplotlib.pyplot as plt
import mulder
import os


# ==============================================================
# Args
# ==============================================================
parser = argparse.ArgumentParser()
parser.add_argument("flux_file", type=str, help="Path to Mulder flux .txt output")
parser.add_argument("--input-path", type=str, default="./",
                     help="Path to the DEM grid used for the geometry (vesuvio_5m_cut.asc)")
parser.add_argument("--az", type=float, required=True,
                     help="Azimuth column to inspect [deg], in the SAME convention as the flux file "
                          "(i.e. az_written = az_sim + 136, see your flux script)")
parser.add_argument("--el-min", type=float, default=8.0)
parser.add_argument("--el-max", type=float, default=16.0)
parser.add_argument("--rho", type=float, default=2.65e3, help="Rock density [kg/m3]")
parser.add_argument("--a-ion", type=float, default=2.60e-3, help="a term [GeV cm2/g]")
parser.add_argument("--b-rad", type=float, default=3.10e-6, help="b term [cm2/g]")
parser.add_argument("--tol", type=float, default=1e-6,
                     help="Tolerance [deg] for matching azimuth values in the flux file")
parser.add_argument("--output", type=str, default="flux_kink_diagnosis.png")
args = parser.parse_args()

os.makedirs("images/diagnostics_flux_kink", exist_ok=True)
name = f"flux_kink_diagnosis_az{args.az}_el{args.el_min}-{args.el_max}_rho{args.rho:.3g}.png"
image_path = os.path.join("images/diagnostics_flux_kink", name)


# ==============================================================
# 1. Load flux file, select one azimuth column, elevation window
# ==============================================================
df = np.genfromtxt(args.flux_file, comments="#", names=[
    "phi", "theta", "flux_rock", "flux_rock_err",
    "flux_open", "transmission", "transmission_err", "n_events",
])

mask_az = np.isclose(df["phi"], args.az, atol=args.tol)
if not mask_az.any():
    unique_az = np.unique(df["phi"])
    nearest = unique_az[np.argmin(np.abs(unique_az - args.az))]
    print(f"No exact match for az={args.az}. Nearest available: {nearest}. Using that instead.")
    mask_az = np.isclose(df["phi"], nearest, atol=args.tol)
    args.az = nearest

sub = df[mask_az]
mask_el = (sub["theta"] >= args.el_min) & (sub["theta"] <= args.el_max)
sub = sub[mask_el]
order = np.argsort(sub["theta"])
sub = sub[order]

el_flux = sub["theta"]
flux_rock = sub["flux_rock"]

print(f"Flux column: az={args.az}, {len(el_flux)} elevation points "
      f"from {el_flux.min():.2f} to {el_flux.max():.2f} deg")


# ==============================================================
# 2. Re-scan rock thickness on the SAME azimuth, fine elevation grid
#    (matching the DEM azimuth convention: flux file has az+136 baked in,
#     so subtract 136 to get back to the geometry's azimuth frame)
# ==============================================================
geometry = mulder.EarthGeometry(
    mulder.Layer(
        mulder.Grid(args.input_path + "vesuvio_5m_cut.asc", crs=32633),
        density=args.rho,
        material="Rock",
    ),
)

latitude = 40.810251
longitude = 14.411708
rock_layer = geometry.layers[0]
altitude = rock_layer.altitude(latitude, longitude)

az_geom = args.az - 136.0  # undo the +136 shift applied when writing the flux file

el_fine = np.linspace(args.el_min, args.el_max, 400)
az_fine = np.full_like(el_fine, az_geom)

thickness = geometry.scan(
    latitude=latitude,
    longitude=longitude,
    altitude=altitude,
    azimuth=az_fine,
    elevation=el_fine,
    output="thickness",
)[..., 0]


# ==============================================================
# 3. Convert thickness -> slant grammage -> E_min
# ==============================================================
rho_g_cm3 = args.rho / 1000.0            # kg/m3 -> g/cm3
X = thickness * 100.0 * rho_g_cm3        # g/cm2

a, b = args.a_ion, args.b_rad
E_min = (a / b) * (np.expm1(b * X))      # GeV


# ==============================================================
# 4. Plot
# ==============================================================
fig, axes = plt.subplots(3, 1, sharex=True, figsize=(9, 10))

ax0, ax1, ax2 = axes

ax0.plot(el_flux, flux_rock, "o-", ms=3, color="tab:blue")
ax0.set_yscale("log")
ax0.set_ylabel(r"Flux [$\mathrm{m^{-2}\,s^{-1}\,sr^{-1}}$]")
ax0.set_title(f"Azimuth = {args.az:.1f} deg  |  rho = {args.rho:.3g} kg/m3")
ax0.grid(alpha=0.3, which="both")

ax1.plot(el_fine, thickness, "-", color="tab:orange")
ax1.set_ylabel("Rock thickness [m]")
ax1.grid(alpha=0.3)

ax2.plot(el_fine, E_min, "-", color="tab:green")
ax2.set_yscale("log")
ax2.set_ylabel(r"Estimated $E_{\min}$ [GeV]")
ax2.set_xlabel("Elevation [deg]", loc="right")
ax2.grid(alpha=0.3, which="both")

plt.tight_layout()
plt.savefig(image_path, dpi=200)
plt.show()

print(f"Saved: {image_path}")
print("\nInspect ax0 (flux) for the kink location in elevation, then read off")
print("the corresponding thickness (ax1) and E_min (ax2) at that elevation.")
print("Check your energy-loss model / table for any hard switch near that E_min,")
print("or any grid boundary in your energy array (np.logspace(0.9, 3000, n_E))")
print("that falls close to that value.")