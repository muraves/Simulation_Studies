// 
// Michael Tytgat, Marwa Moussaoui
//

#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"

// Standard headers
#include <vector>
#include <sstream>

// GEANT4 headers
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4Transform3D.hh"
#include "G4GenericTrap.hh"
#include "G4GenericMessenger.hh"
#include "G4TwoVector.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4SDManager.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSDoseDeposit.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include <cmath>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(const char *detectorName)
  : G4VUserDetectorConstruction(),
    fScoringVolume(0),
    _nBars(32), _nModules(2), _nPlanes(1), _nStations(4), _stationSpacing(50 * cm), _barLength(107 * cm), _barHeight(1.7 * cm),
    _barBase(3.3 * cm), _triangEffectiveBase(3.3 * cm), _rotUpperX(NULL), _rotLowerX(NULL), _rotUpperY(NULL), _rotLowerY(NULL),
    _halfContLengthZ(0.), _halfContLengthXY(0.), _looseAccCheck(0.), _detType("triangular")
{
  _messenger = new G4GenericMessenger(this, std::string("/muraves/").append(detectorName).append("/"));
  _messenger->DeclareProperty("nBars", _nBars, "Set the number of scintillating bars per module");
  _messenger->DeclareProperty("nModules", _nModules, "Set the number of modules per plane");
  _messenger->DeclareProperty("nPlanes", _nPlanes, "Set the number of planes per station");
  _messenger->DeclareProperty("nStations", _nStations, "Set the number of XY stations.");
  _messenger->DeclarePropertyWithUnit("barLength","cm", _barLength, "Set the length of the scintillating bars.");
  _messenger->DeclarePropertyWithUnit("barHeight","cm", _barHeight, "Set the height of the scintillating bars.");
  _messenger->DeclarePropertyWithUnit("barBase","cm", _barBase, "Set the size of the base of the scintillating bars.");
  _messenger->DeclarePropertyWithUnit("triangEffBase","cm", _triangEffectiveBase,
				      "Set the effective size of the base of the triangular bars due to cut edges (valid only for triangular detector type).");
  _messenger->DeclareProperty("looseAcceptanceCheck", _looseAccCheck,
			      "Detector-face-enlargement factor for acceptance check (e.g. 1.1 -> enlarge face size by 10% when checking acceptance, 1 -> no enlargement)");
  _messenger->DeclareProperty("detectorType", _detType,
			      "Set the detector type (triangular bars, square bars, monolithic layers)").SetCandidates("triangular square monolithic");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
  delete _messenger;
  delete _rotUpperX;
  delete _rotLowerX;
  delete _rotUpperY;
  delete _rotLowerY;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{  

  //static const char *routineName = "[MuravesDetector::Construct] ";

  float xySafety = 0.1 * mm; //  XY shift to separate bars apart
  float zSafety = 10 * cm; //  Z shift to separate layers apart
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();


    // Envelope parameters
    //
    G4double env_sizeXY = 30*cm, env_sizeZ = 40*cm;
    //G4Material* env_mat = nist->FindOrBuildMaterial("G4_WATER");

    // Option to switch on/off checking of volumes overlaps
    //
    G4bool checkOverlaps = true;

    //     
    // World
    //
    G4double world_sizeXY = 10.*env_sizeXY;
    G4double world_sizeZ  = 10.*env_sizeZ;
    G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

    G4Box* solidWorld =    
        new G4Box("World",                       //its name
		  //               0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);     //its size
		  200*m, 200*m, 200*m);     //its size

    G4LogicalVolume* logicWorld =                         
        new G4LogicalVolume(solidWorld,          //its solid
                world_mat,           //its material
                "World");            //its name

    G4VPhysicalVolume* physWorld = 
        new G4PVPlacement(0,                     //no rotation
                G4ThreeVector(),       //at (0,0,0)
                logicWorld,            //its logical volume
                "World",               //its name
                0,                     //its mother  volume
                false,                 //no boolean operation
                0,                     //copy number
                checkOverlaps);        //overlaps checking


    G4Material* Aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
    //Aluminum shell
    //
    G4Box* solidAlShellOut =    
        new G4Box("AlShell",                       //its name
                0.5*1.2*m, 0.5*1.2*m, 0.5*10*cm);     //its size
    G4Box* solidAlShellIn =    
        new G4Box("AlShell",                       //its name
                0.5*1.10*m, 0.5*1.1*m, 0.5*10*cm);     //its size
    G4SubtractionSolid* solidAlShell = new G4SubtractionSolid("solidAlShell",solidAlShellOut,solidAlShellIn);

    G4LogicalVolume* logicAlShell =                         
        new G4LogicalVolume(solidAlShell,          //its solid
                Aluminum_mat,           //its material
                "AlShell");            //its name
    
    //Aluminum foils
    
    
    G4Box* solidAlFoil =    
        new G4Box("AlFoil",                       //its name
                0.5*_barLength, 0.5*_barLength, 0.5*0.15*cm);     //its size

    G4LogicalVolume* logicAlFoil =                         
        new G4LogicalVolume(solidAlFoil,          //its solid
                Aluminum_mat,           //its material
                "AlFoil");            //its name

    
    //TEC
    
    G4Box* solidTEC =    
        new G4Box("TEC",                       //its name
                0.5*30*cm, 0.5*20*cm, 0.5*0.15*cm);     //its size

    G4LogicalVolume* logicTEC =                         
        new G4LogicalVolume(solidTEC,          //its solid
                Aluminum_mat,           //its material
                "TEC");            //its name

    //scintillator bar
    G4Material* airMaterial = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* scintMaterial = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"); //density = 1.032 g/cm3


    G4ThreeVector pos3 = G4ThreeVector(0*cm, 0*cm, 0*cm);



    // 1. Apply the necessary quirks for specific detector types
    if (_detType != "triangular")
        _triangEffectiveBase = _barBase;
    // 2. Build the logical box containing the detector
    _halfContLengthXY = (std::max(_barLength, (_barBase + xySafety) * (_nBars + 0.5))) / 2.;
    std::cout << "[MuravesDetector::Construct] _halfContLengthXY=" << _halfContLengthXY << std::endl;
    // 2.1 compute the Z height of a layer, i.e. of a single view (X or Y) of a station
    float layerThickness = _barHeight + (_barHeight / (_barBase / 2.) * ((_barBase - _triangEffectiveBase) / 2.)); // Z offset due to cut edges at the base
    std::cout << "[MuravesDetector::Construct] layerThickness=" << layerThickness << std::endl;
    _halfContLengthZ = (float) (_nStations - 1) / 2 * _stationSpacing + (layerThickness + zSafety / 2);

    std::cout << "[MuravesDetector::Construct] _halContLengthZ=" << _halfContLengthZ << std::endl;

    //2.2 construct detector geometry
    G4Box *detContainerSolid = 
        new G4Box("Detector",
                _halfContLengthXY * 1.05,
                _halfContLengthXY * 1.05,
                _halfContLengthZ * 1.05);

    G4LogicalVolume *detContainerLog = 
        new G4LogicalVolume(detContainerSolid, 
                airMaterial, 
                "Detector",
                NULL,
                NULL,
                NULL,
                false);
    /*new G4PVPlacement(0,                       //no rotation
      pos3,                    //at position
      detContainerLog,             //its logical volume
      "Detector",                //its name
      logicWorld,                //its mother  volume
      false,                   //no boolean operation
      0,                       //copy number
      checkOverlaps);   */       //overlaps checking


    // 2. Build the scintillating bar
    std::vector<G4TwoVector> vertices;
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //0
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.)); //1
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.)); //2
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));  //3
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //4
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.)); //5
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.)); //6
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //7

    G4LogicalVolume *barLog = NULL;
    if (_detType == "triangular") {
        G4GenericTrap* triangSolid = new G4GenericTrap("triangSolid",
                _barLength / 2,
                vertices);
        if (_triangEffectiveBase < _barBase) {
            // Cut edges
            G4Box * boxSolid = new G4Box("boxSolid",
                    _triangEffectiveBase / 2.,
                    _barHeight / 2.,
                    _barLength / 2.);
            G4IntersectionSolid* triangCutEdgeSolid = new G4IntersectionSolid("boxSolid*triangSolid",
                    boxSolid,
                    triangSolid);
            barLog = new G4LogicalVolume(triangCutEdgeSolid,
                    scintMaterial,
                    "BARSH2E",
                    NULL,
                    NULL,
                    NULL,
                    false);
        }
        else {
            // Use plain G4Trap
            barLog = new G4LogicalVolume(triangSolid,
                    scintMaterial, 
                    "BARSH2E", 
                    NULL, 
                    NULL, 
                    NULL, 
                    false);
        }
    }

    _triangEffectiveBase += xySafety; // Add safety margin

    // 3. Build the stations
    double posFirstBarMod0 = -((float) _nBars - 0.5) * _triangEffectiveBase / 2.; //position of first bar (i.e., bar at most-negative coordinate)
    double posFirstBarMod1 = posFirstBarMod0 + (_nBars) * _triangEffectiveBase/2. ;
    double posFirstBar[] = {posFirstBarMod0,posFirstBarMod1};

    std::stringstream barName;
    std::stringstream moduleName;
    std::stringstream planeName;
    std::stringstream stationName;

    // Transformation to the detector frame: stations are formed by two X-Y layers. Each layer, called plane is formed by two modules made each with 16 + 16 upper and lower row of triangular scintillating bars.
    // Note: X bars can be put in position with a single rotation about the X axis (bar frame). Y bars will need an additional
    //       rotation about the Y axis (bar frame) before the one about X axis (bar frame). To this end, note that Geant seems
    //       to "right multiply" the position vector by the transformation matrix, i.e., v' = v*A. So, in order to first rotate
    //       about Y and then about X, one obtains the net transformation as rotY*rotX.
    _rotUpperX = new G4RotationMatrix();
    _rotUpperX->rotateX(90 * deg);

    _rotLowerX = new G4RotationMatrix();
    _rotLowerX->rotateX(-90 * deg);

    _rotX = new G4RotationMatrix();
    _rotX->rotateX(+180 * deg);
    
    _rotUpperY = new G4RotationMatrix();
    _rotUpperY->rotateY(-90 * deg);
    *_rotUpperY *= *_rotUpperX;

    _rotLowerY = new G4RotationMatrix();
    _rotLowerY->rotateY(90 * deg);
    *_rotLowerY *= *_rotLowerX;

    _rotZ = new G4RotationMatrix();
    _rotZ->rotateZ(90* deg);
    //*_rotZ *= *_rotX;

    float zOffset = layerThickness / 2. - _barHeight / 2.;

    float zPosStation0 = - layerThickness / 2.; // Z position of upper X station
    float zPosStation1 = zPosStation0 - 26*cm; // Z position of upper X station
    float zPosStation2 = zPosStation0 + 26.2*cm; // Z position of upper X station
    float zPosStation3 = zPosStation0 + 147.5*cm; // Z position of upper X station
    float zPosStationX[] = {zPosStation0, zPosStation1, zPosStation2, zPosStation3};

    float yPosStation0 = 0*cm ; // y position of upper X station
    float yPosStation1 = 4*cm; // y position of upper X station
    float yPosStation2 = -4.5*cm; // y position of upper X station
    float yPosStation3 = -24.9*cm; // y position of upper X station
    float yPosStation[] = {yPosStation0, yPosStation1, yPosStation2, yPosStation3};
    

    // Implement scintillator bars copy number as bitpattern
    // bit 0-7 -> iBar number
    // bit 8-11 -> iModule
    // bit 12-14 -> iStation
    // bit 15 -> X:0, Y:1
    G4int BarCopyNo = 0;
    
    // 3.1 X Plane
    for (G4int iStation=0; iStation<_nStations;iStation++) {
      stationName << iStation;
      G4VPhysicalVolume* physAlShell = 
	new G4PVPlacement(0,                     //no rotation
			  G4ThreeVector(0,yPosStation[iStation],zPosStationX[iStation]),       //at (0,0,0)
			  logicAlShell,            //its logical volume
			  "AlShell",               //its name
			  logicWorld,                     //its mother  volume
			  false,                 //no boolean operation
			  0,                     //copy number
			  checkOverlaps);        //overlaps checking
      G4VPhysicalVolume* physTEC = 
	new G4PVPlacement(0,                     //no rotation
			  G4ThreeVector(0,yPosStation[iStation]+20*cm,zPosStationX[iStation]+5.5*cm),       //at (0,0,0)
			  logicTEC,            //its logical volume
			  "TEC",               //its name
			  logicWorld,                     //its mother  volume
			  false,                 //no boolean operation
			  0,                     //copy number
			  checkOverlaps);        //overlaps checking
      

      //std::cout << routineName << "Placing upper X layer at z=" << zPosStationX[iStation] + zOffset << std::endl;
      for (G4int iModule = 0; iModule < _nModules;iModule++){
	moduleName << iModule;
	G4VPhysicalVolume* physAlFoil = 
	  new G4PVPlacement(0,                     //no rotation
			    G4ThreeVector(0,yPosStation[iStation],zPosStationX[iStation] + (std::pow(-1,iModule))*((layerThickness/2)+(zOffset/2))),       //at (0,0,0)
			    logicAlFoil,            //its logical volume
			    "AlFoil",               //its name
			    logicWorld,                     //its mother  volume
			    false,                 //no boolean operation
			    2,                     //copy number
			    checkOverlaps);        //overlaps checking

	for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
	  barName << 2 * iBar;
	  BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);
	  // 3.1.1 upper half-layer
	  // Bar 0 is at most-negative X coordinates
	  new G4PVPlacement(_rotUpperX,
			    G4ThreeVector(posFirstBar[iModule] + iBar * _triangEffectiveBase, yPosStation[iStation], zPosStationX[iStation] + zOffset),                
			    barLog,
			    "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),      
			    logicWorld,
			    false,
			    //2 * iBar);
			    BarCopyNo);
	  
	  barName.str("");
	}
	for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
	  barName << 2 * iBar + 1;
	  BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);
	  // 3.1.2 lower half-layer
	  new G4PVPlacement(_rotLowerX,
			    G4ThreeVector(posFirstBar[iModule] + (iBar + 0.5) * _triangEffectiveBase,yPosStation[iStation],zPosStationX[iStation] - zOffset),
			    barLog,
			    "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
			    logicWorld,
			    false,
			    //2 * iBar + 1);
			    BarCopyNo);
	  
	  barName.str("");
	}
	moduleName.str("");
      }
      stationName.str("");
    }


    


      zPosStation0 = - layerThickness / 2. - layerThickness - zSafety; // Z position of upper Y station
      zPosStation1 = zPosStation0 - 26*cm ; // Z position of upper Y station
      zPosStation2 = zPosStation0 + 26.2*cm ; // Z position of upper Y station
      zPosStation3 = zPosStation0 + 147.5*cm ; // Z position of upper Y station


      float zPosStationY[] = {zPosStation0, zPosStation1, zPosStation2, zPosStation3};


    // 3.2 Y view
      for (G4int iStation=0; iStation<_nStations;iStation++){
	stationName << iStation;
	
	new G4PVPlacement(0,                     
			  G4ThreeVector(0,yPosStation[iStation],zPosStationY[iStation]),
			  logicAlShell,            
			  "AlShell",               
			  logicWorld,                     
			  false,                
			  0,                     
			  checkOverlaps);        
	
	G4VPhysicalVolume* physTEC = 
	  new G4PVPlacement(_rotZ,                     
			    G4ThreeVector(0,yPosStation[iStation]+20*cm,zPosStationY[iStation]-5.5*cm),       
			    logicTEC,            
			    "TEC",               
			    logicWorld,                     
			    false,                 
			    0,                     
			    checkOverlaps);        
        for (G4int iModule = 0; iModule < _nModules;iModule++){
	  moduleName << iModule;
	  G4VPhysicalVolume* physAlFoil = 
	    new G4PVPlacement(0,                     
			      G4ThreeVector(0,yPosStation[iStation],zPosStationY[iStation] + (std::pow(-1,iModule))*((layerThickness/2)+(zOffset/2))),       
			      logicAlFoil,            
			      "AlFoil",               
			      logicWorld,                     
			      false,                 
			      2,                     
			      checkOverlaps);
	  for (G4int iBar = 0; iBar <_nBars/2; iBar++) {
	    barName << 2 * iBar;
	    BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);
	    // 3.2.1 upper half-layer
	    //       Bar 0 is at most-negative Y coordinates
	    new G4PVPlacement(_rotUpperY,
			      G4ThreeVector(0, posFirstBar[iModule] + iBar * _triangEffectiveBase +yPosStation[iStation], zPosStationY[iStation] + zOffset ),
			      barLog,
			      "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
			      logicWorld,
			      false,
			      //2 * iBar);
			      BarCopyNo);
	    barName.str("");
          }
	  for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
	    barName << 2 * iBar + 1;
	    BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);
	    // 3.2.2 lower half-layer
	    new G4PVPlacement(_rotLowerY,
			      G4ThreeVector(0, (posFirstBar[iModule] + (iBar + 0.5) * _triangEffectiveBase)+yPosStation[iStation], zPosStationY[iStation] - zOffset ),
			      barLog,
			      "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
			      logicWorld,
			      false,
			      //2 * iBar + 1);
			      BarCopyNo);
	    
	    barName.str("");
	  }
	  moduleName.str("");
	}
	stationName.str("");
	//zPosStation -= _stationSpacing;
      }


      //construct Lead block
      G4Material* lead_mat = nist->FindOrBuildMaterial("G4_Pb");
      G4ThreeVector posLead = G4ThreeVector(0*cm, -(25)*cm, (zPosStation0 + (96.2*cm)));

      G4Box *leadSolid = 
          new G4Box("LeadBlock",
                  (130/2)*cm,
                  (120/2)*cm,
                  (60/2)*cm);

      G4LogicalVolume *leadLog = 
          new G4LogicalVolume(leadSolid, 
                  lead_mat, 
                  "LeadBlock",
                  NULL,
                  NULL,
                  NULL,
                  false);
      new G4PVPlacement(0,                       //no rotation
              posLead,                    //at position
              leadLog,             //its logical volume
              "LeadBlock",                //its name
              logicWorld,                //its mother  volume
              false,                   //no boolean operation
              0,                       //copy number
              checkOverlaps);          //overlaps checking


      //method 1
      fScoringVolume=barLog;

      // visualization attributes
      G4VisAttributes* whitecol = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
      G4VisAttributes* pinkcol = new G4VisAttributes(G4Colour(0.6,0.0,0.6));
      G4VisAttributes* graycol = new G4VisAttributes(G4Colour(0.9,0.9,0.9));
      G4VisAttributes* cyancol = new G4VisAttributes(G4Colour(0.0,1.0,1.0,0.3));
      G4VisAttributes* redcol = new G4VisAttributes(G4Colour(0.5,0.0,0.0));
      G4VisAttributes* darkgraycol = new G4VisAttributes(G4Colour(0.8,0.8,0.8));
      G4VisAttributes* orangecol = new G4VisAttributes(G4Colour(0.8,0.5,0.));
      G4VisAttributes* yellowcol = new G4VisAttributes(G4Colour(1.0,1.0,0.));
      G4VisAttributes* greencol = new G4VisAttributes(G4Colour(0.,1.0,0.,0.4));
      G4VisAttributes* bluecol = new G4VisAttributes(G4Colour(0.,0.,0.8));

      logicAlFoil->SetVisAttributes(bluecol);
      logicAlShell->SetVisAttributes(cyancol);
      logicTEC->SetVisAttributes(greencol);

      //
      //always return the physical World
      //
      return physWorld;
}

