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
/// \file CellParameterisation.hh
/// \brief Definition of the B5::CellParameterisation class

#pragma once

#include "G4VPVParameterisation.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include <vector>

class BarParameterisation : public G4VPVParameterisation {
public:
    BarParameterisation(G4int nStations, G4int nModules, G4int nBars,
                                  G4double barBase, G4double barHeight, G4double barLength,
                                  G4double layerThickness,
                                  G4RotationMatrix* rotUpperX,
                                  G4RotationMatrix* rotLowerX,
                                  G4RotationMatrix* rotUpperY,
                                  G4RotationMatrix* rotLowerY);

    virtual ~BarParameterisation();

    virtual void ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const override;
    virtual void ComputeDimensions(G4Box& bar, const G4int copyNo, const G4VPhysicalVolume* physVol) const override;
    virtual G4int GetNumberOfCopies() const;

private:
    G4int _nStations, _nModules, _nBars;
    G4int _totalBars;
    G4double _barBase, _barHeight, _barLength;
    G4double _layerThickness;

    std::vector<G4double> _posFirstBar;
    std::vector<G4double> _zPosStation;

    G4RotationMatrix* _rotUpperX;
    G4RotationMatrix* _rotLowerX;
    G4RotationMatrix* _rotUpperY;
    G4RotationMatrix* _rotLowerY;
};