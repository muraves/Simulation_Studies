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

CosMuSimPhysicsList::CosMuSimPhysicsList():  G4VUserPhysicsList()
{ 
  fWLSProcess                = NULL;
  theCerenkovProcess           = NULL;
  theScintillationProcess      = NULL;
  theAbsorptionProcess         = NULL;
  theRayleighScatteringProcess = NULL;
  theMieHGScatteringProcess    = NULL;
  theBoundaryProcess           = NULL;
  
  defaultCutValue = 1.*mm;
  SetVerboseLevel(1);

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

  // Construct all ions
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
  G4Proton::ProtonDefinition();
  G4AntiProton::AntiProtonDefinition();

  G4Neutron::NeutronDefinition();
  G4AntiNeutron::AntiNeutronDefinition();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosMuSimPhysicsList::ConstructIons()
{
  //  nuclei
  G4Alpha::AlphaDefinition();
  G4Deuteron::DeuteronDefinition();
  G4Triton::TritonDefinition();
  G4He3::He3Definition();
  //  generic ion
  G4GenericIon::GenericIonDefinition();
}

void CosMuSimPhysicsList::ConstructProcess()
{
  AddTransportation();
  AddDecay();
  AddRadioactiveDecay(); 

  ConstructEM(); // electromagnetic physics
  ConstructOp(); // optical physics
  //ConstructGeneral();
  //ConstructInteractions();
  
}

//----------------------------------------------------------------------------//
// Define electromagnetic transportation processes
//----------------------------------------------------------------------------//

#include "G4PhysicsListHelper.hh"

// gamma
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

// e- e+
#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

// muon
#include "G4MuMultipleScattering.hh"
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

// neutron
#include "G4HadronElasticProcess.hh"
//#include "G4LElastic.hh"
#include "G4HadronElastic.hh"
#include "G4HadronInelasticProcess.hh"
//#include "G4LENeutronInelastic.hh"
#include "G4NeutronCaptureProcess.hh"
//#include "G4LCapture.hh"
#include "G4NeutronRadCapture.hh"
#include "G4NeutronFissionProcess.hh"
#include "G4LFission.hh"
#include "G4NeutronElasticXS.hh"
#include "G4NeutronFissionVI.hh"
#include "G4NeutronCaptureXS.hh"

// proton
#include "G4hMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"

// alpha
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
    ph->RegisterProcess(new G4PhotoElectricEffect, particle);
    ph->RegisterProcess(new G4ComptonScattering,   particle);
    ph->RegisterProcess(new G4GammaConversion,     particle);
    } 
  else if (particleName == "e-")
  {
      ph->RegisterProcess(new G4eMultipleScattering, particle);
      ph->RegisterProcess(new G4eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung,     particle);      
    } 
  else if (particleName == "e+") 
  {
      ph->RegisterProcess(new G4eMultipleScattering, particle);
      ph->RegisterProcess(new G4eIonisation,         particle);
      ph->RegisterProcess(new G4eBremsstrahlung,     particle);
      ph->RegisterProcess(new G4eplusAnnihilation,   particle);
    } 
  else if( particleName == "mu+" || particleName == "mu-"    ) 
  {
      ph->RegisterProcess(new G4MuMultipleScattering, particle);
      ph->RegisterProcess(new G4MuIonisation,         particle);
      ph->RegisterProcess(new G4MuBremsstrahlung,     particle);
      ph->RegisterProcess(new G4MuPairProduction,     particle);     
    } 
  /*else if (particleName == "neutron") {
    // elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      G4HadronElastic* theElasticModel = new G4HadronElastic;
      theElasticProcess->RegisterMe(theElasticModel);
      theElasticProcess->AddDataSet(new G4NeutronElasticXS());
      ph->RegisterProcess(theElasticProcess, particle);
      // inelastic scattering
      //G4NeutronInelasticProcess* theInelasticProcess =
      //                           new G4NeutronInelasticProcess("inelastic");
      //G4RPGNeutronInelastic* theInelasticModel = new G4RPGNeutronInelastic;
      //theInelasticProcess->RegisterMe(theInelasticModel);
      //pmanager->AddDiscreteProcess(theInelasticProcess);
      // capture
      G4NeutronCaptureProcess* theCaptureProcess = new G4NeutronCaptureProcess;
      G4NeutronRadCapture* theCaptureModel = new G4NeutronRadCapture;
      theCaptureProcess->RegisterMe(theCaptureModel);
      //theCaptureProcess->AddDataSet(new G4NeutronCaptureXS());
      ph->RegisterProcess(theCaptureProcess, particle);
      // fission
      G4NeutronFissionProcess* theFissionProcess = new G4NeutronFissionProcess;
      G4LFission* theFissionModel = new G4LFission;
      theFissionProcess->RegisterMe(theFissionModel);
      ph->RegisterProcess(theFissionProcess, particle);
  }*/
  else if( particleName == "proton" || particleName == "pi-" ||
               particleName == "pi+"    )
  {
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4hIonisation,         particle);
      ph->RegisterProcess(new G4hBremsstrahlung,     particle);
      ph->RegisterProcess(new G4hPairProduction,     particle);       
    } 
  else if( particleName == "alpha" || particleName == "He3" ) 
  {
      ph->RegisterProcess(new G4hMultipleScattering, particle);
      ph->RegisterProcess(new G4ionIonisation,       particle);
    } 
  else if( particleName == "GenericIon" ) 
  { 
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

//----------------------------------------------------------------------------//
// Define optical transportation processes
//----------------------------------------------------------------------------//

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

//----------------------------------------------------------------------------//
// Define decay processes
//----------------------------------------------------------------------------//

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
  SetCutsWithDefault();  
  if (verboseLevel > 0) DumpCutValuesTable(); 

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......