//method 2
void DetectorConstruction::ConstructSDandField()
{
  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
  G4String SDname;
  
  /*
    // approach with scorers
  // declare scintillator  as a MultiFunctionalDetector scorer
  //  
  //G4MultiFunctionalDetector* barScint = new G4MultiFunctionalDetector("stationX1bar25");
  G4MultiFunctionalDetector* barScint = new G4MultiFunctionalDetector("Scintbars");
  G4SDManager::GetSDMpointer()->AddNewDetector(barScint);
  G4VPrimitiveScorer* Edep = new G4PSEnergyDeposit("Edep");
  barScint->RegisterPrimitive(Edep);
  SetSensitiveDetector("BARSH2E",barScint);
  */

  // approach with hits
  auto Scintbars = new ScintbarSD(SDname="/Scintbars");
  G4SDManager::GetSDMpointer()->AddNewDetector(Scintbars);
  //fScoringVolume->SetSensitiveDetector(Scintbars);
  GetScoringVolume()->SetSensitiveDetector(Scintbars);
  
}


// Track must hit front and back layer
G4bool DetectorConstruction::IsInsideAcceptance(const G4ThreeVector &pos, const G4ThreeVector &dir) const
{
  // 1. Reject particles not entering from front side
  if (pos[2] < _halfContLengthZ)
    return false;

  // 2. Reject particles not hitting the front side
  float hitPoint = fabs(pos[0] + (dir[0] / dir[2]) * (_halfContLengthZ - pos[2])); // X coordinate
  if (hitPoint > _halfContLengthXY * _looseAccCheck)
    return false;
  hitPoint = fabs(pos[1] + (dir[1] / dir[2]) * (_halfContLengthZ - pos[2])); // Y coordinate
  if (hitPoint > _halfContLengthXY * _looseAccCheck)
    return false;

  // 3. Reject particles not hitting the back side
  hitPoint = fabs(pos[0] + (dir[0] / dir[2]) * (-_halfContLengthZ - pos[2])); // X coordinate
  if (hitPoint > _halfContLengthXY * _looseAccCheck)
    return false;
  hitPoint = fabs(pos[1] + (dir[1] / dir[2]) * (-_halfContLengthZ - pos[2])); // Y coordinate
  if (hitPoint > _halfContLengthXY * _looseAccCheck)
    return false;

  return true;
}


void DetectorConstruction::DeleteMessenger()
{
  delete _messenger;
  _messenger = NULL;
}

