// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Acts/Definitions/Units.hpp"
#include "ActsExamples/Options/CommonOptions.hpp"
#include "ActsExamples/Framework/Sequencer.hpp"

#include "ttlindkvistACTS/NTupleReaderOptions.hpp"
#include "ttlindkvistACTS/RootNTupleReader.hpp"

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
  
  auto vars = Options::parse(desc, argc, argv);
  if (vars.empty()) {
    return EXIT_FAILURE;
  }

  // basic setup
  auto logLevel = Options::readLogLevel(vars);
  Sequencer sequencer(Options::readSequencerConfig(vars));
  
  
  //Setup root file reader
  ttlindkvist::RootNTupleReader::Config ntupleReaderConf = ttlindkvist::Options::readNTupleReaderOptions(vars);
  sequencer.addReader(std::make_shared<ttlindkvist::RootNTupleReader>(ntupleReaderConf, logLevel));
  
  std::cout << "\nRun sequencer\n";
  return sequencer.run();
}