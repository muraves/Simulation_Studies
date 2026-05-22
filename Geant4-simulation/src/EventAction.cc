/// \file EventAction.cc
/// \brief Implementation of the EventAction class

#include "EventAction.hh"

#include "TrackingAction.hh"
#include "ScintBarHit.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4AnalysisManager.hh"
//#include "SteppingAction.hh"
#include <sys/resource.h>

using std::array;
using std::vector;

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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
: G4UserEventAction(),
    fScintbarsHCID(-1)
{
  // set printing per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* event)
{
  G4int eventID = event->GetEventID();
    if (eventID % 50000 == 0) 
        G4cout << ">>> Starting event " << eventID << "..." << G4endl;

  // Find hit collections by names (just once)
  if ( fScintbarsHCID == -1 ) {
    auto SDManager = G4SDManager::GetSDMpointer();
    fScintbarsHCID = SDManager->GetCollectionID("Scintbars/ScintbarsColl");
  }
  
  TrackingAction::Instance()->ResetParents();
  //fTouchedRock=false;
  //if (fSteppingAction) fSteppingAction->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{
  const char* clusterId = std::getenv("CLUSTER_ID");
  const char* processId = std::getenv("PROCESS_ID");
  G4int clusterIdInt = clusterId ? std::atoi(clusterId) : -1;
  G4int processIdInt = processId ? std::atoi(processId) : -1;

  /*if (event->IsAborted()) {
        std::cout << "Event " << event->GetEventID() << " was aborted, skipping save." << std::endl;
        return;  // don't save anything
    }*/
  
  // ------------------- Fill histograms & ntuple -------------------
  
  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  auto mytracking = TrackingAction::Instance();

  G4int eventID = event->GetEventID();
  G4int nGenPart = 0;
  G4int nScintHit = 0;
 
  auto hc = GetHC(event, fScintbarsHCID);
  if ( !hc ) return;
  
  auto nhit = hc->GetSize();
  //G4cout << "Nhit: " << nhit << G4endl;

  // Trigger condition (hit in first 6 planar arrays)
  // stationIDs to be checked: 0 (X0), 8 (Y0), 1 (X1), 9 (Y1), 2 (X2), 10 (Y2)
  bool planeHit[12] = {false};

  for (unsigned long i = 0; i < nhit; i++) {
      auto hit = static_cast<ScintbarHit*>(hc->GetHit(i));
      int stationID = hit->GetStationID();

      if (stationID >= 0 && stationID < 12)
          planeHit[stationID] = true;
  }

  // Require X0,Y0,X1,Y1,X2,Y2
  bool goodEvent =
      planeHit[0]  && planeHit[8]  &&
      planeHit[1]  && planeHit[9]  &&
      planeHit[2]  && planeHit[10];

  if (goodEvent) {
    // Loop over hits
    for ( unsigned long i = 0; i < nhit; i++ ) {
      auto hit = static_cast<ScintbarHit*>(hc->GetHit(i));
      auto entrypoint = hit->GetEntryPoint();
      auto exitpoint = hit->GetExitPoint();

      analysisManager->FillNtupleIColumn(1, 0, eventID);
      //analysisManager->FillNtupleIColumn(1, 1, nhit); // not really necessary
      analysisManager->FillNtupleIColumn(1, 1, hit->GetParentId());
      analysisManager->FillNtupleDColumn(1, 2, hit->GetEdep());
      analysisManager->FillNtupleDColumn(1, 3, entrypoint.x());
      analysisManager->FillNtupleDColumn(1, 4, entrypoint.y());
      analysisManager->FillNtupleDColumn(1, 5, entrypoint.z());
      analysisManager->FillNtupleDColumn(1, 6, exitpoint.x());
      analysisManager->FillNtupleDColumn(1, 7, exitpoint.y());
      analysisManager->FillNtupleDColumn(1, 8, exitpoint.z());
      analysisManager->FillNtupleDColumn(1, 9, hit->GetPathLength());
      analysisManager->FillNtupleIColumn(1, 10, hit->GetStationID());
      analysisManager->FillNtupleIColumn(1, 11, hit->GetModuleID());
      analysisManager->FillNtupleIColumn(1, 12, hit->GetBarID());
      analysisManager->FillNtupleIColumn(1, 13, hit->GetPDGcode());
      analysisManager->FillNtupleIColumn(1, 14, hit->GetTrackID());
      analysisManager->FillNtupleIColumn(1, 15, clusterIdInt);
      analysisManager->FillNtupleIColumn(1, 16, processIdInt);
      analysisManager->FillNtupleDColumn(1, 17, hit->GetHitTime());
    
      analysisManager->AddNtupleRow(1);
    }
    
    // Loop over primaries
    analysisManager->FillNtupleIColumn(0, 0, eventID);
    for ( int i = 0; i < event->GetNumberOfPrimaryVertex(); i++ ) {

      G4ThreeVector vertexPos = event->GetPrimaryVertex(i)->GetPosition();

      for ( int q = 0; q < event->GetPrimaryVertex(i)->GetNumberOfParticle(); q++ ) {
                  
        auto primary = event->GetPrimaryVertex(i)->GetPrimary(q);
     
        analysisManager->FillNtupleIColumn(0, 1, primary->GetTrackID());
        analysisManager->FillNtupleIColumn(0, 2, primary->GetG4code()->GetPDGEncoding());
        analysisManager->FillNtupleDColumn(0, 3, primary->GetTotalEnergy());
        analysisManager->FillNtupleDColumn(0, 4, primary->GetMomentumDirection().theta());
        analysisManager->FillNtupleDColumn(0, 5, primary->GetMomentumDirection().phi()); 
        analysisManager->FillNtupleIColumn(0, 6, event->IsAborted());
        analysisManager->FillNtupleIColumn(0, 7, clusterIdInt);
        analysisManager->FillNtupleIColumn(0, 8, processIdInt); 
        analysisManager->FillNtupleDColumn(0, 9, vertexPos.x() / mm);
        analysisManager->FillNtupleDColumn(0, 10, vertexPos.y() / mm);
        analysisManager->FillNtupleDColumn(0, 11, vertexPos.z() / mm);
      }    
      analysisManager->AddNtupleRow(0);
    }
}

  /*else { //return;   
    for ( unsigned long i = 0; i < nhit; i++ ) {
      auto hit = static_cast<ScintbarHit*>(hc->GetHit(i));
      auto pos = hit->GetPos();

      analysisManager->FillNtupleIColumn(3, 0, eventID);
      analysisManager->FillNtupleIColumn(3, 1, nhit); // not really necessary
      analysisManager->FillNtupleIColumn(3, 2, hit->GetParentId());
      analysisManager->FillNtupleDColumn(3, 3, hit->GetEdep());
      analysisManager->FillNtupleDColumn(3, 4, pos.x());
      analysisManager->FillNtupleDColumn(3, 5, pos.y());
      analysisManager->FillNtupleDColumn(3, 6, pos.z());
      analysisManager->FillNtupleIColumn(3, 7, hit->GetStationID());
      analysisManager->FillNtupleIColumn(3, 8, hit->GetModuleID());
      analysisManager->FillNtupleIColumn(3, 9, hit->GetBarID());
      analysisManager->FillNtupleIColumn(3, 10, hit->GetPDGcode());
      analysisManager->FillNtupleIColumn(3, 11, hit->GetTrackID());
    
      analysisManager->AddNtupleRow(3);
    }
    
    // Loop over primaries
    analysisManager->FillNtupleIColumn(2, 0, eventID);
    for ( int i = 0; i < event->GetNumberOfPrimaryVertex(); i++ ) {
      for ( int q = 0; q < event->GetPrimaryVertex(i)->GetNumberOfParticle(); q++ ) {
                  
        auto primary = event->GetPrimaryVertex(i)->GetPrimary(q);
                  
        analysisManager->FillNtupleIColumn(2, 2, primary->GetTrackID());
        analysisManager->FillNtupleIColumn(2, 3, primary->GetG4code()->GetPDGEncoding());
        analysisManager->FillNtupleDColumn(2, 4, primary->GetTotalEnergy());
        analysisManager->FillNtupleDColumn(2, 5, primary->GetMomentumDirection().theta());
        analysisManager->FillNtupleDColumn(2, 6, primary->GetMomentumDirection().phi()); 
        analysisManager->FillNtupleIColumn(2, 7, event->IsAborted());
        //analysisManager->FillNtupleIColumn(0, 8, fTouchedRock);
      }    
      analysisManager->AddNtupleRow(2);
    }
  }*/

/*struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    long rss_kb = r.ru_maxrss / 1024; // macOS: bytes → KB
    
    static long lastRSS = 0;
    long delta = rss_kb - lastRSS;
    
    G4int id = event->GetEventID();
    if (id % 1 == 0 || delta > 1000) { // print every 100 or on big jumps
        G4cout << "Event " << id 
               << " RSS: " << rss_kb << " KB"
               << " delta: " << delta << " KB"
               << G4endl;
    }
    lastRSS = rss_kb;*/
  
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

