///////////////////////////////////////////////////////////////////////////////////////////////////////
// Stage 4 of Method 1: adds one new branch, "energy_npe", to a copy of the merged simulation's HitData tree.
// Per hit:
//   npe_per_cm = gRandom->Landau(MPV_npePerCm[plane], Sigma_npePerCm[plane])
//                -- rejected and redrawn unless npe.min_npe_per_cm <= npe_per_cm <= npe.max_npe_per_cm
//   energy_npe = npe_per_cm * pathlength_cm

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TString.h>
#include <TRandom.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>

using namespace std;

// Reads "key = value" lines from config.cfg into a map. "#" starts a comment.
map<string, string> readConfig(const char* path) {
    map<string, string> cfg;
    ifstream f(path);
    string line;
    while (getline(f, line)) {
        line = line.substr(0, line.find('#'));
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t"));
        if (!val.empty()) val.erase(val.find_last_not_of(" \t\r") + 1);
        cfg[key] = val;
    }
    return cfg;
}

void add_energy_npe(const char* configFile = "config.cfg")
{
    map<string, string> cfg = readConfig(configFile);

    string outDirBase = cfg.count("output.dir") ? cfg["output.dir"] : "output";
    string inputFileName = cfg.count("sim.output_name") ? cfg["sim.output_name"] : "merged_musimData_with_uid.root";
    string infile  = outDirBase + "/3_merged_sim/" + inputFileName;
    string cfgfile = cfg["npe.cfg_file"];
    string outputName = cfg.count("npe.output_name") ? cfg["npe.output_name"] : "musimData_with_npe.root";

    const double NPE_PER_CM_MIN = cfg.count("npe.min_npe_per_cm") ? atof(cfg["npe.min_npe_per_cm"].c_str()) : 5.0;
    const double NPE_PER_CM_MAX = cfg.count("npe.max_npe_per_cm") ? atof(cfg["npe.max_npe_per_cm"].c_str()) : 22.5;

    TString outDir = TString::Format("%s/4_npe_energy", outDirBase.c_str());
    gSystem->mkdir(outDir, true);
    TString outfile = outDir + "/" + outputName.c_str();

    const char* pname[8] = {"Y1","Y2","Y3","Y4","Z1","Z2","Z3","Z4"};
    auto st_idx = [](int st) { return st <= 3 ? st : st - 4; };

    double stripEnergy[8][64] = {}; // [plane][bar], bar = module*32 + bar_in_module

    // read MPV_npePerCm and Sigma_npePerCm per plane straight from the data cfg
    double mpv_per_cm[8], sigma_per_cm[8];
    {
        ifstream cfgIn(cfgfile);
        string line;

        getline(cfgIn, line); // header row: "Plane   MPV_npePerCm  ..."
        for (int i = 0; i < 8; i++) {
            getline(cfgIn, line); // read the WHOLE row -- ignores any extra trailing
                                   
            istringstream row(line);
            string plane;
            double mpv, mpv_err, sigma, sigma_err, chi2ndf;
            long long entries;
            row >> plane >> mpv >> mpv_err >> sigma >> sigma_err >> chi2ndf >> entries;
            mpv_per_cm[i]   = mpv;
            sigma_per_cm[i] = sigma;
            cout << plane << "  MPV_npePerCm=" << mpv << "  Sigma_npePerCm=" << sigma << endl;
        }
    }

    auto draw_npe_per_cm = [&](int si) {
        double v;
        do {
            v = gRandom->Landau(mpv_per_cm[si], sigma_per_cm[si]);
        } while (v < NPE_PER_CM_MIN || v > NPE_PER_CM_MAX);
        return v;
    };

    // open input sim file, clone HitData, add the new branch
    TFile *fin = TFile::Open(infile.c_str());
    if (!fin || fin->IsZombie()) { cerr << "Cannot open " << infile << endl; return; }
    TTree *tin = (TTree*)fin->Get("HitData");

    TFile *fout = TFile::Open(outfile, "RECREATE");
    TTree *tout = tin->CloneTree(0); // same branches as input

    double energy_npe;
    tout->Branch("energy_npe", &energy_npe, "energy_npe/D");


    TTree *toutBar = new TTree("StripEnergyData", "Per-bar npe, per event");
    toutBar->SetAutoSave(0);
    Long64_t out_uid;
    toutBar->Branch("UniqueID", &out_uid, "UniqueID/L");
    for (int i = 0; i < 8; i++)
      toutBar->Branch(Form("StripEnergy_%s", pname[i]), stripEnergy[i], Form("StripEnergy_%s[64]/D", pname[i]));



    int    station, module, bar;
    Long64_t uid;
    double pathlength;
    tin->SetBranchAddress("ScintHitStation",    &station);
    tin->SetBranchAddress("ScintHitModule",     &module);
    tin->SetBranchAddress("ScintHitBar",        &bar);
    tin->SetBranchAddress("UniqueID",           &uid);
    tin->SetBranchAddress("ScintHitPathLength", &pathlength);
    tin->CopyAddresses(tout); // sync every other (unmodified) branch to tout
    tout->SetBranchAddress("ScintHitStation",    &station);
    tout->SetBranchAddress("ScintHitPathLength", &pathlength);

    Long64_t nHits = tin->GetEntries();
    Long64_t prev_uid = -1;
    bool first = true;
    cout << "Processing " << nHits << " hits..." << endl;

    for (Long64_t i = 0; i < nHits; i++) {
        tin->GetEntry(i);

        int si = st_idx(station);
        double pathlength_cm = pathlength / 10.0; // ScintHitPathLength is in mm

        double npe_per_cm = draw_npe_per_cm(si);

        energy_npe = npe_per_cm * pathlength_cm;

        if (!first && uid != prev_uid) {
            out_uid = prev_uid;
            toutBar->Fill();
            memset(stripEnergy, 0, sizeof(stripEnergy));
        }
        first = false;
        prev_uid = uid;

        int bin = module * 32 + bar;
        stripEnergy[si][bin] += energy_npe;

        tout->Fill();
    }

    out_uid = prev_uid;
    toutBar->Fill();

    fout->cd();
    tout->Write();
    toutBar->Write();

    //copy PrimaryGenData through unchanged
    TTree *tgenin  = (TTree*)fin->Get("PrimaryGenData");
    TTree *tgenout = tgenin->CloneTree(-1, "fast");
    tgenout->Write();

    fout->Close();
    fin->Close();

    cout << "Done -> " << outfile << endl;
}

add_energy_npe();
