//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file RunAction.cc
/// \brief Implementation of the B5::RunAction class

#include "RunAction.hh"

#include "EventAction.hh"
#include "RunInformation.hh"

#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include <chrono>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(EventAction* eventAction, PrimaryGeneratorInfo* generatorInfo, long seed) : fEventAction(eventAction), fGeneratorInfo(generatorInfo), fSeed(seed)
{
  // Create the generic analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  
  
  analysisManager->SetFileName("MuravesSim_Data"); 

  fDataPath = ".";  // fallback (command will overwrite this)

  fDataPathCmd = new G4UIcmdWithAString("/muraves/dataPath", this);
  fDataPathCmd->SetGuidance("Set the output data directory path");
  fDataPathCmd->SetParameterName("path", false);
  fDataPathCmd->SetDefaultValue("."); 

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::SetNewValue(G4UIcommand* cmd, G4String val) {
    if (cmd == fDataPathCmd) fDataPath = val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  startTime = std::chrono::high_resolution_clock::now();

  G4int nEvents = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();

  auto det = static_cast<const DetectorConstruction*>(
                  G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  fTimestamp = GetTimestamp();

  const char* clusterId = std::getenv("CLUSTER_ID");
  const char* processId = std::getenv("PROCESS_ID");
  const char* nameFlag = std::getenv("NAME_FLAG");
  fIsHTCondor = (clusterId && processId);

  fClusterId = clusterId ? clusterId : "local";
  fProcessId = processId ? processId : "local";
  fNameFlag = nameFlag ? nameFlag : "local";
  
  //std::string configFilename = "../../../Muraves_SimData/run_config_" + timestamp + ".txt";
  //std::string runFilename = "../../../MuravesSim_Data/MuravesSim_Data_" + fTimestamp;
  //std::string runFilename    = std::string(fDataPath) + "/MuravesSim_Data_" + fTimestamp + "_FS_UR";
  std::string runFilename;
  if (fIsHTCondor) {
    runFilename = std::string(fDataPath) + "/musimData_c" + fClusterId + "_p" + fProcessId + "_" + fNameFlag;
} else {
     runFilename = std::string(fDataPath) + "/musimData_" + fTimestamp;
}

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true); // merge tuples from worker threads
  
  //std::string configFilename = "../../../Muraves_SimData/run_config_" + timestamp + ".txt";
  //std::string runFilename = "../../../MuravesSim_Data/MuravesSim_Data_" + fTimestamp;
  //std::string runFilename = "/output/MuravesSim_Data_" + fTimestamp;
  analysisManager->OpenFile(runFilename);

  //analysisManager->Reset();


  // Creating ntuples
  
    // tuple Id = 0 --- Primary particle-generation-level: triggering muon---------
    analysisManager->CreateNtuple("PrimaryGenData", "Event-level information of generated primaries");

    analysisManager->CreateNtupleIColumn("Event"); // column Id = 0
    
    //analysisManager->CreateNtupleIColumn("NGenPart"); 
    analysisManager->CreateNtupleIColumn("GenPartID");  // column Id = 1
    analysisManager->CreateNtupleIColumn("GenPartPDG");  // column Id = 2
    analysisManager->CreateNtupleDColumn("GenPartE"); // column Id = 3   
    analysisManager->CreateNtupleDColumn("GenPartTheta"); // column Id = 4  
    analysisManager->CreateNtupleDColumn("GenPartPhi"); // column Id = 5
    analysisManager->CreateNtupleIColumn("Aborted"); // column Id = 6
    analysisManager->CreateNtupleIColumn("ClusterID");     // 7  
    analysisManager->CreateNtupleIColumn("ProcessID");     // 8  
    analysisManager->CreateNtupleDColumn("GenPosX"); // 9
    analysisManager->CreateNtupleDColumn("GenPosY"); // 10
    analysisManager->CreateNtupleDColumn("GenPosZ"); // 11
    //analysisManager->CreateNtupleIColumn("touchedRock"); // column Id = 12
    
    analysisManager->FinishNtuple();

     // tuple Id = 1 --- Hit-level: triggering muon ------------------------
    analysisManager->CreateNtuple("HitData", "Hit-level information of hits in scintillator bars");

    analysisManager->CreateNtupleIColumn("Event"); // column Id = 0
    
    //analysisManager->CreateNtupleIColumn("NScintHit"); 
    analysisManager->CreateNtupleIColumn("ScintHitParentID"); // column Id = 1
    analysisManager->CreateNtupleDColumn("ScintHitE"); // column Id = 2
    analysisManager->CreateNtupleDColumn("ScintHitEntryX"); // column Id = 3
    analysisManager->CreateNtupleDColumn("ScintHitEntryY"); // column Id = 4
    analysisManager->CreateNtupleDColumn("ScintHitEntryZ"); // column Id = 5
    analysisManager->CreateNtupleDColumn("ScintHitExitX"); // column Id = 6
    analysisManager->CreateNtupleDColumn("ScintHitExitY"); // column Id = 7
    analysisManager->CreateNtupleDColumn("ScintHitExitZ"); // column Id = 8
    analysisManager->CreateNtupleDColumn("ScintHitPathLength"); // column Id = 9
    analysisManager->CreateNtupleIColumn("ScintHitStation"); // column Id = 10
    analysisManager->CreateNtupleIColumn("ScintHitModule"); // column Id = 11
    analysisManager->CreateNtupleIColumn("ScintHitBar"); // column Id = 12
    analysisManager->CreateNtupleIColumn("ScintHitPDG"); // column Id = 13
    analysisManager->CreateNtupleIColumn("ScintHitTrackID"); // column Id = 14
    analysisManager->CreateNtupleIColumn("ClusterID");         // 15 
    analysisManager->CreateNtupleIColumn("ProcessID");         // 16
    analysisManager->CreateNtupleDColumn("ScintHitTime");   // 17
  
    analysisManager->FinishNtuple();

    // tuple Id = 2 --- Primary particle-generation-level: non-triggering muon ---------
    /*analysisManager->CreateNtuple("PrimaryGenData_NT", "Event-level information of generated primaries");

    analysisManager->CreateNtupleIColumn("Event"); // column Id = 0
    
    analysisManager->CreateNtupleIColumn("NGenPart"); // column Id = 1
    analysisManager->CreateNtupleIColumn("GenPartID");  // column Id = 2
    analysisManager->CreateNtupleIColumn("GenPartPDG");  // column Id = 3
    analysisManager->CreateNtupleDColumn("GenPartE"); // column Id = 4   
    analysisManager->CreateNtupleDColumn("GenPartTheta"); // column Id = 5  
    analysisManager->CreateNtupleDColumn("GenPartPhi"); // column Id = 6
    analysisManager->CreateNtupleIColumn("Aborted"); // column Id = 7
    //analysisManager->CreateNtupleIColumn("touchedRock"); // column Id = 8
    

    analysisManager->FinishNtuple();

     // tuple Id = 3 --- Hit-level: non-triggering muon ------------------------
    analysisManager->CreateNtuple("HitData_NT", "Hit-level information of hits in scintillator bars");

    analysisManager->CreateNtupleIColumn("Event"); // column Id = 0
    
    analysisManager->CreateNtupleIColumn("NScintHit"); // column Id = 1
    analysisManager->CreateNtupleIColumn("ScintHitParentID"); // column Id = 2
    analysisManager->CreateNtupleDColumn("ScintHitE"); // column Id = 3
    analysisManager->CreateNtupleDColumn("ScintHitPosX"); // column Id = 4
    analysisManager->CreateNtupleDColumn("ScintHitPosY"); // column Id = 5
    analysisManager->CreateNtupleDColumn("ScintHitPosZ"); // column Id = 6
    analysisManager->CreateNtupleIColumn("ScintHitStation"); // column Id = 7
    analysisManager->CreateNtupleIColumn("ScintHitModule"); // column Id = 8
    analysisManager->CreateNtupleIColumn("ScintHitBar"); // column Id = 9
    analysisManager->CreateNtupleIColumn("ScintHitPDG"); // column Id = 10
    analysisManager->CreateNtupleIColumn("ScintHitTrackID"); // column Id = 11
  
    analysisManager->FinishNtuple();*/

    //analysisManager->CreateH1("theta", "Muon Theta", 50, 0., M_PI);
    //analysisManager->CreateH1("phi",   "Muon Phi",   50, -M_PI, M_PI);

    //analysisManager->CreateH1("x", "Muon generated x", 50, -80*cm, 200*cm);
    //analysisManager->CreateH1("y",   "Muon generated y",   50, -140*cm, 140*cm);
    //analysisManager->CreateH1("z",   "Muon generated z",   50, -150*cm, 130*cm);
  

    if (fGeneratorInfo) fGeneratorInfo->Initialize();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  // ntuples
  //
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile(false);
  // Keep content so that they are plotted.
  // The content will be reset at start of the next run.
  

  // Record end time
    endTime = std::chrono::high_resolution_clock::now();

    // Calculate elapsed time in milliseconds
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Detector info
    auto det = static_cast<const DetectorConstruction*>(
                    G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    G4int nEvents = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();


    std::string generatorSummary;
    if (fGeneratorInfo) {
        generatorSummary = fGeneratorInfo->GetInfoSummary();
    }


    // Generate filename
    //std::string configFilename = "../../../MuravesSim_Data/run_config_" + fTimestamp + ".txt";
    //std::string configFilename = std::string(fDataPath) + "/run_config_" + fTimestamp + "_FS_UR.txt";

    std::string configFilename ;
    if (fIsHTCondor) {
    configFilename = std::string(fDataPath) + "/musimLog_c" + fClusterId + "_p" + fProcessId + "_" + fNameFlag + ".txt";
    } else {
       configFilename = std::string(fDataPath) + "/musimLog_" + fTimestamp + ".txt";
    }

    // Write all run info including runtime
      if (!generatorSummary.empty()) {
    RunInformation::Write(configFilename, det, nEvents, generatorSummary, total_ms, fSeed);}

    // Write & close analysis data
    //auto analysisManager = G4AnalysisManager::Instance();
    //analysisManager->Write();
    //analysisManager->CloseFile(false);

}

// generate YYYYMMDD_HHMMSS timestamp to get unique names for output files (avoids overwriting)
std::string RunAction::GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* t = std::localtime(&now);

    std::ostringstream oss;
    oss << (t->tm_year + 1900)
        << std::setw(2) << std::setfill('0') << (t->tm_mon + 1)
        << std::setw(2) << std::setfill('0') << t->tm_mday
        << "_"
        << std::setw(2) << std::setfill('0') << t->tm_hour
        << std::setw(2) << std::setfill('0') << t->tm_min
        << std::setw(2) << std::setfill('0') << t->tm_sec;
    return oss.str();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

