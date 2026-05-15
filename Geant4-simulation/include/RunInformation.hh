#ifndef RUNCONFIGURATIONWRITER_H
#define RUNCONFIGURATIONWRITER_H

#include <string>

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction_EcoMug.hh"
#include "MuravesMessenger.hh"

class RunInformation {
public:
  static void Write(const std::string& filename,
                    const DetectorConstruction* det,
                    //const PrimaryGeneratorAction_EcoMug* gen,
                    //const MuravesMessenger* mes,
                    G4int nEvents,
                    const std::string& generatorSummary = "",
                  long long total_ms = -1,
                long seed=0);
};

#endif