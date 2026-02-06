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

  private:
    G4GenericMessenger* _messenger;
    G4LogicalVolume* barLog = nullptr;

    // Detector properties
    G4int _nBars;
    G4int _nModules;
    G4int _nPlanes;
    G4int _nStations;
    G4double _stationSpacing;

    // Single bar properties
    G4double _barLength;
    G4double _barHeight;
    G4double _barBase;
    G4double _triangEffectiveBase;

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

    // Detector type
    std::string _detType;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
