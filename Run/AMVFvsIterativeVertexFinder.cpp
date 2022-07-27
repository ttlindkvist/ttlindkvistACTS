// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Logger.hpp"

#include "ActsExamples/Framework/Sequencer.hpp"
#include "ActsExamples/MagneticField/MagneticFieldOptions.hpp"
#include "ActsExamples/Options/CommonOptions.hpp"
#include "ActsExamples/Options/Pythia8Options.hpp"
#include "ActsExamples/Reconstruction/ReconstructionBase.hpp"
#include "ActsExamples/TruthTracking/ParticleSelector.hpp"
#include "ActsExamples/TruthTracking/ParticleSmearing.hpp"
#include "ActsExamples/Vertexing/VertexingOptions.hpp"

//Replace these with modified
#include "ttlindkvistACTS/ModifiedAMVFAlgorithm.hpp"
#include "ttlindkvistACTS/ModifiedIterativeVertexFinderAlgorithm.hpp"

#include <typeinfo>
#include <memory>

using namespace Acts::UnitLiterals;
using namespace ActsExamples;

int main(int argc, char* argv[]) {
  // setup and parse options
  auto desc = Options::makeDefaultOptions();
  Options::addSequencerOptions(desc);
  Options::addRandomNumbersOptions(desc);
  Options::addPythia8Options(desc);
  ParticleSelector::addOptions(desc);
  Options::addVertexingOptions(desc);
  Options::addMagneticFieldOptions(desc);
  Options::addOutputOptions(desc, OutputFormat::DirectoryOnly);
  Options::addParticleSmearingOptions(desc);
  auto vars = Options::parse(desc, argc, argv);
  if (vars.empty()) {
    return EXIT_FAILURE;
  }
  std::cout << "\nArguments loaded\n";

  // basic setup
  std::cout << "\nSequencer setup\n";
  auto logLevel = Options::readLogLevel(vars);
  auto rnd =
      std::make_shared<RandomNumbers>(Options::readRandomNumbersConfig(vars));
  Sequencer sequencer(Options::readSequencerConfig(vars));

  // Setup the magnetic field
  std::cout << "\nSetup magnetic field\n";
  auto magneticField = Options::readMagneticField(vars);

  // setup event generator
  std::cout << "\nSetup event generator\n";
  EventGenerator::Config evgen = Options::readPythia8Options(vars, logLevel);
  evgen.outputParticles = "particles_generated";
  evgen.randomNumbers = rnd;
  sequencer.addReader(std::make_shared<EventGenerator>(evgen, logLevel));
  // ACTS_DEBUG("Sequencer: add event generation");

  // pre-select particles
  std::cout << "\nPreselect particles\n";
  ParticleSelector::Config selectParticles = ParticleSelector::readConfig(vars);
  std::cout << "\nselectParticles has been initialized\n";
  selectParticles.inputParticles = evgen.outputParticles;
  selectParticles.outputParticles = "particles_selected";
  // smearing only works with charge particles for now
  selectParticles.removeNeutral = true;
  
  selectParticles.absEtaMax = vars["vertexing-eta-max"].as<double>();
  selectParticles.rhoMax = vars["vertexing-rho-max"].as<double>() * 1_mm;
  selectParticles.ptMin = vars["vertexing-pt-min"].as<double>() * 1_MeV;
  sequencer.addAlgorithm(
      std::make_shared<ParticleSelector>(selectParticles, logLevel));
  // ACTS_DEBUG("Sequencer: add particle pre-selection");

  // Run the particle smearing
  std::cout << "\nRun particle smearing\n";
  auto particleSmearingCfg = setupParticleSmearing(
      vars, sequencer, rnd, selectParticles.outputParticles);

  // find vertices via AFVM method
  std::cout << "\nFind verteces\n";
  ttlindkvist::TutorialVertexFinderAlgorithm::Config findVerticesAMVF;
  findVerticesAMVF.bField = magneticField;
  findVerticesAMVF.inputTrackParameters = particleSmearingCfg.outputTrackParameters;
  findVerticesAMVF.outputProtoVertices = "protovertices";
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::TutorialVertexFinderAlgorithm>(findVerticesAMVF, logLevel));
  // ACTS_DEBUG("Sequencer: vertex finding algorithm");

  // Find vertices using iterative method
  ttlindkvist::IterativeVertexFinderAlgorithm::Config findVerticesIterative;
  findVerticesIterative.bField = magneticField;
  findVerticesIterative.inputTrackParameters = particleSmearingCfg.outputTrackParameters;
  findVerticesIterative.outputProtoVertices = "protovertices";
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::IterativeVertexFinderAlgorithm>(findVerticesIterative, logLevel));

  std::cout << "\nRun sequence\n";
  return sequencer.run();
}
