// Michael Tytgat, 03/06/2020
//
/// \file RunAction.cc
/// \brief Implementation of the RunAction class

#include "RunAction.hh"
#include "EventAction.hh"
#include "Analysis.hh"
#include "ROOTManager.hh"

#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"


RunAction::RunAction(EventAction* eventAction)
 : G4UserRunAction(), 
   fEventAction(eventAction)
{
  /*
  // Create analysis manager
  // The choice of analysis technology is done via selection of a namespace
  // in Analysis.hh
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  // Default settings
  analysisManager->SetNtupleMerging(true);
     // Note: merging ntuples is available only with Root output
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFileName("MuravesSim.root");

  // Creating ntuple
  //
  if ( fEventAction ) {
    analysisManager->CreateNtuple("MuravesSim", "GenMuons");

    analysisManager->CreateNtupleIColumn("NGenMuon"); // id =0
    analysisManager->CreateNtupleDColumn("GenMuonE", );  // column Id = 0
    analysisManager->CreateNtupleIColumn("GenMuonTheta");  // column Id = 1
    analysisManager->CreateNtupleDColumn("GenMuonPhi"); // column Id = 2
    analysisManager                                   // column Id = 6
      ->CreateNtupleDColumn("ECEnergyVector", fEventAction->GetEmCalEdep()); 
    analysisManager                                   // column Id = 7
    ->CreateNtupleDColumn("HCEnergyVector", fEventAction->GetHadCalEdep());
    analysisManager->FinishNtuple();
  }*/

  ROOTManager::Instance()->Init();
}

RunAction::~RunAction()
{
  delete G4AnalysisManager::Instance();  
}

void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{ 
  //inform the runManager to save random number seed
  //G4RunManager::GetRunManager()->SetRandomNumberStore(true);

  /*
  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Open an output file 
  // The default file name is set in RunAction::RunAction(),
  // it can be overwritten in a macro
  analysisManager->OpenFile();
  */
}

void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  // save histograms & ntuple
  //
  /*
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
  */
  ROOTManager::Instance()->Save();
}

