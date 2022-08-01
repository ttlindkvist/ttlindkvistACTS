// This file is part of the Acts project.
//
// Copyright (C) 2019-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/NTuplePrinting.hpp"

#include "Acts/EventData/TrackParameters.hpp"
#include "ActsExamples/Framework/ProcessCode.hpp"
#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Utilities/Logger.hpp>
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "Acts/Vertexing/Vertex.hpp"

#include "Acts/Definitions/TrackParametrization.hpp"

// Extra
#include <boost/format.hpp>
// #include <boost/str.hpp>
#include <fstream>

ttlindkvist::NTuplePrinting::NTuplePrinting(
    const Config& config, Acts::Logging::Level level)
    : ActsExamples::BareAlgorithm("NTuplePrinting", level),
      m_cfg(config) {
  if(m_cfg.outputDir.empty()) {
    throw std::invalid_argument("Missing output directory");
  }
  if(m_cfg.ntupleTruthVtxParameters.empty()) {
    throw std::invalid_argument("Missing truth vtx parameter handle");
  }
}

ActsExamples::ProcessCode ttlindkvist::NTuplePrinting::execute(
    const ActsExamples::AlgorithmContext& ctx) const {
    
  const auto& inputTruthVtxs =
        ctx.eventStore.get<std::vector<Acts::Vector4>>(m_cfg.ntupleTruthVtxParameters);
  const auto& inputNTupleRecoVtxs =
        ctx.eventStore.get<std::vector<Acts::Vector4>>(m_cfg.ntupleRecoVtxParameters);
  
  //Printing truth vtxs
  {
    //Open event file
    std::ofstream outputFile(boost::str(boost::format("%1%/event%2%_truth.txt") % m_cfg.outputDir % ctx.eventNumber));
    outputFile << inputTruthVtxs.size() << " " << inputNTupleRecoVtxs.size() << " 0" << std::endl; 
    
    for(const auto& vtx : inputTruthVtxs){
      //Writing to file
      outputFile << vtx.transpose() << std::endl;
    }
    outputFile.close();
  }
  //Printing reco vtxs
  {
    //Open event file
    std::ofstream outputFile(boost::str(boost::format("%1%/event%2%_ntuple_reco.txt") % m_cfg.outputDir % ctx.eventNumber));
    outputFile << inputTruthVtxs.size() << " " << inputNTupleRecoVtxs.size() << " 0" << std::endl; 
    
    for(const auto& vtx : inputNTupleRecoVtxs){
      //Writing to file
      outputFile << vtx.transpose() << std::endl;
    }
    outputFile.close();
  }
   
  const auto& inputTracks =
      ctx.eventStore.get<std::vector<Acts::BoundTrackParameters>>(m_cfg.ntupleTrackParameters);
  if(ctx.eventNumber < 10){
    //Open event file
    std::ofstream outputFile(boost::str(boost::format("%1%/event%2%_cov.txt") % m_cfg.outputDir % ctx.eventNumber));
    outputFile << inputTracks.size() << std::endl; 
    
    int trackCounter = 0;
    for(const auto& track : inputTracks){
        if(trackCounter > 10) break;
        if(track.covariance().has_value()){
            auto cov = track.covariance().value();
            outputFile << cov << std::endl << std::endl;
        }
        else{
            std::cout << "No covariance matrix!!!" << std::endl;
        }
        
        trackCounter++;
    }
    outputFile.close();
  }
   
  if(!m_cfg.iterativeRecoVtxParameters.empty()){
    const auto& IVFVtxs =
      ctx.eventStore.get<std::vector<Acts::Vertex<Acts::BoundTrackParameters>>>(m_cfg.iterativeRecoVtxParameters);
    //Open event file
    std::ofstream outputFile(boost::str(boost::format("%1%/event%2%_IVF.txt") % m_cfg.outputDir % ctx.eventNumber));
    outputFile << inputTruthVtxs.size() << " " << inputNTupleRecoVtxs.size() << " " << IVFVtxs.size() << std::endl; 
    
    unsigned int vtxId = 0;
    for(const auto& vtx : IVFVtxs){
      //Writing to file
      outputFile << vtx.fullPosition().transpose() << " " << vtx.tracks().size() << " "
                << vtx.fitQuality().first << " " << vtx.fitQuality().second << std::endl;
      
      //Print out the tracks of verteces with track multiplicity above some value
      if(vtx.tracks().size() > 300){
        std::ofstream vtxTrackOutputFile(boost::str(boost::format("%1%/vtx/event%2%_IVF_vtx%3%.txt") % m_cfg.outputDir % ctx.eventNumber % vtxId));    
        for(const auto& track : vtx.tracks()){
          vtxTrackOutputFile << track.fittedParams.parameters().transpose() << " \t " 
                            << track.originalParams->parameters().transpose() << " " << track.chi2Track << " " 
                            << track.ndf << " " << track.trackWeight << std::endl;
        }
        vtxTrackOutputFile.close();
      }
      vtxId++;
    }
    outputFile.close();
  }  
  if(!m_cfg.AMVFRecoVtxParameters.empty()){
    const auto& AMVFVtxs =
      ctx.eventStore.get<std::vector<Acts::Vertex<Acts::BoundTrackParameters>>>(m_cfg.AMVFRecoVtxParameters);
    //Open event file
    std::ofstream outputFile(boost::str(boost::format("%1%/event%2%_AMVF.txt") % m_cfg.outputDir % ctx.eventNumber));
    outputFile << inputTruthVtxs.size() << " " << inputNTupleRecoVtxs.size() << " " << AMVFVtxs.size() << std::endl; 
    
    for(const auto& vtx : AMVFVtxs){
      //Writing to file
      outputFile << vtx.fullPosition().transpose() << " " << vtx.tracks().size() << " "
                << vtx.fitQuality().first << " " << vtx.fitQuality().second << std::endl;
    }
    outputFile.close();
  }
  return ActsExamples::ProcessCode::SUCCESS;
}
