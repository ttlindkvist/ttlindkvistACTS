// This file is part of the Acts project.
//
// Copyright (C) 2017-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/RootNTupleReader.hpp"

#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "ActsExamples/EventData/SimParticle.hpp"
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "ActsExamples/Utilities/Paths.hpp"

#include <iostream>

#include <TChain.h>
#include <TFile.h>
#include <TMath.h>

/* TODO

Load some data in - try to print it out

Should load relevant EventTree<x> automatically from .root

*/


ttlindkvist::RootNTupleReader::RootNTupleReader(
    const ttlindkvist::RootNTupleReader::Config& config,
    Acts::Logging::Level level)
    : ActsExamples::IReader(),
      m_cfg(config),
      m_logger(Acts::getDefaultLogger(name(), level)),
      m_events(0),
      m_inputChain(nullptr) {
  m_inputChain = new TChain(m_cfg.treeName.c_str());

  if (m_cfg.filePath.empty()) {
    throw std::invalid_argument("Missing input filename");
  }
  if (m_cfg.treeName.empty()) {
    throw std::invalid_argument("Missing tree name");
  }

  // Set the branches
  m_inputChain->SetBranchAddress("EventNumber",   &m_eventNumber);
  m_inputChain->SetBranchAddress("track_d0",      &m_track_d0);
  m_inputChain->SetBranchAddress("track_z0",      &m_track_z0);
  m_inputChain->SetBranchAddress("track_theta",   &m_track_theta);
  m_inputChain->SetBranchAddress("track_phi",     &m_track_phi);
  m_inputChain->SetBranchAddress("track_qOverP",  &m_track_qOverP);
  m_inputChain->SetBranchAddress("track_t",       &m_track_t);
  m_inputChain->SetBranchAddress("track_z",       &m_track_z);
  
  // Covariance stuff
  m_inputChain->SetBranchAddress("track_var_d0",            &m_track_var_d0);
  m_inputChain->SetBranchAddress("track_var_z0",            &m_track_var_z0);
  m_inputChain->SetBranchAddress("track_var_phi",           &m_track_var_phi);
  m_inputChain->SetBranchAddress("track_var_theta",         &m_track_var_theta);
  m_inputChain->SetBranchAddress("track_var_qOverP",        &m_track_var_qOverP);
  m_inputChain->SetBranchAddress("track_cov_d0z0",          &m_track_cov_d0z0);
  m_inputChain->SetBranchAddress("track_cov_d0phi",         &m_track_cov_d0phi);
  m_inputChain->SetBranchAddress("track_cov_d0theta",       &m_track_cov_d0theta);
  m_inputChain->SetBranchAddress("track_cov_d0qOverP",      &m_track_cov_d0qOverP);
  m_inputChain->SetBranchAddress("track_cov_z0phi",         &m_track_cov_z0phi);
  m_inputChain->SetBranchAddress("track_cov_z0theta",       &m_track_cov_z0theta);
  m_inputChain->SetBranchAddress("track_cov_z0qOverP",      &m_track_cov_z0qOverP);
  m_inputChain->SetBranchAddress("track_cov_phitheta",      &m_track_cov_phitheta);
  m_inputChain->SetBranchAddress("track_cov_phiqOverP",     &m_track_cov_phiqOverP);
  m_inputChain->SetBranchAddress("track_cov_tehtaqOverP",   &m_track_cov_tehtaqOverP);

  auto path = m_cfg.filePath;

  // add file to the input chain
  m_inputChain->Add(path.c_str());
  ACTS_DEBUG("Adding File " << path << " to tree '" << m_cfg.treeName << "'.");

  m_events = m_inputChain->GetEntries();
  ACTS_DEBUG("The full chain has " << m_events << " entries.");

  // If the events are not in order, get the entry numbers for ordered events
  if (not m_cfg.orderedEvents) {
    m_entryNumbers.resize(m_events);
    m_inputChain->Draw("EventNumber", "", "goff");
    // Sort to get the entry numbers of the ordered events
    TMath::Sort(m_inputChain->GetEntries(), m_inputChain->GetV1(),
                m_entryNumbers.data(), false);
  }
}

std::pair<size_t, size_t> ttlindkvist::RootNTupleReader::availableEvents()
    const {
  return {0u, m_events};
}


