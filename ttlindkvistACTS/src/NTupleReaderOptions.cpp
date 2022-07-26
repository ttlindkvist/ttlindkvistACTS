// This file is part of the Acts project.
//
// Copyright (C) 2019-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ttlindkvistACTS/NTupleReaderOptions.hpp"

#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "ActsExamples/Utilities/Options.hpp"

#include <exception>
#include <fstream>
#include <regex>
#include <system_error>

using namespace boost::program_options;

void ttlindkvist::Options::addNTupleReaderOptions(
    boost::program_options::options_description& opt) {
  // sequencer options
  opt.add_options()(
      "ntuple-file-path", value<std::string>(),
      "Path to the ntuple root file");
}
ttlindkvist::RootNTupleReader::Config ttlindkvist::Options::readNTupleReaderOptions(
    const boost::program_options::variables_map& vm) {
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("NTupleReader", Acts::Logging::INFO));
  ACTS_INFO("Before arg read");
  
  ttlindkvist::RootNTupleReader::Config cfg;
  if (not vm["ntuple-file-path"].empty())
    cfg.filePath = vm["ntuple-file-path"].as<std::string>();
  
  ACTS_INFO("File path to ntuple loaded: " << cfg.filePath);
  
  return cfg;
}
