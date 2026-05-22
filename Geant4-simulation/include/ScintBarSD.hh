/// \file ScintBarSD.hh
/// \brief Definition of the ScintBarSD class

#ifndef ScintbarSD_h
#define ScintbarSD_h 1

#include "G4VSensitiveDetector.hh"

#include "ScintBarHit.hh"

#include "globals.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

/// Hodoscope sensitive detector

class ScintbarSD : public G4VSensitiveDetector
{
  public:
    ScintbarSD(G4String name);
    virtual ~ScintbarSD();

    virtual void Initialize( G4HCofThisEvent *hitcollection ) override;
    virtual G4bool ProcessHits( G4Step *step, G4TouchableHistory *history ) override;
    virtual void EndOfEvent( G4HCofThisEvent* hitCollection ) override;

  private:
    ScintbarHitsCollection* fHitsCollection = nullptr;
    G4int fHCID = -1;
    G4int fHitCount = 0;
    std::map<std::pair<G4int, G4int>, G4ThreeVector> fEntryPointMap; 
};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
