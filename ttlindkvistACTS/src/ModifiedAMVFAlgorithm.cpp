// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/ModifiedAMVFAlgorithm.hpp"

#include "Acts/Definitions/Units.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"

#include "Acts/Vertexing/IterativeVertexFinder.hpp"

#include "Acts/Vertexing/AdaptiveMultiVertexFitter.hpp"
#include "Acts/Vertexing/HelicalTrackLinearizer.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"
#include "Acts/Vertexing/LinearizedTrack.hpp"
#include "Acts/Vertexing/TrackDensityVertexFinder.hpp"
#include "Acts/Vertexing/Vertex.hpp"
#include "Acts/Vertexing/VertexFinderConcept.hpp"
#include "Acts/Vertexing/VertexingOptions.hpp"
#include "ActsExamples/Framework/RandomNumbers.hpp"
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "ActsExamples/Utilities/Options.hpp"

#include "VertexingHelpers.hpp"

// Extra
#include <boost/format.hpp>
// #include <boost/algorithm/string.hpp>
#include <fstream>

ttlindkvist::TutorialVertexFinderAlgorithm::TutorialVertexFinderAlgorithm(
    const Config& cfg, Acts::Logging::Level lvl)
    : ActsExamples::BareAlgorithm("TutorialVertexFinder", lvl), m_cfg(cfg) {
  if (m_cfg.inputTrackParameters.empty()) {
    throw std::invalid_argument("Missing input track parameters collection");
  }
  if (m_cfg.outputProtoVertices.empty()) {
    throw std::invalid_argument("Missing output proto vertices collection");
  }
}

ActsExamples::ProcessCode ttlindkvist::TutorialVertexFinderAlgorithm::execute(
    const ActsExamples::AlgorithmContext& ctx) const {
  // retrieve input tracks and convert into the expected format
  const auto& inputTrackParameters =
      ctx.eventStore.get<ActsExamples::TrackParametersContainer>(m_cfg.inputTrackParameters);
  const auto& inputTrackPointers =
      ActsExamples::makeTrackParametersPointerContainer(inputTrackParameters);
  //* Do not change the code above this line *//

  //////////////////////////////////////////////////////////////////////////////
  /*****   Note: This is a skeleton file to be filled with tutorial code  *****/
  /*****   provided in the Acts Docs - Vertexing section under the link:  *****/
  /* https://acts.readthedocs.io/en/latest/howto/setup_and_run_vertexing.html */
  /*** or in the Acts repository in  docs/howto/setup_and_run_vertexing.md  ***/
  //////////////////////////////////////////////////////////////////////////////
  ACTS_DEBUG("AMVF start");

    // Set up EigenStepper
  Acts::EigenStepper<> stepper(m_cfg.bField);
  // Set up the propagator
  using Propagator = Acts::Propagator<Acts::EigenStepper<>>;
  auto propagator = std::make_shared<Propagator>(stepper);
  
  // Set up ImpactPointEstimator
  using IPEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, Propagator>;
  IPEstimator::Config ipEstimatorCfg(m_cfg.bField, propagator);
  IPEstimator ipEstimator(ipEstimatorCfg);


  // Set up the helical track linearizer
  using Linearizer = Acts::HelicalTrackLinearizer<Propagator>;
  Linearizer::Config ltConfig(m_cfg.bField, propagator);
  Linearizer linearizer(ltConfig);
  
  // Set up deterministic annealing with user-defined temperatures
  std::vector<double> temperatures{8.0, 4.0, 2.0, 1.4142136, 1.2247449, 1.0};
  Acts::AnnealingUtility::Config annealingConfig(temperatures);
  Acts::AnnealingUtility annealingUtility(annealingConfig);
  
  // Set up the vertex fitter with user-defined annealing
  using Fitter = Acts::AdaptiveMultiVertexFitter<Acts::BoundTrackParameters, Linearizer>;
  Fitter::Config fitterCfg(ipEstimator);
  fitterCfg.annealingTool = annealingUtility;
  Fitter fitter(fitterCfg);
    
  // Set up the vertex seed finder
  using SeedFinder = Acts::TrackDensityVertexFinder<Fitter, Acts::GaussianTrackDensity<Acts::BoundTrackParameters>>;
  SeedFinder seedFinder;
  
  
  // The vertex finder type
  using Finder = Acts::AdaptiveMultiVertexFinder<Fitter, SeedFinder>;
  // using Finder = Acts::IterativeVertexFinder<Fitter, SeedFinder>;
  
  Finder::Config finderConfig(std::move(fitter), seedFinder, ipEstimator, linearizer, m_cfg.bField);
  // We do not want to use a beamspot constraint here
  finderConfig.useBeamSpotConstraint = false;
  
  // Instantiate the finder
  Finder finder(finderConfig);
  // The vertex finder state
  Finder::State state;
  
  // Default vertexing options, this is where e.g. a constraint could be set
  using VertexingOptions = Acts::VertexingOptions<Acts::BoundTrackParameters>;
  VertexingOptions finderOpts(ctx.geoContext, ctx.magFieldContext);
  
  // Find vertices
  auto res = finder.find(inputTrackPointers, finderOpts, state);

  if (res.ok()) {
    // Retrieve vertices found by vertex finder
    auto vertexCollection = *res;
    ACTS_INFO("Found " << vertexCollection.size() << " vertices in event " << ctx.eventNumber);
    
    //Writing to file
    std::ofstream outputFile(boost::str(boost::format("output/event%1%_AMVF.txt") % ctx.eventNumber));    
    
    unsigned int count = 0;
    for (const auto& vtx : vertexCollection) {
      ACTS_INFO("\t" << ++count << ". vertex at "
                    << "(" << vtx.position().x() << "," << vtx.position().y()
                    << "," << vtx.position().z() << ") with "
                    << vtx.tracks().size() << " tracks.");
      outputFile << vtx.fullPosition().transpose() << " " << vtx.tracks().size() << std::endl;
    }
    outputFile.close();
    // store found vertices
    ctx.eventStore.add(m_cfg.outputVertices, std::move(vertexCollection)); 
  } else {
    ACTS_ERROR("Error in vertex finder: " << res.error().message());
  }
  ACTS_DEBUG("AMVF end");
  //* Do not change the code below this line *//
  return ActsExamples::ProcessCode::SUCCESS;
}
