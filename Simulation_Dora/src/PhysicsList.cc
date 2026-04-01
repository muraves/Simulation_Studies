/// \file PhysicsList.cc
/// \brief Muography physics list adapted from FTFP_BERT
///
///  Base:    FTFP_BERT — full hadronic coverage 0.001–1000 GeV,
///                       includes G4NeutronHP for accurate low-energy
///                       neutron transport (required for secondary studies)
///  EM:      G4EmStandardPhysics_option4 — most accurate muon ionisation (paper Zhang2025, 2507.03914v2)
///                       and scattering, Goudsmit-Saunderson for e±,
///                       Doppler-broadened Compton below 20 MeV
///  Steps:   G4StepLimiterPhysics — activated per-volume via G4UserLimits
///  Optical: G4OpticalPhysics — scintillation, WLS, boundary processes

#include "PhysicsList.hh"

#include "G4SystemOfUnits.hh"

// --- Base list ---
#include "FTFP_BERT.hh"

// --- EM replacement ---
#include "G4EmStandardPhysics_option4.hh"

// --- Step limiter ---
#include "G4StepLimiterPhysics.hh"

// --- Optical ---
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  // -- Start from FTFP_BERT, copying all its constructors except the
  //    default EM (G4EmStandardPhysics / "emstandard_opt0") which we replace.
  //    FTFP_BERT already includes:
  //      G4HadronPhysicsFTFP_BERT   hadronic inelastic (Bertini + FTF)
  //      G4HadronElasticPhysics     hadronic elastic
  //      G4NeutronHP                high-precision neutrons < 20 MeV
  //      G4DecayPhysics             decays
  //      G4RadioactiveDecayPhysics  radioactive decay
  //      G4StoppingPhysics          at-rest (mu- capture, pi- absorption)
  //      G4IonPhysics               ion transport
  //      G4EmExtraPhysics           muon nuclear, gamma nuclear, synchrotron

  G4VModularPhysicsList* base = new FTFP_BERT();
  for (G4int i = 0; ; ++i)
  {
    G4VPhysicsConstructor* elem =
        const_cast<G4VPhysicsConstructor*>(base->GetPhysics(i));
    if (!elem) break;

    // Skip default EM — replaced below with option4
    if (elem->GetPhysicsName() == "emstandard_opt0") continue;

    RegisterPhysics(elem);
  }
  //delete base;

  // -- EM option4: best muon ionisation + scattering accuracy,
  //    Goudsmit-Saunderson for e±, Monash Compton below 20 MeV,
  //    Penelope ionisation below 100 keV. All relevant for secondaries.
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // -- Step limiter: enabled per-volume in DetectorConstruction via
  //    logVol->SetUserLimits(new G4UserLimits(maxStepSize))
  //    See recommended values below in SetCuts().
  RegisterPhysics(new G4StepLimiterPhysics());

  // -- Optical: scintillation, WLS, Cerenkov, boundary, absorption.
  //    Configure via G4OpticalParameters after registration.
  G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
  G4OpticalParameters* optParams   = G4OpticalParameters::Instance();

  optParams->SetScintFiniteRiseTime(true);
  optParams->SetScintTrackSecondariesFirst(true);
  // optParams->SetCerenkovMaxPhotonsPerStep(300); // uncomment if using Cerenkov
  // optParams->SetWLSTimeProfile("delta");         // or "exponential"

  RegisterPhysics(opticalPhysics);

  defaultCutValue = 1.*mm;
  SetVerboseLevel(1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::~PhysicsList() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructProcess()
{
  G4VModularPhysicsList::ConstructProcess();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCuts()
{
  // Production cuts: secondary particles below these ranges are not tracked.
  // Tighter cuts = more secondaries tracked = slower but more complete.
  //
  // Recommended for muography with secondary studies:
  //   - Rock / absorber volumes : 1 mm  (default — no need to track every delta ray)
  //   - Active detector volumes : 0.1 mm (track more secondaries near detector)
  //   - Very thin detector layers: 0.01 mm
  //
  // Per-region cuts are set in DetectorConstruction via G4Region + G4ProductionCuts.
  // The defaultCutValue (1 mm) applies everywhere no region cut is set.

  SetCutsWithDefault();

  if (verboseLevel > 0) DumpCutValuesTable();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
// G4StepLimiterPhysics — recommended step sizes by volume type
// (set these in DetectorConstruction.cc):
//
//   Rock / bulk absorber:
//     new G4UserLimits(100.*mm)   // large steps fine, muon barely deflects
//
//   Scintillator / active detector:
//     new G4UserLimits(1.*mm)     // resolve light yield and track topology
//
//   Thin foils / windows / PCBs:
//     new G4UserLimits(0.1*mm)    // resolve thin-layer scattering accurately
//
//   Air / vacuum gaps:
//     new G4UserLimits(10.*mm)    // no physics, just geometry crossing
//
// Without G4UserLimits set on a volume, G4StepLimiterPhysics has no effect.
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......