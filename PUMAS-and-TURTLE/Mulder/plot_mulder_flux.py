import os
import re
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm


def parse_params(filepath):
    base = os.path.basename(filepath)
    patterns = {
        "phi_min":  r"phi(-?\d+\.?\d*)-",
        "phi_max":  r"phi-?\d+\.?\d*-(-?\d+\.?\d*)_",
        "el_min":   r"el(-?\d+\.?\d*)-",
        "el_max":   r"el-?\d+\.?\d*-(-?\d+\.?\d*)_",
        "dphi":     r"dphi(\d+\.?\d*)",
        "del":      r"del(\d+\.?\d*)",
        "rho":      r"rho([\d.]+E?\+?-?\d*)",
        "nE":       r"nE(\d+)",
        "nEvents":  r"nEvents(\d+)",
        "date":     r"_(\d{6})\.txt$",
    }
    params = {}
    for key, pat in patterns.items():
        m = re.search(pat, base)
        params[key] = m.group(1) if m else None
    return params


def human_readable(n):
    """1000000 -> '1M', 500000 -> '500k', 1500000 -> '1.5M'"""
    n = float(n)
    for unit, div in [("M", 1_000_000), ("k", 1_000)]:
        if n >= div:
            val = n / div
            s = f"{val:.1f}".rstrip("0").rstrip(".")
            return f"{s}{unit}"
    return str(int(n))


def build_tag(params):
    rho_val = int(float(params["rho"]))  # "3.5E3" -> 3500
    total_events = int(params["nE"]) * int(params["nEvents"])
    return (
        f"rho{rho_val}"
        f"_{human_readable(total_events)}events"
        f"_d{params['dphi']}"
    )


def edges_from_centers(values):
    values = np.sort(values)
    if len(values) > 1:
        step = np.mean(np.diff(values))
    else:
        step = 1.0
    return np.concatenate([values - step / 2, [values[-1] + step / 2]])


def plot_map(df, column, title, label, filename, cmap="viridis"):
    grid = df.pivot(index="theta_bin", columns="phi_bin", values=column)

    phi_vals = grid.columns.values
    theta_vals = grid.index.values
    values = grid.values

    phi_edges = edges_from_centers(phi_vals)
    theta_edges = edges_from_centers(theta_vals)

    fig, ax = plt.subplots(figsize=(9, 6))

    mesh = ax.pcolormesh(phi_edges, theta_edges, values,
                          shading="flat", cmap=cmap, norm=LogNorm())

    cbar = fig.colorbar(mesh, ax=ax)
    cbar.set_label(label)

    ax.set_xlabel("Azimuth φ [deg]", loc="right")
    ax.set_ylabel("Elevation θ [deg]", loc="top")
    ax.set_title(title)
    
    ax.minorticks_on()

    ax.tick_params(axis='both', which='major', direction='out')
    ax.tick_params(axis='both', which='minor', direction='in')

    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.show()


def main():
    parser = argparse.ArgumentParser(description="Plot flux/transmission maps from Mulder output.")
    parser.add_argument("input_path", type=str, help="Path to the Mulder output .txt file")
    parser.add_argument("--output_path", type=str, default="images",
                         help="Base output directory (default: 'images')")
    args = parser.parse_args()

    filename = args.input_path
    params = parse_params(filename)
    tag = build_tag(params)  # e.g. "rho3500_1Mevents_d0.2"

    dir_FS = os.path.join(args.output_path, "flux_FS")
    dir_Ves = os.path.join(args.output_path, "flux_Ves")
    dir_transmission = os.path.join(args.output_path, "transmission")
    os.makedirs(dir_FS, exist_ok=True)
    os.makedirs(dir_Ves, exist_ok=True)
    os.makedirs(dir_transmission, exist_ok=True)

    outfile_FS = os.path.join(dir_FS, f"flux_FS_{tag}.png")
    outfile_Ves = os.path.join(dir_Ves, f"flux_Ves_{tag}.png")
    outfile_transmission = os.path.join(dir_transmission, f"transmission_{tag}.png")

    # ==========================================================
    # Load data
    # ==========================================================
    df = pd.read_csv(
        filename, sep=r"\s+", comment="#", header=None,
        names=[
            "phi",
            "theta",
            "flux_rock",
            "flux_rock_err",
            "flux_open",
            "transmission",
            "transmission_err",
            "n_events",
        ],
    )

    # Rebin 0.2 degree bins -> 1 degree bins
    df["phi_bin"] = np.floor(df["phi"])
    df["theta_bin"] = np.floor(df["theta"])

    #df["phi_bin"] = df["phi"]
    #df["theta_bin"] = df["theta"]

    df_deg = (
        df.groupby(["theta_bin", "phi_bin"], as_index=False)
        .agg({
            "flux_rock": "mean",
            "flux_open": "mean",
            "flux_rock_err": lambda x: np.sqrt(np.sum(x**2)),
        })
    )

    # Recompute transmission after rebinning and averaging
    df_deg["transmission"] = df_deg["flux_rock"] / df_deg["flux_open"]
    df_deg["transmission_err"] = df_deg["flux_rock_err"] / df_deg["flux_open"]

    # ==========================================================
    # Plot rock flux
    # ==========================================================
    plot_map(
        df_deg,
        "flux_rock",
        f"Rock transmitted muon flux (density {params['rho']} kg/cm3, 1°x1° bins)",
        r"Flux [$\mathrm{m^{-2}\,s^{-1}\,sr^{-1}}$]",
        outfile_FS,
        cmap="viridis",
    )

    # ==========================================================
    # Plot open sky flux
    # ==========================================================
    plot_map(
        df_deg,
        "flux_open",
        f"Open sky MCEq flux (density {params['rho']} kg/cm3, 1°x1° bins)",
        r"Flux [$\mathrm{m^{-2}\,s^{-1}\,sr^{-1}}$]",
        outfile_Ves,
        cmap="viridis",
    )

    # ==========================================================
    # Plot transmission
    # ==========================================================
    plot_map(
        df_deg,
        "transmission",
        f"Muon transmission (density {params['rho']} kg/cm3, 1°x1° bins)",
        "Transmission",
        outfile_transmission,
        cmap="viridis",
    )


if __name__ == "__main__":
    main()