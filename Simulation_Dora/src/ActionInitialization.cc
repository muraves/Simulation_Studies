/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "ActionInitialization.hh"

#include "PrimaryGeneratorAction_PG.hh"
#include "PrimaryGeneratorAction_GPS.hh"
#include "PrimaryGeneratorAction_CRY.hh"
#include "PrimaryGeneratorAction_EcoMug.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "MuravesMessenger.hh"
#include "PrimaryGeneratorInfo.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(long seed, const std::string& ecomugFile) : G4VUserActionInitialization(), fSeed(seed), fEcoMugFile(ecomugFile)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  //auto theEventAction = new EventAction();         
  //SetUserAction(theEventAction);
  //SetUserAction(new MySteppingAction(theEventAction));
  PrimaryGeneratorInfo* fGeneratorInfo = nullptr;
  SetUserAction(new RunAction(nullptr, fGeneratorInfo, fSeed));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  PrimaryGeneratorInfo* fGeneratorInfo = nullptr;
  G4VUserPrimaryGeneratorAction* generatorAction = nullptr;

  auto generator = MuravesMessenger::Instance()->GetPrimaryGenerator();
  if ( generator == "PG" )
    //SetUserAction(new PrimaryGeneratorAction_PG());
    fGeneratorInfo = new PrimaryGeneratorAction_PG();
  else if ( generator == "GPS" )
    //SetUserAction(new PrimaryGeneratorAction_GPS());
    fGeneratorInfo = new PrimaryGeneratorAction_GPS();
  //else if ( generator == "CRY" )
     //SetUserAction(new PrimaryGeneratorAction_CRY("cmd-dora.file"));
     //fGeneratorInfo = new PrimaryGeneratorAction_CRY();
  else if ( generator == "EcoMug" )
     //SetUserAction(new PrimaryGeneratorAction_EcoMug());
     fGeneratorInfo = new PrimaryGeneratorAction_EcoMug(fEcoMugFile);
  
  generatorAction = dynamic_cast<G4VUserPrimaryGeneratorAction*>(fGeneratorInfo);
  SetUserAction(generatorAction);

  auto theEventAction = new EventAction();         
  SetUserAction(theEventAction);
  SetUserAction(new MySteppingAction(theEventAction));

  SetUserAction(new RunAction(theEventAction, fGeneratorInfo, fSeed));

  SetUserAction(new TrackingAction());

  //SetUserAction(theSteppingAction);
}


