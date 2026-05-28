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
#include "G4SystemOfUnits.hh"

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
  auto track = step->GetTrack();
  auto prestep  = step->GetPreStepPoint();
  auto poststep = step->GetPostStepPoint();
  auto touchable = prestep->GetTouchable();

  G4double edep    = step->GetTotalEnergyDeposit();
  G4double stepLen = step->GetStepLength();

  if (edep == 0. && stepLen == 0.) return false;

  // StationNo X0: 0; Y0: 8;
  //           X1: 1; Y1: 9;
  //           X2: 2; Y2: 10;
  //           X3: 3; Y3: 11;
  G4int BarCopyNo  = touchable->GetVolume()->GetCopyNo();
  G4int BarNo      = (BarCopyNo & 0xFF);
  G4int ModuleNo   = (BarCopyNo >> 8)  & 0xF;
  G4int StationNo  = (BarCopyNo >> 12) & 0xF;
  G4int trackID    = track->GetTrackID();

  // Search for existing hit for this track-bar-combintation in this event
  ScintbarHit* hit = nullptr;
  for (size_t i = 0; i < fHitsCollection->entries(); i++) {
    auto* h = (*fHitsCollection)[i];
    if (h->GetTrackID()   == trackID  &&
        h->GetStationID() == StationNo &&
        h->GetModuleID()  == ModuleNo  &&
        h->GetBarID()     == BarNo) {
      hit = h;
      break;
    }
  }

  if (!hit) { // if particle has no entry yet for that certain bar-module-station combination
    if (edep == 0.) {
      // No energy deposited yet, cache geometric entry point but don't create hit (probably the new hit could be created already, I just try to be memory efficient)
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

    auto key = std::make_pair(trackID, BarCopyNo);
    auto it  = fEntryPointMap.find(key);
    if (it != fEntryPointMap.end()) {
      hit->SetEntryPoint(it->second);
      fEntryPointMap.erase(it);
    } else {
      hit->SetEntryPoint(prestep->GetPosition());
    }

    fHitsCollection->insert(hit);
  }

  hit->AddEdep(edep);

  if (stepLen > 0.) { // maybe this check unnecessary
    hit->AddPathLength(stepLen);
    hit->SetExitPoint(poststep->GetPosition());
  }
  return true;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ScintbarSD::EndOfEvent( G4HCofThisEvent *hitCollection ) {}

