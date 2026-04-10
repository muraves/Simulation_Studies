/// \file PrimaryGeneratorAction_EcoMug.hh
/// \brief Definition of the PrimaryGeneratorAction_EcoMug class

#ifndef PrimaryGeneratorAction_EcoMug_h
#define PrimaryGeneratorAction_EcoMug_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include "EcoMug.h"
#include "MuravesMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "PrimaryGeneratorInfo.hh"

class G4ParticleGun;
class G4Event;
class G4Box;

/// The primary generator action class with particle gun.
///
/// The default kinematic is a 6 MeV gamma, randomly distribued 
/// in front of the phantom across 80% of the (X,Y) phantom size.

class PrimaryGeneratorAction_EcoMug : public G4VUserPrimaryGeneratorAction, public PrimaryGeneratorInfo
{
  public:
    PrimaryGeneratorAction_EcoMug(const std::string& inputFile = "");    
    virtual ~PrimaryGeneratorAction_EcoMug();

    // method from the base class
    virtual void GeneratePrimaries(G4Event*) override;         
  
    // method to access particle gun
    const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

    G4double fHSphereRadius;
    std::array<G4double, 3> fHSphereCenter; // use std::array, EcoMug cannot take G4ThreeVector as input
    G4double fMinTheta, fMaxTheta;
    G4double fMinPosTheta, fMaxPosTheta;
    G4double fMinPhi, fMaxPhi;
    G4double fMinPosPhi, fMaxPosPhi;
    G4double fHorizontalRate;
    mutable G4double rateHSphere, errorHSphere;
    mutable G4double genSurfaceArea;
    long seedEcomug;

    std::array<G4double, 3> GetHSphereCenter() const { return fHSphereCenter; }
    G4double GetHSphereRadius() const { return fHSphereRadius; }
    G4double GetMinTheta() const { return fMinTheta; }
    G4double GetMaxTheta() const { return fMaxTheta; }
    G4double GetMinPhi() const { return fMinPhi; }
    G4double GetMaxPhi() const { return fMaxPhi; }
    G4bool genHSphere; // Sky or HSphere
    G4bool customFlux;
    void SetGenHSphere(G4bool val) { genHSphere = val; }
    void SetCustomFlux(G4bool val) { customFlux = val; }

    G4double fSkySizeX = 1.*m;
    G4double fSkySizeY = 1.*m;
    //G4double fSkyCenterX = 0.;
    //G4double fSkyCenterY = 0.;
    //G4double fSkyCenterZ = 0.;
    std::array<G4double, 3> fSkyCenter; 
    std::array<G4double, 2> fSkySize; 

    virtual std::string GetGeneratorName() const override {return "EcoMug";}
    virtual std::string GetInfoSummary() const override;
    void Initialize() override;
    void ReadConfigFile(const std::string& filename);

  private:
    G4ParticleGun*  fParticleGun; 
    G4ParticleDefinition *mu_plus;
    G4ParticleDefinition *mu_minus;
    mutable EcoMug fGenHSphere;
    //EcoMug* genCostumHSphere = nullptr;
    G4Box* fEnvelopeBox;
    G4double E_in;
    G4double pathlength;
    G4double J(G4double p, G4double theta); // flux parameterisation

    //MuravesMessenger* fMessenger;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif