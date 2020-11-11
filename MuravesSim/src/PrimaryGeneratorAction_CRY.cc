//******************************************************************************
// PrimaryGeneratorAction.cc
//
// 1.00 JMV, LLNL, Jan-2007:  First version.
//******************************************************************************
//

#include <iomanip>
#include "PrimaryGeneratorAction_CRY.hh"
#include "DetectorConstruction.hh"
#include "MuravesSim.hh"

using namespace std;
using namespace CLHEP;

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4RotationMatrix.hh"
#include "G4ParticleGun.hh"

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_CRY::PrimaryGeneratorAction_CRY(const char *inputfile)
{
  // define a particle gun
  particleGun = new G4ParticleGun();

  // Determine CRY data directory path
  char *str;
  std::string dataString("");
  if ((str = getenv("CRYDATAPATH")) != NULL) {
    dataString += std::string(str);
  }
  else {
    dataString += std::string("../data");
  }

  // Read the cry input file
  std::ifstream inputFile;
  inputFile.open(inputfile,std::ios::in);
  char buffer[1000];

  if (inputFile.fail()) {
    if( *inputfile !=0)  //....only complain if a filename was given
      G4cout << "PrimaryGeneratorAction: Failed to open CRY input file= "
	     << inputfile << G4endl;
    InputState=-1;
  }else{
    std::string setupString("");
    while ( !inputFile.getline(buffer,1000).eof()) {
      setupString.append(buffer);
      setupString.append(" ");
    }

    cout << "CRY data directory: " << dataString << endl;
    
    //CRYSetup *setup=new CRYSetup(setupString,"../data");
    CRYSetup *setup=new CRYSetup(setupString, dataString);

    gen = new CRYGenerator(setup);

    // set random number generator
    RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
    setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
    InputState=0;
  }
  // create a vector to store the CRY particle properties
  vect=new std::vector<CRYParticle*>;

  // Create the table containing all particle names
  particleTable = G4ParticleTable::GetParticleTable();

  // Create the messenger file
  gunMessenger = new PrimaryGeneratorMessenger(this);
}

//----------------------------------------------------------------------------//
PrimaryGeneratorAction_CRY::~PrimaryGeneratorAction_CRY()
{
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_CRY::InputCRY()
{
  InputState=1;
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_CRY::UpdateCRY(std::string* MessInput)
{
  char *str;
  std::string dataString("");
  if ((str = getenv("CRYDATAPATH")) != NULL) {
    dataString += std::string(str);
  }
  else {
    dataString += std::string("../data");
  }
  
  //CRYSetup *setup=new CRYSetup(*MessInput,"../data");
  CRYSetup *setup=new CRYSetup(*MessInput, dataString);

  gen = new CRYGenerator(setup);

  // set random number generator
  RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
  setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
  InputState=0;

}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_CRY::CRYFromFile(G4String newValue)
{
  char *str;
  std::string dataString("");
  if ((str = getenv("CRYDATAPATH")) != NULL) {
    dataString += std::string(str);
  }
  else {
    dataString += std::string("../data");
  }
  
  // Read the cry input file
  std::ifstream inputFile;
  inputFile.open(newValue,std::ios::in);
  char buffer[1000];

  if (inputFile.fail()) {
    G4cout << "Failed to open input file " << newValue << G4endl;
    G4cout << "Make sure to define the cry library on the command line" << G4endl;
    InputState=-1;
  }else{
    std::string setupString("");
    while ( !inputFile.getline(buffer,1000).eof()) {
      setupString.append(buffer);
      setupString.append(" ");
    }

    //CRYSetup *setup=new CRYSetup(setupString,"../data");
    CRYSetup *setup=new CRYSetup(setupString, dataString);

    gen = new CRYGenerator(setup);

  // set random number generator
    RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(),&CLHEP::HepRandomEngine::flat);
    setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
    InputState=0;
  }
}

//----------------------------------------------------------------------------//
void PrimaryGeneratorAction_CRY::GeneratePrimaries(G4Event* anEvent)
{ 
  if (InputState != 0) {
    G4String* str = new G4String("CRY library was not successfully initialized");
    //G4Exception(*str);
    G4Exception("PrimaryGeneratorAction", "1",
                RunMustBeAborted, *str);
  }
  G4String particleName;

  G4bool InAcceptance=false;

  //  auto mydetector = G4RunManager::GetRunManager()->GetUserDetectorConstruction();

  vect->clear();
  gen->genEvent(vect);

  auto _rotX = new G4RotationMatrix();
  _rotX->rotateX(-90 * deg);
  
  //....debug output
  G4cout << "\nEvent=" << anEvent->GetEventID() << " "
         << "CRY generated nparticles=" << vect->size()
         << G4endl;

  // CRY uses the following units:
  // energy: MeV; position: m; time: seconds
  // Geant4 uses standard units:
  // energy: MeV; position: mm 
 
  for ( unsigned j = 0; j < vect->size(); j++) {

    // shifting the CRY particles vertically upward by 2m (otherwise they are generated inside detector ...)
    G4ThreeVector ppos = G4ThreeVector((*vect)[j]->x()*m, (*vect)[j]->y()*m, (((*vect)[j]->z())+2.)*m);
    G4ThreeVector pmom = G4ThreeVector((*vect)[j]->u(), (*vect)[j]->v(), (*vect)[j]->w());

    // rotate from CRY to Geant4 xyz reference system
    ppos.transform(*_rotX);
    pmom.transform(*_rotX);
    
    particleName=CRYUtils::partName((*vect)[j]->id());

    //....debug output  
    cout << "  "          << particleName << " "
         << "charge="      << (*vect)[j]->charge() << " "
         << setprecision(4)
         << "energy (MeV)=" << (*vect)[j]->ke() << " "
         << "pos (m)"
         << G4ThreeVector((*vect)[j]->x(), (*vect)[j]->y(), (*vect)[j]->z()) 
	 << " (mm)" << ppos 
         << " " << "direction cosines "
         << G4ThreeVector((*vect)[j]->u(), (*vect)[j]->v(), (*vect)[j]->w())
	 << " " << pmom
         << endl;

    // inside acceptance ?
    //    if ( theDetector->IsInsideAcceptance( G4ThreeVector((*vect)[j]->x(), (*vect)[j]->y(), (*vect)[j]->z()),
    //					  G4ThreeVector((*vect)[j]->u(), (*vect)[j]->v(), (*vect)[j]->w()) ) ) {
    if ( theDetector->IsInsideAcceptance( ppos, pmom ) ) {
      G4cout << "Yes, inside acceptance !" << endl;
    }
    
    particleGun->SetParticleDefinition(particleTable->FindParticle((*vect)[j]->PDGid()));
    particleGun->SetParticleEnergy(((*vect)[j]->ke())*MeV);
    //particleGun->SetParticlePosition(G4ThreeVector((*vect)[j]->x()*m, (*vect)[j]->y()*m, (*vect)[j]->z()*m));
    particleGun->SetParticlePosition(ppos);
    //particleGun->SetParticleMomentumDirection(G4ThreeVector((*vect)[j]->u(), (*vect)[j]->v(), (*vect)[j]->w()));
    particleGun->SetParticleMomentumDirection(pmom);
    particleGun->SetParticleTime((*vect)[j]->t());
    particleGun->GeneratePrimaryVertex(anEvent);
    //G4cout << "Particle position readback: " << particleGun->GetParticlePosition() << G4endl;
    //G4cout << "Particle energy readback: " << particleGun->GetParticleEnergy() << G4endl;
    delete (*vect)[j];   // not sure we need to do this ...
  }

  delete _rotX;
}
