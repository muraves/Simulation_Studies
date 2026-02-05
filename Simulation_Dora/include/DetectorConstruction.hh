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
/// \file DetectorConstruction.hh
/// \brief Definition of the B5::DetectorConstruction class

#ifndef B5DetectorConstruction_h
#define B5DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

#include "G4RotationMatrix.hh"
#include "G4Threading.hh"
#include "globals.hh"
#include "G4Material.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include "G4VPhysicalVolume.hh"
#include "G4Cache.hh"

class G4FieldManager;
class G4VPhysicalVolume;
class G4GenericMessenger;
class Materials;
class G4LogicalVolume;

namespace B5
{

class MagneticField;

/// Detector construction

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

    void ConstructMaterials();
    G4Material* FindMaterial(G4String);

  private:
    Materials* fMaterials;
    G4GenericMessenger *_messenger;
    G4LogicalVolume*  fScoringVolume;
    G4LogicalVolume* barLog = nullptr;
    static G4ThreadLocal G4FieldManager* fFieldMgr;

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

}  // namespace B5

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
