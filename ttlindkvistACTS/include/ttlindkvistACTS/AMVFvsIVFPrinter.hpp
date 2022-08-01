// This file is part of the Acts project.
//
// Copyright (C) 2016-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "ActsExamples/Framework/BareAlgorithm.hpp"

#include <string>

namespace ttlindkvist {

class AMVFvsIVFPrinter final : public ActsExamples::BareAlgorithm{
 public:
  struct Config {
    /// Number of events to print, negative -> all
    int numberOfEvents = -1;
    std::string outputDir = "AMVFvsIVF/";
    std::string generatedTrackParameters = "generatedTrackParameters";
    std::string iterativeRecoVtxParameters;
    std::string AMVFRecoVtxParameters;
};

  AMVFvsIVFPrinter(const Config& config,
                                 Acts::Logging::Level level);

  /// @param ctx is the algorithm context with event information
  /// @return a process code indication success or failure
  ActsExamples::ProcessCode execute(const ActsExamples::AlgorithmContext& ctx) const final;

  /// Get readonly access to the config parameters
  const Config& config() const { return m_cfg; }

 private:
  Config m_cfg;
};

}  // namespace ttlindkvist
