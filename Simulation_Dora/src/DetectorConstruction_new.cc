/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4FieldManager.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVParameterised.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4ExtrudedSolid.hh"
#include "G4GenericTrap.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Element.hh"

#include "ScintBarSD.hh"
#include "Materials.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//G4ThreadLocal G4FieldManager* DetectorConstruction::fFieldMgr = nullptr;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(const char *detectorName)
  : G4VUserDetectorConstruction(),
    _nBars(32), _nModules(2), _nStations(4), _stationSpacing(50 * cm), _barLength(107 * cm), _barHeight(1.7 * cm),
    _barBase(3.2 * cm), _triangEffectiveBase(3.2 * cm), _rotUpperX(NULL), _rotLowerX(NULL), _rotUpperY(NULL), _rotLowerY(NULL),
    _halfContLengthZ(0.), _halfContLengthXY(0.), _looseAccCheck(0.), _detType("triangular")
{
  _messenger = new G4GenericMessenger(this, std::string("/muraves/").append(detectorName).append("/"));
  _messenger->DeclareProperty("nBars", _nBars, "Set the number of scintillating bars per module");
  _messenger->DeclareProperty("nModules", _nModules, "Set the number of modules per plane");
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

  Materials::makeMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
  delete _messenger;
  delete _rotUpperX;
  delete _rotLowerX;
  delete _rotUpperY;
  delete _rotLowerY;
  //if(fMaterials)  delete fMaterials; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  //sfMaterials = Materials::GetInstance();

  xySafety = 0.5 * mm; //  XY shift to separate bars apart
  zSafety = 5.4 * cm; //  Z shift to separate layers apart
  
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* air_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* Aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
  G4Material* polystyrene_mat = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // standard polystyrene
  G4Material* rock_mat = new G4Material("StandardRock", 11.0, 22.0*g/mole, 2.65*g/cm3);
  //G4Material* polystyrene_mat = FindMaterial(Materials::kPOLYSTYRENE); // with costum scintillator properties

  G4bool checkOverlaps = true; // Option to switch on/off checking of volumes overlaps

       
    // ------------------- World -------------------
    
    G4double world_sizeX = 5.*m;
    G4double world_sizeY = 2.*m;
    G4double world_sizeZ  = 3.*m;
    
    G4Box* solidWorld = new G4Box("World", 0.5*world_sizeX, 0.5*world_sizeY, 0.5*world_sizeZ);    

    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air_mat, "World");            

    G4VPhysicalVolume* physWorld = 
        new G4PVPlacement(0,           //no rotation
                G4ThreeVector(),       //at (0,0,0)
                logicWorld,            //its logical volume
                "World",               //its name
                0,                     //its mother  volume
                false,                 //no boolean operation
                0,                     //copy number
                checkOverlaps);        //overlaps checking
      
    
    //------------------- Aluminum foil -------------------
    
    G4double _AlFoilThickness = 0.15*cm;

    G4Box* solidAlFoil = new G4Box("AlFoil", 0.5*_barLength, 0.5*_barLength, 0.5*_AlFoilThickness); 

    G4LogicalVolume* logicAlFoil = new G4LogicalVolume(solidAlFoil, Aluminum_mat, "AlFoil");  

    //------------------- TEC -------------------
    
    G4double TEC_thickness = 0.15*cm;

    G4Box* solidTEC = new G4Box("TEC", 0.5*30*cm, 0.5*20*cm, 0.5*TEC_thickness);   

    G4LogicalVolume* logicTEC = new G4LogicalVolume(solidTEC, Aluminum_mat, "TEC");          

    //------------------- Scintillator bar -------------------
    
    _detType = "triangular";
    _looseAccCheck = 0.;

    // 1. Apply the necessary quirks for specific detector types -------------------
    if (_detType != "triangular")
        _triangEffectiveBase = _barBase;
    // 2. Build the logical box containing the detector -------------------
    _halfContLengthXY = (std::max(_barLength, (_barBase + xySafety) * (_nBars + 0.5))) / 2.;
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars + 0.5) / 2. = " << (_barBase + xySafety) * (_nBars + 0.5) / 2. << G4endl;
    G4cout << "[MuravesDetector::Construct] ((_barBase + xySafety) * (_nBars + 0.5)-xySafety) / 2. = " << ((_barBase + xySafety) * (_nBars + 0.5)-xySafety) / 2. << G4endl;
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars/2 + 0.5) - xySafety/2 = " << (_barBase + xySafety) * (_nBars/2 + 0.5) - xySafety/2 << G4endl;
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars/2) - xySafety = " << (_barBase + xySafety) * (_nBars/2) - xySafety << G4endl;
    G4cout << "[MuravesDetector::Construct] _halfContLengthXY = " << _halfContLengthXY << G4endl;
    // 2.1 compute the Z height of a layer, i.e. of a single view (X or Y) of a station -------------------
    float layerThickness = _barHeight + (_barHeight / (_barBase / 2.) * ((_barBase - _triangEffectiveBase) / 2.)); // Z offset due to cut edges at the base
    std::cout << "[MuravesDetector::Construct] _barHeight=" << _barHeight << std::endl;
    std::cout << "[MuravesDetector::Construct] _barBase=" << _barBase << std::endl;
    std::cout << "[MuravesDetector::Construct] __triangEffectiveBase=" << _triangEffectiveBase << std::endl;
    std::cout << "[MuravesDetector::Construct] layerThickness=" << layerThickness << std::endl;
    _halfContLengthZ = (float) (_nStations - 1) / 2 * _stationSpacing + (layerThickness + zSafety / 2);

    std::cout << "[MuravesDetector::Construct] _halContLengthZ=" << _halfContLengthZ << std::endl;

    //------------------- Aluminum shell -------------------
  
    //G4double _AlShellHeight = layerThickness + zSafety; // Thickness (height) of aluminum shell box
    G4double _AlShellHeight = zSafety;
    G4double AlShellThickness = 1.2*m;
    
    G4Box* solidAlShellOut = new G4Box("AlShell", 0.5*AlShellThickness, 0.5*AlShellThickness, 0.5*_AlShellHeight);    
    G4Box* solidAlShellIn =  new G4Box("AlShell", 0.5*1.10*m, 0.5*1.1*m, 0.5*(_AlShellHeight-2*_AlFoilThickness));    
    G4SubtractionSolid* solidAlShell = new G4SubtractionSolid("solidAlShell",solidAlShellOut,solidAlShellIn);

    G4LogicalVolume* logicAlShell = new G4LogicalVolume(solidAlShell, Aluminum_mat, "AlShell");   

    // 3. Build the scintillating bar -------------------
    std::vector<G4TwoVector> vertices;
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //0
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.)); //1
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.)); //2
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));  //3
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //4
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.)); //5
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.)); //6
    vertices.push_back(G4TwoVector(0, _barHeight / 2.)); //7

    if (_detType == "triangular") {
        G4GenericTrap* triangSolid = new G4GenericTrap("triangSolid", _barLength / 2, vertices);

        if (_triangEffectiveBase < _barBase) {
            // Cut edges
            G4Box * boxSolid = new G4Box("boxSolid", _triangEffectiveBase / 2., _barHeight / 2., _barLength / 2.);
            G4IntersectionSolid* triangCutEdgeSolid = new G4IntersectionSolid("boxSolid*triangSolid", boxSolid, triangSolid);

            barLog = new G4LogicalVolume(triangCutEdgeSolid,
                    polystyrene_mat,
                    "BARSH2E",
                    NULL, // field manager
                    NULL, // sensitive detector
                    NULL, // user-defined limits for particles in this volume
                    false); 
        }
        else {
            // Use plain G4Trap
            barLog = new G4LogicalVolume(triangSolid,
                    polystyrene_mat,
                    "BARSH2E",
                    NULL, 
                    NULL, 
                    NULL, 
                    false);
        }
    }

