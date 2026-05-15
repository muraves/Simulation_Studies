/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class

#ifndef B5DetectorConstruction_h
#define B5DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

#include "G4RotationMatrix.hh"
#include "globals.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Cache.hh"
#include "G4UIcmdWithAString.hh"

class G4VPhysicalVolume;
class G4GenericMessenger;
class G4LogicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction(const char *detectorName);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;
  
    G4bool IsInsideAcceptance(const G4ThreeVector &pos, const G4ThreeVector &dir) const;

    void DeleteMessenger();

    G4Material* FindMaterial(G4String);

    // getters for run information
    G4int GetNBars() const { return _nBars; }
    G4int GetNModules() const { return _nModules; }
    G4int GetNStations() const { return _nStations; }

    G4double GetBarLength() const { return _barLength; }
    G4double GetBarHeight() const { return _barHeight; }
    G4double GetBarBase() const { return _barBase; }
    G4double GetTriangEffectiveBase() const { return _triangEffectiveBase; }
    G4double GetStationSpacing() const { return zSafety; }
    G4double GetBarGap() const { return xySafety; }
    G4double GetLayerThickness() const { return layerThickness; }
    G4double GetCornerCut() const {return _cornerCut; }

    G4double GetLooseAccCheck() const { return _looseAccCheck; }
    std::string GetDetectorType() const { return _detType; }

    const std::vector<double>& GetZPosStations() const { return _zPosStations; }
    //const float* GetZPosStationsY() const { return zPosStationY; }
    const std::vector<double>& GetYPosStations() const { return _yPosStations; }

  private:
    G4GenericMessenger* _messenger;
    G4LogicalVolume* barLog = nullptr;
    G4LogicalVolume* topCornerLog = nullptr;
    G4LogicalVolume* leftCornerLog = nullptr;
    G4LogicalVolume* rightCornerLog = nullptr;

    G4ThreeVector _topCornerOffset;
    G4ThreeVector _rightCornerOffset;
    G4ThreeVector _leftCornerOffset;

    // Detector properties
    G4int _nBars;
    G4int _nModules;
    G4int _nStations;
    G4double _stationSpacing;

    // Single bar properties
    G4double _barLength;
    G4double _barHeight;
    G4double _barBase;
    G4double _triangEffectiveBase;
    G4double _cornerCut;

    // Configuration properties
    G4double xySafety;
    G4double zSafety;
    std::vector<double> _zPosStations; 
    std::vector<double> _yPosStations;

    // Station positions
    float zPosStationX[4];
    float zPosStationY[4];
    float yPosStation[4];

    // Rotation matrices to place bars
    G4RotationMatrix *_rotUpperX;
    G4RotationMatrix *_rotLowerX;
    G4RotationMatrix *_rotX;
    G4RotationMatrix *_rotUpperY;
    G4RotationMatrix *_rotLowerY;
    G4RotationMatrix *_rotZ;

    // Container dimensions
    float _halfContLengthZ;
    float _halfContLengthXY;

    // Acceptance check
    float _looseAccCheck;
    G4double layerThickness;

    // Detector type
    std::string _detType;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
