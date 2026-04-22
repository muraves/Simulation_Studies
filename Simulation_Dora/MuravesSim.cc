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
/// \file exampleB5.cc
/// \brief Main program of the basic/B5 example

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "FTFP_BERT.hh"

#include "G4RunManagerFactory.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"
#include "MuravesMessenger.hh"
#include "PhysicsList.hh"
#include "MuSimPhysicsList.hh"
#include "RunInformation.hh"
#include "QGSP_BERT.hh"

//#include "G4MPImanager.hh"
//#include "G4MPIsession.hh"
//#include "G4VMPIseedGenerator.hh"
#include "time.h"
#include <chrono>
using namespace std;
using namespace chrono;

#include "MuravesSim.hh"
DetectorConstruction* theDetector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{
  auto start = high_resolution_clock::now();

  std::cout << "argc = " << argc << std::endl;

    //for (int i = 0; i < argc; i++) {
        //std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    //}

  G4int nThreads = 1;
  G4UIExecutive* ui = nullptr;
  bool openUI = (argc == 1);
  
  /*if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }*/
  auto theMessenger = new MuravesMessenger();

  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  G4Random::setTheSeed(seed);
  // --- Set the seed ---
  //int seed = std::stoi(argv[2]);
  //CLHEP::HepRandom::setTheSeed(seed);
  /*
  auto start = high_resolution_clock::now();
  
  G4MPImanager* g4MPI = new G4MPImanager(argc, argv);
  G4MPIsession* session = g4MPI-> GetMPIsession(); 
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine());
  //G4int rank_ = MPI::COMM_WORLD.Get_rank(); //获取并行进程号
  G4int rank_;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_); 
    time_t systime = time(NULL);
  G4long seed = rank_*10.0+systime;
  G4Random::setTheSeed(seed); 
  */

  // --- Command-line arguments (needs to be improved) ---
  G4String inputfile="";
  
G4int iarg = 1;
  if (argc > 1) {
    while ( iarg < argc ) {
      if (G4String(argv[iarg]).compare("--generator") == 0) {
	if ( ++iarg < argc ) {
	  if ( //(G4String(argv[iarg]).compare("CRY") == 0) // leave CRY out for now
	       (G4String(argv[iarg]).compare("PG") == 0)
	       || (G4String(argv[iarg]).compare("GPS") == 0)  
         || (G4String(argv[iarg]).compare("EcoMug") == 0) ) {
	    theMessenger->SetPrimaryGenerator(G4String(argv[iarg]));
	    iarg++;
	  }
	  else {
	    G4cout << "Unknown generator option " << argv[iarg] << G4endl;
	    return EXIT_FAILURE;
	  }
	} else {
	  G4cout << "Missing generator choice" << G4endl;
	  return EXIT_FAILURE;
	}
      }
      else if (G4String(argv[iarg]).compare("--m") == 0) {
        if (++iarg < argc) {
          inputfile = G4String(argv[iarg]);
          iarg++;
        } else {
          G4cout << "Missing macro file" << G4endl;
          return EXIT_FAILURE;
        }
      }
      else if (G4String(argv[iarg]).compare("--ui") == 0) {
        openUI = true;
        iarg++;
      }
      else if (G4String(argv[iarg])[0] != '-') {
        inputfile = G4String(argv[iarg]);
        iarg++;
      }
      else if (G4String(argv[iarg]).compare("--threads") == 0) {
  if (++iarg < argc) {
    nThreads = std::stoi(argv[iarg]);
    iarg++;
  } else {
    G4cout << "Missing thread count" << G4endl;
    return EXIT_FAILURE;
  }
}
      else {
        G4cout << "Unknown option " << argv[iarg] << G4endl;
        return EXIT_FAILURE;
      }
    } 
  }

    if (openUI) {
  ui = new G4UIExecutive(argc, argv);
}

  // Run manager
  //------------ no need to switch between MT and single-threaded, G4RunManagerType::Default automatically picks the right case
  auto theRunManager = G4RunManagerFactory::CreateRunManager(
  nThreads > 1 ? G4RunManagerType::MTOnly : G4RunManagerType::Serial
);
theRunManager->SetNumberOfThreads(nThreads);
  // UserInitialization classes
  //---------------------------
  //DetectorConstruction* theDetector = new DetectorConstruction("scintillator");
  theDetector = new DetectorConstruction("Muraves");
  theRunManager->SetUserInitialization(theDetector);

  // ************** CHECK PHYSICS LIST FROM CRY GEANT4 EXAMPLE, it doesn't fully work at the moment ***********************
  //PhysicsList* thePhysicsList = new PhysicsList;   
  auto thePhysicsList = new FTFP_BERT;
  //auto thePhysicsList = new CosMuSimPhysicsList;
  //auto thePhysicsList = new PhysicsList;
  thePhysicsList->SetVerboseLevel(1);
  //theRunManager->SetUserInitialization(new PhysicsList);
  theRunManager->SetUserInitialization(thePhysicsList);

  // UserAction classes
  //-------------------
  //theRunManager->SetUserAction(new PrimaryGeneratorAction(""));
  theRunManager->SetUserInitialization(new ActionInitialization(seed));
  
  // Initialize G4 kernel
  //---------------------
  theRunManager->Initialize();

  // Initialize visualization
  //---------------------  
  G4VisManager* theVisManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  theVisManager->Initialize();

  // User interactions
  //------------------
  G4UImanager* UImanager = G4UImanager::GetUIpointer();  

  if (ui) {
  UImanager->ApplyCommand("/control/execute init_vis.mac");
  if (ui->IsGUI()) {
    UImanager->ApplyCommand("/control/execute gui.mac");
  }
  ui->SessionStart();
  delete ui;
}
else if (inputfile != "") {
  G4String command = "/control/execute ";
  UImanager->ApplyCommand(command + inputfile);
}
else {
  G4cout << "No macro file specified." << G4endl;
}

  auto end = high_resolution_clock::now();

  auto duration = duration_cast<milliseconds>(end - start).count();
  
  cout << "Time taken by program: " << duration << " milliseconds" << endl;

  auto hours = duration / (1000 * 60 * 60);
    duration %= (1000 * 60 * 60);

    auto minutes = duration / (1000 * 60);
   duration %= (1000 * 60);

    auto seconds = duration / 1000;
    auto milliseconds = duration % 1000;

    cout << "Time taken by program: "
         << hours << "h "
         << minutes << "m "
         << seconds << "s "
         << milliseconds << "ms" << endl;


  delete theVisManager;
  delete theRunManager;
  return EXIT_SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
