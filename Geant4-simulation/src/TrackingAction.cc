#include "TrackingAction.hh"

#include "G4Track.hh"
#include "G4RunManager.hh"

//TrackingAction* TrackingAction::fgInstance = 0;
G4ThreadLocal TrackingAction* TrackingAction::fgInstance = nullptr;

void TrackingAction::PreUserTrackingAction( const G4Track* track )
{
  //G4cout << "UserTracking: trackID = " << track->GetTrackID()
  //	 << " PDG = " << track->GetDefinition()->GetPDGEncoding()
  //	 << " Parent = " << track->GetParentID() << G4endl;
  /*if (track->GetTrackID() != 1 || track->GetParentID() != 0) {
    // Stop tracking this particle
    G4Track* nonConstTrack = const_cast<G4Track*>(track);
    nonConstTrack->SetTrackStatus(fStopAndKill);
}*/
  AddParent( track->GetTrackID(), track->GetParentID() );

  
}

void TrackingAction::PostUserTrackingAction( const G4Track* track ) {}


void TrackingAction::AddParent( G4int trackID, G4int parentID )
{
  // check if parentID is not known yet for this track
  /*for ( auto it = fParentIDs.begin(); it != fParentIDs.end(); ++it ) {
    if ( it->first == trackID ) {
      if ( it->second != parentID ) G4cout << "Houston, we do have a problem" << G4endl;
      return;}
    }*/

    // check if parentID is not known yet for this track (no loop)
    auto result = fParentIDs.insert({trackID, parentID});
    if (!result.second && result.first->second != parentID) {
        G4cout << "Houston, we do have a problem" << G4endl;
    }
  //fParentIDs.insert(std::pair<int,int>(trackID, parentID));
}
    
G4int TrackingAction::GetParent( G4int trackID )
{
  if ( fParentIDs.find(trackID) != fParentIDs.end() )
    return fParentIDs.at(trackID);
  else {
    G4cout << "Didn't find parent of track ID " << trackID << G4endl; 
    return 0;
  }
}

G4int TrackingAction::GetPrimary( G4int trackID )
{
  if ( GetParent(trackID) == 0 )
    return trackID;
  else
    return GetPrimary( GetParent(trackID) );
}
 