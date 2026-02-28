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
#include "PrimaryGeneratorInfo.hh"

class G4Event;
class G4ParticleGun;

class PrimaryGeneratorAction_PG : public G4VUserPrimaryGeneratorAction, public PrimaryGeneratorInfo
{
public:
  PrimaryGeneratorAction_PG();
  ~PrimaryGeneratorAction_PG();
  void GeneratePrimaries(G4Event* anEvent) override;

  virtual std::string GetGeneratorName() const override {return "PG";}
  virtual std::string GetInfoSummary() const override;

private:
  G4ParticleTable* particleTable;
  G4ParticleGun* particleGun;

};

#endif