_triangEffectiveBase     += xySafety; // Add safety margin

    // 4. Build the stations -------------------
    double posFirstBarMod0 = -((float) _nBars -0.5) * _triangEffectiveBase / 2.; //position of first bar (i.e., bar at most-negative coordinate)
    G4cout << "[MuravesDetector::Construct] -((float) _nBars -0.5) * _triangEffectiveBase / 2. = " << posFirstBarMod0 << G4endl;
    double posFirstBarMod1 = posFirstBarMod0 + (_nBars) * _triangEffectiveBase/2. ;
    G4cout << "[MuravesDetector::Construct] posFirstBarMod0 + (_nBars) * _triangEffectiveBase/2. = " << posFirstBarMod1 << G4endl;
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

    float zOffset = layerThickness / 2. - _barHeight / 2.;

    float zPosStation0 = - layerThickness / 2.; // Z position of upper X station
    float zPosStation1 = zPosStation0 - 26.7*cm; // Z position of upper X station
    float zPosStation2 = zPosStation0 + 26.*cm; // Z position of upper X station
    float zPosStation3 = zPosStation0 + 147.5*cm; // Z position of upper X station

    //zPosStationX[] = {zPosStation0, zPosStation1, zPosStation2, zPosStation3};
    zPosStationX[0] = zPosStation0;
    zPosStationX[1] = zPosStation1;
    zPosStationX[2] = zPosStation2;
    zPosStationX[3] = zPosStation3;
    
    // y positions are wrt second station
    float offSet = 24*cm; // position of station 2
    float yPosStation0 = 24*cm - offSet; // y position of X station 2
    float yPosStation1 = 28*cm - offSet; // y position of X station 1 (closest to Vesuvius)
    float yPosStation2 = 19.5*cm - offSet; // y position of X station 3
    float yPosStation3 = - offSet; // y position of X station 4
    //float yPosStation[] = {yPosStation0, yPosStation1, yPosStation2, yPosStation3};
    yPosStation[0] = yPosStation0;
    yPosStation[1] = yPosStation1;
    yPosStation[2] = yPosStation2;
    yPosStation[3] = yPosStation3;

    _halfContLengthXY = 0.75*m;
    G4double detectorBottom = yPosStation[3] - 0.5*AlShellThickness;  // = -0.85 m
  // Top of highest shell in detector frame
  G4double detectorTop = yPosStation[1] + 0.5*AlShellThickness;  // station 1 is highest
    _halfContLengthXY  = std::max(std::abs(detectorBottom), std::abs(detectorTop));
    //_halfContLengthXY = ;
    //_halfContLengthZ = 1*m;
    G4Box *detContainerSolid = 
        new G4Box("Detector",
                _halfContLengthXY,
                _halfContLengthXY,
                _halfContLengthZ*2); 

    G4LogicalVolume *detContainerLog = 
        new G4LogicalVolume(detContainerSolid, 
                air_mat, 
                "Detector",
                NULL,
                NULL,
                NULL,
                false);

    // Rotation such that: x-axis = perpendicular to planes, pointing from first plane to last plane & z-axis = pointing upwards
    G4ThreeVector axis(1., 1., 1.);
    axis = axis.unit();
    G4RotationMatrix* rotDet = new G4RotationMatrix();
    rotDet->rotate(-120.*deg, axis);
    
    // Implement scintillator bars copy number as bitpattern
    // bit 0-7 -> iBar number
    // bit 8-11 -> iModule
    // bit 12-14 -> iStation
    // bit 15 -> X:0, Y:1
    G4int BarCopyNo = 0;

    // 4.1 X Plane -------------------
    for (G4int iStation=0; iStation<_nStations;iStation++) {
      stationName << iStation;

      G4VPhysicalVolume* physAlShell = 
        new G4PVPlacement(0,                     //no rotation
              G4ThreeVector(0,yPosStation[iStation],zPosStationX[iStation]),       //at (0,0,0)
              logicAlShell,            //its logical volume
              "AlShellX",               //its name
              detContainerLog,                     //its mother  volume
              false,                 //no boolean operation
              0,                     //copy number
              checkOverlaps);        //overlaps checking

      G4VPhysicalVolume* physTEC = 
        new G4PVPlacement(0,                     //no rotation
              G4ThreeVector(0,yPosStation[iStation]+20*cm,zPosStationX[iStation]+(_AlShellHeight+TEC_thickness)/2),       //at (0,0,0)
              logicTEC,            //its logical volume
              "TECX",               //its name
              detContainerLog,                     //its mother  volume
              false,                 //no boolean operation
              0,                     //copy number
              checkOverlaps);        //overlaps checking
      
      for (G4int iModule = 0; iModule < _nModules;iModule++){
        moduleName << iModule;

        /*G4VPhysicalVolume* physAlFoil = 
          new G4PVPlacement(0,                     //no rotation
                G4ThreeVector(0,yPosStation[iStation], zPosStationX[iStation] + (std::pow(-1,iModule))*((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2))),       //at (0,0,0)
                logicAlFoil,            //its logical volume
                "AlFoilX",               //its name
                detContainerLog,                     //its mother  volume
                false,                 //no boolean operation
                2,                     //copy number
                checkOverlaps);*/        //overlaps checking

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar;
          BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);

          // 4.1.1 upper half-layer -------------------
          // Bar 0 is at most-negative X coordinates
          new G4PVPlacement(_rotUpperX,
                G4ThreeVector(posFirstBar[iModule] + iBar * _triangEffectiveBase, yPosStation[iStation], zPosStationX[iStation] + zOffset),                
                barLog,
                "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),      
                detContainerLog,
                false,
                //2 * iBar);
                BarCopyNo,
              checkOverlaps);
          
          barName.str("");
        }

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar + 1;
          BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);

          // 4.1.2 lower half-layer -------------------
          new G4PVPlacement(_rotLowerX,
                G4ThreeVector(posFirstBar[iModule] + (iBar + 0.5) * _triangEffectiveBase,yPosStation[iStation],zPosStationX[iStation] - zOffset),
                barLog,
                "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar + 1);
                BarCopyNo,
              checkOverlaps);
          
        barName.str("");
        }
	    moduleName.str("");
      }
    stationName.str("");
    }

    // zPosStation0 = - layerThickness / 2. - layerThickness - zSafety; // old: caused overlap
    //zPosStation0 = - layerThickness / 2. - _AlShellHeight; // Z position of upper Y station
    //zPosStation1 = zPosStation0 - 25*cm ; // Z position of upper Y station
    //zPosStation2 = zPosStation0 + 25*cm ; // Z position of upper Y station
    //zPosStation3 = zPosStation0 + 145*cm ; // Z position of upper Y station

    // shift Z stations 1 shell thickness
    zPosStation0 -= _AlShellHeight; // Z position of upper Y station 
    zPosStation1 -= _AlShellHeight;; // Z position of upper Y station
    zPosStation2 -= _AlShellHeight;  ; // Z position of upper Y station
    zPosStation3 -= _AlShellHeight;  ; // Z position of upper Y station
  
    //float zPosStationY[] = {zPosStation0, zPosStation1, zPosStation2, zPosStation3};

    zPosStationY[0] = zPosStation0;
    zPosStationY[1] = zPosStation1;
    zPosStationY[2] = zPosStation2;
    zPosStationY[3] = zPosStation3;

    // 4.2 Y view -------------------
    for (G4int iStation=0; iStation<_nStations;iStation++){
      stationName << iStation;
      
      new G4PVPlacement(0,                     
            G4ThreeVector(0,yPosStation[iStation],zPosStationY[iStation]),
            logicAlShell,            
            "AlShellY",               
            detContainerLog,                     
            false,                
            0,                     
            checkOverlaps);        
      
      G4VPhysicalVolume* physTEC = 
        new G4PVPlacement(_rotZ,                     
              G4ThreeVector(0,yPosStation[iStation]+20*cm,zPosStationY[iStation]-(_AlShellHeight+TEC_thickness)/2),       
              logicTEC,            
              "TECY",               
              detContainerLog,                     
              false,                 
              0,                     
              checkOverlaps); 
                     
      for (G4int iModule = 0; iModule < _nModules;iModule++){
        moduleName << iModule;

        /*G4VPhysicalVolume* physAlFoil = 
          new G4PVPlacement(0,                     
                G4ThreeVector(0,yPosStation[iStation],zPosStationY[iStation] + (std::pow(-1,iModule))*((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2))),       
                logicAlFoil,            
                "AlFoilY",               
                detContainerLog,                     
                false,                 
                2,                     
                checkOverlaps);*/

        for (G4int iBar = 0; iBar <_nBars/2; iBar++) {
          barName << 2 * iBar;
          BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);

          // 4.2.1 upper half-layer -------------------
          //       Bar 0 is at most-negative Y coordinates
          new G4PVPlacement(_rotUpperY,
                G4ThreeVector(0, posFirstBar[iModule] + iBar * _triangEffectiveBase +yPosStation[iStation], zPosStationY[iStation] + zOffset ),
                barLog,
                "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar);
                BarCopyNo);
          barName.str("");
              }

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar + 1;
          BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);

          // 4.2.2 lower half-layer -------------------
          new G4PVPlacement(_rotLowerY,
                G4ThreeVector(0, (posFirstBar[iModule] + (iBar + 0.5) * _triangEffectiveBase)+yPosStation[iStation], zPosStationY[iStation] - zOffset ),
                barLog,
                "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar + 1);
                BarCopyNo);
          
        barName.str("");
        }
      moduleName.str("");
      }
    stationName.str("");
    }

      //construct Lead block
    G4Material* lead_mat = nist->FindOrBuildMaterial("G4_Pb");
    G4ThreeVector posLead = G4ThreeVector(0*cm, yPosStation3, (zPosStation0 + (96.2*cm)));

    G4Box *leadSolid = new G4Box("LeadBlock", (130/2)*cm, (120/2)*cm, (60/2)*cm);

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
              detContainerLog,                //its mother  volume
              false,                   //no boolean operation
              0,                       //copy number
              checkOverlaps);          //overlaps checking

