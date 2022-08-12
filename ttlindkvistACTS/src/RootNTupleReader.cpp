// This file is part of the Acts project.
//
// Copyright (C) 2017-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/RootNTupleReader.hpp"

#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Vertexing/Vertex.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "ActsExamples/EventData/SimParticle.hpp"
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "ActsExamples/Utilities/Paths.hpp"

#include <iostream>

#include <TChain.h>
#include <TFile.h>
#include <TMath.h>

//Debugging - For printing read parameters
#include <boost/format.hpp>
#include <fstream>

#include "ttlindkvistACTS/goodRecoVtxTracks.hpp"

/* TODO

Load some data in - try to print it out

Should load relevant EventTree<x> as = ctx.eventStore.get<ttlindkvist::RootNTupleReader::BranchPointerWrapper>(m_cfg.generatedTrackParameters);

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
  m_inputChain->SetBranchAddress("track_d0",      &m_branches.m_track_d0);
  m_inputChain->SetBranchAddress("track_z0",      &m_branches.m_track_z0);
  m_inputChain->SetBranchAddress("track_theta",   &m_branches.m_track_theta);
  m_inputChain->SetBranchAddress("track_phi",     &m_branches.m_track_phi);
  m_inputChain->SetBranchAddress("track_qOverP",  &m_branches.m_track_qOverP);
  m_inputChain->SetBranchAddress("track_t",       &m_branches.m_track_t);
  m_inputChain->SetBranchAddress("track_z",       &m_branches.m_track_z);
  
  // Covariance stuff
  m_inputChain->SetBranchAddress("track_var_d0",            &m_branches.m_track_var_d0);
  m_inputChain->SetBranchAddress("track_var_z0",            &m_branches.m_track_var_z0);
  m_inputChain->SetBranchAddress("track_var_phi",           &m_branches.m_track_var_phi);
  m_inputChain->SetBranchAddress("track_var_theta",         &m_branches.m_track_var_theta);
  m_inputChain->SetBranchAddress("track_var_qOverP",        &m_branches.m_track_var_qOverP);
  m_inputChain->SetBranchAddress("track_cov_d0z0",          &m_branches.m_track_cov_d0z0);
  m_inputChain->SetBranchAddress("track_cov_d0phi",         &m_branches.m_track_cov_d0phi);
  m_inputChain->SetBranchAddress("track_cov_d0theta",       &m_branches.m_track_cov_d0theta);
  m_inputChain->SetBranchAddress("track_cov_d0qOverP",      &m_branches.m_track_cov_d0qOverP);
  m_inputChain->SetBranchAddress("track_cov_z0phi",         &m_branches.m_track_cov_z0phi);
  m_inputChain->SetBranchAddress("track_cov_z0theta",       &m_branches.m_track_cov_z0theta);
  m_inputChain->SetBranchAddress("track_cov_z0qOverP",      &m_branches.m_track_cov_z0qOverP);
  m_inputChain->SetBranchAddress("track_cov_phitheta",      &m_branches.m_track_cov_phitheta);
  m_inputChain->SetBranchAddress("track_cov_phiqOverP",     &m_branches.m_track_cov_phiqOverP);
  m_inputChain->SetBranchAddress("track_cov_tehtaqOverP",   &m_branches.m_track_cov_tehtaqOverP);
  
  // Truth vertex
  m_inputChain->SetBranchAddress("truthvertex_x", &m_branches.m_truthvertex_x); 
  m_inputChain->SetBranchAddress("truthvertex_y", &m_branches.m_truthvertex_y); 
  m_inputChain->SetBranchAddress("truthvertex_z", &m_branches.m_truthvertex_z); 
  m_inputChain->SetBranchAddress("truthvertex_t", &m_branches.m_truthvertex_t);
   
  m_inputChain->SetBranchAddress("recovertex_x", &m_branches.m_recovertex_x); 
  m_inputChain->SetBranchAddress("recovertex_y", &m_branches.m_recovertex_y); 
  m_inputChain->SetBranchAddress("recovertex_z", &m_branches.m_recovertex_z); 
  // m_inputChain->SetBranchAddress("truthvertex_tracks_idx",   &m_truthvertex_tracks_idx);
  
  m_inputChain->SetBranchAddress("beamspot_x", &m_branches.m_beamspot_x); 
  m_inputChain->SetBranchAddress("beamspot_y", &m_branches.m_beamspot_y); 
  m_inputChain->SetBranchAddress("beamspot_z", &m_branches.m_beamspot_z); 
  m_inputChain->SetBranchAddress("beamspot_sigX", &m_branches.m_beamspot_sigX); 
  m_inputChain->SetBranchAddress("beamspot_sigY", &m_branches.m_beamspot_sigY); 
  m_inputChain->SetBranchAddress("beamspot_sigZ", &m_branches.m_beamspot_sigZ); 
  
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
  ACTS_DEBUG("Trying to read track parameters from ntuple.");

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

    unsigned int nTracks    = m_branches.m_track_d0->size();
    unsigned int nTruthVtx  = m_branches.m_truthvertex_z->size();
    unsigned int nRecoVtx  = m_branches.m_recovertex_z->size();
    
    ACTS_DEBUG("nTracks = " << nTracks);
    ACTS_DEBUG("nTruthVtx = " << nTruthVtx);
    ACTS_DEBUG("nRecoVtx = " << nRecoVtx);
    
    std::vector<Acts::BoundTrackParameters> trackContainer;
    for(unsigned int i = 0; i<nTracks; i++){
    // for(const int& i : goodTrackIdxs[entry]){
      //Debugging check by printing read parameters to file - to compare later via external program
      using ParametersVector = Acts::BoundVector;
      Acts::BoundVector params;
      
      params[Acts::BoundIndices::eBoundLoc0]    = (*m_branches.m_track_d0)[i];
      params[Acts::BoundIndices::eBoundLoc1]    = (*m_branches.m_track_z0)[i];
      params[Acts::BoundIndices::eBoundPhi]     = (*m_branches.m_track_phi)[i];
      params[Acts::BoundIndices::eBoundTheta]   = (*m_branches.m_track_theta)[i];
      params[Acts::BoundIndices::eBoundQOverP]  = (*m_branches.m_track_qOverP)[i];
      params[Acts::BoundIndices::eBoundTime]    = (*m_branches.m_track_t)[i];
      
      double q = 1;
      //Construct and fill covariance matrix
      Acts::BoundSymMatrix cov;
      //Variances
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc0)     = (*m_branches.m_track_var_d0)[i];
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundLoc1)     = (*m_branches.m_track_var_z0)[i];
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundPhi)       = (*m_branches.m_track_var_phi)[i];
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundTheta)   = (*m_branches.m_track_var_theta)[i];
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundQOverP) = (*m_branches.m_track_var_qOverP)[i];
      
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc1)    = (*m_branches.m_track_cov_d0z0)[i];
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundPhi)     = (*m_branches.m_track_cov_d0phi)[i];
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundTheta)   = (*m_branches.m_track_cov_d0theta)[i];
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundQOverP)  = (*m_branches.m_track_cov_d0qOverP)[i];
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundPhi)     = (*m_branches.m_track_cov_z0phi)[i];
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundTheta)   = (*m_branches.m_track_cov_z0theta)[i];
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundQOverP)  = (*m_branches.m_track_cov_z0qOverP)[i];
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundTheta)    = (*m_branches.m_track_cov_phitheta)[i];
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundQOverP)   = (*m_branches.m_track_cov_phiqOverP)[i];
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundQOverP) = (*m_branches.m_track_cov_tehtaqOverP)[i];
      
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundLoc0)    = (*m_branches.m_track_cov_d0z0)[i];
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundLoc0)     = (*m_branches.m_track_cov_d0phi)[i];
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundLoc0)   = (*m_branches.m_track_cov_d0theta)[i];
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundLoc0)  = (*m_branches.m_track_cov_d0qOverP)[i];
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundLoc1)     = (*m_branches.m_track_cov_z0phi)[i];
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundLoc1)   = (*m_branches.m_track_cov_z0theta)[i];
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundLoc1)  = (*m_branches.m_track_cov_z0qOverP)[i];
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundPhi)    = (*m_branches.m_track_cov_phitheta)[i];
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundPhi)   = (*m_branches.m_track_cov_phiqOverP)[i];
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundTheta) = (*m_branches.m_track_cov_tehtaqOverP)[i];
      
      //Try add time covariances
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundTime)   = 1;
      
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundLoc0)   = 0;
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundLoc1)   = 0;
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundPhi)    = 0;
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundTheta)  = 0;
      cov(Acts::BoundIndices::eBoundTime, Acts::BoundIndices::eBoundQOverP) = 0;
      cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundTime)   = 0;
      cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundTime)   = 0;
      cov(Acts::BoundIndices::eBoundPhi, Acts::BoundIndices::eBoundTime)    = 0;
      cov(Acts::BoundIndices::eBoundTheta, Acts::BoundIndices::eBoundTime)  = 0;
      cov(Acts::BoundIndices::eBoundQOverP, Acts::BoundIndices::eBoundTime) = 0;

      Acts::BoundTrackParameters tc(surface, params, q, cov);
      trackContainer.push_back(tc);
    }
    
    std::vector<Acts::Vector4> truthVertexContainer;
    for(unsigned int i = 0; i<nTruthVtx; i++){
      Acts::Vector4 vtx((*m_branches.m_truthvertex_x)[i],
                        (*m_branches.m_truthvertex_y)[i],
                        (*m_branches.m_truthvertex_z)[i],
                        (*m_branches.m_truthvertex_t)[i] );
      truthVertexContainer.push_back(vtx);
    }
    std::vector<Acts::Vector4> recoVertexContainer;
    for(unsigned int i = 0; i<nRecoVtx; i++){
      Acts::Vector4 vtx((*m_branches.m_recovertex_x)[i],
                        (*m_branches.m_recovertex_y)[i],
                        (*m_branches.m_recovertex_z)[i],
                        0 );
      recoVertexContainer.push_back(vtx);
    }
    
    Acts::Vertex<Acts::BoundTrackParameters> beamspotConstraint;
    // Acts::Vector3 beamspotPos;
    // Acts::SymMatrix3 beamspotCov;

    // beamspotPos << m_branches.m_beamspot_x, m_branches.m_beamspot_y,
    //     m_branches.m_beamspot_z;
    // beamspotCov << m_branches.m_beamspot_sigX*m_branches.m_beamspot_sigX, 0, 0, 
    //         0, m_branches.m_beamspot_sigY*m_branches.m_beamspot_sigY, 0, 
    //         0, 0, m_branches.m_beamspot_sigZ*m_branches.m_beamspot_sigZ;
            
    // beamspotConstraint.setPosition(beamspotPos);
    // beamspotConstraint.setCovariance(beamspotCov);
    
    
    Acts::Vector4 beamspotFullPos;
    Acts::SymMatrix4 beamspotFullCov;

    beamspotFullPos << m_branches.m_beamspot_x, m_branches.m_beamspot_y,
        m_branches.m_beamspot_z, 0;
    beamspotFullCov << m_branches.m_beamspot_sigX*m_branches.m_beamspot_sigX, 0, 0, 0,
                       0, m_branches.m_beamspot_sigY*m_branches.m_beamspot_sigY, 0, 0,
                       0, 0, m_branches.m_beamspot_sigZ*m_branches.m_beamspot_sigZ, 0,
                       0, 0, 0, 0.18*0.18;    
    
    beamspotConstraint.setFullPosition(beamspotFullPos);
    beamspotConstraint.setFullCovariance(beamspotFullCov);
    context.eventStore.add("beamspotConstraint", std::move(beamspotConstraint));
    
    
    context.eventStore.add(m_cfg.nTupleTrackParameters, std::move(trackContainer));
    context.eventStore.add(m_cfg.nTupleTruthVtxParameters, std::move(truthVertexContainer));
    context.eventStore.add(m_cfg.nTupleRecoVtxParameters, std::move(recoVertexContainer));
    // Hasn't been checked yet, but should work
    // BranchPointerWrapper branchesCopy = m_branches; 
    context.eventStore.add(m_cfg.nTupleBranchPointerWrapper, &m_branches); 
  }

  // Return success flag
  return ActsExamples::ProcessCode::SUCCESS;
}

ttlindkvist::RootNTupleReader::~RootNTupleReader() {
}

//For checking propper reading
// std::ofstream outputFile(boost::str(boost::format("ntuple_check/event%1%.txt") % context.eventNumber));;
  // if(context.eventNumber == 0){
  //   outputFile << params.transpose() << std::endl;
  // }
// outputFile.close();

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
