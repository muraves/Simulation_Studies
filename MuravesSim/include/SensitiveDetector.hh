#ifndef SensitiveDetector_h
#define SensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include "Hits.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class ScintbarSD : public G4VSensitiveDetector
{
public:
  ScintbarSD( const G4String &SDname );
  virtual ~ScintbarSD();

  virtual void Initialize( G4HCofThisEvent *hitcollection );
  virtual G4bool ProcessHits( G4Step *step, G4TouchableHistory *history );
  virtual void EndOfEvent( G4HCofThisEvent* hitCollection );
  
private:
  ScintbarHitsCollection* fHitsCollection;
  G4int fHCID;
};

#endif
