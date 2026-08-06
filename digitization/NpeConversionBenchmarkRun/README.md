# MURAVES NPE Conversion Benchmark Run

A 4-stage pipeline: calibrate the real-data energy-per-cluster response
(NPE/cm) with a Landau fit, then use that calibration to draw realistic
per-hit NPE energy into simulated events. All 4 stages read from the same
`config.cfg`. Run them in order:

```
root -l -b -q '1_select_4p_tracks.C'
root -l -b -q '2_landau_fit.C'
root -l -b -q '3_merge_sim_add_uid.C'
root -l -b -q '4_add_energy_npe.C'
```

Each stage writes into its own subfolder of `output/`, and by default reads
the previous stage's output, so running them in order with an unmodified
`config.cfg` works end to end.

## Stages

1. **`1_select_4p_tracks.C`** -- real data. Chains over `data.dir`/`data.run`,
   applies the `chi2cutmax` cut on the best 3p track, finds the matching 4p
   track, and writes per-plane (Y1-4, Z1-4) cluster position/energy/size and
   path-length-normalized energy(/branch name->corrEnergy) to `output/1_4p_selection/goodtracks_*.root`.

2. **`2_landau_fit.C`** -- fits a Landau to the energy/cm and raw
   cluster-energy distributions per plane (fit windows from
   `landau.fit.<plane>.*` in `config.cfg`), and writes a ROOT file and a
   `.cfg` calibration table (MPV/Sigma per plane) to
   `output/2_landau_fit/`. That `.cfg` is the input stage 4 needs.

3. **`3_merge_sim_add_uid.C`** -- simulation. Merges every `.root` file found
   directly under `sim.dir` and adds `UniqueID = ProcessID*1,000,000 + Event`
   to `PrimaryGenData` and `HitData`, writing to `output/3_merged_sim/`.
   `ProcessID` identifies which simulation job an event came from, `Event` is
   that job's own event counter (not unique on its own across jobs) --
   combining them gives one unambiguous key per event after merging.

4. **`4_add_energy_npe.C`** -- reads stage 3's merged+UID'd file and stage
   2's calibration `.cfg` (`npe.cfg_file`), draws a per-hit `npe_per_cm` from
   a Landau centered on that plane's fitted MPV/Sigma (clipped to
   `[npe.min_npe_per_cm, npe.max_npe_per_cm]`), multiplies by path length,
   and writes `energy_npe` per hit plus a per-event `StripEnergyData` tree
   to `output/4_npe_energy/`.

## config.cfg

`data.run` selects which real-data input files to use:

- `"5700-5749"` -- a range: chains every run number in that range
- `"5700"` -- a single run
- anything else, e.g. `"MURAVES_AnalyzedData_5700_5749.root"` -- treated as
  a literal file name under `data.dir` (an already-merged file)

Stage 3 has no equivalent `sim.run` field -- it merges every `.root`
file it finds directly under `sim.dir`.

Everything in `config.cfg` belongs to one of the 4 stages:

- `data.dir`, `data.run`, `chi2cutmax` -- stage 1


- `landau.auto_range`, `landau.clus_left/right`, `landau.npecm_left/right`,
  `landau.fit.<plane>.clus_min/max`, `landau.fit.<plane>.npecm_min/max` --
  stage 2's per-plane Landau fit windows (8 planes: Y1-4, Z1-4) -- see below


- `sim.dir`, `sim.output_name` -- stage 3


- `npe.cfg_file` -- stage 4's input: the calibration `.cfg` table stage 2 wrote
- `npe.output_name` -- stage 4's output `.root` file name.

- `npe.min_npe_per_cm`, `npe.max_npe_per_cm` -- stage 4's clip bounds on the
  randomly drawn Landau npe/cm value (a draw outside this range is rejected
  and redrawn)
  
- `output.dir` -- base output folder (default `output`)


### Landau fit range: manual vs. auto

`landau.auto_range` picks which of two ways stage 2 decides each plane's
Landau fit range:

- **`false` (default)** -- the fit range is whatever you set per plane in
  `landau.fit.<plane>.clus_min/max` and `landau.fit.<plane>.npecm_min/max`
  (falling back to `18.0`-`30.0` and `10.0`-`15.0` if a plane isn't set).
  This is today's behavior, unchanged.
- **`true`** -- the fit range is found automatically for every plane instead:
  `[peak - left, peak + right]`, where `peak` is that plane's own histogram
  peak, and `left`/`right` come from `landau.clus_left`/`landau.clus_right`
  (for the cluster-energy fit) and `landau.npecm_left`/`landau.npecm_right`
  (for the npe/cm fit) -- the same 4 numbers used for every plane, instead of
  hand-tuning 32 `landau.fit.<plane>.*` values across 8 planes each time you
  run this on new data.

