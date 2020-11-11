// $Id: DetectorConstruction.hh 69565 2013-05-08 12:35:31Z gcosmo $
//
/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4RotationMatrix.hh"
#include <string>

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4GenericMessenger;

/// Detector construction class to define materials and geometry.

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction(const char *detectorName);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();
    //virtual G4LogicalVolume* Construct();
    
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

    G4bool IsInsideAcceptance(const G4ThreeVector &pos, const G4ThreeVector &dir) const;

    void DeleteMessenger();

  protected:
    G4GenericMessenger *_messenger;
    G4LogicalVolume*  fScoringVolume;

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

#endif

