////////////////////////////////////////////////////////////////////////////
// MURAVES Energy Analysis: Landau fits:  raw cluster, energy per cm (npe/cm)
// Reads the output of 1_select_4p_tracks.C
// Per-plane Landau fit windows come from config.cfg (landau.fit.<plane>.*);
///////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TSystem.h>

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

void landau_fit(const char* configFile = "config.cfg") {

    ///////////////////config///////////////////////////////

    map<string, string> cfg = readConfig(configFile);

    string dataDir    = cfg["data.dir"];
    string dataRun     = cfg["data.run"];
    string outDirBase  = cfg.count("output.dir") ? cfg["output.dir"] : "output";

    // Reconstruct the same input file name that 1_select_4p_tracks.C wrote,
    // from the same data.run field (range / single run / file name).
    string outTag;
    size_t dash = dataRun.find('-');
    if (dash != string::npos && dash > 0 &&
        dataRun.substr(0, dash).find_first_not_of("0123456789") == string::npos &&
        dataRun.substr(dash + 1).find_first_not_of("0123456789") == string::npos) {
        int runFirst = atoi(dataRun.substr(0, dash).c_str());
        int runLast  = atoi(dataRun.substr(dash + 1).c_str());
        outTag = Form("%d_%d.root", runFirst, runLast);
    } else if (dataRun.find_first_not_of("0123456789") == string::npos) {
        int run = atoi(dataRun.c_str());
        outTag = Form("run%d.root", run);
    } else {
        outTag = gSystem->BaseName(dataRun.c_str());
    }

    string mergedFile = outDirBase + "/1_4p_selection/goodtracks_" + outTag;

    string outputDir = outDirBase + "/2_landau_fit";

    vector<string> planes = {"Y1","Y2","Y3","Y4","Z1","Z2","Z3","Z4"};

    //Clsuter

    string clus_clsCut = "all"; // can put cuts on cluster size ("==1", "==2", >=2, <=2, ... ect)

    // Cluster (raw)

    double clus_eMin   = 0.0;
    int    clus_bins   = 202;
    double clus_hMin   = -0.1,  clus_hMax = 100.9;

    // Energy per cm (npe/cm)
    double npecm_eMin  = 0.0;
    double thickness  = 1.7;
    int    npecm_bins  = 122;
    double npecm_hMin  = -0.1,  npecm_hMax = 60.9;

    // Per-plane Landau fit windows, from config.cfg

    bool   autoRange      = cfg.count("landau.auto_range") && cfg["landau.auto_range"] == "true";
    double autoClusLeft   = cfg.count("landau.clus_left")   ? atof(cfg["landau.clus_left"].c_str())   : 5.0;
    double autoClusRight  = cfg.count("landau.clus_right")  ? atof(cfg["landau.clus_right"].c_str())  : 7.0;
    double autoNpecmLeft  = cfg.count("landau.npecm_left")  ? atof(cfg["landau.npecm_left"].c_str())  : 2.5;
    double autoNpecmRight = cfg.count("landau.npecm_right") ? atof(cfg["landau.npecm_right"].c_str()) : 3.0;

    map<string, pair<double,double>> clus_fit;
    map<string, pair<double,double>> npecm_fit;
    for (auto& p : planes) {
        double cmin = cfg.count("landau.fit." + p + ".clus_min") ? atof(cfg["landau.fit." + p + ".clus_min"].c_str()) : 18.0;
        double cmax = cfg.count("landau.fit." + p + ".clus_max") ? atof(cfg["landau.fit." + p + ".clus_max"].c_str()) : 30.0;
        double rmin = cfg.count("landau.fit." + p + ".npecm_min") ? atof(cfg["landau.fit." + p + ".npecm_min"].c_str()) : 10.0;
        double rmax = cfg.count("landau.fit." + p + ".npecm_max") ? atof(cfg["landau.fit." + p + ".npecm_max"].c_str()) : 15.0;
        clus_fit[p] = {cmin, cmax};
        npecm_fit[p] = {rmin, rmax};
    }

    //////////////////////////////////////////////////

    TChain *chain = new TChain("AnalyzedData");
    chain->Add(mergedFile.c_str());
    Long64_t nentries = chain->GetEntries();
    if (nentries == 0) { cerr << "[ERROR] No entries found. Check data.dir/data.run in config.cfg.\n"; return; }
    cout << "[INFO] " << nentries << " entries from " << mergedFile << "\n";

    int nP = (int)planes.size();

    vector<int>                     Nclusters(nP, 0);
    vector<vector<int>*>            ClusterSize(nP, nullptr);
    vector<vector<double>*>         ClusterEnergy(nP, nullptr);

    vector<double>                  CorrEnergy(nP, 0.0);

    for (int i = 0; i < nP; i++) {
        const string& p = planes[i];
        chain->SetBranchAddress(("Nclusters_"    + p).c_str(), &Nclusters[i]);
        chain->SetBranchAddress(("ClusterSize_"  + p).c_str(), &ClusterSize[i]);
        chain->SetBranchAddress(("ClusterEnergy_"+ p).c_str(), &ClusterEnergy[i]);

        chain->SetBranchAddress(("CorrEnergy_"   + p).c_str(), &CorrEnergy[i]);
    }

    vector<TH1F*> hClus(nP), hNpecm(nP);
    for (int i = 0; i < nP; i++) {
      const string& p = planes[i];
      hClus[i] = new TH1F(("hClus_"+p).c_str(), ("Cluster Energy "+p+";E [a.u.];Entries").c_str(), clus_bins, clus_hMin, clus_hMax);
      hNpecm[i] = new TH1F(("hNpecm_"+p).c_str(), ("MPV (npe per cm) " +p+";npe/cm;Entries").c_str(), npecm_bins, npecm_hMin, npecm_hMax);
      hClus[i]->SetDirectory(0);
      hNpecm[i]->SetDirectory(0);
    }

    auto passCut = [](const string& cut, int val) -> bool {
        if (cut == "all") return true;
        if (cut.size() >= 2 && cut.substr(0,2) == "==") return val == stoi(cut.substr(2));
        if (cut.size() >= 2 && cut.substr(0,2) == "<=") return val <= stoi(cut.substr(2));
        if (cut.size() >= 2 && cut.substr(0,2) == ">=") return val >= stoi(cut.substr(2));
        if (cut[0] == '<') return val <  stoi(cut.substr(1));
        if (cut[0] == '>') return val >  stoi(cut.substr(1));
        size_t d = cut.find('-');
        if (d != string::npos && d > 0)
            return val >= stoi(cut.substr(0,d)) && val <= stoi(cut.substr(d+1));
        return true;
    };


    cout << "[INFO] Starting event loop...\n";
    for (Long64_t ev = 0; ev < nentries; ev++) {
        if (ev % 100000 == 0) cout << "  Event " << ev << "/" << nentries << "\r" << flush;
        chain->GetEntry(ev);

        for (int i = 0; i < nP; i++) {


            // Cluster (raw)

	  int cs = (ClusterSize[i] && !ClusterSize[i]->empty()) ? (*ClusterSize[i])[0] : 0;

	  if (ClusterEnergy[i] && !ClusterEnergy[i]->empty() &&
	      passCut(clus_clsCut, cs)) {
	    double e = (*ClusterEnergy[i])[0];
	    if (e >= clus_eMin) hClus[i]->Fill(e);
	  }

            // npe/cm


	      if (passCut(clus_clsCut, cs) &&
	       	CorrEnergy[i]/thickness >= npecm_eMin) {
	      hNpecm[i]->Fill(CorrEnergy[i]/thickness);
	    }


        }
    }
    cout << "\n[INFO] Event loop done.\n";


    //output file
    gSystem->mkdir(outputDir.c_str(), kTRUE);
    gStyle->SetOptStat(1110);
    gStyle->SetOptFit(1111);


    string outRoot = outputDir + "/fit_" + gSystem->BaseName(mergedFile.c_str());

    string outCfg  = outputDir + "/fit_" + gSystem->BaseName(mergedFile.c_str());
    outCfg = outCfg.substr(0, outCfg.rfind(".root")) + ".cfg";


    //////////////////////////////

    TFile *outFile = new TFile(outRoot.c_str(), "RECREATE");

    struct FitResult { double mpv, mpv_err, sigma, sigma_err, chi2ndf; long long entries; bool ok; };
    struct PlaneResult { string plane; FitResult clus; FitResult npecm; };
    vector<PlaneResult> results(nP);

    // Fit histogram, save canvas+histo to ROOT file, return fit result
    auto fitAndSave = [&](TH1F* h, const string& tag, const string& pl,
                          double fmin, double fmax) -> FitResult {
        TCanvas *c = new TCanvas(("c_"+tag+"_"+pl).c_str(), (tag+" "+pl).c_str(), 800, 600);
        h->Draw();
        FitResult r = {0, 0, 0, 0, 0, 0, false};
        if (h->GetEntries() > 10) {
            TF1 *f = new TF1(("f_"+tag+"_"+pl).c_str(), "landau", fmin, fmax);
            f->SetParameters(h->GetMaximum(), (fmin + fmax) / 2.0, 1.0);
            f->SetLineColor(kRed+1);
            h->Fit(f, "R");
            gPad->Update();
            int ndf    = f->GetNDF();
            r.mpv      = f->GetParameter(1);
            r.mpv_err  = f->GetParError(1);
            r.sigma     = f->GetParameter(2);
            r.sigma_err = f->GetParError(2);
            r.chi2ndf  = (ndf > 0) ? f->GetChisquare() / ndf : 0.0;
            r.entries  = (long long)h->GetEntries();
            r.ok       = true;
            cout << "[" << tag << " " << pl << "] MPV = " << r.mpv << " ± " << r.mpv_err
                 << "   Sigma = " << r.sigma << " ± " << r.sigma_err << "\n";
        } else {
            cout << "[WARN] " << tag << " " << pl << ": too few entries to fit.\n";
        }
        c->SaveAs((outputDir + "/" + tag + "_" + pl + ".png").c_str());
        outFile->cd();
        c->Write();
        h->Write();
        delete c;
        return r;
    };

    for (int i = 0; i < nP; i++) {
        const string& pl = planes[i];
        results[i].plane = pl;

        double cmin, cmax, rmin, rmax;
        if (autoRange) {
            double peakClus  = hClus[i]->GetBinCenter(hClus[i]->GetMaximumBin());
            double peakNpecm = hNpecm[i]->GetBinCenter(hNpecm[i]->GetMaximumBin());
            cmin = peakClus - autoClusLeft;    cmax = peakClus + autoClusRight;
            rmin = peakNpecm - autoNpecmLeft;  rmax = peakNpecm + autoNpecmRight;
        } else {
            cmin = clus_fit[pl].first;   cmax = clus_fit[pl].second;
            rmin = npecm_fit[pl].first;  rmax = npecm_fit[pl].second;
        }

        results[i].clus  = fitAndSave(hClus[i],  "cluster", pl, cmin, cmax);
        results[i].npecm = fitAndSave(hNpecm[i], "npecm",   pl, rmin, rmax);
    }

    // Write table-format cfg (strip results excluded)
    ofstream cfgOut(outCfg);

    auto cell = [](double v) -> string {
        ostringstream ss; ss << fixed << setprecision(4) << v; return ss.str();
    };

    const int W0=8, W1=14, W2=16, W3=16, W4=16, W5=16, W6=16;
    const int W7=14, W8=16, W9=16, W10=16;
    cfgOut << left
           << setw(W0) << "Plane"
           << setw(W1) << "MPV_npePerCm"
           << setw(W2) << "MPV_err"
           << setw(W5) << "Sigma_npePerCm"
           << setw(W6) << "Sigma_err"
           << setw(W3) << "chi2ndf"
           << setw(W4) << "entries"
           << setw(W7) << "Clus_MPV"
           << setw(W8) << "Clus_MPV_err"
           << setw(W9) << "Clus_Sigma"
           << setw(W10) << "Clus_chi2ndf"
           << "\n";

    for (auto& r : results) {
        cfgOut << left << setw(W0) << r.plane;
        if (r.npecm.ok) {
            cfgOut << setw(W1) << cell(r.npecm.mpv)
                   << setw(W2) << cell(r.npecm.mpv_err)
                   << setw(W5) << cell(r.npecm.sigma)
                   << setw(W6) << cell(r.npecm.sigma_err)
                   << setw(W3) << cell(r.npecm.chi2ndf)
                   << setw(W4) << r.npecm.entries;
        } else {
            cfgOut << setw(W1)<<"N/A" << setw(W2)<<"N/A" << setw(W5)<<"N/A" << setw(W6)<<"N/A" << setw(W3)<<"N/A" << setw(W4)<<"N/A";
        }
        // raw cluster energy (NPE, uncorrected) -- Method 2 input, added on request
        if (r.clus.ok) {
            cfgOut << setw(W7) << cell(r.clus.mpv)
                   << setw(W8) << cell(r.clus.mpv_err)
                   << setw(W9) << cell(r.clus.sigma)
                   << setw(W10) << cell(r.clus.chi2ndf);
        } else {
            cfgOut << setw(W7)<<"N/A" << setw(W8)<<"N/A" << setw(W9)<<"N/A" << setw(W10)<<"N/A";
        }
        cfgOut << "\n";
    }

    outFile->Close();
    cfgOut.close();
    cout << "[INFO] ROOT: " << outRoot << "\n";
    cout << "[INFO] CFG:  " << outCfg  << "\n";

    delete chain;
}

landau_fit();
