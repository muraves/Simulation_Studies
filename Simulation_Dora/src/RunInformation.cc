#include "RunInformation.hh"
#include "G4SystemOfUnits.hh"
#include "CLHEP/Random/Random.h"
#include <fstream>
#include <ctime>

void RunInformation::Write(const std::string& filename,
                                   const DetectorConstruction* det,
                                   //const PrimaryGeneratorAction_EcoMug* gen,
                                   //const MuravesMessenger* mes,
                                   int nEvents,
                                    const std::string& generatorSummary)
{
    std::ofstream out(filename);

    // Timestamp
    std::time_t now = std::time(nullptr);
    out << "Run timestamp: " << std::ctime(&now) << "\n";

    // Detector parameters
    out << "===== Detector Parameters =====\n";
    out << "nBars: " << det->GetNBars() << "\n";
    out << "nModules: " << det->GetNModules() << "\n";
    out << "nPlanes: " << det->GetNPlanes() << "\n";
    out << "nStations: " << det->GetNStations() << "\n";

    out << "barLength (cm): " << det->GetBarLength()/cm << "\n";
    out << "barHeight (cm): " << det->GetBarHeight()/cm << "\n";
    out << "barBase (cm): " << det->GetBarBase()/cm << "\n";
    out << "triangEffectiveBase (cm): "
        << det->GetTriangEffectiveBase()/cm << "\n";

    out << "stationSpacing (cm): " << det->GetStationSpacing()/cm << "\n";
    out << "looseAcceptanceCheck: " << det->GetLooseAccCheck() << "\n";
    out << "detectorType: " << det->GetDetectorType() << "\n\n";

    /*
    // Generator parameters
    out << "===== Generator Parameters =====\n";
    //out << "Generator: " << mes->GetGeneratorName() << "\n";
    //if (mes->GetGeneratorName() == "EcoMug") {
    auto c = gen->GetHSphereCenter();
    out << "HSphereRadius (cm): " << gen->GetHSphereRadius()/cm << "\n";
    out << "HSphereCenter (cm): "
        << c[0]/cm << " " << c[1]/cm << " " << c[2]/cm << "\n";
    out << "ThetaMin: " << gen->GetMinTheta() << "\n";
    out << "ThetaMax: " << gen->GetMaxTheta() << "\n";
    out << "PhiMin: " << gen->GetMinPhi() << "\n";
    out << "PhiMax: " << gen->GetMaxPhi() << "\n\n";
    //}*/

    // Generator parameters
    if (!generatorSummary.empty()) {
        out << "===== Generator Parameters =====\n";
        out << generatorSummary << "\n\n";
    }

    // Run info
    out << "===== Run Info =====\n";
    out << "Number of events: " << nEvents << "\n";

    // Random seeds
    //long seeds[2];
    //CLHEP::HepRandom::getTheSeeds(seeds);
    //out << "Random seeds: " << seeds[0] << " " << seeds[1] << "\n";

    out.close();
}