//
// Michael Tytgat
//

#include "EventAction.hh"
#include "Analysis.hh"
#include "Hits.hh"
#include "ROOTManager.hh"
#include "TrackingAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

using std::array;
using std::vector;

namespace {

// Utility function which finds a hit collection with the given Id
// and print warnings if not found 
G4VHitsCollection* GetHC(const G4Event* event, G4int collId) {
  auto hce = event->GetHCofThisEvent();
  if (!hce) {
      G4ExceptionDescription msg;
      msg << "No hits collection of this event found." << G4endl; 
      G4Exception("EventAction::EndOfEventAction()",
                  "Code001", JustWarning, msg);
      return nullptr;
  }

  auto hc = hce->GetHC(collId);
  if ( ! hc) {
    G4ExceptionDescription msg;
    msg << "Hits collection " << collId << " of this event not found." << G4endl; 
    G4Exception("EventAction::EndOfEventAction()",
                "Code001", JustWarning, msg);
  }
  return hc;  
}

}

EventAction::EventAction()
  : G4UserEventAction(),
    fScintbarsHCID(-1)
{
  // set printing per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);
}

EventAction::~EventAction()
{}

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

void EventAction::EndOfEventAction(const G4Event* event)
{
  // Get analysis manager
  //auto analysisManager = G4AnalysisManager::Instance();
  auto myrootmanager = ROOTManager::Instance();
  auto mytracking = TrackingAction::Instance();

  myrootmanager->ROOTTreeStruct.NGenPart = 0;
  myrootmanager->ROOTTreeStruct.NScintHit = 0;
  
  myrootmanager->ROOTTreeStruct.Event = event->GetEventID();
 
  G4cout << ">>> Event " << event->GetEventID()
	 << " >>> Simulation truth : " << G4endl;

  auto hc = GetHC(event, fScintbarsHCID);
  if ( !hc ) return;

  auto nhit = hc->GetSize();
  G4cout << "Nhit: " << nhit << G4endl;

  for ( unsigned long i = 0; i < nhit; i++ ) {
    auto hit = static_cast<ScintbarHit*>(hc->GetHit(i));
    auto pos = hit->GetPos();
    myrootmanager->ROOTTreeStruct.ScintHitE[myrootmanager->ROOTTreeStruct.NScintHit] = (Float_t)hit->GetEdep();
    myrootmanager->ROOTTreeStruct.ScintHitPrimaryID[myrootmanager->ROOTTreeStruct.NScintHit] = (Int_t)mytracking->GetPrimary(hit->GetTrackID());
    if ( myrootmanager->ROOTTreeStruct.ScintHitPrimaryID[myrootmanager->ROOTTreeStruct.NScintHit] == 0 )
      G4cout << "Hooooold your horses !" << G4endl;
    myrootmanager->ROOTTreeStruct.ScintHitPosX[myrootmanager->ROOTTreeStruct.NScintHit] = (Float_t)pos.x();
    myrootmanager->ROOTTreeStruct.ScintHitPosY[myrootmanager->ROOTTreeStruct.NScintHit] = (Float_t)pos.y();
    myrootmanager->ROOTTreeStruct.ScintHitPosZ[myrootmanager->ROOTTreeStruct.NScintHit] = (Float_t)pos.z();
    myrootmanager->ROOTTreeStruct.ScintHitStation[myrootmanager->ROOTTreeStruct.NScintHit] = (Int_t)hit->GetStationID();
    myrootmanager->ROOTTreeStruct.ScintHitModule[myrootmanager->ROOTTreeStruct.NScintHit] = (Int_t)hit->GetModuleID();
    myrootmanager->ROOTTreeStruct.ScintHitBar[myrootmanager->ROOTTreeStruct.NScintHit] = (Int_t)hit->GetBarID();
    
    hit->Print();
    G4cout << "primary ID = "
	   << myrootmanager->ROOTTreeStruct.ScintHitPrimaryID[myrootmanager->ROOTTreeStruct.NScintHit]
	   << G4endl;
    
    myrootmanager->ROOTTreeStruct.NScintHit++;
    if ( myrootmanager->ROOTTreeStruct.NScintHit >= 500) {
      G4cout << "*********** Too many primary scint hits !!!!" << G4endl;
      return;
    }
  }

  //
  // Print diagnostics -- need to include this stuff in the ntuple somehow
  // 
  
  auto printModulo = G4RunManager::GetRunManager()->GetPrintProgress();
  if ( printModulo == 0 || event->GetEventID() % printModulo != 0) return;

  for ( int i = 0; i < event->GetNumberOfPrimaryVertex(); i++ ) {
    
    G4cout << "#primary vertex: " << i << G4endl;

    for ( int q = 0; q < event->GetPrimaryVertex(i)->GetNumberOfParticle(); q++ ) {

      auto primary = event->GetPrimaryVertex(i)->GetPrimary(q);
      G4cout << "TrackID: " << primary->GetTrackID() 
	     << ", Particle type: " << primary->GetG4code()->GetParticleName()
	     << ", PDG encoding: " << primary->GetG4code()->GetPDGEncoding()
	     << ", Momentum " << primary->GetMomentum()
	     << G4endl;

      //if ( abs(primary->GetG4code()->GetPDGEncoding()) == 13 ) {
      myrootmanager->ROOTTreeStruct.GenPartID[myrootmanager->ROOTTreeStruct.NGenPart] = (Int_t)primary->GetTrackID();
      myrootmanager->ROOTTreeStruct.GenPartPDG[myrootmanager->ROOTTreeStruct.NGenPart] =
	(Int_t)primary->GetG4code()->GetPDGEncoding();
      myrootmanager->ROOTTreeStruct.GenPartE[myrootmanager->ROOTTreeStruct.NGenPart] = (Float_t)primary->GetTotalEnergy();
	//myroottree.GenMuonTheta[myroottree.NGenPart] = primary->GetTheta();
	//myroottree.GenMuonPhi[myroottree.NGenPart] = primary->GetPhi();
      myrootmanager->ROOTTreeStruct.NGenPart++;
	//}
      if ( myrootmanager->ROOTTreeStruct.NGenPart >= 500) {
	G4cout << "*********** Too many primary particles !!!!" << G4endl;
	return;
      }
    }
  }
  // G4cout << " Before filling" << G4endl;
  myrootmanager->Fill();
  
  //analysisManager->AddNtupleRow();  
  
}

