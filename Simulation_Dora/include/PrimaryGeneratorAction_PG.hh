//******************************************************************************
// PrimaryGeneratorAction_PartGun.hh
//
// Michael Tytgat
//******************************************************************************
// 
#ifndef PrimaryGeneratorAction_PartGun_h
#define PrimaryGeneratorAction_PartGun_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "G4DataVector.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"
#include "globals.hh"
#include "vector"

class G4Event;
class G4ParticleGun;

class PrimaryGeneratorAction_PG : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction_PG();
  ~PrimaryGeneratorAction_PG();
  void GeneratePrimaries(G4Event* anEvent);

private:
  G4ParticleTable* particleTable;
  G4ParticleGun* particleGun;

};

#endif