// This file is part of the Acts project.
//
// Copyright (C) 2017-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ActsExamples/Framework/IReader.hpp"
#include "ActsExamples/Framework/IService.hpp"
#include "ActsExamples/Framework/ProcessCode.hpp"
#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Propagator/MaterialInteractor.hpp>
#include <Acts/Utilities/Logger.hpp>

#include <mutex>
#include <vector>

class TChain;

namespace ttlindkvist {

/// @class RootParticleReader
///
/// @brief Reads in Particles information from a root file
class RootNTupleReader : public ActsExamples::IReader {
 public:
  /// @brief The nested configuration struct
  struct Config {
    std::string treeName = "EventTree;1";  ///< name of the input tree
    std::string filePath = "/workspaces/CERN/4DTracking/ntuples_condor/OutDir_1/hist-Rel21sample.root";                ///< The name of the input file
    /// Whether the events are ordered or not
    bool orderedEvents = true;
  };

  /// Constructor
  /// @param config The Configuration struct
  RootNTupleReader(const Config& config, Acts::Logging::Level level);

  /// Destructor
  ~RootNTupleReader();

  /// Framework name() method
  std::string name() const final override { return "RootNTupleReader"; }

  /// Return the available events range.
  std::pair<size_t, size_t> availableEvents() const final override;

  /// Read out data from the input stream
  ///
  /// @param context The algorithm context
  ActsExamples::ProcessCode read(
      const ActsExamples::AlgorithmContext& context) final override;

  /// Readonly access to the config
  const Config& config() const { return m_cfg; }

 private:
  /// Private access to the logging instance
  const Acts::Logger& logger() const { return *m_logger; }

  /// The config class
  Config m_cfg;

  std::unique_ptr<const Acts::Logger> m_logger;

  /// mutex used to protect multi-threaded reads
  std::mutex m_read_mutex;

  /// The number of events
  size_t m_events = 0;

  /// The input tree name
  TChain* m_inputChain = nullptr;

  /// Event identifier.
  int32_t m_eventNumber;
  /// The entry numbers for accessing events in increased order (there could be
  /// multiple entries corresponding to one event number)
  std::vector<long long> m_entryNumbers = {};
  
  std::vector<float> *m_track_d0      = new std::vector<float>;
  std::vector<float> *m_track_z0      = new std::vector<float>;
  std::vector<float> *m_track_theta   = new std::vector<float>;
  std::vector<float> *m_track_phi     = new std::vector<float>;
  std::vector<float> *m_track_qOverP  = new std::vector<float>;
  std::vector<float> *m_track_t       = new std::vector<float>;
  std::vector<float> *m_track_z       = new std::vector<float>;


  // std::vector<uint64_t>* m_particleId       = new std::vector<uint64_t>;
  // std::vector<int32_t>* m_particleType      = new std::vector<int32_t>;
  // std::vector<uint32_t>* m_process          = new std::vector<uint32_t>;
  // std::vector<float>* m_vx                  = new std::vector<float>;
  // std::vector<float>* m_vy                  = new std::vector<float>;
  // std::vector<float>* m_vz                  = new std::vector<float>;
  // std::vector<float>* m_vt                  = new std::vector<float>;
  // std::vector<float>* m_px                  = new std::vector<float>;
  // std::vector<float>* m_py                  = new std::vector<float>;
  // std::vector<float>* m_pz                  = new std::vector<float>;
  // std::vector<float>* m_m                   = new std::vector<float>;
  // std::vector<float>* m_q                   = new std::vector<float>;
  // std::vector<float>* m_eta                 = new std::vector<float>;
  // std::vector<float>* m_phi                 = new std::vector<float>;
  // std::vector<float>* m_pt                  = new std::vector<float>;
  // std::vector<float>* m_p                   = new std::vector<float>;
  // std::vector<uint32_t>* m_vertexPrimary    = new std::vector<uint32_t>;
  // std::vector<uint32_t>* m_vertexSecondary  = new std::vector<uint32_t>;
  // std::vector<uint32_t>* m_particle         = new std::vector<uint32_t>;
  // std::vector<uint32_t>* m_generation       = new std::vector<uint32_t>;
  // std::vector<uint32_t>* m_subParticle      = new std::vector<uint32_t>;
};

}  // namespace ttlindkvist
