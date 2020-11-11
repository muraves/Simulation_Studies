#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"
#include <map>

class TrackingAction : public G4UserTrackingAction
{
  
public:
  TrackingAction() { fgInstance = this; };
  virtual ~TrackingAction() { fgInstance = 0; };

  static TrackingAction* Instance() { return fgInstance; };
  
  virtual void PreUserTrackingAction( const G4Track* );
  virtual void PostUserTrackingAction( const G4Track* );

  void AddParent( G4int, G4int );
  G4int GetParent( G4int );
  G4int GetPrimary( G4int );
  void ResetParents() { fParentIDs.clear(); }
  
private:
  static TrackingAction* fgInstance;
  std::map<G4int, G4int> fParentIDs;

};

#endif
