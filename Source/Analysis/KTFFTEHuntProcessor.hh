/**
 @file KTFFTEHuntProcessor.hh
 @brief Contains KTFFTEHuntProcessor
 @details Performs the FFT-based electron hunt
 @author: N. S. Oblath
 @date: Jan 24, 2012
 */

#ifndef KTFFTEHUNTPROCESSOR_HH_
#define KTFFTEHUNTPROCESSOR_HH_

#include "KTProcessor.hh"

#include "KTSimpleFFTProcessor.hh"
#include "KTSlidingWindowFFTProcessor.hh"
#include "KTGainNormalizationProcessor.hh"
#include "KTSimpleClusteringProcessor.hh"

#include "TFile.h"

#include <fstream>
#include <list>
#include <map>
#include <utility>
#include <vector>

namespace Katydid
{
    /*!
     @class KTFFTEHuntProcessor
     @author N. S. Oblath

     @brief Performs an FFT-based electron hunt.

     @details
     Uses a windows FFT of Egg events to search for clusters of high-peaked bins moving up in frequency.
    */

    class KTFFTEHuntProcessor : public KTProcessor
    {
        private:
            typedef std::list< std::multimap< Int_t, Int_t >* > EventPeakBinsList;
            typedef std::pair< Double_t, Double_t > CutRange;

        public:
            KTFFTEHuntProcessor();
            virtual ~KTFFTEHuntProcessor();

            Bool_t ApplySetting(const KTSetting* setting);

            void ProcessHeader(KTEgg::HeaderInfo headerInfo);

            void ProcessEvent(UInt_t iEvent, const KTEvent* event);

            void FinishHunt();

        private:
            void EmptyEventPeakBins();

        private:
            EventPeakBinsList fEventPeakBins;

            UInt_t fMinimumGroupSize;

            std::vector< CutRange > fCutRanges;

            KTSimpleFFTProcessor fSimpleFFTProc;
            KTSlidingWindowFFTProcessor fWindowFFTProc;
            KTGainNormalizationProcessor fGainNormProc;
            KTSimpleClusteringProcessor fClusteringProc;

            string fTextFilename;
            string fROOTFilename;
            Bool_t fWriteTextFileFlag;
            Bool_t fWriteROOTFileFlag;
            ofstream fTextFile;
            TFile fROOTFile;

            Double_t fFrequencyMultiplier;
            Int_t fTotalCandidates;


            //****************
            // Slot connection
            //****************

    };

} /* namespace Katydid */
#endif /* KTFFTEHUNTPROCESSOR_HH_ */
