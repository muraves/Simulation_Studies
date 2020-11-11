//******************************************************************************
// MuravesSim.cc
//
// a GEANT4 based Muraves simulation
//
// Michael Tytgat
//******************************************************************************
//

// misc includes
//
#include <fstream>
#include <math.h>
#include "G4ios.hh"

// package includes
//
//#ifdef G4MULTITHREADED
//#include "G4MTRunManager.hh"
//#else
#include "G4RunManager.hh"
//#endif

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"
#include "ROOTManager.hh"
#include "MuravesMessenger.hh"

// geant4 includes
//
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "FTFP_BERT.hh"

using namespace std;

#include "MuravesSim.hh"
DetectorConstruction* theDetector;

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

  auto theMessenger = new MuravesMessenger();

  G4String inputfile="";
  
  G4int iarg = 1;
  if (argc > 1) {
    while ( iarg < argc ) {
      if (G4String(argv[iarg]).compare("--generator") == 0) {
	if ( ++iarg < argc ) {
	  if ( (G4String(argv[iarg]).compare("CRY") == 0)
	       || (G4String(argv[iarg]).compare("PartGun") == 0)
	       || (G4String(argv[iarg]).compare("GPS") == 0) ) {
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
      else if (G4String(argv[iarg]).compare("--inputfile") == 0) {
	if ( ++iarg < argc ) {
	  inputfile = G4String(argv[iarg]);
	  iarg++;
	} else {
	  G4cout << "Missing inputfile" << G4endl;
	  return EXIT_FAILURE;
	}
      }
      else {
	G4cout << "Unknown option " << argv[iarg] << G4endl;
	return EXIT_FAILURE;
      }      
    } 
  }

  // Run manager
  //------------
  //#ifdef G4MULTITHREADED
  //G4MTRunManager* theRunManager = new G4MTRunManager;
  //#else  
  G4RunManager* theRunManager = new G4RunManager;
  //#endif

  // UserInitialization classes
  //---------------------------
  //DetectorConstruction* theDetector = new DetectorConstruction("scintillator");
  theDetector = new DetectorConstruction("Muraves");

  // ************** CHECK PHYSICS LIST FROM CRY GEANT4 EXAMPLE, it doesn't fully work at the moment ***********************
  //PhysicsList* thePhysicsList = new PhysicsList;   
  auto thePhysicsList = new FTFP_BERT;
  thePhysicsList->SetVerboseLevel(1);

  // ROOT output
  auto theROOTmanager = new ROOTManager();

  // UserAction classes
  //-------------------
  theRunManager->SetUserInitialization(theDetector);
  theRunManager->SetUserInitialization(thePhysicsList);
  //theRunManager->SetUserAction(new PrimaryGeneratorAction(""));
  theRunManager->SetUserInitialization(new ActionInitialization());
  
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
  G4UImanager* UI = G4UImanager::GetUIpointer();  

  //if (argc > 1) {  //....geant command file specified on command line
  //  UI->ApplyCommand("/control/execute "+G4String(argv[1]));
  if ( inputfile != "" ) {
    UI->ApplyCommand("/control/execute " + inputfile);
  } else {           //....no command file specified, Start interactive session 
    UI->ApplyCommand("/control/execute init_vis.mac");
    //G4UIsession* theUIsession = new G4UIterminal(new G4UItcsh);
    G4UIExecutive* theUIsession = new G4UIExecutive(argc, argv);
    theUIsession->SessionStart();
    delete theUIsession;
  }

  delete theVisManager;
  delete theRunManager;
  return EXIT_SUCCESS;
}
