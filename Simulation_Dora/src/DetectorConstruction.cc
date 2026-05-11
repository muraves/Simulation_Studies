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

DetectorConstruction::DetectorConstruction(const char *detectorName)
  : G4VUserDetectorConstruction(),
    _nBars(32), _nModules(2), _nStations(4), _stationSpacing(50 * cm), _barLength(1070 * mm), _barHeight(17 * mm),
    _barBase(33. * mm), _triangEffectiveBase(32.5 * mm), _rotUpperX(NULL), _rotLowerX(NULL), _rotUpperY(NULL), _rotLowerY(NULL),
    _halfContLengthZ(0.), _halfContLengthXY(0.), _looseAccCheck(0.), _detType("triangular"), _cornerCut(1.*mm), _zPosStations({-0.26*m,0.*m,0.262*m,1.492*m}), _yPosStations({0.289*m,0.249*m,0.207*m,0*m})
{
  _messenger = new G4GenericMessenger(this, std::string("/muraves/").append(detectorName).append("/"));
  _messenger->DeclareProperty("nBars", _nBars, "Set the number of scintillating bars per module");
  _messenger->DeclareProperty("nModules", _nModules, "Set the number of modules per plane");
  _messenger->DeclareProperty("nStations", _nStations, "Set the number of XY stations.");
  _messenger->DeclarePropertyWithUnit("barLength","mm", _barLength, "Set the length of the scintillating bars.");
  _messenger->DeclarePropertyWithUnit("barHeight","mm", _barHeight, "Set the height of the scintillating bars.");
  _messenger->DeclarePropertyWithUnit("barBase","mm", _barBase, "Set the size of the base of the scintillating bars (this includes the gap).");
  _messenger->DeclarePropertyWithUnit("triangEffBase","mm", _triangEffectiveBase,
				      "Set the effective size of the base of the triangular bars due to gap (this base length does not include air gap).");
  _messenger->DeclarePropertyWithUnit("cornerCut","mm", _cornerCut,
				      "Set the size of the unusable (dead) tip of the triangular bar.");

      // because _zPos here becomes xPos IRL (rotation later on)
    _messenger->DeclarePropertyWithUnit("xPos0", "m", _zPosStations[0], "X position of station 0");
    _messenger->DeclarePropertyWithUnit("xPos1", "m", _zPosStations[1], "X position of station 1");
    _messenger->DeclarePropertyWithUnit("xPos2", "m", _zPosStations[2], "X position of station 2");
    _messenger->DeclarePropertyWithUnit("xPos3", "m", _zPosStations[3], "X position of station 3");

    // because _yPos here becomes zPos IRL (rotation later on)
    _messenger->DeclarePropertyWithUnit("zPos0", "m", _yPosStations[0], "Z position of station 0");
    _messenger->DeclarePropertyWithUnit("zPos1", "m", _yPosStations[1], "Z position of station 1");
    _messenger->DeclarePropertyWithUnit("zPos2", "m", _yPosStations[2], "Z position of station 2");
    _messenger->DeclarePropertyWithUnit("zPos3", "m", _yPosStations[3], "Z position of station 3");

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

  xySafety = _barBase - _triangEffectiveBase; //  XY shift for gap inbetween bars
  G4cout << "[MuravesDetector::Construct] xySafety = " << xySafety << G4endl;
  zSafety = 5.5 * cm; //  distance between X and Y layer
  
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* air_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* Aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
  G4Material* polystyrene_mat = nist->FindOrBuildMaterial("G4_POLYSTYRENE"); // standard polystyrene
  G4Material* rock_mat = new G4Material("StandardRock", 11.0, 22.0*g/mole, 2.65*g/cm3);
  G4Material* plastic_mat = nist->FindOrBuildMaterial("G4_POLYVINYL_CHLORIDE"); // PVC plastic
  //G4Material* polystyrene_mat = FindMaterial(Materials::kPOLYSTYRENE); // with costum scintillator properties


  G4bool checkOverlaps = false; // Option to switch on/off checking of volume overlaps

       
    // ------------------- World -------------------
    
    G4double world_sizeX = 5.*m;
    G4double world_sizeY = 4.*m;
    G4double world_sizeZ  = 3.5*m;
    
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
      
    
    //------------------- Aluminum foil (plates) -------------------
    
    G4double _AlFoilThickness = 2*mm;

    G4Box* solidAlFoil = new G4Box("AlFoil", 
    0.5 * (_nBars/2 * _barBase),   // covers one module width
    0.5 * _barLength,               // full bar length
    0.5 * _AlFoilThickness);

    G4LogicalVolume* logicAlFoil = new G4LogicalVolume(solidAlFoil, Aluminum_mat, "AlFoil");  

    //------------------- TEC -------------------
    
    G4double TEC_thickness = 0.15*cm;

    G4Box* solidTEC = new G4Box("TEC", 0.5*30*cm, 0.5*20*cm, 0.5*TEC_thickness);   

    G4LogicalVolume* logicTEC = new G4LogicalVolume(solidTEC, Aluminum_mat, "TEC");  
    
   // ------------------- Aluminum tape (covers left and right edge of modules) -------------------

  G4double tapeThickness  = 0.1 * mm;                    
  G4double tapeHalfLen    = _barLength / 2.;              
  G4double tapeHalfHeight = sqrt(_barHeight*_barHeight + (_triangEffectiveBase/2.)*(_triangEffectiveBase/2.) )/2.;
  //G4double slopeAngle = std::atan2(_triangEffectiveBase/ 2., _barHeight);
  //G4double slopeAngle = std::atan2(-2*_barHeight, _triangEffectiveBase);
  G4double slopeAngle = std::atan2(_triangEffectiveBase, 2*_barHeight);


  G4Box* tapeSolid = new G4Box("AlTape",
      tapeThickness / 2.,   
      tapeHalfHeight,         
      tapeHalfLen);     

  G4LogicalVolume* tapeLog = new G4LogicalVolume(tapeSolid, Aluminum_mat, "AlTape");

    //------------------- Scintillator bar -------------------
    
    _detType = "triangular";
    _looseAccCheck = 0.;

    // 1. Apply the necessary quirks for specific detector types -------------------
    if (_detType != "triangular")
        _triangEffectiveBase = _barBase;
    // 2. Build the logical box containing the detector -------------------
    _halfContLengthXY = (std::max(_barLength, (_barBase + xySafety) * (_nBars + 0.5))) / 2.;
    layerThickness = _barHeight + (_barHeight / (_barBase / 2.) * ((_barBase - _triangEffectiveBase) / 2.)); // Z offset due to cut edges at the base
    _halfContLengthZ = (float) (_nStations - 1) / 2 * _stationSpacing + (layerThickness + zSafety / 2);

    /* // ------ Some diagnostics ------
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars + 0.5) / 2. = " << (_barBase + xySafety) * (_nBars + 0.5) / 2. << G4endl;
    G4cout << "[MuravesDetector::Construct] ((_barBase + xySafety) * (_nBars + 0.5)-xySafety) / 2. = " << ((_barBase + xySafety) * (_nBars + 0.5)-xySafety) / 2. << G4endl;
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars/2 + 0.5) - xySafety/2 = " << (_barBase + xySafety) * (_nBars/2 + 0.5) - xySafety/2 << G4endl;
    G4cout << "[MuravesDetector::Construct] (_barBase + xySafety) * (_nBars/2) - xySafety = " << (_barBase + xySafety) * (_nBars/2) - xySafety << G4endl;
    G4cout << "[MuravesDetector::Construct] _halfContLengthXY = " << _halfContLengthXY << G4endl;
    G4cout << "[MuravesDetector::Construct] layerThickness = " << layerThickness << G4endl;
    G4cout << "[MuravesDetector::Construct] layerThickness=" << layerThickness << G4cout;
    G4cout << "[MuravesDetector::Construct] _halContLengthZ=" << _halfContLengthZ << G4cout ;
    */
   
    //------------------- Aluminum cask -------------------
  
    G4double _AlShellHeight = zSafety;
    G4double AlShellThickness = 1.2*m;
    
    G4Box* solidAlShellOut = new G4Box("AlShell", 0.5*AlShellThickness, 0.5*AlShellThickness, 0.5*_AlShellHeight);    
    G4Box* solidAlShellIn =  new G4Box("AlShell", 0.5*1.10*m, 0.5*1.1*m, 0.5*(_AlShellHeight-2*_AlFoilThickness));    
    G4SubtractionSolid* solidAlShell = new G4SubtractionSolid("solidAlShell",solidAlShellOut,solidAlShellIn);

    G4LogicalVolume* logicAlShell = new G4LogicalVolume(solidAlShell, Aluminum_mat, "AlShell");   

if (_detType == "triangular") {
    G4double H = _barHeight;
    G4double B = _triangEffectiveBase;
    G4double s = 2.*(H / B);

    G4double halfLen = _barLength / 2.;

    // --- Main bar ---
    std::vector<G4TwoVector> barPoly = {
        G4TwoVector( _cornerCut/s,      H/2. - _cornerCut),
        G4TwoVector( B/2.-_cornerCut, -H/2.+_cornerCut*s),
        G4TwoVector( B/2.-_cornerCut,   -H/2.),
        G4TwoVector(-B/2.+_cornerCut,   -H/2.),
        G4TwoVector(-B/2.+_cornerCut, -H/2.+_cornerCut*s),
        G4TwoVector(-_cornerCut/s,      H/2. - _cornerCut),
    };
    G4ExtrudedSolid* barCutSolid = new G4ExtrudedSolid("barCutSolid", barPoly,
        halfLen, G4TwoVector(0,0), 1.0, G4TwoVector(0,0), 1.0);

     G4double holeRadius = 1.6/2. * mm;

    // If we want an actual hole in triangular bar:
    /*G4Tubs* holeTubs = new G4Tubs("hole", 
        0.,           // inner radius
        holeRadius,   // outer radius
        halfLen,      // half length
        0.,           // start angle
        360.*deg);    // full circle

    // Hole centered at (0, 0, 0) in bar local frame
    G4SubtractionSolid* barWithHole = new G4SubtractionSolid("barWithHole",
        barCutSolid, holeTubs, 0, G4ThreeVector(0., 0., 0.));*/

    barLog = new G4LogicalVolume(barCutSolid, polystyrene_mat, "BARSH2E", NULL, NULL, NULL, false);

    // --- Model WLS holes as PVC rods, daughter of bar --- (hits will not be processed for daughter volumes -> just what we want: we don't record Edep in these WLS holes, only in scintillator bar)
    G4Tubs* rodSolid = new G4Tubs("rod",
    0.,
    holeRadius,
    halfLen,
    0.,
    360.*deg);

    G4LogicalVolume* rodLog = new G4LogicalVolume(rodSolid, plastic_mat, "Rod", NULL, NULL, NULL, false);

    auto rodVis = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0)); // solid green
    rodVis->SetForceSolid(true);
    rodLog->SetVisAttributes(rodVis);

    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), rodLog, "Rod", barLog, false, 0, checkOverlaps);

    // --- Top corner: model apex as PVC ---
    std::vector<G4TwoVector> topPoly = {
        G4TwoVector( 0.,   H/2.),       // original apex
        G4TwoVector( _cornerCut/s,  H/2. - _cornerCut),
        G4TwoVector(-_cornerCut/s,  H/2. - _cornerCut),
    };
    G4ExtrudedSolid* topCornerSolid = new G4ExtrudedSolid("topCornerSolid", topPoly,
        halfLen, G4TwoVector(0,0), 1.0, G4TwoVector(0,0), 1.0);
    topCornerLog = new G4LogicalVolume(topCornerSolid, plastic_mat, "TopCornerLog", NULL, NULL, NULL, false);

    // --- Right corner as PVC ---
    std::vector<G4TwoVector> rightPoly = {
        G4TwoVector( B/2.,     -H/2.),      // original corner
        G4TwoVector( B/2.-_cornerCut,   -H/2.),
        G4TwoVector( B/2.-_cornerCut, -H/2.+_cornerCut*s),
    };
    G4ExtrudedSolid* rightCornerSolid = new G4ExtrudedSolid("rightCornerSolid", rightPoly,
        halfLen, G4TwoVector(0,0), 1.0, G4TwoVector(0,0), 1.0);
    rightCornerLog = new G4LogicalVolume(rightCornerSolid, plastic_mat, "RightCornerLog", NULL, NULL, NULL, false);

    // --- Left corner as PVC---
    std::vector<G4TwoVector> leftPoly = {
        G4TwoVector(-B/2.,     -H/2.),      // original corner
        G4TwoVector(-B/2.+_cornerCut, -H/2.+_cornerCut*s),
        G4TwoVector(-B/2.+_cornerCut,   -H/2.),
    };
    G4ExtrudedSolid* leftCornerSolid = new G4ExtrudedSolid("leftCornerSolid", leftPoly,
        halfLen, G4TwoVector(0,0), 1.0, G4TwoVector(0,0), 1.0);
    leftCornerLog = new G4LogicalVolume(leftCornerSolid, plastic_mat, "LeftCornerLog", NULL, NULL, NULL, false);
}

    // 4. Build the stations -------------------
    double posFirstBarMod0 = -((float) _nBars -0.5) * _barBase / 2.; //position of first bar (i.e., bar at most-negative coordinate)
    //G4cout << "[MuravesDetector::Construct] -((float) _nBars -0.5) * _barBase / 2. = " << -((float) _nBars -0.5) * _barBase / 2. << G4endl;
    double posFirstBarMod1 = posFirstBarMod0 + (_nBars) * _barBase/2. ;
    //G4cout << "[MuravesDetector::Construct] posFirstBarMod0 + (_nBars) * _barBase/2. = " << -((float) _nBars -0.5) * _barBase / 2.+ (_nBars) * _barBase/2. << G4endl;
    
    double posFirstBar[] = {posFirstBarMod0,posFirstBarMod1};

    // to name the individual bars:
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
    
    float offset_zPos = - layerThickness / 2.;
    _zPosStations[0] += offset_zPos; 
    _zPosStations[1] += offset_zPos; 
    _zPosStations[2] += offset_zPos;
    _zPosStations[3] += offset_zPos; 
   
    float offset_yPos = _yPosStations[1];
    _yPosStations[0] -= offset_yPos;
    _yPosStations[1] -= offset_yPos; 
    _yPosStations[2] -= offset_yPos; 
    _yPosStations[3] -= offset_yPos; 

    /*// --- Diagnostics ---
     G4cout << "_zPosStations[0]" << _zPosStations[0] << G4endl;
    G4cout << "_zPosStations[1]" << _zPosStations[1] << G4endl;
    G4cout << "_zPosStations[2]" << _zPosStations[2] << G4endl;
    G4cout << "_zPosStations[3]" << _zPosStations[3] << G4endl;
    G4cout << "_yPosStations[0]" << _yPosStations[0] << G4endl;
    G4cout << "_yPosStations[1]" << _yPosStations[1] << G4endl;
    G4cout << "_yPosStations[2]" << _yPosStations[2] << G4endl;
    G4cout << "_yPosStations[3]" << _yPosStations[3] << G4endl;
    */

    _halfContLengthXY = 0.75*m;
    G4double detectorBottom = _yPosStations[3] - 0.5*AlShellThickness;  // = -0.85 m
  // Top of highest shell in detector frame
  G4double detectorTop = _yPosStations[0] + 0.5*AlShellThickness; 
    _halfContLengthXY  = std::max(std::abs(detectorBottom), std::abs(detectorTop));
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

    // Rotation such that: x-axis = perpendicular to detector planes, pointing from first plane to last plane & z-axis = pointing upwards (wrt ground)
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
              G4ThreeVector(0,_yPosStations[iStation],_zPosStations[iStation]),       //at (0,0,0)
              logicAlShell,            //its logical volume
              "AlShellX",               //its name
              detContainerLog,                     //its mother  volume
              false,                 //no boolean operation
              0,                     //copy number
              checkOverlaps);        //overlaps checking

      G4VPhysicalVolume* physTEC = 
        new G4PVPlacement(0,                     //no rotation
              G4ThreeVector(0,_yPosStations[iStation]+20*cm,_zPosStations[iStation]+(_AlShellHeight+TEC_thickness)/2),       //at (0,0,0)
              logicTEC,            //its logical volume
              "TECX",               //its name
              detContainerLog,                     //its mother  volume
              false,                 //no boolean operation
              0,                     //copy number
              checkOverlaps);        //overlaps checking
      
      for (G4int iModule = 0; iModule < _nModules;iModule++){
        moduleName << iModule;

        G4double foilCenterUpper = posFirstBar[iModule] + (_nBars/2 - 1) * _barBase / 2.;
        G4double foilCenterLower = posFirstBar[iModule] + _nBars/2 * _barBase / 2.;

        new G4PVPlacement(0,
            G4ThreeVector(foilCenterUpper, _yPosStations[iStation],
                _zPosStations[iStation] - ((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2)+xySafety)),
            logicAlFoil, "AlFoilX", detContainerLog, false, iModule*2, checkOverlaps);

        new G4PVPlacement(0,
            G4ThreeVector(foilCenterLower, _yPosStations[iStation],
                _zPosStations[iStation] + ((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2)+xySafety)),
            logicAlFoil, "AlFoilX", detContainerLog, false, iModule*2+1, checkOverlaps);

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar + 1;
          BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);

          // 4.1.1 upper half-layer -------------------
          // Bar 0 is at most-negative X coordinates
          new G4PVPlacement(_rotUpperX,
                G4ThreeVector(posFirstBar[iModule] + (iBar+0.5) * (_barBase), _yPosStations[iStation], _zPosStations[iStation] + zOffset),                
                barLog,
                "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),      
                detContainerLog,
                false,
                //2 * iBar);
                BarCopyNo,
              checkOverlaps);
          
          barName.str("");

          G4ThreeVector barPos(posFirstBar[iModule] + (iBar+0.5) * (_barBase),
                     _yPosStations[iStation],
                     _zPosStations[iStation] + zOffset);

          // ---- Place PVC corners ----     
          new G4PVPlacement(_rotUpperX, barPos, topCornerLog,   "TopCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),   detContainerLog, false, BarCopyNo, checkOverlaps);
          new G4PVPlacement(_rotUpperX, barPos, rightCornerLog, "RightCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(), detContainerLog, false, BarCopyNo, checkOverlaps);
          new G4PVPlacement(_rotUpperX, barPos, leftCornerLog,  "LeftCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),  detContainerLog, false, BarCopyNo, checkOverlaps);
        }

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar;
          BarCopyNo = ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);

          // 4.1.2 lower half-layer -------------------
          new G4PVPlacement(_rotLowerX,
                G4ThreeVector(posFirstBar[iModule] + (iBar) * (_barBase),_yPosStations[iStation],_zPosStations[iStation] - zOffset),
                barLog,
                "stationX" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar + 1);
                BarCopyNo,
              checkOverlaps);
          
        barName.str("");

        G4ThreeVector barPos(posFirstBar[iModule] + (iBar) * (_barBase),_yPosStations[iStation],_zPosStations[iStation] - zOffset);

                  
        new G4PVPlacement(_rotLowerX, barPos, topCornerLog,   "TopCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),   detContainerLog, false, BarCopyNo, checkOverlaps);
        new G4PVPlacement(_rotLowerX, barPos, rightCornerLog, "RightCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(), detContainerLog, false, BarCopyNo, checkOverlaps);
        new G4PVPlacement(_rotLowerX, barPos, leftCornerLog,  "LeftCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),  detContainerLog, false, BarCopyNo, checkOverlaps);
        }
	    moduleName.str("");

      // -------- module edge tape --------
    G4double B = _triangEffectiveBase;

    G4double eps = 0.103 * mm; // place as close as possible to the bar (unfortunately do manually...)

    G4double xLeft = posFirstBar[iModule] - B/4. - (tapeThickness/2.) + eps;
    G4double xRight = posFirstBar[iModule] + (_nBars/2 - 0.5)*_barBase + B/4. + (tapeThickness/2.) -eps;

    G4double zCenter = _zPosStations[iStation] ; 
    G4double yCenter = _yPosStations[iStation];

    //G4RotationMatrix* rotTapeLeftX  = new G4RotationMatrix(*_rotUpperX);
    //rotTapeLeftX->rotateZ(+slopeAngle);  // tilt to match left slope, then rotUpperX       

G4RotationMatrix* rotTapeLeftX  = new G4RotationMatrix(*_rotUpperX);
rotTapeLeftX->rotateZ(-slopeAngle);

    new G4PVPlacement(rotTapeLeftX,
        G4ThreeVector(xLeft, yCenter, zCenter),
        tapeLog,
        "TapeLeft_X" + stationName.str() + "mod" + moduleName.str(),
        detContainerLog, false, 0, checkOverlaps);

    new G4PVPlacement(rotTapeLeftX,
        G4ThreeVector(xRight, yCenter, zCenter),
        tapeLog,
        "TapeRight_X" + stationName.str() + "mod" + moduleName.str(),
        detContainerLog, false, 0, checkOverlaps);
      }
    stationName.str("");
    }

    // shift Z stations 1 shell thickness
    _zPosStations[0] -= _AlShellHeight; 
    _zPosStations[1] -= _AlShellHeight;; 
    _zPosStations[2] -= _AlShellHeight; 
    _zPosStations[3] -= _AlShellHeight; 


    // 4.2 Y view -------------------
    for (G4int iStation=0; iStation<_nStations;iStation++){
      stationName << iStation;
      
      new G4PVPlacement(0,                     
            G4ThreeVector(0,_yPosStations[iStation],_zPosStations[iStation]),
            logicAlShell,            
            "AlShellY",               
            detContainerLog,                     
            false,                
            0,                     
            checkOverlaps);        
      
      G4VPhysicalVolume* physTEC = 
        new G4PVPlacement(_rotZ,                     
              G4ThreeVector(0,_yPosStations[iStation]+20*cm,_zPosStations[iStation]-(_AlShellHeight+TEC_thickness)/2),       
              logicTEC,            
              "TECY",               
              detContainerLog,                     
              false,                 
              0,                     
              checkOverlaps); 
                     
      for (G4int iModule = 0; iModule < _nModules;iModule++){
        moduleName << iModule;

              G4RotationMatrix* rotFoilY = new G4RotationMatrix();
        rotFoilY->rotateZ(90.*deg);

        G4double foilCenterUpper = posFirstBar[iModule] + (_nBars/2 - 1) * _barBase / 2. + _yPosStations[iStation];
        G4double foilCenterLower = posFirstBar[iModule] + _nBars/2 * _barBase / 2. + _yPosStations[iStation];

        new G4PVPlacement(rotFoilY,
            G4ThreeVector(0, foilCenterUpper,
                _zPosStations[iStation] - ((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2)+xySafety)),
            logicAlFoil, "AlFoilY", detContainerLog, false, iModule*2, checkOverlaps);

        new G4PVPlacement(rotFoilY,
            G4ThreeVector(0, foilCenterLower,
                _zPosStations[iStation] + ((layerThickness/2)+(zOffset/2)+(_AlFoilThickness/2)+xySafety)),
            logicAlFoil, "AlFoilY", detContainerLog, false, iModule*2+1, checkOverlaps);

        for (G4int iBar = 0; iBar <_nBars/2; iBar++) {
          barName << 2 * iBar + 1;
          BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar & 0xFF);

          // 4.2.1 upper half-layer -------------------
          //       Bar 0 is at most-negative Y coordinates
          new G4PVPlacement(_rotUpperY,
                G4ThreeVector(0, posFirstBar[iModule] + (iBar+0.5) * _barBase +_yPosStations[iStation], _zPosStations[iStation] + zOffset ),
                barLog,
                "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar);
                BarCopyNo);
          barName.str("");

           G4ThreeVector barPos(0, posFirstBar[iModule] + (iBar+0.5) * _barBase +_yPosStations[iStation], _zPosStations[iStation] + zOffset);
           
          new G4PVPlacement(_rotUpperY, barPos, topCornerLog,   "TopCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),   detContainerLog, false, BarCopyNo, checkOverlaps);
          new G4PVPlacement(_rotUpperY, barPos, rightCornerLog, "RightCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(), detContainerLog, false, BarCopyNo, checkOverlaps);
          new G4PVPlacement(_rotUpperY, barPos, leftCornerLog,  "LeftCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),  detContainerLog, false, BarCopyNo, checkOverlaps);
                    
              }

        for (G4int iBar = 0; iBar < _nBars/2; iBar++) {
          barName << 2 * iBar;
          BarCopyNo = (0x1 << 15) + ((iStation & 0xF) << 12) + ((iModule & 0xF) << 8) + (2*iBar+1 & 0xFF);

          // 4.2.2 lower half-layer -------------------
          new G4PVPlacement(_rotLowerY,
                G4ThreeVector(0, (posFirstBar[iModule] + (iBar) * _barBase)+_yPosStations[iStation], _zPosStations[iStation] - zOffset ),
                barLog,
                "stationY" + stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),
                detContainerLog,
                false,
                //2 * iBar + 1);
                BarCopyNo);
          
        barName.str("");

        G4ThreeVector barPos(0, posFirstBar[iModule] + (iBar) * _barBase +_yPosStations[iStation], _zPosStations[iStation] - zOffset);

                    
        new G4PVPlacement(_rotLowerY, barPos, topCornerLog,   "TopCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),   detContainerLog, false, BarCopyNo, checkOverlaps);
        new G4PVPlacement(_rotLowerY, barPos, rightCornerLog, "RightCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(), detContainerLog, false, BarCopyNo, checkOverlaps);
        new G4PVPlacement(_rotLowerY, barPos, leftCornerLog,  "LeftCorner_X"+stationName.str() + "mod" + moduleName.str() + "bar" + barName.str(),  detContainerLog, false, BarCopyNo, checkOverlaps);
        }
      moduleName.str("");

            // ------- module edge tape ----------
    G4double B = _triangEffectiveBase;

    G4double eps = 0.103 * mm;
  
    G4double yLeft = posFirstBar[iModule] - B/4. - (tapeThickness/2.) +_yPosStations[iStation] + eps;
    G4double yRight = posFirstBar[iModule] + (_nBars/2 - 0.5)*_barBase + B/4. + (tapeThickness/2.) +_yPosStations[iStation] - eps;
    G4double zCenter = _zPosStations[iStation]; 
    G4double xCenter = 0;

    G4RotationMatrix* rotTapeRightY = new G4RotationMatrix(*_rotUpperY);
rotTapeRightY->rotateZ(+slopeAngle);

    new G4PVPlacement(rotTapeRightY,
        G4ThreeVector(xCenter, yLeft, zCenter),
        tapeLog,
        "TapeLeft_Y" + stationName.str() + "mod" + moduleName.str(),
        detContainerLog, false, 0, checkOverlaps);

    new G4PVPlacement(rotTapeRightY,
        G4ThreeVector(xCenter, yRight, zCenter),
        tapeLog,
        "TapeRight_Y" + stationName.str() + "mod" + moduleName.str(),
        detContainerLog, false, 0, checkOverlaps);

      }
    stationName.str("");
    }

      //construct Lead block
    G4Material* lead_mat = nist->FindOrBuildMaterial("G4_Pb");

    G4ThreeVector posLead = G4ThreeVector(0*cm, _yPosStations[3], (_zPosStations[1] + _AlShellHeight + (96.2*cm))); // + AlShellHeight to position lead block w.r.t. Aluminum cask of second station (origin station)

    G4Box *leadSolid = new G4Box("LeadBlock", (120/2)*cm, (120/2)*cm, (60/2)*cm);

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
G4cout << "[CONSTRUCT] detBottom_world" << detBottom_world << G4endl;

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
    logicWorld,   
    false, 0, checkOverlaps);
    //rotDet
    new G4PVPlacement(rotDet,                       //no rotation
      G4ThreeVector(0.5*_AlShellHeight+0.5*layerThickness -(_zPosStations[1]+_AlShellHeight)-0.5*layerThickness,0,0),  // centered in middle of station 1
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
    G4VisAttributes* bluecol = new G4VisAttributes(G4Colour(0.,0.,0.8,0.5));

    //logicAlFoil->SetVisAttributes(bluecol);
    logicAlShell->SetVisAttributes(cyancol);
    logicTEC->SetVisAttributes(greencol);

  bluecol->SetForceSolid(true);
  logicAlFoil->SetVisAttributes(bluecol);
     
  auto barVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.4)); 
  barVis->SetForceSolid(true);  // fill volume color
  barLog->SetVisAttributes(barVis);

  auto tapeVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.5)); // yellow
  tapeVis->SetForceSolid(true);
  tapeLog->SetVisAttributes(tapeVis);

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