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
                                    const std::string& generatorSummary,
                                long long total_ms, 
                            long seed)
{
    std::ofstream out(filename);

    // Timestamp
    std::time_t now = std::time(nullptr);
    out << "Run timestamp: " << std::ctime(&now) << "\n";

    // Detector parameters
    out << "===== Detector Parameters =====\n";
    out << "nBars: " << det->GetNBars() << "\n";
    out << "nModules: " << det->GetNModules() << "\n";
    out << "nStations: " << det->GetNStations() << "\n";

    out << "barLength (cm): " << det->GetBarLength()/cm << "\n";
    out << "barHeight (cm): " << det->GetBarHeight()/cm << "\n";
    out << "barBase (cm): " << det->GetBarBase()/cm << "\n";
    out << "triangEffectiveBase (cm): "
        << det->GetTriangEffectiveBase()/cm << "\n";

    out << "Bar gap (mm): " << det->GetBarGap()/cm << "\n";
    out << "stationSpacing (cm): " << det->GetStationSpacing()/cm << "\n";
    //out << "looseAcceptanceCheck: " << det->GetLooseAccCheck() << "\n";
    out << "detectorType: " << det->GetDetectorType() << "\n\n";

    out << " X Station positioning (cm): ";
    for (int i = 0; i < det->GetNStations(); i++)
        out << det->GetZPosStationsX()[i]/cm << " ; ";
    out << "\n";

    out << " Y Station positioning (cm): ";
    for (int i = 0; i < det->GetNStations(); i++)
        out << det->GetZPosStationsY()[i]/cm << " ; ";
    out << "\n";

    out << " Station height w.r.t. z = 0 (cm): ";
    G4double yPos3 = det->GetYPosStations()[3];
    for (int i = 0; i < det->GetNStations(); i++)
        out << (det->GetYPosStations()[i] - yPos3)/cm << " ; ";
    out << "\n\n";


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
    out << "CLHEP seed: " << seed << "\n";  

    // Random seeds
    //long seeds[2];
    //CLHEP::HepRandom::getTheSeeds(seeds);
    //out << "Random seeds: " << seeds[0] << " " << seeds[1] << "\n";

    // Runtime 
    if (total_ms >= 0) {
        long long ms = total_ms;
        long long hours = ms / (1000 * 60 * 60);
        ms %= (1000 * 60 * 60);
        long long minutes = ms / (1000 * 60);
        ms %= (1000 * 60);
        long long seconds = ms / 1000;
        long long milliseconds = ms % 1000;

        out << "Time taken by program: "
            << hours << "h "
            << minutes << "m "
            << seconds << "s "
            << milliseconds << "ms\n";
    }

    out.close();
}