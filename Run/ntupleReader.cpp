// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Acts/Definitions/Units.hpp"
#include "ActsExamples/Options/CommonOptions.hpp"
#include "ActsExamples/MagneticField/MagneticFieldOptions.hpp"
#include "ActsExamples/Framework/Sequencer.hpp"
#include "ActsExamples/Vertexing/VertexingOptions.hpp"

#include "ttlindkvistACTS/NTupleReaderOptions.hpp"
#include "ttlindkvistACTS/RootNTupleReader.hpp"
#include "ttlindkvistACTS/NTupleIterativeVertexFinderAlgorithm.hpp"

#include <typeinfo>
#include <memory>

using namespace Acts::UnitLiterals;
using namespace ActsExamples;

int main(int argc, char* argv[]) {
  std::cout << "\nThe ntuple reader in src folder works!!\n";
  // setup and parse options
  auto desc = Options::makeDefaultOptions();
  Options::addSequencerOptions(desc);
  ttlindkvist::Options::addNTupleReaderOptions(desc);
  
  Options::addVertexingOptions(desc);
  Options::addMagneticFieldOptions(desc);
  Options::addOutputOptions(desc, OutputFormat::DirectoryOnly);
  
  auto vars = Options::parse(desc, argc, argv);
  if (vars.empty()) {
    return EXIT_FAILURE;
  }

  // basic setup
  auto logLevel = Options::readLogLevel(vars);
  Sequencer sequencer(Options::readSequencerConfig(vars));
  
  // Setup the magnetic field
  std::cout << "\nSetup magnetic field\n";
  auto magneticField = Options::readMagneticField(vars);

  //Setup root file reader
  ttlindkvist::RootNTupleReader::Config ntupleReaderConf = ttlindkvist::Options::readNTupleReaderOptions(vars);
  sequencer.addReader(std::make_shared<ttlindkvist::RootNTupleReader>(ntupleReaderConf, logLevel));
  
  // Find vertices using iterative method
  ttlindkvist::NTupleIterativeVertexFinderAlgorithm::Config findVerticesIterative;
  findVerticesIterative.bField = magneticField;
  findVerticesIterative.outputProtoVertices = "protovertices";
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::NTupleIterativeVertexFinderAlgorithm>(findVerticesIterative, logLevel));
  
  std::cout << "\nRun sequencer\n";
  return sequencer.run();
}