/// \file EventAction.hh
/// \brief Definition of the EventAction class

#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"

#include "globals.hh"

#include <array>
#include <vector>

class G4Event;

/// Event action

class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    ~EventAction() override = default;

    virtual void BeginOfEventAction(const G4Event*) override;
    virtual void EndOfEventAction(const G4Event*) override;

  private:
    // hit collections Ids
    G4int fScintbarsHCID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
