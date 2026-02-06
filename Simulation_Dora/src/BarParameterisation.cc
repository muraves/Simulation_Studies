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
/// \file CellParameterisation.cc
/// \brief Implementation of the B5::CellParameterisation class


#include "BarParameterisation.hh"
#include "G4Box.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include "G4SystemOfUnits.hh"

BarParameterisation::BarParameterisation(G4int nStations, G4int nModules, G4int nBars,
                                                           G4double barBase, G4double barHeight, G4double barLength,
                                                           G4double layerThickness,
                                                           G4RotationMatrix* rotUpperX,
                                                           G4RotationMatrix* rotLowerX,
                                                           G4RotationMatrix* rotUpperY,
                                                           G4RotationMatrix* rotLowerY)
: _nStations(nStations), _nModules(nModules), _nBars(nBars),
  _barBase(barBase), _barHeight(barHeight), _barLength(barLength),
  _layerThickness(layerThickness),
  _rotUpperX(rotUpperX), _rotLowerX(rotLowerX),
  _rotUpperY(rotUpperY), _rotLowerY(rotLowerY)
{
    _totalBars = _nStations * _nModules * _nBars;

    // Precompute first-bar positions per module
    _posFirstBar.resize(_nModules);
    G4double effectiveBase = _barBase; // adjust if needed
    G4double start = -((_nBars / 2.) - 0.5) * effectiveBase;
    for (G4int m = 0; m < _nModules; ++m)
        _posFirstBar[m] = start + m * effectiveBase / 2.;

    // Precompute Z positions of stations
    _zPosStation.resize(_nStations);
    G4double z0 = -_layerThickness / 2.;
    _zPosStation[0] = z0;
    if (_nStations > 1) _zPosStation[1] = z0 - 26*cm;
    if (_nStations > 2) _zPosStation[2] = z0 + 26.2*cm;
    if (_nStations > 3) _zPosStation[3] = z0 + 147.5*cm;
}

BarParameterisation::~BarParameterisation() = default;

void BarParameterisation::ComputeTransformation(const G4int copyNo,
                                                         G4VPhysicalVolume* physVol) const
{
    // Decode copy number
    G4int station = (copyNo >> 12) & 0xF;
    G4int module  = (copyNo >> 8) & 0xF;
    G4int bar     = copyNo & 0xFF;

    // Choose rotation: even = upper, odd = lower
    G4RotationMatrix* rot = (bar % 2 == 0) ? _rotUpperX : _rotLowerX;

    // Compute position
    G4double x = _posFirstBar[module] + (bar / 2) * _barBase;
    G4double y = 0.;
    G4double z = _zPosStation[station];

    physVol->SetTranslation(G4ThreeVector(x, y, z));
    physVol->SetRotation(rot);
}

void BarParameterisation::ComputeDimensions(G4Box& /*bar*/, const G4int /*copyNo*/, const G4VPhysicalVolume* /*physVol*/) const
{
    // All bars same dimensions: nothing to do
}

G4int BarParameterisation::GetNumberOfCopies() const
{
    return _totalBars;
}
