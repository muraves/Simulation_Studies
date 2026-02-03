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
    analysisManager->FillNtupleIColumn(8, hit->GetTrackID());
    analysisManager->FillNtupleDColumn(9, hit->GetEdep());
    analysisManager->FillNtupleDColumn(10, pos.x());
    analysisManager->FillNtupleDColumn(11, pos.y());
    analysisManager->FillNtupleDColumn(12, pos.z());
    analysisManager->FillNtupleIColumn(13, hit->GetStationID());
    analysisManager->FillNtupleIColumn(14, hit->GetModuleID());
    analysisManager->FillNtupleIColumn(15, hit->GetBarID());
    analysisManager->FillNtupleIColumn(16, hit->GetPDGcode());
  }
  analysisManager->AddNtupleRow();

  analysisManager->FillNtupleIColumn(0, eventID);
  for ( int i = 0; i < event->GetNumberOfPrimaryVertex(); i++ ) {
    for ( int q = 0; q < event->GetPrimaryVertex(i)->GetNumberOfParticle(); q++ ) {
                
      auto primary = event->GetPrimaryVertex(i)->GetPrimary(q);
                 
      analysisManager->FillNtupleIColumn(1, primary->GetTrackID());
      analysisManager->FillNtupleIColumn(2, primary->GetG4code()->GetPDGEncoding());
      analysisManager->FillNtupleDColumn(4, primary->GetTotalEnergy());
      analysisManager->FillNtupleDColumn(5, primary->GetMomentumDirection().theta());
      analysisManager->FillNtupleDColumn(6, primary->GetMomentumDirection().phi());
        
    }    
    analysisManager->AddNtupleRow(0);
  }
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B5
