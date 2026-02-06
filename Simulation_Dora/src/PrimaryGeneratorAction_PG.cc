//******************************************************************************
// PrimaryGeneratorAction_PartGun.cc
//
// Michael Tytgat
//******************************************************************************
//

#include "PrimaryGeneratorAction_PG.hh"

#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include <G4MuonMinus.hh>

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_PG::PrimaryGeneratorAction_PG()
{
  // define a particle gun
  particleGun = new G4ParticleGun();

  // Create the table containing all particle names
  //particleTable = G4ParticleTable::GetParticleTable();
  particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName = "mu-";
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
  particleGun->SetParticleDefinition(particle);  
}

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_PG::~PrimaryGeneratorAction_PG()
{
  if (particleGun) delete particleGun;
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_PG::GeneratePrimaries(G4Event* anEvent)
{ 
  G4double x = 0; 
  G4double z = -1 * m;
  G4double y = 0; 

  G4ThreeVector pos(x, y, z); 

  G4double px = 0; 
  G4double py = 0; 
  G4double pz = 1.;

  G4ThreeVector mom(px, py, pz); 

  particleGun->SetParticlePosition(pos);
  particleGun->SetParticleMomentumDirection(mom.unit());
  particleGun->SetParticleMomentum(4. * GeV);    

  particleGun->GeneratePrimaryVertex(anEvent);
}