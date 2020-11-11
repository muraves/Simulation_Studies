//
// Michael Tytgat
//

#include "Hits.hh"
#include "DetectorConstruction.hh"

#include "G4AttDefStore.hh"
#include "G4AttDef.hh"
#include "G4AttValue.hh"
#include "G4UIcommand.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<ScintbarHit>* ScintbarHitAllocator;

ScintbarHit::ScintbarHit()
  : G4VHit(),
    fStationID(-1), fModuleID(-1), fBarID(-1)
{}


ScintbarHit::ScintbarHit( G4int iStationID, G4int iModuleID, G4int iBarID )
  : G4VHit(),
    fStationID(iStationID), fModuleID(iModuleID), fBarID(iBarID)
{}

ScintbarHit::~ScintbarHit()
{}

ScintbarHit::ScintbarHit( const ScintbarHit &right )
  : G4VHit(),
    fStationID(right.fStationID), fModuleID(right.fModuleID), fBarID(right.fBarID)
{}

const ScintbarHit& ScintbarHit::operator=( const ScintbarHit &right )
{
  fStationID = right.fStationID;
  fModuleID = right.fModuleID;
  fBarID = right.fBarID;
  return *this;
}

G4bool ScintbarHit::operator==( const ScintbarHit &right ) const
{
  return (fStationID==right.fStationID && fModuleID==right.fModuleID && fBarID==right.fBarID);
}

void ScintbarHit::Draw()
{}

void ScintbarHit::Print()
{
  G4cout << "Station: " << fStationID
	 << ", Module: " << fModuleID
	 << ", BarID: " << fBarID
	 << ", Edep: " << fEdep/MeV << " (MeV), "
	 << ", pos: " << fPos
	 << ", Track ID: " << fTrackID
	 << G4endl;
}

const std::map<G4String,G4AttDef>* ScintbarHit::GetAttDefs() const
{
  G4bool isNew;
  auto store = G4AttDefStore::GetInstance("ScintbarHit", isNew);

  if ( isNew ) {
    
    (*store)["HitType"] = G4AttDef("HitType", "Hit Type", "Physics", "", "G4String" );
    (*store)["Station"] = G4AttDef("Station", "Station ID", "Physics", "", "G4int" );
    (*store)["Module"] = G4AttDef("Module", "Module ID", "Physics", "", "G4int" );
    (*store)["Bar"] = G4AttDef("Bar", "Bar ID", "Physics", "", "G4int" );
    (*store)["Energy"] = G4AttDef("Energy", "Energy deposit", "Physics", "G4BestUnit", "G4double" );
    (*store)["Pos"] = G4AttDef("Pos", "Position", "Physics", "G4BestUnit", "G4ThreeVector" );
    (*store)["TrackID"] = G4AttDef("TrackID", "Track ID", "Physics", "", "G4int" );
    
  }
  
  return store;
}

std::vector<G4AttValue>* ScintbarHit::CreateAttValues() const
{
  auto values = new std::vector<G4AttValue>;
  
  values->push_back( G4AttValue( "HitType", "ScintbarHit", "" ));
  values->push_back( G4AttValue( "Station", G4UIcommand::ConvertToString(fStationID),""));
  values->push_back( G4AttValue( "Module", G4UIcommand::ConvertToString(fModuleID),""));
  values->push_back( G4AttValue( "Bar", G4UIcommand::ConvertToString(fBarID),""));
  values->push_back( G4AttValue( "Energy", G4BestUnit(fEdep,"Energy"),""));
  values->push_back( G4AttValue( "Pos", G4BestUnit(fPos,"Length"),""));
  values->push_back( G4AttValue( "TrackID", G4UIcommand::ConvertToString(fTrackID),""));
  
  return values;
}




