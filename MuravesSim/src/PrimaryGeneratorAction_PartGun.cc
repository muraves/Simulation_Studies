//******************************************************************************
// PrimaryGeneratorAction_PartGun.cc
//
// Michael Tytgat
//******************************************************************************
//

#include "PrimaryGeneratorAction_PartGun.hh"

#include "G4ParticleGun.hh"
#include "G4Event.hh"

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_PartGun::PrimaryGeneratorAction_PartGun()
{
  // define a particle gun
  particleGun = new G4ParticleGun();

  // Create the table containing all particle names
  //particleTable = G4ParticleTable::GetParticleTable();
}

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_PartGun::~PrimaryGeneratorAction_PartGun()
{
  if (particleGun) delete particleGun;
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_PartGun::GeneratePrimaries(G4Event* anEvent)
{ 
  particleGun->GeneratePrimaryVertex(anEvent);
}
