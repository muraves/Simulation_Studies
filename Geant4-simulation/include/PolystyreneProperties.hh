/// \file PolystyreneProperties.hh
/// \brief Definition of the PolystyreneProperties class

#ifndef PolystyreneProperties_h
#define PolystyreneProperties_h 1

namespace Materials {
    static const G4String kPOLYSTYRENE = "G4_POLYSTYRENE";
    static const G4double POLYSTYRENE_DENSITY = 1.06 * g / cm3;
    static constexpr G4int H_NATOMS = 8;
    static constexpr G4int C_NATOMS = 8;


    static const G4double POLYSTYRENE_SCINT_RESLN_SCALE = 1;
    // Quarati et al (not encapsulated)
    static const G4double POLYSTYRENE_SCINT_YIELD = 10. / MeV;
    static const G4double POLYSTYRENE_DECAY_TIME_CONSTANT = 2.5 * ns;
    static const G4double POLYSTYRENE_RISE_TIME = 0.9 * ns;

    // Quarati et al, 2.5mm thickness
    // energies of cebr3 emission spectrum
    const std::vector<G4double> POLYSTYRENE_SCINT_OPTICAL_ENERGIES = {2.067*eV, 2.156*eV, 2.255*eV, 2.296*eV, 2.340*eV, 2.385*eV, 2.431*eV, 2.480*eV, 2.531*eV, 2.583*eV,
    2.638*eV, 2.696*eV, 2.756*eV, 2.818*eV, 2.851*eV, 2.884*eV, 2.952*eV, 3.024*eV, 3.100*eV, 3.179*eV, 3.543*eV};

    // relative intensities of emission spectrum
    const std::vector<G4double> POLYSTYRENE_SCINT_OPTICAL_INTENSITIES = {0.0, 0.1, 0.25, 0.35, 0.7, 0.12, 0.18, 0.2, 0.3, 0.42, 0.4, 0.5, 0.7, 0.72, 0.69, 0.75, 1, 0.5, 0.02, 0.01, 0.0};


    // 2022 February 04: refractive index experimentially is about 2.09 for the emisison range, and relatively constant.
    // update to reflect that here.
    const std::vector<G4double> POLYSTYRENE_REFR_IDX_ENERGIES = {0.1*eV, 5*eV};
    const std::vector<G4double> POLYSTYRENE_REFR_IDXS = { 1.6, 1.6 };

    const std::vector<G4double> POLYSTYRENE_ABS_LEN_ENERGIES = {0.1*eV, 5*eV};

    // Typical absorption length for an inorganic scintillator
    const std::vector<G4double> POLYSTYRENE_ABS_LEN(POLYSTYRENE_ABS_LEN_ENERGIES.size(), 3.5 * m);
}

#endif