/// \file ScintBarSD.cc
/// \brief Implementation of the ScintBarSD class

#include "ScintBarSD.hh"

#include "ScintBarHit.hh"

#include "G4AffineTransform.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ScintbarSD::ScintbarSD(G4String name) : G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1)
{
  collectionName.insert("ScintbarsColl");
}

ScintbarSD::~ScintbarSD() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ScintbarSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new ScintbarHitsCollection(SensitiveDetectorName, collectionName[0]);
  if (fHCID < 0) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
  }
  hce->AddHitsCollection(fHCID, fHitsCollection);
  fHitCount = 0;
  fEntryPointMap.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool ScintbarSD::ProcessHits(G4Step* step, G4TouchableHistory* history)
{
  fHitCount++;
  if (fHitCount % 1000 == 0) {
  G4cout << "Hit count :" << fHitCount << G4endl;}
  /*if (fHitCount > 10000) {  // tune this threshold
        std::cout << "WARNING: Aborting runaway event, hit count exceeded: " 
                  << fHitCount << std::endl;
        G4RunManager::GetRunManager()->AbortEvent();
        fHitCount = 0;
        return false;
    }*/
  // geometry info from PreStepPoint
  auto prestep = step->GetPreStepPoint();
  auto poststep = step->GetPostStepPoint(); 
  auto touchable = step->GetPreStepPoint()->GetTouchable();
  auto track = step->GetTrack();
  
  G4ThreeVector pos = prestep->GetPosition();
  /*G4ThreeVector pos = prestep->GetTouchableHandle()
                            ->GetHistory()
                            ->GetTopTransform()
                            .TransformPoint(prestep->GetPosition());*/
  
  G4double edep = step->GetTotalEnergyDeposit();
  G4double stepLen = step->GetStepLength();

  //if ( edep == 0. ) return false;

  // StationNo X0: 0; Y0: 8;
  //           X1: 1; Y1: 9;
  //           X2: 2; Y2: 10;
  //           X3: 3; Y3: 11;
  G4int BarCopyNo = touchable->GetVolume()->GetCopyNo();
  G4int BarNo = (BarCopyNo & 0xFF);
  G4int ModuleNo = (BarCopyNo >> 8) & 0xF; 
  G4int StationNo = (BarCopyNo >> 12) & 0xF;
  G4double HitTime = prestep->GetGlobalTime();

  //auto StationNo = touchable->GetVolume(1)->GetCopyNo();
  //auto ModuleNo = touchable->GetCopyNumber(3);
  //auto hitID = kNofHadRows*columnNo+rowNo;
  //auto hit = (*fHitsCollection)[hitID];

  G4int trackID = track->GetTrackID();
  ScintbarHit* hit = nullptr;

  // Search for existing hit
for (size_t i = 0; i < fHitsCollection->entries(); i++) {
  auto existingHit = (*fHitsCollection)[i];

  if ( existingHit->GetTrackID() == trackID &&
       existingHit->GetStationID() == StationNo &&
       existingHit->GetModuleID()  == ModuleNo &&
       existingHit->GetBarID()  == BarNo) {
    hit = existingHit;
    //G4cout << "hit exists" << G4endl;
    break;
  }
}

if (!hit) { // Particle has no entry yet for that certain bar-module-station combination yet
    if (edep == 0.) { // If no energy is deposited yet in this bar, just save entry point for later but don't create new hit/entry
      fEntryPointMap[{trackID, BarCopyNo}] = prestep->GetPosition();
      return false;
    }

    hit = new ScintbarHit();
    hit->SetBarID(BarNo);
    hit->SetModuleID(ModuleNo);
    hit->SetStationID(StationNo);
    hit->SetTrackID(trackID);
    hit->SetPDGcode(track->GetDefinition()->GetPDGEncoding());
    hit->SetParentId(track->GetParentID());
    hit->SetHitTime(prestep->GetGlobalTime());
    //hit->SetPos(prestep->GetPosition());

    auto key = std::make_pair(trackID, BarCopyNo);
    if (fEntryPointMap.count(key)) {
      hit->SetEntryPoint(fEntryPointMap[key]);  // true geometric entry
      fEntryPointMap.erase(key);                // clean up
    } else {
      hit->SetEntryPoint(prestep->GetPosition()); 
    }
    
    fHitsCollection->insert(hit);
  }

hit->AddEdep(edep);
hit->AddPathLength(stepLen);
// Keep overwriting exit point 
hit->SetExitPoint(poststep->GetPosition());

// Have to check if this condition works
/*if (poststep->GetStepStatus() == fGeomBoundary ||
    track->GetTrackStatus()   != fAlive) {
  hit->SetExitPoint(poststep->GetPosition());
}*/

  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ScintbarSD::EndOfEvent( G4HCofThisEvent *hitCollection ) {}

