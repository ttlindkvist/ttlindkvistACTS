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
#include "ttlindkvistACTS/NTuplePrinting.hpp"

#include "ttlindkvistACTS/ModifiedAMVFAlgorithm.hpp"
#include "ttlindkvistACTS/NTupleIterativeVertexFinderAlgorithm.hpp"

#include "ttlindkvistACTS/VertexingResolutionAlgorithm.hpp"

#include <typeinfo>
#include <memory>

using namespace Acts::UnitLiterals;
using namespace ActsExamples;

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
  ntupleReaderConf.nTupleTruthVtxParameters = "nTupleTruthVtxParameters";
  ntupleReaderConf.nTupleRecoVtxParameters = "nTupleRecoVtxParameters";
  sequencer.addReader(std::make_shared<ttlindkvist::RootNTupleReader>(ntupleReaderConf, logLevel));
 
  
  // ############################################
  // #### Commented out because it runs slow ####
  // ############################################
  ttlindkvist::TutorialVertexFinderAlgorithm::Config findVerticesAMVF;
  findVerticesAMVF.bField = magneticField;
  findVerticesAMVF.inputTrackParameters = "nTupleTrackParameters";
  findVerticesAMVF.outputProtoVertices = "protovertices";
  findVerticesAMVF.outputVertices = "AMVF_vertices";
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::TutorialVertexFinderAlgorithm>(findVerticesAMVF, logLevel));
  
  // Find vertices using iterative method
  ttlindkvist::NTupleIterativeVertexFinderAlgorithm::Config findVerticesIterative;
  findVerticesIterative.bField = magneticField;
  findVerticesIterative.inputTrackParameters = "nTupleTrackParameters";
  findVerticesIterative.outputProtoVertices = "protovertices";
  findVerticesIterative.outputVertices = "IVF_vertices";
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::NTupleIterativeVertexFinderAlgorithm>(findVerticesIterative, logLevel));
  
  
  // Debugging stuff
  ttlindkvist::NTuplePrinting::Config printingCfg;
  printingCfg.outputDir = "ntuple_check/";
  printingCfg.ntupleTrackParameters = "nTupleTrackParameters";
  printingCfg.ntupleTruthVtxParameters = "nTupleTruthVtxParameters";
  printingCfg.ntupleRecoVtxParameters = "nTupleRecoVtxParameters";
  printingCfg.iterativeRecoVtxParameters = "IVF_vertices";
  printingCfg.AMVFRecoVtxParameters = "AMVF_vertices";
  
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::NTuplePrinting>(printingCfg, logLevel));
  
  
  ttlindkvist::VertexingResolutionAlgorithm::Config IVFVtxResolutionConfig;
  IVFVtxResolutionConfig.outputDir = "ntuple_check/IVFresolution";
  IVFVtxResolutionConfig.ntupleTruthVtxParameters = "nTupleTruthVtxParameters";
  IVFVtxResolutionConfig.recoVtxParameters = "IVF_vertices";
  IVFVtxResolutionConfig.output = "IVFResolutionOutput";
  
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::VertexingResolutionAlgorithm>(IVFVtxResolutionConfig, logLevel));
  
  
  ttlindkvist::VertexingResolutionAlgorithm::Config AMVFVtxResolutionConfig;
  AMVFVtxResolutionConfig.outputDir = "ntuple_check/AMVFresolution";
  AMVFVtxResolutionConfig.ntupleTruthVtxParameters = "nTupleTruthVtxParameters";
  AMVFVtxResolutionConfig.recoVtxParameters = "AMVF_vertices";
  AMVFVtxResolutionConfig.output = "AMVFResolutionOutput";
  
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::VertexingResolutionAlgorithm>(AMVFVtxResolutionConfig, logLevel));
  
  ttlindkvist::VertexingResolutionAlgorithm::Config AMVFvsRecoVtxResolutionConfig;
  AMVFvsRecoVtxResolutionConfig.outputDir = "ntuple_check/AMVFvsRecoResolution";
  AMVFvsRecoVtxResolutionConfig.ntupleTruthVtxParameters = "nTupleRecoVtxParameters";
  AMVFvsRecoVtxResolutionConfig.recoVtxParameters = "AMVF_vertices";
  AMVFvsRecoVtxResolutionConfig.output = "AMVFvsRecoResolutionOutput";
  
  sequencer.addAlgorithm(
      std::make_shared<ttlindkvist::VertexingResolutionAlgorithm>(AMVFvsRecoVtxResolutionConfig, logLevel));
  
  std::cout << "\nRun sequencer\n";
  return sequencer.run();
}