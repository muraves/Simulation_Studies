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
#include "EcomugMessenger.hh"
//#include "TFile.h"
//#include "TH2D.h"
//#include "HistogramManager.hh"

using namespace std;
using namespace chrono;
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction_EcoMug::PrimaryGeneratorAction_EcoMug(const std::string& inputFile): 
G4VUserPrimaryGeneratorAction(), fParticleGun(0), mu_plus(0), mu_minus(0),
  fMinTheta(0.), fMaxTheta(M_PI/2), fMinPhi(0.), fMaxPhi(2*M_PI),
  fMinPosTheta(0.), fMaxPosTheta(M_PI/2), fMinPosPhi(0.), fMaxPosPhi(2*M_PI), genHSphere(true), customFlux(true), fHorizontalRate(138*(EMUnits::hertz/EMUnits::m2)),
  fHSphereRadius(150*cm), fHSphereCenter({56.45*cm,0*cm,-48.5*cm}), fSkyCenter({0.,0.,0.}), fSkySize({1.*m,1.*m})
{ 
	const double DEG_TO_RAD = M_PI / 180.0;

	auto messenger = new EcoMugMessenger(this);

	if (!inputFile.empty()) {
        ReadConfigFile(inputFile);
    }

	seedEcomug = std::chrono::system_clock::now().time_since_epoch().count();

	G4int n_particle = 1;
	fParticleGun  = new G4ParticleGun(n_particle);

	mu_plus = G4ParticleTable::GetParticleTable()->FindParticle("mu+");
	mu_minus = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
  }

PrimaryGeneratorAction_EcoMug::~PrimaryGeneratorAction_EcoMug()
{  
    delete fParticleGun;
}

void PrimaryGeneratorAction_EcoMug::Initialize() {
    // -------- Set muon generation surface (sky or hemisphere) --------
    if (genHSphere) {
        fGenHSphere.SetUseHSphere();

        fGenHSphere.SetHSphereRadius(fHSphereRadius);
        fGenHSphere.SetHSphereCenterPosition(fHSphereCenter);
    } else {
        fGenHSphere.SetUseSky();
		fGenHSphere.SetSkySize(fSkySize);
		fGenHSphere.SetSkyCenterPosition(fSkyCenter);
    }

	fGenHSphere.SetSeed(seedEcomug);

    // -------- Set limitations on generated muon direction -------- 
    fGenHSphere.SetMinimumTheta(fMinTheta);
    fGenHSphere.SetMaximumTheta(fMaxTheta);
    fGenHSphere.SetMinimumPhi(fMinPhi);
    fGenHSphere.SetMaximumPhi(fMaxPhi);

    // -------- Set limitations on generation position on hemisphere --------
    fGenHSphere.SetHSphereMinPositionTheta(fMinPosTheta);
	fGenHSphere.SetHSphereMaxPositionTheta(fMaxPosTheta);
    fGenHSphere.SetHSphereMinPositionPhi(fMinPosPhi);
    fGenHSphere.SetHSphereMaxPositionPhi(fMaxPosPhi);

    fGenHSphere.SetHorizontalRate(fHorizontalRate * EMUnits::hertz/EMUnits::m2);

    // -------- Set custom parameterisation for cosmic muon flux --------
    if (customFlux) {
        fGenHSphere.SetDifferentialFlux([this](double p, double theta) {
            return this->J(p, theta);
        });
    }
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
	if (fMaxPhi < 2*M_PI || fMinPhi > 0.) 
    {
        G4cout << "initializing while loop for phi-restriction" << G4endl;
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
    }
    else {
        fGenHSphere.Generate();  // generate a muon
        muon_pos = fGenHSphere.GetGenerationPosition();
        muon_ptot = fGenHSphere.GetGenerationMomentum() * GeV;
        muon_theta = fGenHSphere.GetGenerationTheta();
        muon_phi = fGenHSphere.GetGenerationPhi();
        muon_charge = fGenHSphere.GetCharge();
    }
	
	//auto analysisManager = G4AnalysisManager::Instance();
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
	E = sqrt(muon_ptot*muon_ptot+0.10566*0.10566); // muon mass = 105.66 MeV = 0.10566 GeV
	fParticleGun->SetParticleEnergy(E); 
		
	if (muon_charge > 0) 
    {fParticleGun->SetParticleDefinition(mu_plus);}
	else 
    {fParticleGun->SetParticleDefinition(mu_minus);}
	
	fParticleGun->GeneratePrimaryVertex(anEvent);
	
}

