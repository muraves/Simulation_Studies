//////////////////////////////////////////////////////////////////////////////////
// Merges every simulated run file found under sim.dir 
// Add UniqueID = ProcessID*1000000 + Event to both PrimaryGenData and HitData
//////////////////////////////////////////////////////////////////////////////////

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

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

void merge_sim_add_uid(const char* configFile = "config.cfg") {

    map<string, string> cfg = readConfig(configFile);

    string simDir      = cfg["sim.dir"];
    string outputName  = cfg.count("sim.output_name") ? cfg["sim.output_name"] : "merged_musimData_with_uid.root";
    string outDirBase  = cfg.count("output.dir") ? cfg["output.dir"] : "output";

    // Every .root file directly under sim.dir is an input to merge.
    string result = gSystem->GetFromPipe(Form("ls %s/*.root 2>/dev/null", simDir.c_str())).Data();
    vector<string> inputs;
    {
        stringstream ss(result);
        string line;
        while (getline(ss, line)) {
            if (!line.empty()) inputs.push_back(line);
        }
    }

    if (inputs.empty()) {
        cout << "No sim files found under " << simDir << " -- check sim.dir in config.cfg!" << endl;
        return;
    }
    cout << "Found " << inputs.size() << " input file(s) under " << simDir << endl;

    string outDir = outDirBase + "/3_merged_sim";
    gSystem->mkdir(outDir.c_str(), true);
    string outPath = outDir + "/" + outputName;

    TFile *fout = TFile::Open(outPath.c_str(), "RECREATE");

    // Copies a tree from all input files into one merged tree, adding UniqueID.
    auto merge_tree = [&](const char* treename) {
        TTree *tout = nullptr;

        Int_t  evtID = 0, procID = 0;
        Long64_t uniqueID = 0;

        for (size_t i = 0; i < inputs.size(); i++) {
            TFile *fin = TFile::Open(inputs[i].c_str());
            TTree *tin = (TTree*)fin->Get(treename);

            if (!tout) {
                fout->cd();
                tout = tin->CloneTree(0);   // same branches, empty
                tout->Branch("UniqueID", &uniqueID, "UniqueID/L");
            }

            tin->SetBranchAddress("Event",     &evtID);
            tin->SetBranchAddress("ProcessID", &procID);

            tin->CopyAddresses(tout);
            tout->SetBranchAddress("Event",     &evtID);
            tout->SetBranchAddress("ProcessID", &procID);

            Long64_t n = tin->GetEntries();
            for (Long64_t j = 0; j < n; j++) {
                tin->GetEntry(j);
                uniqueID = (Long64_t)procID * 1000000LL + evtID;
                tout->Fill();
            }

            fin->Close();
        }

        fout->cd();
        tout->Write("", TObject::kOverwrite);
        cout << treename << ": wrote " << tout->GetEntries() << " entries" << endl;
    };

    merge_tree("PrimaryGenData");
    merge_tree("HitData");

    fout->Close();
    cout << "Done: " << outPath << endl;
}

merge_sim_add_uid();
