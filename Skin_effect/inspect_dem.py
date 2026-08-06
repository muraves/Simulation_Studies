from dem_tools import read_asc, plot_dem, slope_aspect, crop_roi, roi_mean_dip, export_stl, convert_gsc_pcs

elev, hdr = read_asc("/Users/dorageeraerts/Documents/PhD/simulation/Muraves/Simulation_Studies/PUMAS-and-TURTLE/Mulder/vesuvio_5m_cut.asc")

lat_det = 40.810251
lon_det = 14.411708
det_location = convert_gsc_pcs(lat_det, lon_det)

plot_dem(elev, hdr, out="dem_overview.png", marker_xy=det_location)

slope_deg, aspect_deg = slope_aspect(elev, hdr["cellsize"])
plot_dem(slope_deg, hdr, out="slope_map.png", cmap="inferno",
         label="Slope [deg]", elev_for_hillshade=elev, marker_xy=det_location)  # slope, hillshaded

# after eyeballing the overview to pick coordinates of your flank patch:
roi, roi_hdr = crop_roi(elev, hdr, x_min=450000, x_max=452000, y_min=4.5175e6, y_max=4.5195e6)
plot_dem(roi, roi_hdr, out="roi.png", marker_xy=det_location)

roi_slope_deg, roi_aspect_deg = slope_aspect(roi, roi_hdr["cellsize"])

# Plot ROI slope with ROI elevation hillshade
plot_dem(roi_slope_deg, roi_hdr, out="roi_slope_map.png", cmap="inferno", label="Slope [deg]",
    elev_for_hillshade=roi, marker_xy=det_location)

print(roi_mean_dip(roi, roi_hdr["cellsize"]))   # mean slope + true planar dip/dip-direction

export_stl(roi, roi_hdr, "flank_roi.stl", base_margin=100.0)