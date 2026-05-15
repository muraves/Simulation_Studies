//******************************************************************************
// PrimaryGeneratorAction_GenPartSrc.cc
//
// Michael Tytgat
//******************************************************************************
//

#include "PrimaryGeneratorAction_GPS.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_GPS::PrimaryGeneratorAction_GPS()
{
  // define a particle gun
  particleGun = new G4GeneralParticleSource();

  // Create the table containing all particle names
  particleTable = G4ParticleTable::GetParticleTable();

  // Create the messenger file
  //gunMessenger = new PrimaryGeneratorMessenger(this);
}

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_GPS::~PrimaryGeneratorAction_GPS()
{
  delete particleGun;
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_GPS::GeneratePrimaries(G4Event* anEvent)
{ 
  particleGun->GeneratePrimaryVertex(anEvent);
}

std::string PrimaryGeneratorAction_GPS::GetInfoSummary() const {
    std::ostringstream oss;
    oss << "Particle: " << particleGun->GetParticleDefinition()->GetParticleName()
        << ", Energy: " << particleGun->GetParticleEnergy()/CLHEP::MeV << " MeV"
        << ", Direction: " << particleGun->GetParticleMomentumDirection();
    return oss.str();
}