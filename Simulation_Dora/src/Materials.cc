//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file optical/wls/src/CosMuSimMaterials.cc
/// \brief Implementation of the CosMuSimMaterials class
//
//
#include "Materials.hh"

#include "G4SystemOfUnits.hh"

Materials* Materials::fInstance = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Materials::Materials()
{
  fNistMan = G4NistManager::Instance();

  fNistMan->SetVerbose(2);

  CreateMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Materials::~Materials()
{
	delete    fAir;
	delete    fpolystyrene;
	delete    fBC600;
	delete 	  fPTFE;
	delete	  fGlass;
	delete    fPMMA;
	delete    fBCF92;
	delete    fAl;
	delete 	  fFAcrylic;
	delete    fY11;
	
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Materials* Materials::GetInstance()
{
  if (fInstance == 0)
    {
      fInstance = new Materials();
    }
  return fInstance;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* Materials::GetMaterial(const G4String material)
{
  G4Material* mat =  fNistMan->FindOrBuildMaterial(material);

  if (!mat) mat = G4Material::GetMaterial(material);
  if (!mat) {
     std::ostringstream o;
     o << "Material " << material << " not found!";
     G4Exception("Materials::GetMaterial","",
                 FatalException,o.str().c_str());
  }

  return mat;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Materials::CreateMaterials()
{	
	// Get nist material manager
	G4NistManager* nist = G4NistManager::Instance();
	G4double a,z,density,fractionmass;
	G4String name, symbol;
	G4int ncomponents, natoms;
	
	//-------------------------------------------------------------------------------
	//general properties
	//-------------------------------------------------------------------------------
	
	//BC600
	a=12.01*g/mole;
	G4Element* elC=new G4Element(name="Carbon",symbol= "C", z=6., a);

	a=1.01*g/mole;
	G4Element* elH=new G4Element(name="Hydrogen",symbol= "H", z=1., a);

	a=16.00*g/mole;
	G4Element* elO=new G4Element(name="Oxygen",symbol= "O", z=8., a);

	a=28.09*g/mole;
	G4Element* elSi=new G4Element(name="Silicon",symbol= "Si", z=14., a);

	density=1.07*g/cm3;
	fBC600 = new G4Material(name="BC600", density,ncomponents=4);
	fBC600-> AddElement(elC, fractionmass=0.6173);
	fBC600-> AddElement(elH, fractionmass=0.0592);
	fBC600-> AddElement(elO, fractionmass=0.1175);
	fBC600-> AddElement(elSi, fractionmass=0.2060);

	//PTFE
	fPTFE = nist->FindOrBuildMaterial("G4_POLYTRIFLUOROCHLOROETHYLENE");

	G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
	G4Material* polyvinyltoluene = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
	if (polyvinyltoluene) {
    G4cout << "Material G4_PLASTIC_SC_VINYLTOLUENE successfully created." << G4endl;
	} else {
    G4cout << "Error: Material G4_PLASTIC_SC_VINYLTOLUENE could not be created." << G4endl;
	}
	G4Material* WLS = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
	if (WLS) {
    G4cout << "Material G4_POLYSTYRENE successfully created." << G4endl;
	} else {
    G4cout << "Error: Material G4_POLYSTYRENE could not be created." << G4endl;
	}
	G4Material* clad = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
	if (clad) {
    G4cout << "Material G4_PLEXIGLASS successfully created." << G4endl;
	} else {
    G4cout << "Error: Material G4_PLEXIGLAS could not be created." << G4endl;
	}

	// Limestone
  	G4Material *limestone =  nist->FindOrBuildMaterial("G4_CALCIUM_CARBONATE");
	if (limestone) {
    G4cout << "Material G4_CALCIUM_CARBONATE successfully created." << G4endl;
	} else {
    G4cout << "Error: Material G4_CALCIUM_CARBONATE could not be created." << G4endl;
	}

  	// Scilicon
  	double siliconDensity = 2.33 * g / cm3;
  	double siliconAtomicWeight = 28.0855 * g / mole;
  	G4Material *silicon = new G4Material("silicon", 14., siliconAtomicWeight, siliconDensity);
	 
	//EJ-200 
	fpolystyrene = new G4Material("polystyrene", density=1.06*g/cm3,2);
	fpolystyrene -> AddElement(elH, 8);
	fpolystyrene -> AddElement(elC, 8);
	
	//fiber_core
	fBCF92 = new G4Material("BCF92", density=1.05*g/cm3, 2);
	fBCF92->AddElement(elH, 8);
	fBCF92->AddElement(elC, 8);
	
	//cladding
	fPMMA = new G4Material("PMMA", density=1.19*g/cm3,3);
	fPMMA->AddElement(elH,8);
	fPMMA->AddElement(elC,5);
	fPMMA->AddElement(elO,2);	

	//pmt_glass
	G4Material* Oxigen      = nist->FindOrBuildMaterial("G4_O");	
	G4Material* Silicon     = nist->FindOrBuildMaterial("G4_Si");	
	G4Material* Potassium   = nist->FindOrBuildMaterial("G4_K");	
	G4Material* Sodium      = nist->FindOrBuildMaterial("G4_Na");	
	G4Material* lead        = nist->FindOrBuildMaterial("G4_Pb");	
	G4Material* Boron       = nist->FindOrBuildMaterial("G4_B");	
	G4Material* fAl    		= nist->FindOrBuildMaterial("G4_Al");
	fGlass = new G4Material("Glass",density=2.23*g/cm3,6);
	fGlass->AddMaterial(Oxigen,     0.539562); 
	fGlass->AddMaterial(Boron,      0.040064);
	fGlass->AddMaterial(Sodium,     0.028191);
	fGlass->AddMaterial(fAl,   0.011644);
	fGlass->AddMaterial(Silicon,    0.377220);
	fGlass->AddMaterial(Potassium,  0.003321);	
	
	//Air
	fAir = nist->FindOrBuildMaterial("G4_AIR");
	
	//Double Cladding (fluorinated polyethylene)
	fFAcrylic = new G4Material("FAcrylic", density=1.050*g/cm3, 2);
	fFAcrylic->AddElement(elC, 2);
	fFAcrylic->AddElement(elH, 4);
	
	//Y11
	fY11 = new G4Material("Y11", density=1.05*g/cm3, 2);
	fY11->AddElement(elH, 8);
	fY11->AddElement(elC, 8);
	
	
	//-------------------------------------------------------------------------------
	//Scintillator properties
	//-------------------------------------------------------------------------------
	
	const G4int NUMENTRIES = 21;
    G4double EPhoton[NUMENTRIES] = {2.067*eV, 2.156*eV, 2.255*eV, 2.296*eV, 2.340*eV, 2.385*eV, 2.431*eV, 2.480*eV, 2.531*eV, 2.583*eV,
    2.638*eV, 2.696*eV, 2.756*eV, 2.818*eV, 2.851*eV, 2.884*eV, 2.952*eV, 3.024*eV, 3.100*eV, 3.179*eV, 3.543*eV};
		
	//EJ200
	G4double scint_rindex[NUMENTRIES] = {1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6};
										
	G4double scint_absorption[NUMENTRIES] = {3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m};
												
	G4double scint_fastcomponent[NUMENTRIES] = {0.0, 0.1, 0.25, 0.35, 0.7, 0.12, 0.18, 0.2, 0.3, 0.42, 0.4, 0.5, 0.7, 0.72, 0.69, 0.75, 1, 0.5, 0.02, 0.01, 0.0};

		G4MaterialPropertiesTable* scintPMT1 = new G4MaterialPropertiesTable();
		scintPMT1->AddProperty("RINDEX", EPhoton, scint_rindex,NUMENTRIES);
		scintPMT1->AddProperty("ABSLENGTH",EPhoton, scint_absorption, NUMENTRIES);
		scintPMT1->AddProperty("FASTCOMPONENT",EPhoton, scint_fastcomponent, NUMENTRIES,true);	
		scintPMT1->AddConstProperty("SCINTILLATIONYIELD",9700./MeV);  
		scintPMT1->AddConstProperty("RESOLUTIONSCALE",1.0);
		scintPMT1->AddConstProperty("FASTTIMECONSTANT",2.5*ns, true);  
		scintPMT1->AddConstProperty("FASTSCINTILLATIONRISETIME",0.9*ns, true);  
		scintPMT1->AddConstProperty("YIELDRATIO",1.0, true); 
		fpolystyrene->SetMaterialPropertiesTable(scintPMT1);	
		//fEJ200->GetIonisation()->SetBirksConstant(0.111*mm/MeV); //from PPT: Photon Transport Simulations
}