// adware_sterling.cpp
//
// Ver gus/domain/combat/adware_sterling.hpp pro contrato/racional completo.

#include "gus/domain/combat/adware_sterling.hpp"

namespace gus::domain::combat {

AdwareGateResult AdwareExposureTracker::roll_exposure(IRandomSource& rng) {
    ++exposure_count_;

    if (exposure_count_ <= kAdwareAlwaysShowThreshold)
        return AdwareGateResult{AdwareOutcome::ShowFull, exposure_count_};

    const int roll = rng.next(100);
    const AdwareOutcome outcome =
        roll < kAdwareShowChanceAfter3 ? AdwareOutcome::ShowFull : AdwareOutcome::Skip;
    return AdwareGateResult{outcome, exposure_count_};
}

}  // namespace gus::domain::combat
