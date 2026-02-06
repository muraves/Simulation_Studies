/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "ActionInitialization.hh"

#include "PrimaryGeneratorAction_PG.hh"
#include "PrimaryGeneratorAction_GPS.hh"
#include "PrimaryGeneratorAction_CRY.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "MuravesMessenger.hh"

ActionInitialization::ActionInitialization()
 : G4VUserActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  auto eventAction = new EventAction;
  SetUserAction(new RunAction(eventAction));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  auto generator = MuravesMessenger::Instance()->GetPrimaryGenerator();
  if ( generator == "PG" )
    SetUserAction(new PrimaryGeneratorAction_PG());
  else if ( generator == "GPS" )
    SetUserAction(new PrimaryGeneratorAction_GPS());
  else if ( generator == "CRY" )
     SetUserAction(new PrimaryGeneratorAction_CRY("cmd-dora.file"));

  auto theEventAction = new EventAction;
  SetUserAction(theEventAction);

  SetUserAction(new RunAction(theEventAction));

  SetUserAction(new TrackingAction());
}


