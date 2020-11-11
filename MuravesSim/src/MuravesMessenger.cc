#include "MuravesMessenger.hh"

#include "G4GenericMessenger.hh"

#include <string>

MuravesMessenger* MuravesMessenger::fgInstance = 0;

MuravesMessenger::MuravesMessenger()
  :primarygenerator("CRY")
{
  fgInstance = this;

  /*
    fMessenger = new G4GenericMessenger(this, std::string("/Muraves/"));
  
    G4GenericMessenger::Command& primarygen = 
    fMessenger->DeclareProperty("PrimaryGenerator", primarygenerator,
    "Primary Generator: CRY, PartGun, CORSIKA");
    primarygen.SetDefaultValue("CRY");
  */
}

MuravesMessenger::~MuravesMessenger()
{
  fgInstance = 0;  
  //if (fMessenger) delete fMessenger;
}
