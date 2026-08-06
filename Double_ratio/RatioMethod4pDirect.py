#!/usr/bin/env python3
import ROOT
import math

# -----------------------------
# User settings
# -----------------------------
rho = 265

selectedfs  = "Counts_N20_fs_SelectionDirect4p_flipped.txt"
selectedVes = "Counts_N20_SelectionDirect4p_flipped.txt"

reffile_fs  = "../openskyMCEq_598_02bin.txt"
reffile_Ves = f"../HTCondor/flux_5mNew_02bin_rho{rho}_discrete_1M_az150_merged.txt"

# -----------------------------
# 1-degree output grid
#
# Data azimuth bin low edges: 151 ... 210
# Therefore histogram edges: 151 ... 211
# Elevation bin low edges: 0 ... 39
# Therefore histogram edges: 0 ... 40
# -----------------------------
az_min, az_max = 170.0, 190.0
el_min, el_max = 18.0, 25.0

n_az = int(az_max - az_min)   # 60 bins
n_el = int(el_max - el_min)   # 40 bins


def make_h2(name, title):
    h = ROOT.TH2D(
        name,
        title,
        n_az, az_min, az_max,
        n_el, el_min, el_max
    )
    h.Sumw2()
    h.SetContour(255)
    return h


h_fs = make_h2(
    "h_fs",
    "Freesky data;Azimuth [deg];Elevation [deg]"
)

h_ves = make_h2(
    "h_ves",
    "Vesuvius data;Azimuth [deg];Elevation [deg]"
)

h_fs_sim = make_h2(
    "h_fs_sim",
    "Freesky simulation averaged to 1 deg bins;Azimuth [deg];Elevation [deg]"
)

h_ves_sim = make_h2(
    "h_ves_sim",
    "Vesuvius simulation averaged to 1 deg bins;Azimuth [deg];Elevation [deg]"
)

h_final = make_h2(
    "h_final",
    f"Double ratio NERO WP20, #rho_{{sim}} = {rho/100.0:.2f} g/cm^{{3}};"
    "Azimuth [deg];Elevation [deg]"
)


# --------------------------------------------------
# Reader for 1-degree data files:
# format:
#   azimuth  elevation  count_per_degree_per_hour
#
# The file values are bin-low-edge coordinates,
# so use az+0.5 and el+0.5 to fill bin centers.
# --------------------------------------------------
def fill_data_1deg(h, filename, label="data"):
    n_used = 0
    n_outside = 0

    with open(filename, "r") as f:
        for line in f:
            s = line.strip()

            if not s or s.startswith("#"):
                continue

            parts = s.split()
            if len(parts) < 3:
                continue

            try:
                az  = float(parts[0])
                el  = float(parts[1])
                val = float(parts[2])
            except ValueError:
                continue

            if not math.isfinite(val):
                continue

            if az_min <= az < az_max and el_min <= el < el_max:
                ix = h.GetXaxis().FindBin(az + 0.5)
                iy = h.GetYaxis().FindBin(el + 0.5)

                h.SetBinContent(ix, iy, val)
                n_used += 1
            else:
                n_outside += 1

    print(f"\n{label}: {filename}")
    print(f"  Used 1-degree data bins: {n_used}")
    print(f"  Skipped outside range: {n_outside}")


# --------------------------------------------------
# Reader for 0.2-degree simulation files:
# format:
#   azimuth  elevation  flux
#
# For each 1-degree bin, collect the nearby 25 fine bins.
# The value assigned to the 1-degree bin is:
#
#   average of non-zero fine-bin values
#
# not average over all 25 bins.
# --------------------------------------------------
def fill_sim_02deg_average_nonzero_to_1deg(h, filename, label="simulation"):
    fine_sum_nonzero = {}
    fine_count_nonzero = {}
    fine_count_total = {}

    n_used_total = 0
    n_used_nonzero = 0
    n_outside = 0

    with open(filename, "r") as f:
        for line in f:
            s = line.strip()

            if not s or s.startswith("#"):
                continue

            parts = s.split()
            if len(parts) < 3:
                continue

            try:
                az  = float(parts[0])
                el  = float(parts[1])
                val = float(parts[2])
            except ValueError:
                # skip non-numeric header lines
                continue

            if not math.isfinite(val):
                continue

            if not (az_min <= az < az_max and el_min <= el < el_max):
                n_outside += 1
                continue

            # Convert 0.2-degree point into corresponding 1-degree bin
            ix = int(math.floor((az - az_min) / 1.0)) + 1
            iy = int(math.floor((el - el_min) / 1.0)) + 1

            if not (1 <= ix <= n_az and 1 <= iy <= n_el):
                continue

            key = (ix, iy)

            fine_count_total[key] = fine_count_total.get(key, 0) + 1
            n_used_total += 1

            # Average only non-zero fine bins
            if val > 0.0:
                fine_sum_nonzero[key] = fine_sum_nonzero.get(key, 0.0) + val
                fine_count_nonzero[key] = fine_count_nonzero.get(key, 0) + 1
                n_used_nonzero += 1

    # Set averaged non-zero flux value in each 1-degree bin
    for ix in range(1, h.GetNbinsX() + 1):
        for iy in range(1, h.GetNbinsY() + 1):
            key = (ix, iy)

            n_nonzero = fine_count_nonzero.get(key, 0)

            if n_nonzero > 0:
                avg_nonzero = fine_sum_nonzero[key] / n_nonzero
                h.SetBinContent(ix, iy, avg_nonzero)
                h.SetBinError(ix, iy, 0.0)
            else:
                h.SetBinContent(ix, iy, 0.0)
                h.SetBinError(ix, iy, 0.0)

    print(f"\n{label}: {filename}")
    print(f"  Used fine 0.2-degree points in selected range: {n_used_total}")
    print(f"  Non-zero fine points used in averages: {n_used_nonzero}")
    print(f"  Skipped outside range: {n_outside}")

    expected_total = n_az * n_el * 25
    print(f"  Expected fine points for full 0.2-degree coverage: {expected_total}")

    bad_bins = []
    zero_bins = []

    for ix in range(1, h.GetNbinsX() + 1):
        for iy in range(1, h.GetNbinsY() + 1):
            n_total = fine_count_total.get((ix, iy), 0)
            n_nonzero = fine_count_nonzero.get((ix, iy), 0)

            if n_total != 25:
                az_low = h.GetXaxis().GetBinLowEdge(ix)
                el_low = h.GetYaxis().GetBinLowEdge(iy)
                bad_bins.append((az_low, el_low, n_total))

            if n_nonzero == 0:
                az_low = h.GetXaxis().GetBinLowEdge(ix)
                el_low = h.GetYaxis().GetBinLowEdge(iy)
                zero_bins.append((az_low, el_low))

    if bad_bins:
        print("  [WARNING] Some 1-degree bins do not have 25 fine points:")
        for az_low, el_low, n_total in bad_bins[:20]:
            print(
                f"    az=[{az_low:.0f},{az_low+1:.0f}), "
                f"el=[{el_low:.0f},{el_low+1:.0f}) has {n_total} fine points"
            )
        if len(bad_bins) > 20:
            print(f"    ... and {len(bad_bins)-20} more bins")

    print(f"  1-degree bins with no non-zero simulation flux: {len(zero_bins)}")


