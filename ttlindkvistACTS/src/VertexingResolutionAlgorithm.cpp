// This file is part of the Acts project.
//
// Copyright (C) 2019-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/VertexingResolutionAlgorithm.hpp"

#include "Acts/EventData/TrackParameters.hpp"
#include "ActsExamples/Framework/ProcessCode.hpp"
#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Utilities/Logger.hpp>
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "Acts/Vertexing/Vertex.hpp"

// Extra
#include <boost/format.hpp>
// #include <boost/str.hpp>
#include <fstream>

ttlindkvist::VertexingResolutionAlgorithm::VertexingResolutionAlgorithm(
    const Config& config, Acts::Logging::Level level)
    : ActsExamples::BareAlgorithm("VertexingResolutionAlgorithm", level),
      m_cfg(config) {
    if(m_cfg.outputDir.empty()) {
        throw std::invalid_argument("Missing output directory");
    }
    if(m_cfg.ntupleTruthVtxParameters.empty()) {
        throw std::invalid_argument("Missing truth vtx parameter handle");
    }
    if(m_cfg.recoVtxParameters.empty()) {
        throw std::invalid_argument("Missing reco vtx parameter handle");
    }
}

ActsExamples::ProcessCode ttlindkvist::VertexingResolutionAlgorithm::execute(
    const ActsExamples::AlgorithmContext& ctx) const {
    
    const auto& inputTruthVtxs =
        ctx.eventStore.get<std::vector<Acts::Vector4>>(m_cfg.ntupleTruthVtxParameters);
    const auto& inputRecoVtxs =
      ctx.eventStore.get<std::vector<Acts::Vertex<Acts::BoundTrackParameters>>>(m_cfg.recoVtxParameters);
    
    // const double maxDeltaZ = 1; // mm
    // const double minSumPt2 = 15**2; // GeV**2
    // std::vector<Acts::Vertex<Acts::BoundTrackParameters>> filteredRecoVtxs;

    // std::vector<double> deltaZs;

    // for(const auto &recoVtx : inputRecoVtxs){
    //     double sumPt2 = 0;
    //     for(const auto& track : recoVtx.tracks()){
    //         sumPt2 += track.transverseMomentum()**2;
    //     }
    //     if(sumPt2 >= minSumPt2){
    //         //Compute z vertexing resolution
    //         for(const auto &t_vtx : inputTruthVtxs){
    //             double truth_z = t_vtx[2];
                
    //             double deltaZ = truth_z - recoVtx.position()[2];
    //             if(deltaZ*deltaZ < maxDeltaZ*maxDeltaZ){
    //                 deltaZs.push_back(deltaZ);
    //             }
    //         }
    //     }
    // }
    
    //Print output
    
    
    // ctx.eventstore.add(m_cfg.output, std::move(deltaZs));
    
    return ActsExamples::ProcessCode::SUCCESS;
}
