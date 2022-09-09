#ifndef ABLATELIBRARY_CONSTANTS_HPP
#define ABLATELIBRARY_CONSTANTS_HPP

#include <petscsystypes.h>

namespace ablate::utilities {
class Constants {
   public:
    //! Stefan-Boltzman Constant (J/K)
    inline static const PetscReal sbc = 5.6696e-8;

    //! Pi
    inline static const PetscReal pi = 3.1415926535897932384626433832795028841971693993;

    //! A very tiny number
    inline static const PetscReal tiny = 1e-30;

    //! A somewhat small number
    inline static const PetscReal small = 1e-10;
};
}  // namespace ablate::utilities

#endif  // ABLATELIBRARY_CONSTANTS_HPP