ActsExamples::ProcessCode ttlindkvist::RootNTupleReader::read(
    const ActsExamples::AlgorithmContext& context) {
  ACTS_DEBUG("Trying to read recorded particles.");

  //In Iterative algorithm:
  // const auto& inputTrackParameters =
  //     ctx.eventStore.get<TrackParametersContainer>(m_cfg.inputTrackParameters);
  // const auto& inputTrackPointers =
  //     makeTrackParametersPointerContainer(inputTrackParameters);
  Acts::Vector3 pos(0, 0, 0);
  std::shared_ptr<Acts::PerigeeSurface> surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(pos);

  // read in the track parameters
  if (m_inputChain != nullptr && context.eventNumber < m_events) {
    // lock the mutex
    std::lock_guard<std::mutex> lock(m_read_mutex);
    // now read
    // Read the correct entry
    auto entry = context.eventNumber;
    if (not m_cfg.orderedEvents and entry < m_entryNumbers.size()) {
      entry = m_entryNumbers[entry];
    }
    m_inputChain->GetEntry(entry);
    ACTS_INFO("Reading event: " << context.eventNumber
                                << " stored as entry: " << entry);

    
    unsigned int nTracks = m_track_d0->size();
    
    ACTS_DEBUG("nTracks = " << nTracks);
    
    std::vector<Acts::BoundTrackParameters> trackContainer;
    for(unsigned int i = 0; i< nTracks; i++){
      using ParametersVector = Acts::BoundVector;
      Acts::BoundVector params;
       // NOTE the order of Loc to d0, z0 is unknown to me at this point
      params[Acts::BoundIndices::eBoundLoc0]    = (*m_track_d0)[i];
      params[Acts::BoundIndices::eBoundLoc1]    = (*m_track_z0)[i];
      params[Acts::BoundIndices::eBoundPhi]     = (*m_track_phi)[i];
      params[Acts::BoundIndices::eBoundTheta]   = (*m_track_theta)[i];
      params[Acts::BoundIndices::eBoundQOverP]  = (*m_track_qOverP)[i];
      params[Acts::BoundIndices::eBoundTime]    = (*m_track_t)[i];
      
      double q = 1;
      //Construct and fill covariance matrix
      Acts::BoundSymMatrix cov;
      //Variances
      cov[Acts::BoundIndices::eBoundLoc0][Acts::BoundIndices::eBoundLoc0] = (*m_track_var_d0)[i];
      cov[Acts::BoundIndices::eBoundLoc1][Acts::BoundIndices::eBoundLoc1] = (*m_track_var_z0)[i];
      cov[Acts::BoundIndices::eBoundPhi][Acts::BoundIndices::eBoundPhi] =   (*m_track_var_phi)[i];
      cov[Acts::BoundIndices::eBoundTheta][Acts::BoundIndices::eBoundTheta] = (*m_track_var_theta)[i];
      cov[Acts::BoundIndices::eBoundQOverP][Acts::BoundIndices::eBoundQOverP] = (*m_track_var_qOverP)[i];
      
      //TODO: Covariances
      
      
      Acts::BoundTrackParameters tc(surface, params, q, cov);
      trackContainer.push_back(tc);
    }
    context.eventStore.add(m_cfg.nTupleTrackParameters, std::move(trackContainer));
  }

  // Return success flag
  return ActsExamples::ProcessCode::SUCCESS;
}

ttlindkvist::RootNTupleReader::~RootNTupleReader() {
  delete m_track_d0;
  delete m_track_z0;
  delete m_track_theta;
  delete m_track_phi;
  delete m_track_qOverP;
  delete m_track_t;
  delete m_track_z;
  delete m_track_var_d0;
  delete m_track_var_z0;
  delete m_track_var_phi;
  delete m_track_var_theta;
  delete m_track_var_qOverP;
  delete m_track_cov_d0z0;
  delete m_track_cov_d0phi;
  delete m_track_cov_d0theta;
  delete m_track_cov_d0qOverP;
  delete m_track_cov_z0phi;
  delete m_track_cov_z0theta;
  delete m_track_cov_z0qOverP;
  delete m_track_cov_phitheta;
  delete m_track_cov_phiqOverP;
  delete m_track_cov_tehtaqOverP;
}



// name                 | typename                 | interpretation                
// ---------------------+--------------------------+-------------------------------
// mcChannelNumber      | int32_t                  | AsDtype('>i4')
// EventNumber          | int32_t                  | AsDtype('>i4')
// RunNumber            | int32_t                  | AsDtype('>i4')
// BCID                 | int32_t                  | AsDtype('>i4')
// mu                   | float                    | AsDtype('>f4')
// muActual             | float                    | AsDtype('>f4')
// beamspot_x           | float                    | AsDtype('>f4')
// beamspot_y           | float                    | AsDtype('>f4')
// beamspot_z           | float                    | AsDtype('>f4')
// beamspot_sigX        | float                    | AsDtype('>f4')
// beamspot_sigY        | float                    | AsDtype('>f4')
// beamspot_sigZ        | float                    | AsDtype('>f4')
// met_Truth            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// mpx_Truth            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// mpy_Truth            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// sumet_Truth          | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_prob           | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_d0             | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_z0             | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_theta          | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_phi            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_qOverP         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t              | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_z              | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_var_d0         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_var_z0         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_var_phi        | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_var_theta      | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_var_qOverP     | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_d0z0       | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_d0phi      | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_d0theta    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_d0qOverP   | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_z0phi      | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_z0theta    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_z0qOverP   | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_phitheta   | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_phiqOverP  | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_cov_tehtaqO... | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t30            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t60            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t90            | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t120           | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// track_t180           | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// tracks_numPix        | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// tracks_numSCT        | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// tracks_numPix1L      | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// tracks_numPix2L      | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// jet_pt               | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_eta              | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_phi              | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_m                | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_q                | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_ptmatched_pt     | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_ptmatched_eta    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_ptmatched_phi    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_ptmatched_m      | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_drmatched_pt     | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_drmatched_eta    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_drmatched_phi    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_drmatched_m      | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// jet_isPU             | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// jet_isHS             | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// jet_label            | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// recovertex_x         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// recovertex_y         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// recovertex_z         | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// recovertex_sumPt2    | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// recovertex_isPU      | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// recovertex_isHS      | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// truthvertex_x        | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// truthvertex_y        | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// truthvertex_z        | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// truthvertex_t        | std::vector<float>       | AsJagged(AsDtype('>f4'), he...
// truthvertex_isPU     | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// truthvertex_isHS     | std::vector<int32_t>     | AsJagged(AsDtype('>i4'), he...
// jet_tracks_idx       | std::vector<std::vect... | AsObjects(AsVector(True, As...
// recovertex_tracks... | std::vector<std::vect... | AsObjects(AsVector(True, As...
// truthvertex_track... | std::vector<std::vect... | AsObjects(AsVector(True, As...