std::string PrimaryGeneratorAction_EcoMug::GetInfoSummary() const {
    fGenHSphere.GetAverageGenRateAndError(rateHSphere, errorHSphere);
	genSurfaceArea = fGenHSphere.GetGenSurfaceArea();

    std::ostringstream oss;
    oss << "Generator: " << GetGeneratorName() <<
    "\nGenerator seed: " << seedEcomug <<
	"\nCustom flux (bool): " << customFlux <<
    "\nSet horizontal rate (m-2 s-1): " << fGenHSphere.GetHorizontalRate() / (EMUnits::hertz/EMUnits::m2) << "\n";
    if (genHSphere) {
        oss << "\nGeneration surface: hemisphere" << 
        "\nfHSphereCenter (cm): ("
        << fHSphereCenter[0] / cm << ", "
        << fHSphereCenter[1] / cm << ", "
        << fHSphereCenter[2] / cm << ")\n"
        << "fHSphereRadius (cm): " << fHSphereRadius / cm << "\n\n";}
    else {
        oss << "\nGeneration surface: flat sky" << 
        "\nfSkyCenter (cm): ("
        << fSkyCenter[0] / cm << ", "
        << fSkyCenter[1] / cm << ", "
        << fSkyCenter[2] / cm << ")\n"
    << "\nfSkySize (cm x cm): ("
        << fSkySize[0] / cm << " x "
        << fSkySize[1] / cm << "\n\n";}
    
		oss << "Theta range (rad): [" 
		<< fGenHSphere.GetMinimumTheta() << ", "
		<< fGenHSphere.GetMaximumTheta() << "]\n"
		<< "Phi range (rad): [" 
		<< fGenHSphere.GetMinimumPhi() << ", "
		<< fGenHSphere.GetMaximumPhi() << "]\n"
		<< "\nThetaPos range (rad): [" 
		<< fMinPosTheta << ", "
		<< fMaxPosTheta << "]\n"
		<< "PhiPos range (rad): [" 
		<< fMinPosPhi << ", "
		<< fMaxPosPhi << "]\n"
		<< "\nMomentum range (GeV): [" 
		<< fGenHSphere.GetMinimumMomentum() << ", "
		<< fGenHSphere.GetMaximumMomentum() << "]\n"
		<< "\nMuon generation rate (m-2 s-1): " << rateHSphere
		<< "\nMuon generation rate error: " << errorHSphere
		<< "\nGeneration surface area (m^2): " << genSurfaceArea / m2
		;
    return oss.str();
}

void PrimaryGeneratorAction_EcoMug::ReadConfigFile(const std::string& filename) {
	G4cout << "DEBUG ReadConfigFile this=" << this << G4endl;
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        G4cerr << "Cannot open EcoMug input file: " << filename << G4endl;
        return;
    }
	
	std::string line;
	while (std::getline(infile, line)) {
		if (line.empty() || line[0] == '#') continue; // skip blank lines and comments
		std::istringstream iss(line);
		std::string key;
		double val;
		if (!(iss >> key >> val)) continue; // skip malformed lines
		if (key == "theta_min") { fMinTheta = val; }
        else if (key == "theta_max") { fMaxTheta = val; }
        else if (key == "phi_min") { fMinPhi = val; }
        else if (key == "phi_max") { fMaxPhi = val; }
        else if (key == "pos_theta_min") { fMinPosTheta = val; }
        else if (key == "pos_theta_max") { fMaxPosTheta = val; }
        else if (key == "pos_phi_min") { fMinPosPhi = val; }
        else if (key == "pos_phi_max") { fMaxPosPhi = val; }
        else if (key == "seed")          { seedEcomug = static_cast<long>(val); }
        else if (key == "horizontal_rate") { fHorizontalRate = val; }
        else if (key == "gen_hsphere")   { genHSphere = static_cast<bool>(val); }
        else if (key == "custom_flux")   { customFlux = static_cast<bool>(val); }
		else if (key == "hsphere_center_x") { fHSphereCenter[0] = val * cm; }
		else if (key == "hsphere_center_y") { fHSphereCenter[1] = val * cm; }
		else if (key == "hsphere_center_z") { fHSphereCenter[2] = val * cm; }
		else if (key == "hsphere_radius")   { fHSphereRadius = val * cm; }
		else if (key == "sky_size_x")       { fSkySize[0] = val * cm; }
		else if (key == "sky_size_y")       { fSkySize[1] = val * cm; }
		else if (key == "sky_center_x")     { fSkyCenter[0] = val * cm; }
		else if (key == "sky_center_y")     { fSkyCenter[1] = val * cm; }
		else if (key == "sky_center_z")     { fSkyCenter[2] = val * cm; }
	}

    infile.close();
    G4cout << "EcoMug configuration loaded from " << filename << G4endl;
}