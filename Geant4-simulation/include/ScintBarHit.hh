/// \file ScintBarHit.hh
/// \brief Definition of the ScintBarHit class

#ifndef ScintBarHit_h
#define ScintBarHit_h 1

#include "G4VHit.hh"

#include "G4Allocator.hh"
#include "G4RotationMatrix.hh"
#include "G4THitsCollection.hh"
#include "G4Threading.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <map>
#include <vector>

class G4AttDef;
class G4AttValue;
class G4LogicalVolume;


/// Scintintillator bar hit
///
/// It records:
/// ...

class ScintbarHit : public G4VHit
{
  public:
    ScintbarHit();
    // iStation X0: 0; Y0: 8;
  //          X1: 1; Y1: 9;
  //          X2: 2; Y2: 10;
  //          X3: 3; Y3: 11;
    ScintbarHit( G4int iStation, G4int iModule, G4int iBar ); 
    ScintbarHit( const ScintbarHit &right );
    virtual ~ScintbarHit();

    const ScintbarHit& operator=(const ScintbarHit& right);
    G4bool operator==(const ScintbarHit& right) const;

    inline void* operator new(size_t);
    inline void operator delete(void* aHit);

    virtual void Draw();
    virtual const std::map<G4String, G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

    void SetStationID( G4int z ) { fStationID = z; };
    G4int GetStationID() const { return fStationID; };

    void SetModuleID( G4int z ) { fModuleID = z; };
    G4int GetModuleID() const { return fModuleID; };

    void SetBarID( G4int z ) { fBarID = z; };
    G4int GetBarID() const { return fBarID; };

    void SetEdep( G4double de ) { fEdep = de; };
    void AddEdep( G4double de) { fEdep += de; };
    G4double GetEdep() const { return fEdep; }

    void SetPos( G4ThreeVector xyz ) { fPos = xyz; };
    G4ThreeVector GetPos() const { return fPos; };

    void SetTrackID( G4int id ) { fTrackID = id; };
    G4int GetTrackID() const { return fTrackID; };

    void SetParentId( G4int id ) { fParentId = id; };
    G4int GetParentId() const { return fParentId; };

    void SetPDGcode( G4int c ) { fPDGcode = c; };
    G4int GetPDGcode() const { return fPDGcode; };

    void SetHitTime( G4double c ) { fHitTime = c; };
    G4double GetHitTime() const { return fHitTime; };

    void AddPathLength(G4double len)   { fPathLength += len; }
    G4double GetPathLength() const { return fPathLength; };

    void SetEntryPoint(G4ThreeVector p) { fEntryPoint = p; }
    G4ThreeVector GetEntryPoint() const { return fEntryPoint; };

    void SetExitPoint(G4ThreeVector p)  { fExitPoint = p; }
    G4ThreeVector GetExitPoint() const { return fExitPoint; };


  private:
    G4int fStationID;
    G4int fModuleID;
    G4int fBarID;
    G4double fEdep;
    G4double fHitTime;
    G4ThreeVector fPos;
    G4int fTrackID;
    G4int fPDGcode;
    G4int fParentId;
    G4double      fPathLength;
    G4ThreeVector fEntryPoint;
    G4ThreeVector fExitPoint;
};

typedef G4THitsCollection<ScintbarHit> ScintbarHitsCollection;

extern G4ThreadLocal G4Allocator<ScintbarHit>* ScintbarHitAllocator;

inline void* ScintbarHit::operator new(size_t)
{
  if (!ScintbarHitAllocator) {
       ScintbarHitAllocator = new G4Allocator<ScintbarHit>;
  }
  return (void*)ScintbarHitAllocator->MallocSingle();
}

inline void ScintbarHit::operator delete(void* aHit)
{
  ScintbarHitAllocator->FreeSingle((ScintbarHit*) aHit);
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
