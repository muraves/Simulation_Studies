//******************************************************************************
// PrimaryGeneratorAction_CRY.hh
//
// This class is a class derived from G4VUserPrimaryGeneratorAction for 
// constructing the process used to generate incident particles.
//
// 1.00 JMV, LLNL, JAN-2007:  First version.
//******************************************************************************
// 
#ifndef PrimaryGeneratorAction_CRY_h
#define PrimaryGeneratorAction_CRY_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "G4DataVector.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"
#include "globals.hh"
#include "CRYSetup.h"
#include "CRYGenerator.h"
#include "CRYParticle.h"
#include "CRYUtils.h"
#include "vector"
#include "RNGWrapper.hh"
#include "PrimaryGeneratorMessenger.hh"

class G4Event;
class G4ParticleGun;

class PrimaryGeneratorAction_CRY : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction_CRY(const char * filename);
  ~PrimaryGeneratorAction_CRY();
  void GeneratePrimaries(G4Event* anEvent);

  void InputCRY();
  void UpdateCRY(std::string* MessInput);
  void CRYFromFile(G4String newValue);

private:
  std::vector<CRYParticle*> *vect; // vector of generated particles
  G4ParticleTable* particleTable;
  G4ParticleGun* particleGun;
  CRYGenerator* gen;
  G4int InputState;
  PrimaryGeneratorMessenger* gunMessenger;

};

#endif
