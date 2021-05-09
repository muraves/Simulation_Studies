//
// Michael Tytgat
//

#include "SensitiveDetector.hh"

#include <vector>
#include <sstream>

#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

ScintbarSD::ScintbarSD( const G4String &SDname )
  : G4VSensitiveDetector(SDname),
    fHitsCollection(0), fHCID(-1)
{
  collectionName.insert("ScintbarsColl");
}

ScintbarSD::~ScintbarSD() {}

void ScintbarSD::Initialize( G4HCofThisEvent *hitcollection )
{
  fHitsCollection = new ScintbarHitsCollection(SensitiveDetectorName, collectionName[0]);
  if ( fHCID < 0 ) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
  }
  hitcollection->AddHitsCollection( fHCID, fHitsCollection);
  //G4cout << "Initialize ScintbarSD hitcoll ID: " << fHCID << G4endl; 
}

G4bool ScintbarSD::ProcessHits( G4Step *step, G4TouchableHistory* history )
{

  // geometry info from PreStepPoint
  auto prestep = step->GetPreStepPoint();
  auto touchable = step->GetPreStepPoint()->GetTouchable();
  auto track = step->GetTrack();
    
  G4cout << "Hit Phys.Vol: " << prestep->GetPhysicalVolume()->GetName() << G4endl;
  G4cout << "particle: " << track->GetDefinition()->GetPDGEncoding() << G4endl;
  G4cout << "parent: " << track->GetParentID() << G4endl;
  
  G4ThreeVector pos = prestep->GetPosition();
  
  G4double edep = step->GetTotalEnergyDeposit();
  if ( edep == 0. ) return true;

  // StationNo X0: 0; Y0: 8;
  //           X1: 1; Y1: 9;
  //           X2: 2; Y2: 10;
  //           X3: 3; Y3: 11;
  G4int BarCopyNo = touchable->GetVolume()->GetCopyNo();
  G4int BarNo = (BarCopyNo & 0xFF);
  G4int ModuleNo = (BarCopyNo >> 8) & 0xF; 
  G4int StationNo = (BarCopyNo >> 12) & 0xF;

  //auto StationNo = touchable->GetVolume(1)->GetCopyNo();
  //auto ModuleNo = touchable->GetCopyNumber(3);
  //auto hitID = kNofHadRows*columnNo+rowNo;
  //auto hit = (*fHitsCollection)[hitID];
  
  /*
  // check if it is first touch
  if ( hit->GetStationID() < 0 ) {
    hit->SetColumnID(columnNo);
    hit->SetRowID(rowNo);
    auto depth = touchable->GetHistory()->GetDepth();
    auto transform = touchable->GetHistory()->GetTransform(depth-2);
    transform.Invert();
    hit->SetRot(transform.NetRotation());
    hit->SetPos(transform.NetTranslation());
  }
  // add energy deposition
  hit->AddEdep(edep);
  */

  auto hit = new ScintbarHit();
  hit->SetEdep(edep);
  hit->SetPos(pos);
  hit->SetBarID(BarNo);
  hit->SetModuleID(ModuleNo);
  hit->SetStationID(StationNo);
  hit->SetTrackID(track->GetTrackID());
  hit->SetPDGcode(track->GetDefinition()->GetPDGEncoding());
  
  G4cout << "BarCopyNo: " << BarCopyNo << G4endl;
  hit->Print();
  
  fHitsCollection->insert(hit);

  return true;
}

void ScintbarSD::EndOfEvent( G4HCofThisEvent *hitCollection ) {}

