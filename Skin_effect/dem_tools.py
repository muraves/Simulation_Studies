"""
dem_tools.py
------------
Small, dependency-light toolkit (numpy + matplotlib only) for:
  1. Reading an ESRI ASCII grid DEM (.asc)
  2. Plotting elevation + hillshade
  3. Computing slope (and aspect)
  4. Cutting out a region of interest (ROI)
  5. Exporting the ROI as a watertight STL solid (terrain top + flat base + walls)
     ready to import into Geant4 as a G4TessellatedSolid (or via GDML/CAD import).

Usage (edit the __main__ block at the bottom, or import the functions):

    from dem_tools import read_asc, plot_dem, slope_aspect, crop_roi, export_stl

    elev, hdr = read_asc("my_volcano.asc")
    plot_dem(elev, hdr, out="dem_overview.png")

    slope_deg, aspect_deg = slope_aspect(elev, hdr["cellsize"])
    plot_dem(slope_deg, hdr, out="slope_map.png", cmap="inferno",
             label="Slope [deg]", elev_for_hillshade=elev)

    roi_elev, roi_hdr = crop_roi(elev, hdr, x_min=..., x_max=..., y_min=..., y_max=...)
    plot_dem(roi_elev, roi_hdr, out="roi.png")

    export_stl(roi_elev, roi_hdr, "flank_roi.stl", base_elevation=None, base_margin=50.0)
"""

import numpy as np
import matplotlib.pyplot as plt
import struct
from pyproj import Transformer


# ----------------------------------------------------------------------
# 1. Reading the ASC file
# ----------------------------------------------------------------------
def read_asc(path, nodata_to_nan=True):
    """Read an ESRI ASCII grid (.asc) file.

    Returns
    -------
    elev : (nrows, ncols) ndarray, row 0 = NORTH (top) row, as stored in the file
    hdr  : dict with ncols, nrows, xllcorner, yllcorner, cellsize, NODATA_value
           (xllcorner/yllcorner = SW corner of the grid, standard ESRI convention)
    """
    hdr = {}
    with open(path, "r") as f:
        # header: 6 keyword lines (case-insensitive keys)
        for _ in range(6):
            key, val = f.readline().split()
            hdr[key.lower()] = float(val)
        data = np.loadtxt(f)

    hdr["ncols"] = int(hdr["ncols"])
    hdr["nrows"] = int(hdr["nrows"])
    # some tools write xllcenter/yllcenter instead of corner -- normalize
    if "xllcenter" in hdr and "xllcorner" not in hdr:
        hdr["xllcorner"] = hdr["xllcenter"] - hdr["cellsize"] / 2.0
    if "yllcenter" in hdr and "yllcorner" not in hdr:
        hdr["yllcorner"] = hdr["yllcenter"] - hdr["cellsize"] / 2.0

    data = data.reshape(hdr["nrows"], hdr["ncols"])
    if nodata_to_nan and "nodata_value" in hdr:
        data = np.where(data == hdr["nodata_value"], np.nan, data)

    return data, hdr


def _xy_extent(hdr):
    """Return (xmin, xmax, ymin, ymax) of the grid in map units, for imshow(extent=...)."""
    xmin = hdr["xllcorner"]
    xmax = hdr["xllcorner"] + hdr["ncols"] * hdr["cellsize"]
    ymin = hdr["yllcorner"]
    ymax = hdr["yllcorner"] + hdr["nrows"] * hdr["cellsize"]
    return xmin, xmax, ymin, ymax


# ----------------------------------------------------------------------
# 2. Plotting (elevation map, with optional hillshade)
# ----------------------------------------------------------------------
def hillshade(elev, cellsize, azimuth=315.0, altitude=45.0):
    """Simple analytical hillshade (same algorithm as ESRI/GDAL)."""
    az = np.radians(360.0 - azimuth + 90.0)
    alt = np.radians(altitude)

    dzdy, dzdx = np.gradient(elev, cellsize)
    slope = np.pi / 2.0 - np.arctan(np.hypot(dzdx, dzdy))
    aspect = np.arctan2(-dzdx, dzdy)

    shaded = (np.sin(alt) * np.sin(slope) +
              np.cos(alt) * np.cos(slope) * np.cos(az - aspect))
    return np.clip(shaded, 0, 1)


