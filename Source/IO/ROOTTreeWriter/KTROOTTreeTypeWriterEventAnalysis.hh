/*
 * KTROOTTreeTypeWriterEventAnalysis.hh
 *
 *  Created on: Jan 23, 2013
 *      Author: nsoblath
 */

#ifndef KTROOTTREETYPEWRITEREVENTANALYSIS_HH_
#define KTROOTTREETYPEWRITEREVENTANALYSIS_HH_

#include "KTROOTTreeWriter.hh"

#include "KTData.hh"

#include "Rtypes.h"
#include "TClonesArray.h"
#include "TNtupleD.h"
#include "KTROOTData.hh"

#include <vector>

class TGraph;
class TGraph2D;
class TH2D;
class TTree;

namespace Cicada
{
    class TProcessedTrackData;
    class TProcessedMPTData;
    class TMultiTrackEventData;
    class TMTEWithClassifierResultsData;
}

namespace Katydid
{
    
    //class KTFrequencyCandidateData;
    //class KTWaterfallCandidateData;
    // class TSparseWaterfallCandidateData;

    struct TFrequencyCandidateData
    {
        UInt_t fComponent;
        ULong64_t fSlice;
        Double_t fTimeInRun;
        Double_t fThreshold;
        UInt_t fFirstBin;
        UInt_t fLastBin;
        Double_t fMeanFrequency;
        Double_t fPeakAmplitude;
        Double_t fAmplitudeSum;
    };

    struct TWaterfallCandidateData
    {
        UInt_t fComponent;
        Double_t fTimeInRun;
        Double_t fTimeLength;
        ULong64_t fFirstSliceNumber;
        ULong64_t fLastSliceNumber;
        Double_t fMinFrequency;
        Double_t fMaxFrequency;
        Double_t fMeanStartFrequency;
        Double_t fMeanEndFrequency;
        Double_t fFrequencyWidth;
        UInt_t fStartRecordNumber;
        UInt_t fStartSampleNumber;
        UInt_t fEndRecordNumber;
        UInt_t fEndSampleNumber;
        TH2D* fCandidate;
    };

    // commented-out fields match fields not yet implemented in KTSparseWaterfallCandidateData

    // struct TDiscriminatedPoint 
    // {
    //     Double_t fTimeInRunC;
    //     Double_t fFrequency;
    //     Double_t fAmplitude;
    //     Double_t fTimeInAcq;
    //     Double_t fMean;
    //     Double_t fVariance;
    //     Double_t fNeighborhoodAmplitude;
    // };

    // struct TSparseWaterfallCandidateData
    // {
    //     UInt_t fComponent;
    //     UInt_t fAcquisitionID;
    //     UInt_t fCandidateID;
    //     //Double_t fTimeBinWidth;
    //     //Double_t fFreqBinWidth;
    //     Double_t fTimeInRunC;
    //     Double_t fTimeLength;
    //     //ULong64_t fFirstSliceNumber;
    //     //ULong64_t fLastSliceNumber;
    //     Double_t fMinFrequency;
    //     Double_t fMaxFrequency;
    //     //Double_t fMeanStartFrequency;
    //     //Double_t fMeanEndFrequency;
    //     Double_t fFrequencyWidth;
    //     TNtupleD* fPoints; //<- TimeInRunc, Frequency, Amplitude, Threshold, ..., ...
    // };

    struct TMultiPeakTrackData
    {
        UInt_t fComponent;
        UInt_t fMultiplicity;
        UInt_t fEventSequenceID;
        Double_t fMeanStartTimeInRunC;
        Double_t fSumStartTimeInRunC;
        Double_t fMeanEndTimeInRunC;
        Double_t fSumEndTimeInRunC;
        UInt_t fAcquisitionID;
        UInt_t fUnknownEventTopology;
    };

    struct TLinearFitResult
    {
        UInt_t fFitNumber;
        Double_t fSlope;
        Double_t fIntercept;
        Double_t fStartingFrequency;
        Double_t fTrackDuration;
        Double_t fSidebandSeparation;
        //Double_t fFineProbe_sigma_1;
        //Double_t fFineProbe_sigma_2;
        //Double_t fFineProbe_SNR_1;
        //Double_t fFineProbe_SNR_2;
        Double_t fFFT_peak;
        Double_t fFFT_SNR;
        Double_t fFit_width;
        UInt_t fNPoints;
        Double_t fProbeWidth;
    };

    struct TPowerFitData
    {
/*
        Double_t fScale;
        Double_t fBackground;
        Double_t fCenter;
        Double_t fCurvature;
        Double_t fWidth;

        Double_t fScaleErr;
        Double_t fBackgroundErr;
        Double_t fCenterErr;
        Double_t fCurvatureErr;
        Double_t fWidthErr;
*/

        std::vector<double> fNorm;
        std::vector<double> fMean;
        std::vector<double> fSigma;
        std::vector<double> fMaximum;

        std::vector<double> fNormErr;
        std::vector<double> fMeanErr;
        std::vector<double> fSigmaErr;
        std::vector<double> fMaximumErr;
        
