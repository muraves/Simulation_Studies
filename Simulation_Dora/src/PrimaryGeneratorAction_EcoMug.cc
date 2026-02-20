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
    fGenHSphere.SetHSphereRadius(130*cm);
	float yPosStation1 = 5*cm;
	float planeHalfwidth = 107./2.*cm;
	fGenHSphere.SetHSphereCenterPosition({60.*cm, 0., -planeHalfwidth + yPosStation1 });

    //fGenHSphere.SetMinimumMomentum(100.*MeV);
	//fGenHSphere.SetMaximumMomentum(200.*GeV); 
	fGenHSphere.SetMinimumTheta(1.0434);
	fGenHSphere.SetMaximumTheta(M_PI-1.0434); 

	//fGenHSphere.SetMinimumTheta(0);
	//fGenHSphere.SetMaximumTheta(M_PI/2.);

	//fGenHSphere.SetMinimumPhi(M_PI-0.5618);
	//fGenHSphere.SetMaximumPhi(0.5618+M_PI);

	fGenHSphere.SetMinimumPhi(0);
	fGenHSphere.SetMaximumPhi(M_PI/2.);

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
	G4double P1 = 0.102573;
	G4double P2 = -0.068287;
	G4double P3 = 0.958633;
	G4double P4 = 0.0407253;
	G4double P5 = 0.817285;

	G4double cos_theta = cos(theta);
	G4double num = pow(cos_theta,2) + pow(P1,2) + P2*pow(cos_theta,P3) + P4*pow(cos_theta,P5);
	G4double denom = 1 + pow(P1,2) + P2 + P4;
	G4double cos_theta_atm = sqrt(num/denom);

	// Guan's flux formula is give in terms of energy
	G4double E_muon;
	E_muon = sqrt(p*p+0.10566*0.10566);

    G4double A = 0.14;
	G4double B = E_muon*(1 + 3.64/(E_muon*pow(cos_theta_atm,1.29)));
    G4double C = 1. / (1. + 1.1*E_muon*cos_theta_atm/115.);
    G4double D = 0.054 / (1. + 1.1*E_muon*cos_theta_atm/850.);
    return A*pow(B,-2.7)*(C+D);
};

#define EPSILON 0.00001

  struct Point {
    double x;
    double y;
    double z;
};

void PrimaryGeneratorAction_EcoMug::GeneratePrimaries(G4Event* anEvent)
{	
	/*fGenHSphere.Generate();
	array<double,3>  muon_pos = fGenHSphere.GetGenerationPosition();
	G4double muon_ptot = fGenHSphere.GetGenerationMomentum() * GeV;
	G4double muon_theta = fGenHSphere.GetGenerationTheta();
	G4double muon_phi = fGenHSphere.GetGenerationPhi();
	G4double muon_charge = fGenHSphere.GetCharge();*/
	//G4cout << "muon_theta" << muon_theta / deg << G4endl;
	//G4cout << "muon_phi" << muon_phi / deg << G4endl;
	G4double muon_phi;
    G4double muon_theta;
    G4double muon_ptot;
    G4double muon_charge;
    std::array<double,3> muon_pos;

	//double phi_min = -M_PI/2.;
    double angle = M_PI/4.; //fGenHSphere.GetMaximumPhi()

	do {
        fGenHSphere.Generate();  // generate a muon
        muon_pos = fGenHSphere.GetGenerationPosition();
        muon_ptot = fGenHSphere.GetGenerationMomentum() * GeV;
        muon_theta = fGenHSphere.GetGenerationTheta();
        muon_phi = fGenHSphere.GetGenerationPhi();
        muon_charge = fGenHSphere.GetCharge();

        // Convert the desired range (-angle, angle) to (0, 2pi) for comparison
		// Window 1: around 0
		double phi0_min = 0.0;
		double phi0_max = angle;
		double phi0_min_wrapped = 2*M_PI - angle; // equivalent of -angle

		// Window 2: around pi
		double phi_pi_min = M_PI - angle;
		double phi_pi_max = M_PI + angle;

		bool in_window0 = (muon_phi >= phi0_min && muon_phi <= phi0_max) ||
						(muon_phi >= phi0_min_wrapped && muon_phi < 2*M_PI);

		bool in_window_pi = (muon_phi >= phi_pi_min && muon_phi <= phi_pi_max);

		if (in_window0 || in_window_pi) {
			break; // phi in either desired window
		}
	} while (true);  // repeat until muon_phi is in range

	auto analysisManager = G4AnalysisManager::Instance();
	analysisManager->FillH1(0, muon_theta);  // histogram ID 0
    analysisManager->FillH1(1, muon_phi);    // histogram ID 1

	analysisManager->FillH1(2, muon_pos[0]);    // histogram ID 1
	analysisManager->FillH1(3, muon_pos[1]);    // histogram ID 1
	analysisManager->FillH1(4, muon_pos[2]);    // histogram ID 1

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