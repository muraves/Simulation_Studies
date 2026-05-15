/// \file EventAction.hh
/// \brief Definition of the EventAction class

#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"

#include "globals.hh"
//#include "SteppingAction.hh"

#include <array>
#include <vector>

class G4Event;

/// Event action

class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    ~EventAction() override = default;

    virtual void BeginOfEventAction(const G4Event* event) override;
    virtual void EndOfEventAction(const G4Event*) override;

    void SetAbortDiagnostics(G4int steps, 
                         const G4String& particle, 
                         const G4String& process,
                         const G4String& volume)
    { 
        fAbortSteps    = steps; 
        fAbortParticle = particle;
        fAbortProcess  = process;
        fAbortVolume   = volume;
    }

    G4int    GetAbortSteps()    const { return fAbortSteps;    }
    G4String GetAbortParticle() const { return fAbortParticle; }
    G4String GetAbortProcess()  const { return fAbortProcess;  }
    G4String GetAbortVolume()   const { return fAbortVolume;   }

    //void SetTouchedRock() { fTouchedRock = true; }
    //G4bool GetTouchedRock() const { return fTouchedRock; }

  private:
    // hit collections Ids
    G4int fScintbarsHCID;
    G4int    fAbortSteps       = 0;
    G4String fAbortParticle    = "";
    G4String fAbortProcess     = "";
    G4String fAbortVolume      = "";
    //G4bool fTouchedRock = false;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
