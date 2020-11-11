//******************************************************************************
// PrimaryGeneratorAction_GenPartSrc.hh
//
// Michael Tytgat
//******************************************************************************
// 
#ifndef PrimaryGeneratorAction_GenPartSrc_h
#define PrimaryGeneratorAction_GenPartSrc_h 1

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

class G4Event;
//class G4ParticleGun;
class G4GeneralParticleSource;

class PrimaryGeneratorAction_GenPartSrc : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction_GenPartSrc();
  ~PrimaryGeneratorAction_GenPartSrc();
  void GeneratePrimaries(G4Event* anEvent);

private:
  G4ParticleTable* particleTable;
  G4GeneralParticleSource* particleGun;

};

#endif
