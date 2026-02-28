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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(EventAction* eventAction, PrimaryGeneratorInfo* generatorInfo) : fEventAction(eventAction), fGeneratorInfo(generatorInfo)
{
  // Create the generic analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  // If the filename extension is not provided, the default file type (root)
  // will be used for all files specified without extension.
  analysisManager->SetVerboseLevel(1);

  // Default settings
  analysisManager->SetNtupleMerging(true);
  // Note: merging ntuples is available only with Root output
  analysisManager->SetFileName("MuravesSim_Data"); 

  // Creating ntuples
  if (fEventAction) {
    // tuple Id = 0 --- Primary particle-generation-level ---------
    analysisManager->CreateNtuple("PrimaryGenData", "Event-level information of generated primaries");

    analysisManager->CreateNtupleIColumn("Event"); // column Id = 0
    
    analysisManager->CreateNtupleIColumn("NGenPart"); // column Id = 1
    analysisManager->CreateNtupleIColumn("GenPartID");  // column Id = 2
    analysisManager->CreateNtupleIColumn("GenPartPDG");  // column Id = 3
    analysisManager->CreateNtupleDColumn("GenPartE"); // column Id = 4   
    analysisManager->CreateNtupleDColumn("GenPartTheta"); // column Id = 5  
    analysisManager->CreateNtupleDColumn("GenPartPhi"); // column Id = 6
    
    analysisManager->FinishNtuple();

     // tuple Id = 1 --- Hit-level ------------------------
    analysisManager->CreateNtuple("HitData", "Hit-level information of hits in scintillator bars");

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
  
    analysisManager->FinishNtuple();

    analysisManager->CreateH1("theta", "Muon Theta", 50, 0., M_PI);
    analysisManager->CreateH1("phi",   "Muon Phi",   50, -M_PI, M_PI);

    analysisManager->CreateH1("x", "Muon generated x", 50, -80*cm, 200*cm);
    analysisManager->CreateH1("y",   "Muon generated y",   50, -140*cm, 140*cm);
    analysisManager->CreateH1("z",   "Muon generated z",   50, -150*cm, 130*cm);
  }

  // Set ntuple output file
  //analysisManager->SetNtupleFileName(0, "Muraves_GenData");
  //analysisManager->SetNtupleFileName(1, "Muraves_HitData");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  G4int nEvents = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();

  auto det = static_cast<const DetectorConstruction*>(
                  G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  std::string timestamp = GetTimestamp();
  
  std::string configFilename = "run_config_" + timestamp + ".txt";
  std::string runFilename = "MuravesSim_Data_" + timestamp;

  std::string generatorSummary;
    if (fGeneratorInfo) {
        generatorSummary = fGeneratorInfo->GetInfoSummary();
    }

  RunInformation::Write(configFilename, det, nEvents, generatorSummary);

  // inform the runManager to save random number seed
  // G4RunManager::GetRunManager()->SetRandomNumberStore(true);

  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->Reset();

  // Open an output file
  // The default file name is set in RunAction::RunAction(),
  // it can be overwritten in a macro
  analysisManager->OpenFile(runFilename);
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

