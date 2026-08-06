#!/usr/bin/awk -f
# combine_all_bins.awk
#
# Single-pass replacement for combine_bin.sh's per-bin loop.
# Reads every job file exactly once and buckets rows by (az,el) bin.
#
# Usage:
#   awk -v dphi="$DPHI" -f combine_all_bins.awk expected_bins.txt data_file1 data_file2 ...

function idx_of(az) { return sprintf("%.0f", az / dphi) }
function el_key(el) { return sprintf("%.6f", el + 0) }   # normalize numeric formatting

BEGIN {
    if (dphi == "") { print "ERROR: must pass -v dphi=<DPHI>" > "/dev/stderr"; exit 1 }
}

FNR == NR {
    if (NF < 2) next
    e_az = $1; e_el = $2
    key = idx_of(e_az) SUBSEP el_key(e_el)
    expected[key] = e_az
    expected_el[key] = e_el
    next
}

{
    az = $1; el = $2
    key = idx_of(az) SUBSEP el_key(el)

    n[key]++
    sum_fr[key]    += $3
    sumsq_fr[key]  += $3*$3
    sum_tr[key]    += $6
    sumsq_tr[key]  += $6*$6
    flux_open[key]  = $5
    nev[key]       += $8
    az_val[key]     = az
    el_val[key]     = el
}

END {
    missing = 0
    for (k in expected) {
        if (!(k in n)) {
            split(k, parts, SUBSEP)
            printf "WARNING: bin az=%s el=%s missing or incomplete - skipped\n", \
                expected[k], expected_el[k] > "/dev/stderr"
            missing++
            continue
        }

        ni = n[k]
        mfr = sum_fr[k]/ni
        mtr = sum_tr[k]/ni

        vfr = (ni > 1) ? (sumsq_fr[k] - ni*mfr*mfr)/(ni-1) : 0
        vtr = (ni > 1) ? (sumsq_tr[k] - ni*mtr*mtr)/(ni-1) : 0
        if (vfr < 0) vfr = 0
        if (vtr < 0) vtr = 0

        semfr = (ni > 1) ? sqrt(vfr)/sqrt(ni) : 0
        semtr = (ni > 1) ? sqrt(vtr)/sqrt(ni) : 0

        printf "%s %s %.6e %.6e %.6e %.6e %.6e %d\n", \
            az_val[k], el_val[k], mfr, semfr, flux_open[k], mtr, semtr, nev[k]
    }
    printf "MISSING_COUNT=%d\n", missing > "/dev/stderr"
}