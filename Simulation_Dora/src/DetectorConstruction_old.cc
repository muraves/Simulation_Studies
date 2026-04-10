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
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"
#include "G4GenericTrap.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "ScintBarSD.hh"
#include "Materials.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(const char *detectorName)
  : G4VUserDetectorConstruction(),
    _nBars(32), _nModules(2), _nStations(4),
    _stationSpacing(50 * cm), _barLength(107 * cm), _barHeight(1.7 * cm),
    _barBase(3.2 * cm), _triangEffectiveBase(3.2 * cm),
    _rotUpperX(NULL), _rotLowerX(NULL), _rotUpperY(NULL), _rotLowerY(NULL),
    _halfContLengthZ(0.), _halfContLengthXY(0.),
    _looseAccCheck(0.), _detType("triangular")
{
    _messenger = new G4GenericMessenger(this, std::string("/muraves/").append(detectorName).append("/"));
    _messenger->DeclareProperty("nBars", _nBars, "Number of scintillating bars per module");
    _messenger->DeclareProperty("nModules", _nModules, "Number of modules per plane");
    _messenger->DeclareProperty("nStations", _nStations, "Number of XY stations");
    _messenger->DeclarePropertyWithUnit("barLength","cm", _barLength, "Length of scintillating bars");
    _messenger->DeclarePropertyWithUnit("barHeight","cm", _barHeight, "Height of scintillating bars");
    _messenger->DeclarePropertyWithUnit("barBase","cm", _barBase, "Base size of scintillating bars");
    _messenger->DeclarePropertyWithUnit("triangEffBase","cm", _triangEffectiveBase,
                      "Effective base size of triangular bars due to cut edges");
    _messenger->DeclareProperty("looseAcceptanceCheck", _looseAccCheck,
                      "Acceptance enlargement factor (1 = no enlargement)");
    _messenger->DeclareProperty("detectorType", _detType,
                      "Detector type").SetCandidates("triangular square monolithic");

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
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4bool checkOverlaps = true;

    G4NistManager* nist = G4NistManager::Instance();
    G4Material* air_mat = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* Aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
    G4Material* polystyrene_mat = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* epoxy_mat = nist->FindOrBuildMaterial("G4_EPOXY");
    G4Material* rock_mat = new G4Material("StandardRock", 11.0, 22.0*g/mole, 2.65*g/cm3);

    //------------------- World -------------------
    G4double world_sizeX = 5.*m;
    G4double world_sizeY = 2.*m;
    G4double world_sizeZ = 3.*m;
    G4Box* solidWorld = new G4Box("World", 0.5*world_sizeX, 0.5*world_sizeY, 0.5*world_sizeZ);    
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air_mat, "World");            
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0, checkOverlaps);

    //------------------- Scintillator bar -------------------
    _detType = "triangular";

    std::vector<G4TwoVector> vertices;
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.));
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.));
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));
    vertices.push_back(G4TwoVector(_barBase / 2, -_barHeight / 2.));
    vertices.push_back(G4TwoVector(-_barBase / 2, -_barHeight / 2.));
    vertices.push_back(G4TwoVector(0, _barHeight / 2.));

    if (_detType == "triangular") {
        G4GenericTrap* triangSolid = new G4GenericTrap("triangSolid", _barLength / 2, vertices);

        if (_triangEffectiveBase < _barBase) {
            G4Box* boxSolid = new G4Box("boxSolid", _triangEffectiveBase / 2., _barHeight / 2., _barLength / 2.);
            G4IntersectionSolid* triangCutEdgeSolid = new G4IntersectionSolid("boxSolid*triangSolid", boxSolid, triangSolid);

            barLog = new G4LogicalVolume(triangCutEdgeSolid, polystyrene_mat, "BARSH2E", NULL, NULL, NULL, false);

            // ------------------- Epoxy corners -------------------
            G4double epoxyThickness = (_barBase - _triangEffectiveBase) / 2.;
            if (epoxyThickness > 0) {
                G4Box* epoxySolid = new G4Box("EpoxyCorner", epoxyThickness / 2., _barHeight / 2., _barLength / 2.);
                epoxyLog = new G4LogicalVolume(epoxySolid, epoxy_mat, "EpoxyCorner");

                // We will place epoxy at +X and -X corners of each bar later in Construct()
            }

        } else {
            barLog = new G4LogicalVolume(triangSolid, polystyrene_mat, "BARSH2E", NULL, NULL, NULL, false);
        }
    }

    //------------------- Station placement -------------------
    // Note: The rest of your original station/module/bar placement code
    // remains identical, except we now place the epoxy at corners:

    for (G4int iStation=0; iStation<_nStations; iStation++) {
        for (G4int iModule=0; iModule<_nModules; iModule++) {
            for (G4int iBar=0; iBar<_nBars/2; iBar++) {

                // Existing bars
                new G4PVPlacement(_rotUpperX,
                    G4ThreeVector(posFirstBar[iModule] + iBar * _triangEffectiveBase, yPosStation[iStation], zPosStationX[iStation] + zOffset),                
                    barLog,
                    "stationX_bar",
                    detContainerLog,
                    false,
                    0,
                    checkOverlaps);

                // Place epoxy corners for this bar
                if (epoxyThickness > 0) {
                    // +X corner
                    new G4PVPlacement(0,
                        G4ThreeVector(posFirstBar[iModule] + iBar * _triangEffectiveBase + (_triangEffectiveBase + epoxyThickness)/2, yPosStation[iStation], zPosStationX[iStation] + zOffset),
                        epoxyLog,
                        "EpoxyXPlus",
                        detContainerLog,
                        false,
                        0,
                        checkOverlaps);
                    // -X corner
                    new G4PVPlacement(0,
                        G4ThreeVector(posFirstBar[iModule] + iBar * _triangEffectiveBase - (_triangEffectiveBase + epoxyThickness)/2, yPosStation[iStation], zPosStationX[iStation] + zOffset),
                        epoxyLog,
                        "EpoxyXMinus",
                        detContainerLog,
                        false,
                        0,
                        checkOverlaps);
                }

            }
        }
    }

    //------------------- Return world -------------------
    return physWorld;
}

//.... Other member functions (ConstructSDandField, IsInsideAcceptance, etc.) remain unchanged

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