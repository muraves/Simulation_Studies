#include "ROOTManager.hh"

#include <TROOT.h>
#include <TFile.h>
#include "TBranch.h"
#include "TTree.h"
#include <CLHEP/Units/SystemOfUnits.h>
#include "G4UnitsTable.hh"

ROOTManager* ROOTManager::fgInstance = 0;

ROOTManager* ROOTManager::Instance()
{
  return fgInstance;
}      

ROOTManager::ROOTManager()
{ 
  fgInstance = this;
}

ROOTManager::~ROOTManager()
{
  if ( ROOTFile ) delete ROOTFile;
  fgInstance = 0;  
}

void ROOTManager::Init()
{ 
 // Creating a tree container to handle histograms and ntuples.
 // This tree is associated to an output file.
 //
  // create ROOT file (dove alla fine salvare dati e grafici)

  ROOTFile = new TFile("MuravesSim.root", "RECREATE");
  if (!ROOTFile) {
    G4cout << " problem creating the ROOT TFile" << G4endl;
    return;
  }

  // creazione ttree p_incident_TOT
  ROOTTree = new TTree("Muraves", "MuravesSim");

  ROOTTree->Branch("Event", &ROOTTreeStruct.Event, "Event/I");

  ROOTTree->Branch("NGenPart", &ROOTTreeStruct.NGenPart, "NGenPart/I");
  ROOTTree->Branch("GenPartID", &ROOTTreeStruct.GenPartID, "GenPartID[NGenPart]/I");
  ROOTTree->Branch("GenPartPDG", &ROOTTreeStruct.GenPartPDG, "GenPartPDG[NGenPart]/I");
  ROOTTree->Branch("GenPartE", &ROOTTreeStruct.GenPartE, "GenPartE[NGenPart]/F");
  ROOTTree->Branch("GenPartTheta", &ROOTTreeStruct.GenPartTheta, "GenPartTheta[NGenPart]/F");
  ROOTTree->Branch("GenPartPhi", &ROOTTreeStruct.GenPartPhi, "GenPartPhi[NGenPart]/F");

  ROOTTree->Branch("NScintHit", &ROOTTreeStruct.NScintHit, "NScintHit/I");
  ROOTTree->Branch("ScintHitPrimaryID", &ROOTTreeStruct.ScintHitPrimaryID, "ScintHitPrimaryID[NScintHit]/I");
  ROOTTree->Branch("ScintHitE", &ROOTTreeStruct.ScintHitE, "ScintHitE[NScintHit]/F");
  ROOTTree->Branch("ScintHitPosX", &ROOTTreeStruct.ScintHitPosX, "ScintHitPosX[NScintHit]/F");
  ROOTTree->Branch("ScintHitPosY", &ROOTTreeStruct.ScintHitPosY, "ScintHitPosY[NScintHit]/F");
  ROOTTree->Branch("ScintHitPosZ", &ROOTTreeStruct.ScintHitPosZ, "ScintHitPosZ[NScintHit]/F");
  ROOTTree->Branch("ScintHitStation", &ROOTTreeStruct.ScintHitStation, "ScintHitStation[NScintHit]/I");
  ROOTTree->Branch("ScintHitModule", &ROOTTreeStruct.ScintHitModule, "ScintHitModule[NScintHit]/I");
  ROOTTree->Branch("ScintHitBar", &ROOTTreeStruct.ScintHitBar, "ScintHitBar[NScintHit]/I");
  ROOTTree->Branch("ScintHitPDG", &ROOTTreeStruct.ScintHitPDG, "ScintHitPDG[NScintHit]/I");
}

void ROOTManager::Save()
{ 
  if (ROOTFile) {
    ROOTFile->Write();       
    ROOTFile->Close();       
    G4cout << "ROOT Tree closed" << G4endl;
  }
}

void ROOTManager::Fill()
{
  ROOTTree->Fill();
}


