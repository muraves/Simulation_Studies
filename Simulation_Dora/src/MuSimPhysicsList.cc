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
/// \file CosMuSimPhysicsList.cc
/// \brief Implementation of the CosMuSimPhysicsList class


#include "MuSimPhysicsList.hh"

#include "globals.hh"


#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"
#include "G4SystemOfUnits.hh"
#include "G4IonConstructor.hh"
#include "G4OpticalPhoton.hh"

#include "G4Cerenkov.hh"
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpRayleigh.hh"
#include "G4OpMieHG.hh"
#include "G4OpBoundaryProcess.hh"
// particles

#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/*CosMuSimPhysicsList::CosMuSimPhysicsList() 
: G4VModularPhysicsList()
{
  
  SetVerboseLevel(1);
  // Default physics
  RegisterPhysics(new G4DecayPhysics());
 
  // EM physics
  RegisterPhysics(new G4EmStandardPhysics());

  // Radioactive decay
  RegisterPhysics(new G4RadioactiveDecayPhysics());

 
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CosMuSimPhysicsList::~CosMuSimPhysicsList()
{ 

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructParticle()
{
  G4BosonConstructor  pBosonConstructor;
  pBosonConstructor.ConstructParticle();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();  
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void CosMuSimPhysicsList::SetCuts()
{    
  G4VUserPhysicsList::SetCuts();
  //std::cout<<"xxxx"<<std::endl; 
}  */
// PhysicsList General
CosMuSimPhysicsList::CosMuSimPhysicsList():  G4VUserPhysicsList()
{ 
  fWLSProcess                = NULL;
  theCerenkovProcess           = NULL;
  theScintillationProcess      = NULL;
  theAbsorptionProcess         = NULL;
  theRayleighScatteringProcess = NULL;
  theMieHGScatteringProcess    = NULL;
  theBoundaryProcess           = NULL;
  
  defaultCutValue = 1.0*cm;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CosMuSimPhysicsList::~CosMuSimPhysicsList()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructParticle()
{
  // In this method, static member functions should be called
  // for all particles which you want to use.
  // This ensures that objects of these particle types will be
  // created in the program. 

  ConstructBosons();
  ConstructLeptons();
  ConstructMesons();
  ConstructBaryons();
  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructBosons()
{
  // pseudo-particles
  G4Geantino::GeantinoDefinition();
  G4ChargedGeantino::ChargedGeantinoDefinition();

  // gamma
  G4Gamma::GammaDefinition();
  
  // optical photon
  G4OpticalPhoton::OpticalPhotonDefinition();


}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructLeptons()
{
  // leptons
  //  e+/-
  G4Electron::ElectronDefinition();
  G4Positron::PositronDefinition();
  // mu+/-
  G4MuonPlus::MuonPlusDefinition();
  G4MuonMinus::MuonMinusDefinition();
  // nu_e
  G4NeutrinoE::NeutrinoEDefinition();
  G4AntiNeutrinoE::AntiNeutrinoEDefinition();
  // nu_mu
  G4NeutrinoMu::NeutrinoMuDefinition();
  G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructMesons()
{
  //  mesons
  //    light mesons
  G4PionPlus::PionPlusDefinition();
  G4PionMinus::PionMinusDefinition();
  G4PionZero::PionZeroDefinition();
  G4Eta::EtaDefinition();
  G4EtaPrime::EtaPrimeDefinition();
  G4KaonPlus::KaonPlusDefinition();
  G4KaonMinus::KaonMinusDefinition();
  G4KaonZero::KaonZeroDefinition();
  G4AntiKaonZero::AntiKaonZeroDefinition();
  G4KaonZeroLong::KaonZeroLongDefinition();
  G4KaonZeroShort::KaonZeroShortDefinition();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructBaryons()
{
  //  barions
  G4Proton::ProtonDefinition();
  G4AntiProton::AntiProtonDefinition();

  G4Neutron::NeutronDefinition();
  G4AntiNeutron::AntiNeutronDefinition();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructProcess()
{
  AddTransportation();
  ConstructEM();
  //ConstructGeneral();
  ConstructOp();
    AddDecay();
  AddRadioactiveDecay(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4PhysicsListHelper.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuMultipleScattering.hh"
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4hMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"

#include "G4ionIonisation.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructEM()
{
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  auto theParticleIterator=GetParticleIterator(); 
  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4String particleName = particle->GetParticleName();
    
    if (particleName == "gamma")
  {
      // gamma         
    ph->RegisterProcess(new G4PhotoElectricEffect, particle);
    ph->RegisterProcess(new G4ComptonScattering,   particle);
    ph->RegisterProcess(new G4GammaConversion,     particle);
      
    } 
  else if (particleName == "e-")
  {
      //electron
      ph->RegisterProcess(new G4eMultipleScattering, particle);
      ph->RegisterProcess(new G4eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung,     particle);      

    } 
  else if (particleName == "e+") 
  {
      //positron
      ph->RegisterProcess(new G4eMultipleScattering, particle);
      ph->RegisterProcess(new G4eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung,     particle);
      ph->RegisterProcess(new G4eplusAnnihilation,   particle);
    
    } 
  else if( particleName == "mu+" || particleName == "mu-"    ) 
  {
      //muon  
      ph->RegisterProcess(new G4MuMultipleScattering, particle);
      ph->RegisterProcess(new G4MuIonisation,         particle);
      ph->RegisterProcess(new G4MuBremsstrahlung,     particle);
      ph->RegisterProcess(new G4MuPairProduction,     particle);
             
    } 
  else if( particleName == "proton" || particleName == "pi-" ||
               particleName == "pi+"    )
  {
      //proton  
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4hIonisation,         particle);
      ph->RegisterProcess(new G4hBremsstrahlung,     particle);
      ph->RegisterProcess(new G4hPairProduction,     particle);       
     
    } 
  else if( particleName == "alpha" || particleName == "He3" ) 
  {
      //alpha 
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4ionIonisation,       particle);
     
    } 
  else if( particleName == "GenericIon" ) 
  { 
      //Ions 
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4ionIonisation,       particle);     
      
    } 
  else if ((!particle->IsShortLived()) &&
               (particle->GetPDGCharge() != 0.0) && 
               (particle->GetParticleName() != "chargedgeantino"))
  {
      //all others charged particles except geantino
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4hIonisation,         particle);        
    }     
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
#include "G4LossTableManager.hh"
#include "G4EmSaturation.hh"
void CosMuSimPhysicsList::ConstructOp()
{
  
  fWLSProcess = new G4OpWLS();
  //theCerenkovProcess           = new G4Cerenkov("Cerenkov");
  theScintillationProcess      = new G4Scintillation("Scintillation");
  theAbsorptionProcess         = new G4OpAbsorption();
  theRayleighScatteringProcess = new G4OpRayleigh();
  theMieHGScatteringProcess    = new G4OpMieHG();
  theBoundaryProcess           = new G4OpBoundaryProcess();

//  theCerenkovProcess->DumpPhysicsTable();    //for standard output stream
//  theScintillationProcess->DumpPhysicsTable();
//  theRayleighScatteringProcess->DumpPhysicsTable();
 // theAbsorptionProcess->DumpPhysicsTable();

  //wls time structure
  fWLSProcess->UseTimeProfile("delta");
  
 // theCerenkovProcess->SetMaxNumPhotonsPerStep(300);
  //theCerenkovProcess->SetMaxBetaChangePerStep(10.0);
  //theCerenkovProcess->SetTrackSecondariesFirst(true);
  
  //theScintillationProcess->SetScintillationYieldFactor(1.);

  theScintillationProcess->SetTrackSecondariesFirst(true);
  theScintillationProcess->SetFiniteRiseTime(true);
  // Use Birks Correction in the Scintillation process

/*  G4EmSaturation* emSaturation = G4LossTableManager::Instance()->EmSaturation();
  theScintillationProcess->AddSaturation(emSaturation);*/

  auto theParticleIterator=GetParticleIterator(); 
  theParticleIterator->reset();
  while( (*theParticleIterator)() )
  {
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
  /*  if (theCerenkovProcess->IsApplicable(*particle)) {
      pmanager->AddProcess(theCerenkovProcess);
      pmanager->SetProcessOrdering(theCerenkovProcess,idxPostStep);
    }*/
    if (theScintillationProcess->IsApplicable(*particle)/* && particleName != "GenericIon"*/)
  {
    pmanager->AddProcess(theScintillationProcess);
    pmanager->SetProcessOrderingToLast(theScintillationProcess, idxAtRest);
    pmanager->SetProcessOrderingToLast(theScintillationProcess, idxPostStep);
    }
    if (particleName == "opticalphoton")
  {
    G4cout << " AddDiscreteProcess to OpticalPhoton " << G4endl;
    pmanager->AddDiscreteProcess(theAbsorptionProcess);
    pmanager->AddDiscreteProcess(theRayleighScatteringProcess);
    pmanager->AddDiscreteProcess(theMieHGScatteringProcess);
    pmanager->AddDiscreteProcess(theBoundaryProcess);
    pmanager->AddDiscreteProcess(fWLSProcess);
    }
  }
}


#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"

void CosMuSimPhysicsList::ConstructGeneral() 
  {

  // Add Decay Process
  G4Decay* theDecayProcess = new G4Decay();
  auto theParticleIterator=GetParticleIterator(); 
  theParticleIterator->reset();
  while( (*theParticleIterator)() )
    {
      G4ParticleDefinition* particle = theParticleIterator->value();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      
      if (theDecayProcess->IsApplicable(*particle) && !particle->IsShortLived()) 
  { 
    pmanager ->AddProcess(theDecayProcess);
    // set ordering for PostStepDoIt and AtRestDoIt
    pmanager ->SetProcessOrdering(theDecayProcess, idxPostStep);
    pmanager ->SetProcessOrdering(theDecayProcess, idxAtRest);
  }
    }

  // Declare radioactive decay to the GenericIon in the IonTable.
  const G4IonTable *theIonTable = 
  G4ParticleTable::GetParticleTable()->GetIonTable();
  G4RadioactiveDecay *theRadioactiveDecay = new G4RadioactiveDecay();

  for (G4int i=0; i<theIonTable->Entries(); i++) 
    {
      G4String particleName = theIonTable->GetParticle(i)->GetParticleName();
      G4String particleType = theIonTable->GetParticle(i)->GetParticleType();
      
      if (particleName == "GenericIon") 
  {
    G4ProcessManager* pmanager = theIonTable->GetParticle(i)->GetProcessManager();
    pmanager ->AddProcess(theRadioactiveDecay);
    pmanager ->SetProcessOrdering(theRadioactiveDecay, idxPostStep);
    pmanager ->SetProcessOrdering(theRadioactiveDecay, idxAtRest);
  } 
    }
  
  
  
}



#include "G4PhysicsListHelper.hh"
#include "G4Decay.hh"

void CosMuSimPhysicsList::AddDecay()
{
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
    
  // Decay Process
  //
  G4Decay* fDecayProcess = new G4Decay();
  auto theParticleIterator=GetParticleIterator(); 
  theParticleIterator->reset();
  while( (*theParticleIterator)() )
  {
    G4ParticleDefinition* particle = theParticleIterator->value();
    if (fDecayProcess->IsApplicable(*particle)) 
    ph->RegisterProcess(fDecayProcess, particle);    
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4PhysicsListHelper.hh"
#include "G4RadioactiveDecay.hh"

void CosMuSimPhysicsList::AddRadioactiveDecay()
{  
  G4RadioactiveDecay* radioactiveDecay = new G4RadioactiveDecay();
  //radioactiveDecay->SetHLThreshold(-1.*s);
  //radioactiveDecay->SetICM(true);                //Internal Conversion
  radioactiveDecay->SetARM(true);                //Atomic Rearangement
  
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();  
  ph->RegisterProcess(radioactiveDecay, G4GenericIon::GenericIon());
}

void CosMuSimPhysicsList::SetCuts()
{
  //G4VUserPhysicsList::SetCutsWithDefault method sets 
  //the default cut value for all particle types 
  //
  SetCutsWithDefault();
     

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......