#include "RunInformation.hh"
#include "G4SystemOfUnits.hh"
#include "CLHEP/Random/Random.h"
#include <fstream>
#include <ctime>
#include "DetectorConstruction.hh"
#include "GitVersion.hh"

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

    const char* clusterId  = std::getenv("CLUSTER_ID");
    const char* processId  = std::getenv("PROCESS_ID");

    // Run info
    // Timestamp
    out << "===== Run Info =====\n";
    std::time_t now = std::time(nullptr);
    out << "Run timestamp: " << std::ctime(&now) << "\n";

    out << "Number of events (/run/beamOn): " << nEvents << "\n";
    out << "CLHEP seed: " << seed << "\n";  
    out << "Physics list: FTFP_BERT" << "\n";

    // Runtime 
    if (total_ms >= 0) {
        long long ms = total_ms;
        long long hours = ms / (1000 * 60 * 60);
        ms %= (1000 * 60 * 60);
        long long minutes = ms / (1000 * 60);
        ms %= (1000 * 60);
        long long seconds = ms / 1000;
        long long milliseconds = ms % 1000;

        out << "Runtime: "
            << hours << "h "
            << minutes << "m "
            << seconds << "s "
            << milliseconds << "ms\n\n";
    }

    out << "Git version : " << GitVersion::kDescribe   << "\n"
    << "Commit      : " << GitVersion::kCommitHash << "\n"
    << "Branch      : " << GitVersion::kBranch     << "\n";
    out << "Cluster ID: " << (clusterId ? clusterId : "local run") << "\n";
    out << "Job ID: " << (processId ? processId : "local run") << "\n\n";

    // Detector parameters
    out << "===== Detector Parameters =====\n";
    out << "nBars: " << det->GetNBars() << "\n";
    out << "nModules: " << det->GetNModules() << "\n";
    out << "nStations: " << det->GetNStations() << "\n\n";

    out << "barType: " << det->GetDetectorType() << "\n";
    out << "barLength (cm): " << det->GetBarLength()/cm << "\n";
    out << "barHeight (cm): " << det->GetBarHeight()/cm << "\n";
    out << "barBase (cm): " << det->GetBarBase()/cm << "\n";
    out << "triangEffectiveBase (cm): "
        << det->GetTriangEffectiveBase()/cm << "\n";
    out << "cornerCut (mm): "
        << det->GetCornerCut()/cm << "\n\n";

    out << "Layer thickness (cm): "
        << det->GetLayerThickness()/cm << "\n";
    out << "Bar gap (mm): " << det->GetBarGap()/mm << "\n";
    out << "Station spacing (cm): " << det->GetStationSpacing()/cm << "\n\n";
    //out << "looseAcceptanceCheck: " << det->GetLooseAccCheck() << "\n";
    
    const std::vector<double>& zPositions = det->GetZPosStations();
    const std::vector<double>& yPositions = det->GetYPosStations();

    out << " X station positioning (cm): ";
    for (int i = 0; i < det->GetNStations(); i++)
        out << zPositions[i]/cm - zPositions[1]/cm << " ; ";
    out << "\n";

    out << "Station height w.r.t. z = 0 (cm): ";
    for (int i = 0; i < det->GetNStations(); i++)
        out << (yPositions[i] - yPositions[3])/cm << " ; ";
    out << "\n\n";


    // Generator parameters
    if (!generatorSummary.empty()) {
        out << "===== Generator Parameters =====\n";
        out << generatorSummary << "\n";
    }


    out.close();
}