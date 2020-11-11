//
// Michael Tytgat
//

#ifndef Hits_h
#define Hits_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class G4AttDef;
class G4AttValue;

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

  const ScintbarHit& operator=(const ScintbarHit &right);
  G4bool operator==(const ScintbarHit &right) const;
  
  inline void *operator new(size_t);
  inline void operator delete(void *aHit);

  virtual void Draw();
  virtual void Print();

  virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
  virtual std::vector<G4AttValue>* CreateAttValues() const;
  
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
  
private:
  G4int fStationID;
  G4int fModuleID;
  G4int fBarID;
  G4double fEdep;
  G4ThreeVector fPos;
  G4int fTrackID;
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


#endif
