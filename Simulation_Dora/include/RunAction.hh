/// \file RunAction.hh
/// \brief Definition of the RunAction class

#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"

class G4Run;

class EventAction;

class RunAction : public G4UserRunAction
{
  public:
    RunAction(EventAction* eventAction);
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

  private:
    EventAction* fEventAction = nullptr;
};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