def plot_dem(grid, hdr, out=None, cmap="terrain", label="Elevation [m]",
             elev_for_hillshade=None, contour=True, marker_xy=None):
    """Plot a 2D grid (elevation or slope or anything on the same footprint)
    georeferenced with hdr, optionally overlaid with a hillshade computed
    from `elev_for_hillshade` (pass the elevation array even when `grid`
    is e.g. the slope map, to get a hillshaded slope map)."""
    extent = _xy_extent(hdr)
    fig, ax = plt.subplots(figsize=(8, 7))

    if elev_for_hillshade is not None:
        hs = hillshade(elev_for_hillshade, hdr["cellsize"])
        ax.imshow(hs, cmap="gray", extent=extent, origin="upper", alpha=1.0)
        im = ax.imshow(grid, cmap=cmap, extent=extent, origin="upper", alpha=0.55)
    else:
        im = ax.imshow(grid, cmap=cmap, extent=extent, origin="upper")

    if contour:
        cs = ax.contour(grid, levels=12, colors="k", linewidths=0.4,
                         extent=extent, origin="upper")
        ax.clabel(cs, inline=True, fontsize=6, fmt="%.0f")

    # Optional location marker
    if marker_xy is not None:
        x, y = marker_xy
        ax.plot(x, y, marker="x", markersize=12,
                markeredgewidth=2, color="red")
        ax.text(x, y, "detector location", color="red",
                fontsize=10, va="center")
        
    cbar = fig.colorbar(im, ax=ax, shrink=0.8)
    cbar.set_label(label)
    ax.set_xlabel("Easting [m]")
    ax.set_ylabel("Northing [m]")
    ax.set_aspect("equal")
    fig.tight_layout()

    if out:
        fig.savefig(out, dpi=150)
        print(f"saved {out}")
    return fig, ax


# ----------------------------------------------------------------------
# 3. Slope / aspect
# ----------------------------------------------------------------------
def slope_aspect(elev, cellsize):
    """Slope and aspect in degrees. Slope = angle from horizontal (0 = flat, 90 = vertical cliff).
    Aspect = downslope direction, degrees clockwise from north (0=N, 90=E, 180=S, 270=W)."""
    dzdy, dzdx = np.gradient(elev, cellsize)  # note: row axis (y) is north-to-south as stored
    slope_rad = np.arctan(np.hypot(dzdx, dzdy))
    aspect_rad = np.arctan2(dzdx, dzdy)  # downslope-facing direction
    slope_deg = np.degrees(slope_rad)
    aspect_deg = (np.degrees(aspect_rad) + 180.0) % 360.0
    return slope_deg, aspect_deg


def roi_mean_dip(elev, cellsize):
    """Convenience: mean slope (deg) and dominant aspect (deg) of a cropped ROI,
    plus a best-fit planar dip (useful to set the tilt angle of the local slab
    model discussed for the flank-scattering study)."""
    slope_deg, aspect_deg = slope_aspect(elev, cellsize)
    mean_slope = np.nanmean(slope_deg)
    # circular mean for aspect
    a = np.radians(aspect_deg[~np.isnan(aspect_deg)])
    mean_aspect = np.degrees(np.arctan2(np.nanmean(np.sin(a)), np.nanmean(np.cos(a)))) % 360.0

    # best-fit plane z = a*x + b*y + c  (least squares) -> true planar dip/dip-direction
    ny, nx = elev.shape
    xs = (np.arange(nx) * cellsize)
    ys = (np.arange(ny) * cellsize)[::-1]  # row 0 = north = largest y
    X, Y = np.meshgrid(xs, ys)
    mask = ~np.isnan(elev)
    A = np.column_stack([X[mask], Y[mask], np.ones(mask.sum())])
    coef, *_ = np.linalg.lstsq(A, elev[mask], rcond=None)
    a_, b_, c_ = coef
    plane_dip = np.degrees(np.arctan(np.hypot(a_, b_)))
    plane_dipdir = (np.degrees(np.arctan2(-a_, -b_)) + 180.0) % 360.0

    return {
        "mean_slope_deg": mean_slope,
        "mean_aspect_deg": mean_aspect,
        "planar_dip_deg": plane_dip,
        "planar_dip_direction_deg": plane_dipdir,
    }


