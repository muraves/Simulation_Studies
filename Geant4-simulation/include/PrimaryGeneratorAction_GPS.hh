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
#include "vector"
#include "PrimaryGeneratorInfo.hh"

class G4Event;
//class G4ParticleGun;
class G4GeneralParticleSource;

class PrimaryGeneratorAction_GPS : public G4VUserPrimaryGeneratorAction, public PrimaryGeneratorInfo
{
public:
  PrimaryGeneratorAction_GPS();
  ~PrimaryGeneratorAction_GPS();
  void GeneratePrimaries(G4Event* anEvent) override;

  virtual std::string GetGeneratorName() const override {return "GPS";}
  virtual std::string GetInfoSummary() const override;

private:
  G4ParticleTable* particleTable;
  G4GeneralParticleSource* particleGun;

};

#endif