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

	G4double HSphereCenterX = 56.45*cm;
	G4double HSphereCenterY = 0.*cm;
	G4double HSphereCenterZ = -48.5*cm;
	fHSphereCenter = {HSphereCenterX, HSphereCenterY, HSphereCenterZ};
	fHSphereRadius = 150*cm;

    fGenHSphere.SetHSphereRadius(fHSphereRadius);
	fGenHSphere.SetHSphereCenterPosition(fHSphereCenter);

	seedEcomug = std::chrono::system_clock::now().time_since_epoch().count();
	fGenHSphere.SetSeed(seedEcomug);

	
	// -------- Set limitations on generation position on hemisphere -------- 
	//fMinPosTheta = 0.31037063232396755;

	//fGenHSphere.SetHSphereMinPositionTheta(fMinPosTheta); 

	//fMinPosPhi = 2.0938588701035186;
	//fMaxPosPhi = -2.0938588701035186+2*M_PI;
	
	// backwards
	//fMinPosPhi = - 1.4705358676698623;
	//fMaxPosPhi = -fMinPosPhi;

	//fGenHSphere.SetHSphereMinPositionPhi(fMinPosPhi);
	//fGenHSphere.SetHSphereMaxPositionPhi(fMaxPosPhi);

	// -------- Set limitations on generated muon direction -------- 
	//fMinTheta = 0.4852877880148983; // angle with negative z

	//fGenHSphere.SetMinimumTheta(fMinTheta); 

	//fMaxPhi = 1.0477337834862748;
	//fMinPhi = -fMaxPhi; // symmetrical view

	// backwards
	//fMinPhi = 2.0938588701035186;
	//fMaxPhi = -2.0938588701035186+2*M_PI;

	//fGenHSphere.SetMinimumPhi(fMinPhi);
	//fGenHSphere.SetMaximumPhi(fMaxPhi);
	
    //fGenHSphere.SetHorizontalRate(200.*EMUnits::hertz/EMUnits::m2);
    fGenHSphere.SetDifferentialFlux(
    [this](double p, double theta)
    {
        return this->J(p, theta);
    });

	genSurfaceArea = fGenHSphere.GetGenSurfaceArea();

	G4int n_particle = 1;
	fParticleGun  = new G4ParticleGun(n_particle);

	mu_plus = G4ParticleTable::GetParticleTable()->FindParticle("mu+");
	mu_minus = G4ParticleTable::GetParticleTable()->FindParticle("mu-");

	fGenHSphere.GetAverageGenRateAndError(rateHSphere,errorHSphere);

	//G4cout << "Rate " << rateHSphere << G4endl;
	//G4cout << "Error" << errorHSphere << G4endl;
  }

PrimaryGeneratorAction_EcoMug::~PrimaryGeneratorAction_EcoMug()
{  
    delete fParticleGun;
}

// Parameterise Guan's flux (EcoMug uses m-2 sr-1 s-1 GeV-1)
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

    G4double A = 1400; 
	G4double B = E_muon*(1 + 3.64/(E_muon*pow(cos_theta_atm,1.29)));
    G4double C = 1. / (1. + 1.1*E_muon*cos_theta_atm/115.);
    G4double D = 0.054 / (1. + 1.1*E_muon*cos_theta_atm/850.);
    return A*pow(B,-2.7)*(C+D);
};

void PrimaryGeneratorAction_EcoMug::GeneratePrimaries(G4Event* anEvent)
{	
	G4double muon_phi;
    G4double muon_theta;
    G4double muon_ptot;
    G4double muon_charge;
    std::array<double,3> muon_pos;

	// ------------- Generate a muon from the custom flux parameterisation on the hemisphere ---------------
	// --- and do this until muon is generated with the azimuthal angle phi of the direction in angular acceptance of detector ---------------
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
        double phi0_max = fMaxPhi;
        double phi0_min_wrapped = 2*M_PI - fMaxPhi; // equivalent of -angle

        // Window 2: around pi
        double phi_pi_min = M_PI - fMaxPhi;
        double phi_pi_max = M_PI + fMaxPhi;

        bool in_window0 = (muon_phi >= phi0_min && muon_phi <= phi0_max) ||
                        (muon_phi >= phi0_min_wrapped && muon_phi < 2*M_PI);

        bool in_window_pi = (muon_phi >= phi_pi_min && muon_phi <= phi_pi_max);

        if (in_window0 || in_window_pi) {
            break; // phi in either desired window
        }
    break;
    } while (true); // repeat until muon_phi is in range
	
	auto analysisManager = G4AnalysisManager::Instance();
	//analysisManager->FillH1(0, muon_theta);  // histogram ID 0

	//G4double muon_phi_hist = muon_phi;
	//if (muon_phi_hist > M_PI)
	//{muon_phi_hist -= 2*M_PI;} 

    //analysisManager->FillH1(1, muon_phi);    // histogram ID 1
	//analysisManager->FillH1(2, muon_pos[0]);    // histogram ID 1
	//analysisManager->FillH1(3, muon_pos[1]);    // histogram ID 1
	//analysisManager->FillH1(4, muon_pos[2]);    // histogram ID 1

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

std::string PrimaryGeneratorAction_EcoMug::GetInfoSummary() const {
    std::ostringstream oss;
    oss << "Generator: " << GetGeneratorName() <<
	"\nfHSphereCenter (cm): ("
    << fHSphereCenter[0] / cm << ", "
    << fHSphereCenter[1] / cm << ", "
    << fHSphereCenter[2] / cm << ")\n"
        << "fHSphereRadius (cm): " << fHSphereRadius / cm
		<< "\nTheta range: [" 
		<< fMinTheta << ", "
		<< fMaxTheta << "]\n"
		<< "Phi range: [" 
		<< fMinPhi << ", "
		<< fMaxPhi << "]\n"
		<< "\nThetaPos range: [" 
		<< fMinPosTheta << ", "
		<< fMaxPosTheta << "]\n"
		<< "PhiPos range: [" 
		<< fMinPosPhi << ", "
		<< fMaxPosPhi << "]\n"
		<< "Muon generation rate (m-2 s-1): " << rateHSphere
		<< "\nMuon generation rate error: " << errorHSphere
		<< "\nGeneration surface area (m^2): " << genSurfaceArea / m2
		<< "\nEcoMug generator seed: " << seedEcomug
		;
    return oss.str();
}