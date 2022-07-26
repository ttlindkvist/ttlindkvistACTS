// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Utilities/Logger.hpp"
#include "ttlindkvistACTS/RootNTupleReader.hpp"


#include <string>

#include <boost/program_options.hpp>

namespace ttlindkvist {

namespace Options {

/// Add sequencer options, e.g. number of events
void addNTupleReaderOptions(boost::program_options::options_description& opt);

ttlindkvist::RootNTupleReader::Config readNTupleReaderOptions(
    const boost::program_options::variables_map& vm);
}  // namespace Options
}  // namespace ttlindkvist
