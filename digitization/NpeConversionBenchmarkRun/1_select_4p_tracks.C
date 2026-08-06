//////////////////////////////////////////////////////////////////////////////////
//  1_select_4p_tracks.C
//  Selects events with a good 4p track (chi-square cut) and extracts cluster
//  properties at all 4 stations for those events.
//  Output: a ROOT file with similar tree name as the reconstructed data.
//        : also has the corrEnergy branch, which is the pathlength normalised cluster energy
/////////////////////////////////////////////////////////////////////////////////

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TSystem.h>
#include <TMath.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>

using namespace std;

const int MAXCAND   = 512;   // 3p/4p track candidate arrays
const int MAXCLUST  = 128;   // per-plane cluster arrays
const int MAXSTRIPS = 256;   // per-cluster strip arrays

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
        // trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t"));
        if (!val.empty()) val.erase(val.find_last_not_of(" \t\r") + 1);
        cfg[key] = val;
    }
    return cfg;
}

void select_4p_tracks(const char* configFile = "config.cfg")
{
    // CONFIGURATION

    map<string, string> cfg = readConfig(configFile);

    string dataDir    = cfg["data.dir"];
    string dataRun    = cfg["data.run"];
    double      chi2cutmax = cfg.count("chi2cutmax") ? atof(cfg["chi2cutmax"].c_str()) : 2.0;
    string outDirBase = cfg.count("output.dir") ? cfg["output.dir"] : "output";

    // Resolve data.run into the list of input files to chain over: a range
    // "A-B", a single run number, or (if it's not numeric) a file name.
    vector<string> inputFiles;
    string outTag;

    size_t dash = dataRun.find('-');
    if (dash != string::npos && dash > 0 &&
        dataRun.substr(0, dash).find_first_not_of("0123456789") == string::npos &&
        dataRun.substr(dash + 1).find_first_not_of("0123456789") == string::npos) {
        int runFirst = atoi(dataRun.substr(0, dash).c_str());
        int runLast  = atoi(dataRun.substr(dash + 1).c_str());
        for (int r = runFirst; r <= runLast; r++) {
            inputFiles.push_back(Form("%s/MURAVES_AnalyzedData_run%d.root", dataDir.c_str(), r));
        }
        outTag = Form("%d_%d.root", runFirst, runLast);
    } else if (dataRun.find_first_not_of("0123456789") == string::npos) {
        int run = atoi(dataRun.c_str());
        inputFiles.push_back(Form("%s/MURAVES_AnalyzedData_run%d.root", dataDir.c_str(), run));
        outTag = Form("run%d.root", run);
    } else {
        string path = (dataRun[0] == '/') ? dataRun : dataDir + "/" + dataRun;
        inputFiles.push_back(path);
        outTag = gSystem->BaseName(dataRun.c_str());
    }

    string outDir = outDirBase + "/1_4p_selection";
    gSystem->mkdir(outDir.c_str(), true);
    string outName = outDir + "/goodtracks_" + outTag;
    cout << "Output file: " << outName << endl;

    TChain* chain = new TChain("AnalyzedData");
    for (size_t i = 0; i < inputFiles.size(); i++) {
        cout << "Adding input file: " << inputFiles[i] << endl;
        chain->Add(inputFiles[i].c_str());
    }


    ////////////////////////////////////////
    
    auto stripSlice = [](const Double_t* stripsArr, const Long64_t* sizeArr, int clIdx) -> vector<double> {
        int offset = 0;
        for (int i = 0; i < clIdx; i++) offset += (int)sizeArr[i];
        int len = (int)sizeArr[clIdx];
        vector<double> out;
        out.reserve(len);
        for (int i = 0; i < len; i++) out.push_back(stripsArr[offset + i]);
        return out;
    };

    
    /////////////////////////////////////////////////

    
    Long64_t nEntries = chain->GetEntries();
    cout << "Total events loaded: " << nEntries << endl;
    if (nEntries == 0) {
        cout << "No events loaded — check data.dir/data.run in config.cfg!" << endl;
        return;
    }

    ////////////////////////////////////////



    //  DECLARE INPUT BRANCH VARIABLES


    // Run info 
    Long64_t Run;

    //Event-level scalars 
    Long64_t BestTrack_3p_xz_index, BestTrack_3p_xy_index;
    double BestTrack_3p_ChiSquare_xz, BestTrack_3p_ChiSquare_xy;
    double BestTrack_4p_ChiSquare_xz, BestTrack_4p_ChiSquare_xy;
    Long64_t Track_3p_of_4p_index_xz,   Track_3p_of_4p_index_xy;
    Long64_t Track_4p_index_xz,         Track_4p_index_xy;
    double Theta_3p, Phi_3p;
    double Theta_4p, Phi_4p;

    //Nclusters per plane
    Long64_t Nclusters_Z1, Nclusters_Z2, Nclusters_Z3, Nclusters_Z4;
    Long64_t Nclusters_Y1, Nclusters_Y2, Nclusters_Y3, Nclusters_Y4;

    // Cluster positions/energies/sizes 
    Double_t ClusterPosition_Z1[MAXCLUST], ClusterPosition_Z2[MAXCLUST], ClusterPosition_Z3[MAXCLUST], ClusterPosition_Z4[MAXCLUST];
    Double_t ClusterPosition_Y1[MAXCLUST], ClusterPosition_Y2[MAXCLUST], ClusterPosition_Y3[MAXCLUST], ClusterPosition_Y4[MAXCLUST];
    Double_t ClusterEnergy_Z1[MAXCLUST], ClusterEnergy_Z2[MAXCLUST], ClusterEnergy_Z3[MAXCLUST], ClusterEnergy_Z4[MAXCLUST];
    Double_t ClusterEnergy_Y1[MAXCLUST], ClusterEnergy_Y2[MAXCLUST], ClusterEnergy_Y3[MAXCLUST], ClusterEnergy_Y4[MAXCLUST];
    Long64_t ClusterSize_Z1[MAXCLUST], ClusterSize_Z2[MAXCLUST], ClusterSize_Z3[MAXCLUST], ClusterSize_Z4[MAXCLUST];
    Long64_t ClusterSize_Y1[MAXCLUST], ClusterSize_Y2[MAXCLUST], ClusterSize_Y3[MAXCLUST], ClusterSize_Y4[MAXCLUST];

   
    Int_t nStripsEnergy_Z1, nStripsEnergy_Z2, nStripsEnergy_Z3, nStripsEnergy_Z4;
    Int_t nStripsEnergy_Y1, nStripsEnergy_Y2, nStripsEnergy_Y3, nStripsEnergy_Y4;
    Double_t StripsEnergy_Z1[MAXSTRIPS], StripsEnergy_Z2[MAXSTRIPS], StripsEnergy_Z3[MAXSTRIPS], StripsEnergy_Z4[MAXSTRIPS];
    Double_t StripsEnergy_Y1[MAXSTRIPS], StripsEnergy_Y2[MAXSTRIPS], StripsEnergy_Y3[MAXSTRIPS], StripsEnergy_Y4[MAXSTRIPS];

    // --- 3p track slopes 
    Int_t nSlope_3p_xz, nSlope_3p_xy;
    Double_t Slope_3p_xz[MAXCAND], Slope_3p_xy[MAXCAND];

    // --- 4p track slopes
    Int_t nSlope_4p_xz, nSlope_4p_xy;
    Double_t Slope_4p_xz[MAXCAND], Slope_4p_xy[MAXCAND];
    Int_t nchiSquare_4p_xz, nchiSquare_4p_xy;
    Double_t chiSquare_4p_xz[MAXCAND], chiSquare_4p_xy[MAXCAND];

    //Track cluster indices
    Int_t nTrackCluster_z1_index, nTrackCluster_z2_index, nTrackCluster_z3_index;
    Int_t nTrackCluster_y1_index, nTrackCluster_y2_index, nTrackCluster_y3_index;
    Long64_t TrackCluster_z1_index[MAXCAND], TrackCluster_z2_index[MAXCAND], TrackCluster_z3_index[MAXCAND];
    Long64_t TrackCluster_y1_index[MAXCAND], TrackCluster_y2_index[MAXCAND], TrackCluster_y3_index[MAXCAND];

    //Station 4 cluster index
    Int_t ncluster_c4_index_xz, ncluster_c4_index_xy;
    Double_t cluster_c4_index_xz[MAXCAND], cluster_c4_index_xy[MAXCAND];


    //  SET BRANCH ADDRESSES

    chain->SetBranchAddress("Run",                        &Run);
    chain->SetBranchAddress("BestTrack_3p_xz_index",      &BestTrack_3p_xz_index);
    chain->SetBranchAddress("BestTrack_3p_xy_index",      &BestTrack_3p_xy_index);
    chain->SetBranchAddress("BestTrack_3p_ChiSquare_xz",  &BestTrack_3p_ChiSquare_xz);
    chain->SetBranchAddress("BestTrack_3p_ChiSquare_xy",  &BestTrack_3p_ChiSquare_xy);
    chain->SetBranchAddress("BestTrack_4p_ChiSquare_xz",  &BestTrack_4p_ChiSquare_xz);
    chain->SetBranchAddress("BestTrack_4p_ChiSquare_xy",  &BestTrack_4p_ChiSquare_xy);
    chain->SetBranchAddress("Track_3p_of_4p_index_xz",    &Track_3p_of_4p_index_xz);
    chain->SetBranchAddress("Track_3p_of_4p_index_xy",    &Track_3p_of_4p_index_xy);
    chain->SetBranchAddress("Track_4p_index_xz",           &Track_4p_index_xz);
    chain->SetBranchAddress("Track_4p_index_xy",           &Track_4p_index_xy);
    chain->SetBranchAddress("Theta_3p",                    &Theta_3p);
    chain->SetBranchAddress("Phi_3p",                      &Phi_3p);
    chain->SetBranchAddress("Theta_4p",                    &Theta_4p);
    chain->SetBranchAddress("Phi_4p",                      &Phi_4p);

    chain->SetBranchAddress("Nclusters_Z1", &Nclusters_Z1);
    chain->SetBranchAddress("Nclusters_Z2", &Nclusters_Z2);
    chain->SetBranchAddress("Nclusters_Z3", &Nclusters_Z3);
    chain->SetBranchAddress("Nclusters_Z4", &Nclusters_Z4);
    chain->SetBranchAddress("Nclusters_Y1", &Nclusters_Y1);
    chain->SetBranchAddress("Nclusters_Y2", &Nclusters_Y2);
    chain->SetBranchAddress("Nclusters_Y3", &Nclusters_Y3);
    chain->SetBranchAddress("Nclusters_Y4", &Nclusters_Y4);

    chain->SetBranchAddress("ClusterPosition_Z1",  ClusterPosition_Z1);
    chain->SetBranchAddress("ClusterPosition_Z2",  ClusterPosition_Z2);
    chain->SetBranchAddress("ClusterPosition_Z3",  ClusterPosition_Z3);
    chain->SetBranchAddress("ClusterPosition_Z4",  ClusterPosition_Z4);
    chain->SetBranchAddress("ClusterPosition_Y1",  ClusterPosition_Y1);
    chain->SetBranchAddress("ClusterPosition_Y2",  ClusterPosition_Y2);
    chain->SetBranchAddress("ClusterPosition_Y3",  ClusterPosition_Y3);
    chain->SetBranchAddress("ClusterPosition_Y4",  ClusterPosition_Y4);

    chain->SetBranchAddress("ClusterEnergy_Z1",  ClusterEnergy_Z1);
    chain->SetBranchAddress("ClusterEnergy_Z2",  ClusterEnergy_Z2);
    chain->SetBranchAddress("ClusterEnergy_Z3",  ClusterEnergy_Z3);
    chain->SetBranchAddress("ClusterEnergy_Z4",  ClusterEnergy_Z4);
    chain->SetBranchAddress("ClusterEnergy_Y1",  ClusterEnergy_Y1);
    chain->SetBranchAddress("ClusterEnergy_Y2",  ClusterEnergy_Y2);
    chain->SetBranchAddress("ClusterEnergy_Y3",  ClusterEnergy_Y3);
    chain->SetBranchAddress("ClusterEnergy_Y4",  ClusterEnergy_Y4);

    chain->SetBranchAddress("ClusterSize_Z1",  ClusterSize_Z1);
    chain->SetBranchAddress("ClusterSize_Z2",  ClusterSize_Z2);
    chain->SetBranchAddress("ClusterSize_Z3",  ClusterSize_Z3);
    chain->SetBranchAddress("ClusterSize_Z4",  ClusterSize_Z4);
    chain->SetBranchAddress("ClusterSize_Y1",  ClusterSize_Y1);
    chain->SetBranchAddress("ClusterSize_Y2",  ClusterSize_Y2);
    chain->SetBranchAddress("ClusterSize_Y3",  ClusterSize_Y3);
    chain->SetBranchAddress("ClusterSize_Y4",  ClusterSize_Y4);

    chain->SetBranchAddress("nStripsEnergy_Z1", &nStripsEnergy_Z1);
    chain->SetBranchAddress("StripsEnergy_Z1",  StripsEnergy_Z1);
    chain->SetBranchAddress("nStripsEnergy_Z2", &nStripsEnergy_Z2);
    chain->SetBranchAddress("StripsEnergy_Z2",  StripsEnergy_Z2);
    chain->SetBranchAddress("nStripsEnergy_Z3", &nStripsEnergy_Z3);
    chain->SetBranchAddress("StripsEnergy_Z3",  StripsEnergy_Z3);
    chain->SetBranchAddress("nStripsEnergy_Z4", &nStripsEnergy_Z4);
    chain->SetBranchAddress("StripsEnergy_Z4",  StripsEnergy_Z4);
    chain->SetBranchAddress("nStripsEnergy_Y1", &nStripsEnergy_Y1);
    chain->SetBranchAddress("StripsEnergy_Y1",  StripsEnergy_Y1);
    chain->SetBranchAddress("nStripsEnergy_Y2", &nStripsEnergy_Y2);
    chain->SetBranchAddress("StripsEnergy_Y2",  StripsEnergy_Y2);
    chain->SetBranchAddress("nStripsEnergy_Y3", &nStripsEnergy_Y3);
    chain->SetBranchAddress("StripsEnergy_Y3",  StripsEnergy_Y3);
    chain->SetBranchAddress("nStripsEnergy_Y4", &nStripsEnergy_Y4);
    chain->SetBranchAddress("StripsEnergy_Y4",  StripsEnergy_Y4);

    chain->SetBranchAddress("nTrackCluster_z1_index", &nTrackCluster_z1_index);
    chain->SetBranchAddress("TrackCluster_z1_index",  TrackCluster_z1_index);
    chain->SetBranchAddress("nTrackCluster_z2_index", &nTrackCluster_z2_index);
    chain->SetBranchAddress("TrackCluster_z2_index",  TrackCluster_z2_index);
    chain->SetBranchAddress("nTrackCluster_z3_index", &nTrackCluster_z3_index);
    chain->SetBranchAddress("TrackCluster_z3_index",  TrackCluster_z3_index);
    chain->SetBranchAddress("nTrackCluster_y1_index", &nTrackCluster_y1_index);
    chain->SetBranchAddress("TrackCluster_y1_index",  TrackCluster_y1_index);
    chain->SetBranchAddress("nTrackCluster_y2_index", &nTrackCluster_y2_index);
    chain->SetBranchAddress("TrackCluster_y2_index",  TrackCluster_y2_index);
    chain->SetBranchAddress("nTrackCluster_y3_index", &nTrackCluster_y3_index);
    chain->SetBranchAddress("TrackCluster_y3_index",  TrackCluster_y3_index);

    chain->SetBranchAddress("ncluster_c4_index_xz", &ncluster_c4_index_xz);
    chain->SetBranchAddress("cluster_c4_index_xz",  cluster_c4_index_xz);
    chain->SetBranchAddress("ncluster_c4_index_xy", &ncluster_c4_index_xy);
    chain->SetBranchAddress("cluster_c4_index_xy",  cluster_c4_index_xy);

    chain->SetBranchAddress("nSlope_3p_xz", &nSlope_3p_xz);
    chain->SetBranchAddress("Slope_3p_xz",  Slope_3p_xz);
    chain->SetBranchAddress("nSlope_3p_xy", &nSlope_3p_xy);
    chain->SetBranchAddress("Slope_3p_xy",  Slope_3p_xy);
    chain->SetBranchAddress("nSlope_4p_xz", &nSlope_4p_xz);
    chain->SetBranchAddress("Slope_4p_xz",  Slope_4p_xz);
    chain->SetBranchAddress("nSlope_4p_xy", &nSlope_4p_xy);
    chain->SetBranchAddress("Slope_4p_xy",  Slope_4p_xy);
    chain->SetBranchAddress("nchiSquare_4p_xz", &nchiSquare_4p_xz);
    chain->SetBranchAddress("chiSquare_4p_xz",  chiSquare_4p_xz);
    chain->SetBranchAddress("nchiSquare_4p_xy", &nchiSquare_4p_xy);
    chain->SetBranchAddress("chiSquare_4p_xy",  chiSquare_4p_xy);


    // OUTPUT

    TFile* fout = new TFile(outName.c_str(), "RECREATE");
    TTree* tout = new TTree("AnalyzedData", "Events with good 4p track");

    // Output variables
    int    out_run;
    double out_chi2_3p_xz, out_chi2_3p_xy;
    double out_chi2_4p_xz, out_chi2_4p_xy;
    double out_theta_3p,   out_phi_3p;
    double out_theta_4p,   out_phi_4p;
    double out_corrE_Z1, out_corrE_Z2, out_corrE_Z3, out_corrE_Z4;
    double out_corrE_Y1, out_corrE_Y2, out_corrE_Y3, out_corrE_Y4;
    double out_slope_xz_3p, out_slope_xy_3p;
    double out_slope_xz_4p, out_slope_xy_4p;

    int    out_ncl_Z1, out_ncl_Z2, out_ncl_Z3, out_ncl_Z4;
    int    out_ncl_Y1, out_ncl_Y2, out_ncl_Y3, out_ncl_Y4;

    vector<double> out_pos_Z1, out_pos_Z2, out_pos_Z3, out_pos_Z4;
    vector<double> out_pos_Y1, out_pos_Y2, out_pos_Y3, out_pos_Y4;

    vector<double> out_en_Z1, out_en_Z2, out_en_Z3, out_en_Z4;
    vector<double> out_en_Y1, out_en_Y2, out_en_Y3, out_en_Y4;

    vector<int> out_size_Z1, out_size_Z2, out_size_Z3, out_size_Z4;
    vector<int> out_size_Y1, out_size_Y2, out_size_Y3, out_size_Y4;

    vector<vector<double>> out_strips_Z1, out_strips_Z2, out_strips_Z3, out_strips_Z4;
    vector<vector<double>> out_strips_Y1, out_strips_Y2, out_strips_Y3, out_strips_Y4;

    // output branches
    tout->Branch("Run",                       &out_run);
    tout->Branch("BestTrack_3p_ChiSquare_xz", &out_chi2_3p_xz);
    tout->Branch("BestTrack_3p_ChiSquare_xy", &out_chi2_3p_xy);
    tout->Branch("BestTrack_4p_ChiSquare_xz", &out_chi2_4p_xz);
    tout->Branch("BestTrack_4p_ChiSquare_xy", &out_chi2_4p_xy);
    tout->Branch("Theta_3p",                  &out_theta_3p);
    tout->Branch("Phi_3p",                    &out_phi_3p);
    tout->Branch("Theta_4p",                  &out_theta_4p);
    tout->Branch("Phi_4p",                    &out_phi_4p);

    tout->Branch("Nclusters_Z1", &out_ncl_Z1);
    tout->Branch("Nclusters_Z2", &out_ncl_Z2);
    tout->Branch("Nclusters_Z3", &out_ncl_Z3);
    tout->Branch("Nclusters_Z4", &out_ncl_Z4);
    tout->Branch("Nclusters_Y1", &out_ncl_Y1);
    tout->Branch("Nclusters_Y2", &out_ncl_Y2);
    tout->Branch("Nclusters_Y3", &out_ncl_Y3);
    tout->Branch("Nclusters_Y4", &out_ncl_Y4);

    tout->Branch("ClusterPosition_Z1", &out_pos_Z1);
    tout->Branch("ClusterPosition_Z2", &out_pos_Z2);
    tout->Branch("ClusterPosition_Z3", &out_pos_Z3);
    tout->Branch("ClusterPosition_Z4", &out_pos_Z4);
    tout->Branch("ClusterPosition_Y1", &out_pos_Y1);
    tout->Branch("ClusterPosition_Y2", &out_pos_Y2);
    tout->Branch("ClusterPosition_Y3", &out_pos_Y3);
    tout->Branch("ClusterPosition_Y4", &out_pos_Y4);

    tout->Branch("ClusterEnergy_Z1", &out_en_Z1);
    tout->Branch("ClusterEnergy_Z2", &out_en_Z2);
    tout->Branch("ClusterEnergy_Z3", &out_en_Z3);
    tout->Branch("ClusterEnergy_Z4", &out_en_Z4);
    tout->Branch("ClusterEnergy_Y1", &out_en_Y1);
    tout->Branch("ClusterEnergy_Y2", &out_en_Y2);
    tout->Branch("ClusterEnergy_Y3", &out_en_Y3);
    tout->Branch("ClusterEnergy_Y4", &out_en_Y4);

    tout->Branch("ClusterSize_Z1", &out_size_Z1);
    tout->Branch("ClusterSize_Z2", &out_size_Z2);
    tout->Branch("ClusterSize_Z3", &out_size_Z3);
    tout->Branch("ClusterSize_Z4", &out_size_Z4);
    tout->Branch("ClusterSize_Y1", &out_size_Y1);
    tout->Branch("ClusterSize_Y2", &out_size_Y2);
    tout->Branch("ClusterSize_Y3", &out_size_Y3);
    tout->Branch("ClusterSize_Y4", &out_size_Y4);

    tout->Branch("StripsEnergy_Z1", &out_strips_Z1);
    tout->Branch("StripsEnergy_Z2", &out_strips_Z2);
    tout->Branch("StripsEnergy_Z3", &out_strips_Z3);
    tout->Branch("StripsEnergy_Z4", &out_strips_Z4);
    tout->Branch("StripsEnergy_Y1", &out_strips_Y1);
    tout->Branch("StripsEnergy_Y2", &out_strips_Y2);
    tout->Branch("StripsEnergy_Y3", &out_strips_Y3);
    tout->Branch("StripsEnergy_Y4", &out_strips_Y4);

    tout->Branch("CorrEnergy_Z1", &out_corrE_Z1);
    tout->Branch("CorrEnergy_Z2", &out_corrE_Z2);
    tout->Branch("CorrEnergy_Z3", &out_corrE_Z3);
    tout->Branch("CorrEnergy_Z4", &out_corrE_Z4);
    tout->Branch("CorrEnergy_Y1", &out_corrE_Y1);
    tout->Branch("CorrEnergy_Y2", &out_corrE_Y2);
    tout->Branch("CorrEnergy_Y3", &out_corrE_Y3);
    tout->Branch("CorrEnergy_Y4", &out_corrE_Y4);
    tout->Branch("Slope_3p_xz",   &out_slope_xz_3p);
    tout->Branch("Slope_3p_xy",   &out_slope_xy_3p);
    tout->Branch("Slope_4p_xz",   &out_slope_xz_4p);
    tout->Branch("Slope_4p_xy",   &out_slope_xy_4p);


    //  EVENT LOOP

    Long64_t nPassed = 0;
    Long64_t nAllSize2 = 0;
    Long64_t nY1Size2=0, nY2Size2=0, nY3Size2=0, nY4Size2=0;
    Long64_t nZ1Size2=0, nZ2Size2=0, nZ3Size2=0, nZ4Size2=0;

    for (Long64_t ev = 0; ev < nEntries; ev++) {

        if (ev % 100000 == 0)
            cout << "Processing event " << ev << " / " << nEntries << endl;

        chain->GetEntry(ev);

        // chiSquarecut

        if (BestTrack_3p_ChiSquare_xz >= chi2cutmax) continue;
        if (BestTrack_3p_ChiSquare_xy >= chi2cutmax) continue;
	if (BestTrack_3p_ChiSquare_xz < 0) continue;
	if (BestTrack_3p_ChiSquare_xy < 0) continue;


        // Best 3p track index (row in TrackCluster_z*_index)

        Long64_t i_xz = BestTrack_3p_xz_index;
        Long64_t i_xy = BestTrack_3p_xy_index;

        // // Safety check

        if (i_xz < 0 || i_xy < 0) continue;
        if (i_xz >= (Long64_t)nTrackCluster_z1_index) continue;
        if (i_xy >= (Long64_t)nTrackCluster_y1_index) continue;

	// Which cluster at each plane

        int cl_z1 = (int)TrackCluster_z1_index[i_xz];
        int cl_z2 = (int)TrackCluster_z2_index[i_xz];
        int cl_z3 = (int)TrackCluster_z3_index[i_xz];
        int cl_y1 = (int)TrackCluster_y1_index[i_xy];
        int cl_y2 = (int)TrackCluster_y2_index[i_xy];
        int cl_y3 = (int)TrackCluster_y3_index[i_xy];


        if (Track_4p_index_xz < 0 || Track_4p_index_xy < 0) continue;

	////////////////best 4p track cluster - match best chisquare with chi value to find hte cluster////////////////

        int flat_xz = -1;
        for (int k = 0; k < nchiSquare_4p_xz; k++) {
            if (chiSquare_4p_xz[k] == BestTrack_4p_ChiSquare_xz) { flat_xz = k; break; }
        }
        int flat_xy = -1;
        for (int k = 0; k < nchiSquare_4p_xy; k++) {
            if (chiSquare_4p_xy[k] == BestTrack_4p_ChiSquare_xy) { flat_xy = k; break; }
        }
        if (flat_xz < 0 || flat_xy < 0) continue;

        int cl_z4 = (int)cluster_c4_index_xz[flat_xz];
        int cl_y4 = (int)cluster_c4_index_xy[flat_xy];

        // Safety check

        if (cl_z1 < 0 || cl_z1 >= Nclusters_Z1) continue;
        if (cl_z2 < 0 || cl_z2 >= Nclusters_Z2) continue;
        if (cl_z3 < 0 || cl_z3 >= Nclusters_Z3) continue;
        if (cl_z4 < 0 || cl_z4 >= Nclusters_Z4) continue;
        if (cl_y1 < 0 || cl_y1 >= Nclusters_Y1) continue;
        if (cl_y2 < 0 || cl_y2 >= Nclusters_Y2) continue;
        if (cl_y3 < 0 || cl_y3 >= Nclusters_Y3) continue;
        if (cl_y4 < 0 || cl_y4 >= Nclusters_Y4) continue;

        // ---- FILL OUTPUT ----

        out_run         = (int)Run;
        out_chi2_3p_xz  = BestTrack_3p_ChiSquare_xz;
        out_chi2_3p_xy  = BestTrack_3p_ChiSquare_xy;
        out_chi2_4p_xz  = BestTrack_4p_ChiSquare_xz;
        out_chi2_4p_xy  = BestTrack_4p_ChiSquare_xy;
        out_theta_3p    = Theta_3p;
        out_phi_3p      = Phi_3p;
        out_theta_4p    = Theta_4p;
        out_phi_4p      = Phi_4p;

        // 3D path-length correction: E_corr = E / sqrt(1 + sx^2 + sy^2)


        double sx3 = Slope_3p_xz[i_xz];
        double sy3 = Slope_3p_xy[i_xy];
        double corr_3p = 1.0 / sqrt(1.0 + sx3*sx3 + sy3*sy3);

        double sx4 = Slope_4p_xz[flat_xz];
        double sy4 = Slope_4p_xy[flat_xy];
        double corr_4p = 1.0 / sqrt(1.0 + sx4*sx4 + sy4*sy4);


        out_corrE_Z1 = ClusterEnergy_Z1[cl_z1] * corr_3p;
        out_corrE_Z2 = ClusterEnergy_Z2[cl_z2] * corr_3p;
        out_corrE_Z3 = ClusterEnergy_Z3[cl_z3] * corr_3p;
        out_corrE_Y1 = ClusterEnergy_Y1[cl_y1] * corr_3p;
        out_corrE_Y2 = ClusterEnergy_Y2[cl_y2] * corr_3p;
        out_corrE_Y3 = ClusterEnergy_Y3[cl_y3] * corr_3p;
        out_corrE_Z4 = ClusterEnergy_Z4[cl_z4] * corr_4p;
        out_corrE_Y4 = ClusterEnergy_Y4[cl_y4] * corr_4p;

        out_slope_xz_3p = sx3;
        out_slope_xy_3p = sy3;
        out_slope_xz_4p = sx4;
        out_slope_xy_4p = sy4;

        // Total clusters in each plane
        out_ncl_Z1 = (int)Nclusters_Z1;
        out_ncl_Z2 = (int)Nclusters_Z2;
        out_ncl_Z3 = (int)Nclusters_Z3;
        out_ncl_Z4 = (int)Nclusters_Z4;
        out_ncl_Y1 = (int)Nclusters_Y1;
        out_ncl_Y2 = (int)Nclusters_Y2;
        out_ncl_Y3 = (int)Nclusters_Y3;
        out_ncl_Y4 = (int)Nclusters_Y4;

        // Position of track cluster
        out_pos_Z1 = {ClusterPosition_Z1[cl_z1]};
        out_pos_Z2 = {ClusterPosition_Z2[cl_z2]};
        out_pos_Z3 = {ClusterPosition_Z3[cl_z3]};
        out_pos_Z4 = {ClusterPosition_Z4[cl_z4]};
        out_pos_Y1 = {ClusterPosition_Y1[cl_y1]};
        out_pos_Y2 = {ClusterPosition_Y2[cl_y2]};
        out_pos_Y3 = {ClusterPosition_Y3[cl_y3]};
        out_pos_Y4 = {ClusterPosition_Y4[cl_y4]};

        // Energy of track cluster
        out_en_Z1 = {ClusterEnergy_Z1[cl_z1]};
        out_en_Z2 = {ClusterEnergy_Z2[cl_z2]};
        out_en_Z3 = {ClusterEnergy_Z3[cl_z3]};
        out_en_Z4 = {ClusterEnergy_Z4[cl_z4]};
        out_en_Y1 = {ClusterEnergy_Y1[cl_y1]};
        out_en_Y2 = {ClusterEnergy_Y2[cl_y2]};
        out_en_Y3 = {ClusterEnergy_Y3[cl_y3]};
        out_en_Y4 = {ClusterEnergy_Y4[cl_y4]};

        // Size of track cluster
        out_size_Z1 = {(int)ClusterSize_Z1[cl_z1]};
        out_size_Z2 = {(int)ClusterSize_Z2[cl_z2]};
        out_size_Z3 = {(int)ClusterSize_Z3[cl_z3]};
        out_size_Z4 = {(int)ClusterSize_Z4[cl_z4]};
        out_size_Y1 = {(int)ClusterSize_Y1[cl_y1]};
        out_size_Y2 = {(int)ClusterSize_Y2[cl_y2]};
        out_size_Y3 = {(int)ClusterSize_Y3[cl_y3]};
        out_size_Y4 = {(int)ClusterSize_Y4[cl_y4]};

        // Strip energies of track cluster (slice out of the flattened,
        // cumulative-offset per-plane strips array)
        out_strips_Z1 = {stripSlice(StripsEnergy_Z1, ClusterSize_Z1, cl_z1)};
        out_strips_Z2 = {stripSlice(StripsEnergy_Z2, ClusterSize_Z2, cl_z2)};
        out_strips_Z3 = {stripSlice(StripsEnergy_Z3, ClusterSize_Z3, cl_z3)};
        out_strips_Z4 = {stripSlice(StripsEnergy_Z4, ClusterSize_Z4, cl_z4)};
        out_strips_Y1 = {stripSlice(StripsEnergy_Y1, ClusterSize_Y1, cl_y1)};
        out_strips_Y2 = {stripSlice(StripsEnergy_Y2, ClusterSize_Y2, cl_y2)};
        out_strips_Y3 = {stripSlice(StripsEnergy_Y3, ClusterSize_Y3, cl_y3)};
        out_strips_Y4 = {stripSlice(StripsEnergy_Y4, ClusterSize_Y4, cl_y4)};

        if (out_size_Y1[0]==2) nY1Size2++;
        if (out_size_Y2[0]==2) nY2Size2++;
        if (out_size_Y3[0]==2) nY3Size2++;
        if (out_size_Y4[0]==2) nY4Size2++;
        if (out_size_Z1[0]==2) nZ1Size2++;
        if (out_size_Z2[0]==2) nZ2Size2++;
        if (out_size_Z3[0]==2) nZ3Size2++;
        if (out_size_Z4[0]==2) nZ4Size2++;
        if (out_size_Y1[0]==2&&out_size_Y2[0]==2&&out_size_Y3[0]==2&&out_size_Y4[0]==2&&
            out_size_Z1[0]==2&&out_size_Z2[0]==2&&out_size_Z3[0]==2&&out_size_Z4[0]==2) nAllSize2++;

        tout->Fill();
        nPassed++;
    }


    //  SAVE AND CLOSE

    fout->cd();
    tout->Write();
    fout->Close();

    cout << "Done. Events passing chi2 cut: " << nPassed << " / " << nEntries << " (" << 100.0*nPassed/nEntries << "%)" << endl;
    cout << "  Plane   size==2 / nPassed   (%)" << endl;
    cout << "  Y1      " << nY1Size2 << " / " << nPassed << " (" << 100.0*nY1Size2/nPassed << "%)" << endl;
    cout << "  Y2      " << nY2Size2 << " / " << nPassed << " (" << 100.0*nY2Size2/nPassed << "%)" << endl;
    cout << "  Y3      " << nY3Size2 << " / " << nPassed << " (" << 100.0*nY3Size2/nPassed << "%)" << endl;
    cout << "  Y4      " << nY4Size2 << " / " << nPassed << " (" << 100.0*nY4Size2/nPassed << "%)" << endl;
    cout << "  Z1      " << nZ1Size2 << " / " << nPassed << " (" << 100.0*nZ1Size2/nPassed << "%)" << endl;
    cout << "  Z2      " << nZ2Size2 << " / " << nPassed << " (" << 100.0*nZ2Size2/nPassed << "%)" << endl;
    cout << "  Z3      " << nZ3Size2 << " / " << nPassed << " (" << 100.0*nZ3Size2/nPassed << "%)" << endl;
    cout << "  Z4      " << nZ4Size2 << " / " << nPassed << " (" << 100.0*nZ4Size2/nPassed << "%)" << endl;
    cout << "  All 8   " << nAllSize2 << " / " << nPassed << " (" << 100.0*nAllSize2/nPassed << "%)" << endl;
    cout << "Output written to: " << outName << endl;
}

select_4p_tracks();
