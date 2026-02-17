/// \file PrimaryGeneratorAction_EcoMug.cc
/// \brief Implementation of the PrimaryGeneratorAction_EcoMug class

#include "PrimaryGeneratorAction_EcoMug.hh"
//#include "GlobalParameters.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4Geantino.hh"
#include "G4IonTable.hh"
#include "G4Event.hh"
#include "G4PhysicalConstants.hh"
#include <fstream>
#include <cmath>
#include "EcoMug.h"
#include "G4ChargedGeantino.hh"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <chrono>
#include "G4AnalysisManager.hh"
//#include "MuonEventInfo.hh"
#include "G4TransportationManager.hh"
//#include "TFile.h"
//#include "TH2D.h"
//#include "HistogramManager.hh"

using namespace std;
using namespace chrono;
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction_EcoMug::PrimaryGeneratorAction_EcoMug(): G4VUserPrimaryGeneratorAction(),fParticleGun(0), mu_plus(0),mu_minus(0)
{ 
    fGenHSphere.SetUseHSphere();
    fGenHSphere.SetHSphereRadius(200*cm);
	fGenHSphere.SetHSphereCenterPosition({0., 0., 0.});

    //fMuonGen.SetMinimumMomentum(35.*GeV);
	//fMuonGen.SetMaximumMomentum(200.*GeV);
	//fMuonGen.SetMinimumTheta(0.);
	//fMuonGen.SetMaximumTheta(M_PI-2); 
	// pi - 2 (theta = 2.2 is smallest angle that is observed by detector field of view, take 2 for a bit of marge)
	// Set maximumtheta instead of minimtheta(2.2) because EcoMug definition works differently
	//fMuonGen.SetMinimumPhi(M_PI/2);
	//fMuonGen.SetMaximumPhi(3*M_PI/2);

    EcoMug genCustomHSphere(fGenHSphere);
    //genCustomHSphere.SetDifferentialFlux(PrimaryGeneratorAction_EcoMug::J);
    //fGenHSphere.SetHorizontalRate(200.*EMUnits::hertz/EMUnits::m2);

    genCustomHSphere.SetDifferentialFlux(
    [this](double p, double theta)
    {
        return this->J(p, theta);
    }
);

    G4double rateHSphere;
	G4double errorHSphere;
    genCustomHSphere.GetAverageGenRateAndError(rateHSphere, errorHSphere, 1e7);

	G4int n_particle = 1;
	fParticleGun  = new G4ParticleGun(n_particle);

	mu_plus = G4ParticleTable::GetParticleTable()->FindParticle("mu+");
	mu_minus = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
  }


PrimaryGeneratorAction_EcoMug::~PrimaryGeneratorAction_EcoMug()
{  
    delete fParticleGun;
}

// Parameterise Guan's flux (currently: Geisser's flux)
G4double PrimaryGeneratorAction_EcoMug::J(G4double p, G4double theta) 
{
    G4double A = 1400.*pow(p, -2.7);
    G4double B = 1. / (1. + 1.1*p*cos(theta)/115.);
    G4double C = 0.054 / (1. + 1.1*p*cos(theta)/850.);
    return A*(B+C);
};

#define EPSILON 0.00001

  struct Point {
    double x;
    double y;
    double z;
};

void PrimaryGeneratorAction_EcoMug::GeneratePrimaries(G4Event* anEvent)
{	
	fGenHSphere.Generate();
	array<double,3>  muon_pos = fGenHSphere.GetGenerationPosition();
	G4double muon_ptot = fGenHSphere.GetGenerationMomentum() * GeV;
	G4double muon_theta = fGenHSphere.GetGenerationTheta();
	G4double muon_phi = fGenHSphere.GetGenerationPhi();
	G4double muon_charge = fGenHSphere.GetCharge();

	G4double a = sin(muon_theta)*cos(muon_phi);
	G4double b = sin(muon_theta)*sin(muon_phi);
	G4double c = cos(muon_theta);

	fParticleGun->SetParticlePosition(G4ThreeVector(muon_pos[0], muon_pos[1], muon_pos[2]));
	fParticleGun->SetParticleMomentumDirection(G4ParticleMomentum(a, b, c)); 

	G4double E;
	E = sqrt(muon_ptot*muon_ptot+0.10566*0.10566);
	fParticleGun->SetParticleEnergy(E); 
		
	if (muon_charge > 0) 
    {fParticleGun->SetParticleDefinition(mu_plus);}
	else 
    {fParticleGun->SetParticleDefinition(mu_minus);}
		

	fParticleGun->GeneratePrimaryVertex(anEvent);
}