# ----------------------------------------------------------------------
# 4. Cropping a region of interest
# ----------------------------------------------------------------------
def crop_roi(elev, hdr, x_min, x_max, y_min, y_max):
    """Crop by map coordinates (same units as xllcorner/yllcorner, e.g. UTM meters)."""
    xmin_g, xmax_g, ymin_g, ymax_g = _xy_extent(hdr)
    x_min = max(x_min, xmin_g)
    x_max = min(x_max, xmax_g)
    y_min = max(y_min, ymin_g)
    y_max = min(y_max, ymax_g)

    cs = hdr["cellsize"]
    col0 = int((x_min - hdr["xllcorner"]) / cs)
    col1 = int(np.ceil((x_max - hdr["xllcorner"]) / cs))
    # row 0 of the array is the NORTH edge (ymax_g); rows increase southward
    row0 = int((ymax_g - y_max) / cs)
    row1 = int(np.ceil((ymax_g - y_min) / cs))

    roi = elev[row0:row1, col0:col1]
    roi_hdr = dict(hdr)
    roi_hdr["nrows"], roi_hdr["ncols"] = roi.shape
    roi_hdr["xllcorner"] = hdr["xllcorner"] + col0 * cs
    roi_hdr["yllcorner"] = ymax_g - row1 * cs
    return roi, roi_hdr


def crop_roi_pixels(elev, hdr, row0, row1, col0, col1):
    """Crop by array indices instead of map coordinates -- handy once you've
    picked pixels off the plot_dem() figure."""
    roi = elev[row0:row1, col0:col1]
    cs = hdr["cellsize"]
    xmin_g, xmax_g, ymin_g, ymax_g = _xy_extent(hdr)
    roi_hdr = dict(hdr)
    roi_hdr["nrows"], roi_hdr["ncols"] = roi.shape
    roi_hdr["xllcorner"] = hdr["xllcorner"] + col0 * cs
    roi_hdr["yllcorner"] = ymax_g - row1 * cs
    return roi, roi_hdr


# ----------------------------------------------------------------------
# 5. STL export -> watertight solid for Geant4 (G4TessellatedSolid / GDML import)
# ----------------------------------------------------------------------
def _write_stl_binary(path, triangles):
    """triangles: (N,3,3) array of vertex coordinates."""
    n = triangles.shape[0]
    with open(path, "wb") as f:
        f.write(b"\0" * 80)
        f.write(struct.pack("<I", n))
        for tri in triangles:
            v0, v1, v2 = tri
            normal = np.cross(v1 - v0, v2 - v0)
            norm = np.linalg.norm(normal)
            normal = normal / norm if norm > 0 else np.zeros(3)
            f.write(struct.pack("<3f", *normal))
            for v in tri:
                f.write(struct.pack("<3f", *v))
            f.write(struct.pack("<H", 0))


