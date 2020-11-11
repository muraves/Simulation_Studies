//******************************************************************************
// PrimaryGeneratorAction_GenPartSrc.cc
//
// Michael Tytgat
//******************************************************************************
//

#include "PrimaryGeneratorAction_GenPartSrc.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_GenPartSrc::PrimaryGeneratorAction_GenPartSrc()
{
  // define a particle gun
  particleGun = new G4GeneralParticleSource();

  // Create the table containing all particle names
  particleTable = G4ParticleTable::GetParticleTable();

  // Create the messenger file
  //gunMessenger = new PrimaryGeneratorMessenger(this);
}

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_GenPartSrc::~PrimaryGeneratorAction_GenPartSrc()
{
  delete particleGun;
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_GenPartSrc::GeneratePrimaries(G4Event* anEvent)
{ 
  particleGun->GeneratePrimaryVertex(anEvent);
}
