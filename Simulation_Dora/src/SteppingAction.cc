/// \file SteppingAction.cc
/// \brief Implementation of the SteppingAction class

#include "SteppingAction.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
//#include "G4cout.hh"

MySteppingAction::MySteppingAction(EventAction* eventAction) : fEventAction(eventAction) {
    std::cout << ">>> MySteppingAction registered!" << std::endl;
}

MySteppingAction::~MySteppingAction() {}

void MySteppingAction::UserSteppingAction(const G4Step* step) {
    const G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

    if (eventID != fLastEventID) {
        fSteps = 0;
        fLastEventID = eventID;
    }

    /*G4String volumeName = step->GetPreStepPoint()
                              ->GetTouchableHandle()
                              ->GetVolume()
                              ->GetName();
    
    if (volumeName == "physGround")  
    {
        fEventAction->SetTouchedRock();
    }*/

    if (++fSteps % 300'000 == 0) {
        G4cout << "Event " << eventID
                  << " | steps: "    << fSteps
                  << " | particle: " << step->GetTrack()->GetDefinition()->GetParticleName()
                  << " | process: "  << step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName()
                  << " | KE: "       << step->GetPreStepPoint()->GetKineticEnergy()
                  << G4endl;
    }

    if (fSteps > 2'000'000) {
        G4cout << "WARNING: Aborting runaway event " << eventID
                  << " | steps: "    << fSteps
                  << " | particle: " << step->GetTrack()->GetDefinition()->GetParticleName()
                  << " | process: "  << step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName()
                  << G4endl;
        G4RunManager::GetRunManager()->AbortEvent();
        fSteps = 0;
    }
}