G4double detBottom_world = - _halfContLengthXY;

// Ground fills from world bottom to detector bottom
G4double ground_sizeZ  = detBottom_world - (-0.5*world_sizeZ);
G4double groundCenterZ = -0.5*world_sizeZ + 0.5*ground_sizeZ;

G4Box* solidGround = new G4Box("Ground", 
    0.5*world_sizeX, 
    0.5*world_sizeY, 
    0.5*ground_sizeZ);
G4LogicalVolume* logicGround = new G4LogicalVolume(solidGround, rock_mat, "logGround");

new G4PVPlacement(0,
    G4ThreeVector(0, 0, groundCenterZ),
    logicGround, "physGround",
    logicWorld,   // in world, not detContainerLog
    false, 0, checkOverlaps);
    
    new G4PVPlacement(rotDet,                       //no rotation
      G4ThreeVector(0.5*_AlShellHeight+0.5*layerThickness,0,0),  //at position 
      detContainerLog,             //its logical volume
      "DetectorMother",                //its name
      logicWorld,                //its mother  volume
      false,                   //no boolean operation
      0,                       //copy number
      checkOverlaps);          //overlaps checking

  // ------------------- visualization attributes -------------------

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

    //logicAlFoil->SetVisAttributes(bluecol);
    logicAlShell->SetVisAttributes(cyancol);
    logicTEC->SetVisAttributes(greencol);

  bluecol->SetForceSolid(true);
  logicAlFoil->SetVisAttributes(bluecol);
     
  auto barVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.4)); 
  barVis->SetForceSolid(true);  // fill volume color
  barLog->SetVisAttributes(barVis);

  // ------------------- Return world -------------------
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
  auto sdManager = G4SDManager::GetSDMpointer();
  G4String SDname;
  auto Scintbars = new ScintbarSD(SDname="/Scintbars");
  sdManager->AddNewDetector(Scintbars);
  barLog->SetSensitiveDetector(Scintbars);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

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

G4Material* DetectorConstruction::FindMaterial(G4String name) 
{
    G4Material* material = G4Material::GetMaterial(name,true);
    return material;
}

void DetectorConstruction::DeleteMessenger()
{
  delete _messenger;
  _messenger = NULL;
}