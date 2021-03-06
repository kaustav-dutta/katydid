# CMakeLists for Katydid/Examples
# Author: N. Oblath

# Tutorial code is optionally built from the Source/SpectrumAnalysis and Source/Data CMakeLists files.
# Processor template code is built from Source/Utility CMakeLists file.

# Config files
set (EXAMPLE_CONFIGFILES
    ConfigFiles/EvaluateCandidates.json
    ConfigFiles/FSPeakAnalysisConfig.json
    ConfigFiles/KatydidPSAnalysisTemplate.json
    ConfigFiles/KatydidPSComplexConfig.json
    ConfigFiles/KatydidWVConfig.json
    ConfigFiles/ThroughputProfiler.json
    ConfigFiles/ts-plot-direct-from-mantis.json
    ConfigFiles/HDF5/AverageFFT.json
    ConfigFiles/HDF5/AveragePS.json
    ConfigFiles/HDF5/ConvertEggToHDF5.json
    
    ConfigFiles/ChannelAggregatorConfig.yaml
    ConfigFiles/ChirpTransformWithFrFFT.yaml
    ConfigFiles/DisplayFFT.yaml
    ConfigFiles/DisplaySpectrograms.yaml
    ConfigFiles/FractionalFFTAsOrdinaryFFT.yaml
    ConfigFiles/GainVariation.yaml
    ConfigFiles/KatydidPSConfig.yaml
    ConfigFiles/NormalizedPS.yaml
    ConfigFiles/PrintInfo.yaml
    ConfigFiles/SidebandTrackAnalysis.yaml    
    ConfigFiles/SparseSpectrogramAsTTree.yaml
    ConfigFiles/SparseSpectrogramFromKDTree.yaml
    ConfigFiles/SpectrogramConfig.yaml
    ConfigFiles/SpectrogramImageConfig.yaml
    ConfigFiles/TrackConstructionConfig.yaml
)

pbuilder_install_config (${EXAMPLE_CONFIGFILES})