def export_stl(elev, hdr, out_path, base_elevation=None, base_margin=50.0,
               local_origin=True, z_units_match_xy=True):
    """
    Build a watertight solid from the ROI: terrain surface on top, flat
    base at `base_elevation` (default: min(elev) - base_margin), vertical
    side walls connecting them. Units follow the DEM (usually meters for
    both xy and z, which Geant4 wants consistently, e.g. all in mm or all
    in m -- just remember to set the same unit when importing).

    local_origin=True shifts (x,y) so the ROI is centered near the origin
    (recommended -- Geant4 doesn't like solids sitting at UTM-scale
    coordinates 1e5-1e6 due to floating point precision in tracking).
    """
    ny, nx = elev.shape
    cs = hdr["cellsize"]
    xs = hdr["xllcorner"] + (np.arange(nx) + 0.5) * cs
    ys = hdr["yllcorner"] + (ny - 0.5 - np.arange(ny)) * cs  # row0 = north = max y

    if np.isnan(elev).any():
        # fill NODATA gaps so the mesh stays watertight (simple constant fill;
        # swap for a proper inpainting/nearest-neighbour fill if gaps are large)
        elev = elev.copy()
        elev[np.isnan(elev)] = np.nanmean(elev)

    if local_origin:
        x0, y0 = xs.mean(), ys.mean()
        xs = xs - x0
        ys = ys - y0

    if base_elevation is None:
        base_elevation = float(np.nanmin(elev) - base_margin)

    X, Y = np.meshgrid(xs, ys)
    top = np.stack([X, Y, elev], axis=-1)          # (ny, nx, 3)
    bot = np.stack([X, Y, np.full_like(elev, base_elevation)], axis=-1)

    tris = []

    def add_quad(p0, p1, p2, p3):
        # split quad p0-p1-p2-p3 (CCW as seen from outside) into 2 triangles
        tris.append(np.array([p0, p1, p2]))
        tris.append(np.array([p0, p2, p3]))

    # top surface (normals up)
    for j in range(ny - 1):
        for i in range(nx - 1):
            add_quad(top[j, i], top[j, i + 1], top[j + 1, i + 1], top[j + 1, i])

    # bottom surface (normals down -> reverse winding)
    for j in range(ny - 1):
        for i in range(nx - 1):
            add_quad(bot[j, i], bot[j + 1, i], bot[j + 1, i + 1], bot[j, i + 1])

    # 4 side walls
    for i in range(nx - 1):  # north edge, j=0
        add_quad(bot[0, i], bot[0, i + 1], top[0, i + 1], top[0, i])
    for i in range(nx - 1):  # south edge, j=ny-1
        add_quad(bot[ny - 1, i + 1], bot[ny - 1, i], top[ny - 1, i], top[ny - 1, i + 1])
    for j in range(ny - 1):  # west edge, i=0
        add_quad(bot[j + 1, 0], bot[j, 0], top[j, 0], top[j + 1, 0])
    for j in range(ny - 1):  # east edge, i=nx-1
        add_quad(bot[j, nx - 1], bot[j + 1, nx - 1], top[j + 1, nx - 1], top[j, nx - 1])

    triangles = np.stack(tris, axis=0)
    _write_stl_binary(out_path, triangles)
    print(f"saved {out_path}  ({triangles.shape[0]} triangles, "
          f"{nx}x{ny} grid, base z={base_elevation:.1f})")
    return triangles

# convert latitude-longitude (geographic coordinate system) to easting-northing (projected coordinate system)
def convert_gsc_pcs(lat,lon, input_crs="EPSG:4326"):
    """
    input_csr: input coordinate system of reference; default is WGS (World Geodetic System) 84, 
    corresponding to 
    """

    zone = int((lon + 180) // 6 + 1)

    if lat >= 0:
        epsg = 32600 + zone   # northern hemisphere
    else:
        epsg = 32700 + zone   # southern hemisphere

    transformer = Transformer.from_crs(
    input_crs,   # WGS84 geographic
    f"EPSG:{epsg}", 
    always_xy=True
)
    
    easting, northing = transformer.transform(lon, lat)
    return (easting, northing)



# ----------------------------------------------------------------------
if __name__ == "__main__":
    import sys
    path = sys.argv[1] if len(sys.argv) > 1 else "volcano.asc"

    elev, hdr = read_asc(path)
    print("grid:", hdr["nrows"], "x", hdr["ncols"], " cellsize:", hdr["cellsize"])
    plot_dem(elev, hdr, out="dem_overview.png")

    slope_deg, aspect_deg = slope_aspect(elev, hdr["cellsize"])
    plot_dem(slope_deg, hdr, out="slope_map.png", cmap="inferno",
             label="Slope [deg]", elev_for_hillshade=elev)

    print("Pick your ROI by inspecting dem_overview.png / slope_map.png,")
    print("then call crop_roi(elev, hdr, x_min, x_max, y_min, y_max) or")
    print("crop_roi_pixels(elev, hdr, row0, row1, col0, col1).")