        UInt_t fIsValid;
        UInt_t fMainPeak;
        UInt_t fNPeaks;

        TGraph* fPoints;

        Double_t fAverage; // fMean is already taken
        Double_t fRMS;
        Double_t fSkewness;
        Double_t fKurtosis;

        Double_t fNormCentral;
        Double_t fMeanCentral;
        Double_t fSigmaCentral;
        Double_t fMaximumCentral;

        Double_t fRMSAwayFromCentral;
        Double_t fCentralPowerFraction;

        Double_t fTrackIntercept;
    };


    class KTROOTTreeTypeWriterEventAnalysis : public KTROOTTreeTypeWriter//, public KTTypeWriterEventAnalysis
    {
        public:
            KTROOTTreeTypeWriterEventAnalysis();
            virtual ~KTROOTTreeTypeWriterEventAnalysis();

            void RegisterSlots();

        public:
            void WriteFrequencyCandidates(Nymph::KTDataPtr data);
            void WriteWaterfallCandidate(Nymph::KTDataPtr data);
            void WriteSparseWaterfallCandidate(Nymph::KTDataPtr data);
            void WriteSequentialLine(Nymph::KTDataPtr data);
            void WriteProcessedMPT(Nymph::KTDataPtr data);
            void WriteProcessedTrack(Nymph::KTDataPtr data);
            void WriteMultiPeakTrack(Nymph::KTDataPtr data);
            void WriteMultiTrackEvent(Nymph::KTDataPtr data);
            void WriteMTEWithClassifierResults(Nymph::KTDataPtr data);
            void WriteLinearFitResultData(Nymph::KTDataPtr data);
            void WritePowerFitData(Nymph::KTDataPtr data);

        public:
            TTree* GetFrequencyCandidateTree() const;
            TTree* GetWaterfallCandidateTree() const;
            TTree* GetSparseWaterfallCandidateTree() const;
            TTree* GetSequentialLineTree() const;
            TTree* GetProcessedMPTTree() const;
            TTree* GetProcessedTrackTree() const;
            TTree* GetMultiPeakTrackTree() const;
            TTree* GetMultiTrackEventTree() const;
            TTree* GetMTEWithClassifierResultsTree() const;
            TTree* GetLinearFitResultTree() const;
            TTree* GetPowerFitDataTree() const;

        private:
            bool SetupFrequencyCandidateTree();
            bool SetupWaterfallCandidateTree();
            bool SetupSparseWaterfallCandidateTree();
            bool SetupSequentialLineTree();
            bool SetupProcessedMPTTree();
            bool SetupProcessedTrackTree();
            bool SetupMultiPeakTrackTree();
            bool SetupMultiTrackEventTree();
            bool SetupMTEWithClassifierResultsTree();
            bool SetupLinearFitResultTree();
            bool SetupPowerFitDataTree();

            TTree* fFreqCandidateTree;
            TTree* fWaterfallCandidateTree;
            TTree* fSparseWaterfallCandidateTree;
            TTree* fSequentialLineTree;
            TTree* fProcessedMPTTree;
            TTree* fProcessedTrackTree;
            TTree* fMultiPeakTrackTree;
            TTree* fMultiTrackEventTree;
            TTree* fMTEWithClassifierResultsTree;
            TTree* fLinearFitResultTree;
            TTree* fPowerFitDataTree;

            TFrequencyCandidateData fFreqCandidateData;
            TWaterfallCandidateData fWaterfallCandidateData;
            TSparseWaterfallCandidateData* fSparseWaterfallCandidateDataPtr;
            TSequentialLineData* fSequentialLineDataPtr;
            Cicada::TProcessedTrackData* fProcessedTrackDataPtr;
            Cicada::TProcessedMPTData* fProcessedMPTDataPtr;
            TMultiPeakTrackData fMultiPeakTrackData;
            Cicada::TMultiTrackEventData* fMultiTrackEventDataPtr;
            Cicada::TMTEWithClassifierResultsData* fMTEWithClassifierResultsDataPtr;
            TLinearFitResult fLineFitData;
            TPowerFitData fPowerFitData;

    };

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetFrequencyCandidateTree() const
    {
        return fFreqCandidateTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetWaterfallCandidateTree() const
    {
        return fWaterfallCandidateTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetSparseWaterfallCandidateTree() const
    {
        return fSparseWaterfallCandidateTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetSequentialLineTree() const
    {
        return fSequentialLineTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetProcessedMPTTree() const
    {
        return fProcessedMPTTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetProcessedTrackTree() const
    {
        return fProcessedTrackTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetMultiPeakTrackTree() const
    {
        return fMultiPeakTrackTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetMultiTrackEventTree() const
    {
        return fMultiTrackEventTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetMTEWithClassifierResultsTree() const
    {
        return fMTEWithClassifierResultsTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetLinearFitResultTree() const
    {
        return fLinearFitResultTree;
    }

    inline TTree* KTROOTTreeTypeWriterEventAnalysis::GetPowerFitDataTree() const
    {
        return fPowerFitDataTree;
    }


} /* namespace Katydid */


#endif /* KTROOTTREETYPEWRITEREVENTANALYSIS_HH_ */
