// EcoMugMessenger.hh
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "PrimaryGeneratorAction_EcoMug.hh"

class EcoMugMessenger : public G4UImessenger {
public:
    EcoMugMessenger(PrimaryGeneratorAction_EcoMug* gen)
        : fGen(gen)
    {
        fReadConfigCmd = new G4UIcmdWithAString("/EcoMug/readConfig", this);
        fReadConfigCmd->SetGuidance("Read EcoMug parameters from a text file.");
        fReadConfigCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

        fGenHSphereCmd = new G4UIcmdWithABool("/EcoMug/genHSphere", this);
        fGenHSphereCmd->SetGuidance("Use hemisphere generation surface (true/false).");
        fGenHSphereCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

        fCustomFluxCmd = new G4UIcmdWithABool("/EcoMug/customFlux", this);
        fCustomFluxCmd->SetGuidance("Use custom Guan flux parameterisation (true/false).");
        fCustomFluxCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    }

    ~EcoMugMessenger() { delete fReadConfigCmd; delete fGenHSphereCmd; delete fCustomFluxCmd; }

    void SetNewValue(G4UIcommand* command, G4String newValue) override {
        if (command == fReadConfigCmd) {
            fGen->ReadConfigFile(newValue);
        } else if (command == fGenHSphereCmd) {
            fGen->SetGenHSphere(fGenHSphereCmd->GetNewBoolValue(newValue));
        } else if (command == fCustomFluxCmd) {
            fGen->SetCustomFlux(fCustomFluxCmd->GetNewBoolValue(newValue));
        }
    }

private:
    PrimaryGeneratorAction_EcoMug* fGen;
    G4UIcmdWithAString* fReadConfigCmd;
    G4UIcmdWithABool* fGenHSphereCmd;
    G4UIcmdWithABool* fCustomFluxCmd;
};