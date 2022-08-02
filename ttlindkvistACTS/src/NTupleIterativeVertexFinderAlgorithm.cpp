// This file is part of the Acts project.
//
// Copyright (C) 2019-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/NTupleIterativeVertexFinderAlgorithm.hpp"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Vertexing/FullBilloirVertexFitter.hpp"
#include "Acts/Vertexing/HelicalTrackLinearizer.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"
#include "Acts/Vertexing/IterativeVertexFinder.hpp"
#include "Acts/Vertexing/LinearizedTrack.hpp"
#include "Acts/Vertexing/Vertex.hpp"
#include "Acts/Vertexing/VertexFinderConcept.hpp"
#include "Acts/Vertexing/VertexingOptions.hpp"
// #include "Acts/Vertexing/ZScanVertexFinder.hpp"
#include "ActsExamples/EventData/ProtoVertex.hpp"
#include "ActsExamples/EventData/Track.hpp"
#include "ActsExamples/Framework/RandomNumbers.hpp"
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "ActsExamples/Utilities/Options.hpp"

#include <chrono>

#include "VertexingHelpers.hpp"

#include "ttlindkvistACTS/CustomVertexSeeder.hpp"

ttlindkvist::NTupleIterativeVertexFinderAlgorithm::NTupleIterativeVertexFinderAlgorithm(
    const Config& config, Acts::Logging::Level level)
    : ActsExamples::BareAlgorithm("IterativeVertexFinder", level),
      m_cfg(config) {
  if (m_cfg.outputVertices.empty()) {
    throw std::invalid_argument("Missing output vertices collection");
  }
  if (m_cfg.outputTime.empty()) {
    throw std::invalid_argument("Missing output reconstruction time");
  }
}

ActsExamples::ProcessCode ttlindkvist::NTupleIterativeVertexFinderAlgorithm::execute(
    const ActsExamples::AlgorithmContext& ctx) const {
  
  // retrieve input tracks and convert into the expected format
  const auto& inputTrackParameters =
      ctx.eventStore.get<ActsExamples::TrackParametersContainer>(m_cfg.inputTrackParameters);
  const auto& inputTrackPointers =
      ActsExamples::makeTrackParametersPointerContainer(inputTrackParameters);

  using Propagator = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptions = Acts::PropagatorOptions<>;
  using Linearizer = Acts::HelicalTrackLinearizer<Propagator>;
  using VertexFitter =
      Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;
  using ImpactPointEstimator =
      Acts::ImpactPointEstimator<Acts::BoundTrackParameters, Propagator>;
  using VertexSeeder = ttlindkvist::CustomVertexSeeder<VertexFitter>;
  using VertexFinder = Acts::IterativeVertexFinder<VertexFitter, VertexSeeder>;
  using VertexFinderOptions =
      Acts::VertexingOptions<Acts::BoundTrackParameters>;

  // Set up EigenStepper
  Acts::EigenStepper<> stepper(m_cfg.bField);

  // Set up propagator with void navigator
  auto propagator = std::make_shared<Propagator>(stepper);
  PropagatorOptions propagatorOpts(ctx.geoContext, ctx.magFieldContext,
                                   Acts::LoggerWrapper{logger()});
  // Setup the vertex fitter
  VertexFitter::Config vertexFitterCfg;
  VertexFitter vertexFitter(std::move(vertexFitterCfg));
  // Setup the track linearizer
  Linearizer::Config linearizerCfg(m_cfg.bField, propagator);
  Linearizer linearizer(std::move(linearizerCfg));
  // Setup the seed finder
  ImpactPointEstimator::Config ipEstCfg(m_cfg.bField, propagator);
  ImpactPointEstimator ipEst(std::move(ipEstCfg));
  VertexSeeder::Config seederCfg(ipEst);
  VertexSeeder seeder(std::move(seederCfg));
  // Set up the actual vertex finder
  VertexFinder::Config finderCfg(std::move(vertexFitter), std::move(linearizer),
                                 std::move(seeder), ipEst);
  finderCfg.maxVertices = 200;
  finderCfg.reassignTracksAfterFirstFit = true;
  
  VertexFinder finder(finderCfg);
  VertexFinder::State state(*m_cfg.bField, ctx.magFieldContext);
  VertexFinderOptions finderOpts(ctx.geoContext, ctx.magFieldContext);

  // find vertices and measure elapsed time
  auto t1 = std::chrono::high_resolution_clock::now();
  auto result = finder.find(inputTrackPointers, finderOpts, state);
  auto t2 = std::chrono::high_resolution_clock::now();

  if (not result.ok()) {
    ACTS_ERROR("Error in vertex finder: " << result.error().message());
    return ActsExamples::ProcessCode::ABORT;
  }
  auto vertices = *result;

  ACTS_INFO("Found " << vertices.size() << " vertices in event " << ctx.eventNumber);
  for (const auto& vtx : vertices) {
    ACTS_INFO("Found vertex at " << vtx.fullPosition().transpose() << " with "
                                 << vtx.tracks().size() << " tracks. chi2 = " << vtx.fitQuality().first << " dof= " << vtx.fitQuality().second);
  }

  // store proto vertices extracted from the found vertices
  ctx.eventStore.add(m_cfg.outputProtoVertices,
                     ActsExamples::makeProtoVertices(inputTrackParameters, vertices));

  // store found vertices
  ctx.eventStore.add(m_cfg.outputVertices, std::move(vertices));

  // time in milliseconds
  int timeMS =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  // store reconstruction time
  ctx.eventStore.add(m_cfg.outputTime, std::move(timeMS));
  return ActsExamples::ProcessCode::SUCCESS;
}