# -----------------------------
# Fill data maps
# -----------------------------
fill_data_1deg(h_fs, selectedfs, label="Freesky data")
fill_data_1deg(h_ves, selectedVes, label="Vesuvius data")

# -----------------------------
# Fill simulation maps
# -----------------------------
fill_sim_02deg_average_nonzero_to_1deg(
    h_fs_sim,
    reffile_fs,
    label="Freesky simulation"
)

fill_sim_02deg_average_nonzero_to_1deg(
    h_ves_sim,
    reffile_Ves,
    label="Vesuvius simulation"
)


# -----------------------------
# Double ratio
#
# D = [(N_Ves / N_FS)_data] / [(Phi_Ves / Phi_FS)_sim]
# -----------------------------
for ix in range(1, h_final.GetNbinsX() + 1):
    for iy in range(1, h_final.GetNbinsY() + 1):

        fs_data  = h_fs.GetBinContent(ix, iy)
        ves_data = h_ves.GetBinContent(ix, iy)

        fs_sim  = h_fs_sim.GetBinContent(ix, iy)
        ves_sim = h_ves_sim.GetBinContent(ix, iy)

        if fs_data <= 0 or ves_data <= 0 or fs_sim <= 0 or ves_sim <= 0:
            h_final.SetBinContent(ix, iy, 0.0)
            h_final.SetBinError(ix, iy, 0.0)
            continue

        data_ratio = ves_data / fs_data
        sim_ratio  = ves_sim / fs_sim

        if sim_ratio <= 0 or not math.isfinite(sim_ratio):
            h_final.SetBinContent(ix, iy, 0.0)
            h_final.SetBinError(ix, iy, 0.0)
            continue

        D = data_ratio / sim_ratio

        if not math.isfinite(D):
            h_final.SetBinContent(ix, iy, 0.0)
            h_final.SetBinError(ix, iy, 0.0)
            continue

        h_final.SetBinContent(ix, iy, D)
        h_final.SetBinError(ix, iy, 0.0)


# -----------------------------
# Optional: also make numerator and denominator maps
# -----------------------------
h_num = make_h2(
    "h_num",
    "(Ves/FS)_{data};Azimuth [deg];Elevation [deg]"
)

h_den = make_h2(
    "h_den",
    "(Ves/FS)_{sim};Azimuth [deg];Elevation [deg]"
)

for ix in range(1, h_final.GetNbinsX() + 1):
    for iy in range(1, h_final.GetNbinsY() + 1):

        fs_data  = h_fs.GetBinContent(ix, iy)
        ves_data = h_ves.GetBinContent(ix, iy)
        fs_sim   = h_fs_sim.GetBinContent(ix, iy)
        ves_sim  = h_ves_sim.GetBinContent(ix, iy)

        if fs_data > 0 and ves_data > 0:
            h_num.SetBinContent(ix, iy, ves_data / fs_data)

        if fs_sim > 0 and ves_sim > 0:
            h_den.SetBinContent(ix, iy, ves_sim / fs_sim)


# -----------------------------
# Draw double ratio
# -----------------------------
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette(ROOT.kTemperatureMap)
ROOT.gStyle.SetNumberContours(255)
ROOT.gStyle.SetPaintTextFormat("4.2f")

c = ROOT.TCanvas("c_double_ratio", "Double ratio", 1200, 750)
c.SetRightMargin(0.15)
c.SetLeftMargin(0.10)
c.SetBottomMargin(0.11)

h_final.SetStats(0)
h_final.GetZaxis().SetTitle("D = (N_{Ves}/N_{FS}) / (#Phi_{Ves}/#Phi_{FS})")
h_final.GetZaxis().SetTitleOffset(1.3)

# Choose one:
h_final.Draw("COLZ TEXT")
# h_final.Draw("COLZ")

c.Update()
c.SaveAs("DoubleRatio_NERO_WP20_data151to210_simAvgNonzero.pdf")

ROOT.gApplication.Run()