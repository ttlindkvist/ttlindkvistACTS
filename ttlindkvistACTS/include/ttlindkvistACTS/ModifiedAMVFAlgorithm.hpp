// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "ActsExamples/Framework/BareAlgorithm.hpp"

#include <string>

namespace ttlindkvist {

class TutorialVertexFinderAlgorithm final : public ActsExamples::BareAlgorithm {
 public:
  struct Config {
    /// Input track parameters collection
    std::string inputTrackParameters;
    /// Output proto vertex collection
    std::string outputProtoVertices;
    /// The magnetic field
    std::shared_ptr<Acts::MagneticFieldProvider> bField;
  };

  TutorialVertexFinderAlgorithm(const Config& cfg, Acts::Logging::Level lvl);

  /// Find vertices.
  ///
  /// @param ctx is the algorithm context with event information
  /// @return a process code indication success or failure
  ActsExamples::ProcessCode execute(const ActsExamples::AlgorithmContext& ctx) const final;

  /// Get readonly access to the config parameters
  const Config& config() const { return m_cfg; }

 private:
  Config m_cfg;
};

}  // namespace ActsExamples
