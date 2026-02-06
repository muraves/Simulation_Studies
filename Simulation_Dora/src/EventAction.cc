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
/// \file EventAction.cc
/// \brief Implementation of the B5::EventAction class

#include "EventAction.hh"

//#include "Analysis.hh"
//#include "ROOTManager.hh"
#include "TrackingAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "ScintBarHit.hh"
#include "G4AnalysisManager.hh"

using std::array;
using std::vector;

namespace
{

// Utility function which finds a hit collection with the given Id
// and print warnings if not found
G4VHitsCollection* GetHC(const G4Event* event, G4int collId)
{
  auto hce = event->GetHCofThisEvent();
  if (!hce) {
    G4ExceptionDescription msg;
    msg << "No hits collection of this event found." << G4endl;
    G4Exception("EventAction::EndOfEventAction()", "Code001", JustWarning, msg);
    return nullptr;
  }

  auto hc = hce->GetHC(collId);
  if (!hc) {
    G4ExceptionDescription msg;
    msg << "Hits collection " << collId << " of this event not found." << G4endl;
    G4Exception("EventAction::EndOfEventAction()", "Code001", JustWarning, msg);
  }
  return hc;
}

}  // namespace

namespace B5
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
: G4UserEventAction(),
    fScintbarsHCID(-1)
{
  // set printing per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{
  // Find hit collections by names (just once)
  if ( fScintbarsHCID == -1 ) {
    auto SDManager = G4SDManager::GetSDMpointer();
    fScintbarsHCID = SDManager->GetCollectionID("Scintbars/ScintbarsColl");
  }
  //G4cout << "BeginOfEventAction fScintbarsHCID: " << fScintbarsHCID << G4endl;
  
  TrackingAction::Instance()->ResetParents();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{
  //
  // Fill histograms & ntuple
  //

  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  auto mytracking = TrackingAction::Instance();

  G4int eventID = event->GetEventID();
  G4int nGenPart = 0;
  G4int nScintHit = 0;
 
  auto hc = GetHC(event, fScintbarsHCID);
  if ( !hc ) return;
  
  auto nhit = hc->GetSize();
  G4cout << "Nhit: " << nhit << G4endl;
  
  for ( unsigned long i = 0; i < nhit; i++ ) {
    auto hit = static_cast<ScintbarHit*>(hc->GetHit(i));
    auto pos = hit->GetPos();

    //analysisManager->FillNtupleIColumn(8, mytracking->GetPrimary(hit->GetTrackID()));
    analysisManager->FillNtupleIColumn(1, 0, eventID);
    analysisManager->FillNtupleIColumn(1, 1, nhit); // not really necessary
    analysisManager->FillNtupleIColumn(1, 2, hit->GetTrackID());
    analysisManager->FillNtupleDColumn(1, 3, hit->GetEdep());
    analysisManager->FillNtupleDColumn(1, 4, pos.x());
    analysisManager->FillNtupleDColumn(1, 5, pos.y());
    analysisManager->FillNtupleDColumn(1, 6, pos.z());
    analysisManager->FillNtupleIColumn(1, 7, hit->GetStationID());
    analysisManager->FillNtupleIColumn(1, 8, hit->GetModuleID());
    analysisManager->FillNtupleIColumn(1, 9, hit->GetBarID());
    analysisManager->FillNtupleIColumn(1, 10, hit->GetPDGcode());
  
    analysisManager->AddNtupleRow(1);
  }
  

  analysisManager->FillNtupleIColumn(0, 0, eventID);
  for ( int i = 0; i < event->GetNumberOfPrimaryVertex(); i++ ) {
    for ( int q = 0; q < event->GetPrimaryVertex(i)->GetNumberOfParticle(); q++ ) {
                
      auto primary = event->GetPrimaryVertex(i)->GetPrimary(q);
                 
      analysisManager->FillNtupleIColumn(0, 2, primary->GetTrackID());
      analysisManager->FillNtupleIColumn(0, 3, primary->GetG4code()->GetPDGEncoding());
      analysisManager->FillNtupleDColumn(0, 4, primary->GetTotalEnergy());
      analysisManager->FillNtupleDColumn(0, 5, primary->GetMomentumDirection().theta());
      analysisManager->FillNtupleDColumn(0, 6, primary->GetMomentumDirection().phi());
        
    }    
    analysisManager->AddNtupleRow(0);
  }
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B5
