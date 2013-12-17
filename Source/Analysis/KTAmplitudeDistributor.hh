/*
 * KTAmplitudeDistributor.hh
 *
 *  Created on: Apr 26, 2013
 *      Author: nsoblath
 */

#ifndef KTAMPLITUDEDISTRIBUTOR_HH_
#define KTAMPLITUDEDISTRIBUTOR_HH_

#include "KTProcessor.hh"

#include "KTAmplitudeDistribution.hh"
#include "KTData.hh"
#include "KTSlot.hh"

#include <vector>

namespace Katydid
{
    class KTCorrelationData;
    class KTDiscriminatedPoints1DData;
    class KTEggHeader;
    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumDataFFTWCore;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumDataPolarCore;
    class KTFrequencySpectrumFFTW;
    class KTFrequencySpectrumPolar;
    class KTNormalizedFSDataFFTW;
    class KTNormalizedFSDataPolar;
    class KTWignerVilleData;


    /*!
     @class KTAmplitudeDistributor
     @author N. S. Oblath

     @brief Collects distributions of amplitudes for each frequency bin over many slices.

     @details
     Config name: "amplitude-distributor"

     Available configuration values:
     - "min-frequency": double -- minimum frequency
     - "max-frequency": double -- maximum frequency
     - "min-bin": unsigned -- minimum frequency
     - "max-bin": unsigned -- maximum frequency
     - "dist-n-bins": unsigned -- number of bins for the distribution histogram
     - "use-buffer": bool -- whether to use the buffer (true; default) or pre-specified distribution min and max (false)
     - "buffer-size": unsigned -- number of spectra to store initially to determine the range of the distribution (if not using "dist-min/max")
     - "dist-min": double -- minimum of the distribution (if not using "buffer-size")
     - "dist-max": double -- maximum of the distribution (if not using "buffer-size")

     Slots:
     - Running
       - "fs-polar": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTFrequencySpectrumDataPolar
       - "fs-fftw": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTFrequencySpectrumDataFFTW
       - "norm-fs-polar": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTFrequencySpectrumDataPolar
       - "norm-fs-fftw": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTNormalizedFSDataFFTW
       - "corr": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTCorrelationData
       - "wv": void (KTDataPtr) -- Adds values from a spectrum to the amplitude distribution; Requires KTWignerVilleData
     - Completion
       - "finish": void () -- Completes the calculation of the amplitude distribution; Emits "amp-dist"

     Signals:
     \li \c "amp-dist": void (KTDataPtr) Emitted upon completion of an amplitude distribution; Guarantees KTAmplitudeDistributor
    */

    class KTAmplitudeDistributor : public KTProcessor
    {
        public:
            typedef std::vector< double > Spectrum; // indexed over frequency-axis bins
            typedef std::vector< Spectrum > Spectra; // indexed over component
            typedef std::vector< Spectra > Buffer; // indexed over slice number

        public:
            KTAmplitudeDistributor(const std::string& name = "amplitude-distributor");
            virtual ~KTAmplitudeDistributor();

            Bool_t Configure(const KTPStoreNode* node);

            double GetMinFrequency() const;
            void SetMinFrequency(double freq);

            double GetMaxFrequency() const;
            void SetMaxFrequency(double freq);

            unsigned GetMinBin() const;
            void SetMinBin(unsigned bin);

            unsigned GetMaxBin() const;
            void SetMaxBin(unsigned bin);

            unsigned GetDistNBins() const;
            void SetDistNBins(unsigned nBins);

            Bool_t GetUseBuffer() const;
            void SetUseBuffer(Bool_t useBuffer);

            unsigned GetBufferSize() const;
            void SetBufferSize(unsigned buffer);

            double GetDistMin() const;
            void SetDistMin(double min);

            double GetDistMax() const;
            void SetDistMax(double max);

        private:
            double fMinFrequency;
            double fMaxFrequency;
            unsigned fMinBin;
            unsigned fMaxBin;
            Bool_t fCalculateMinBin;
            Bool_t fCalculateMaxBin;

            unsigned fDistNBins;
            unsigned fBufferSize;
            double fDistMin;
            double fDistMax;
            Bool_t fUseBuffer;

        public:
            Bool_t Initialize(unsigned nComponents, unsigned nFreqBins);

            Bool_t AddValues(KTFrequencySpectrumDataPolar& data);
            Bool_t AddValues(KTFrequencySpectrumDataFFTW& data);
            Bool_t AddValues(KTNormalizedFSDataPolar& data);
            Bool_t AddValues(KTNormalizedFSDataFFTW& data);
            Bool_t AddValues(KTCorrelationData& data);
            Bool_t AddValues(KTWignerVilleData& data);

            void FinishAmpDist();

        private:
            Bool_t CoreAddValues(KTFrequencySpectrumDataPolarCore& data);
            Bool_t CoreAddValues(KTFrequencySpectrumDataFFTWCore& data);

            Bool_t (KTAmplitudeDistributor::*fTakeValuesPolar)(const KTFrequencySpectrumPolar*, unsigned);
            Bool_t TakeValuesToBuffer(const KTFrequencySpectrumPolar* spectrum, unsigned component);
            Bool_t TakeValuesToDistributions(const KTFrequencySpectrumPolar* spectrum, unsigned component);

            Bool_t (KTAmplitudeDistributor::*fTakeValuesFFTW)(const KTFrequencySpectrumFFTW*, unsigned);
            Bool_t TakeValuesToBuffer(const KTFrequencySpectrumFFTW* spectrum, unsigned component);
            Bool_t TakeValuesToDistributions(const KTFrequencySpectrumFFTW* spectrum, unsigned component);

            Bool_t CreateDistributionsEmpty();
            Bool_t CreateDistributionsFromBuffer();

            double fInvDistBinWidth;

            unsigned fNFreqBins;
            unsigned fNComponents;

            Buffer fBuffer;
            unsigned fNBuffered;

            unsigned fNSlicesProcessed;

            KTDataPtr fDistributionData;
            KTAmplitudeDistribution* fDistributions;


            //***************
            // Signals
            //***************

        private:
            KTSignalData fAmpDistSignal;

            //***************
            // Slots
            //***************

        private:
            KTSlotDataOneType< KTFrequencySpectrumDataPolar > fFSPolarSlot;
            KTSlotDataOneType< KTFrequencySpectrumDataFFTW > fFSFFTWSlot;
            KTSlotDataOneType< KTNormalizedFSDataPolar > fNormFSPolarSlot;
            KTSlotDataOneType< KTNormalizedFSDataFFTW > fNormFSFFTWSlot;
            KTSlotDataOneType< KTCorrelationData > fCorrSlot;
            KTSlotDataOneType< KTWignerVilleData > fWVSlot;

            KTSlotNoArg< void () > fCompleteDistributions;

    };

    inline double KTAmplitudeDistributor::GetMinFrequency() const
    {
        return fMinFrequency;
    }

    inline void KTAmplitudeDistributor::SetMinFrequency(double freq)
    {
        fMinFrequency = freq;
        fCalculateMinBin = true;
        return;
    }

    inline double KTAmplitudeDistributor::GetMaxFrequency() const
    {
        return fMaxFrequency;
    }

    inline void KTAmplitudeDistributor::SetMaxFrequency(double freq)
    {
        fMaxFrequency = freq;
        fCalculateMaxBin = true;
        return;
    }

    inline unsigned KTAmplitudeDistributor::GetMinBin() const
    {
        return fMinBin;
    }

    inline void KTAmplitudeDistributor::SetMinBin(unsigned bin)
    {
        fMinBin = bin;
        fCalculateMinBin = false;
        return;
    }

    inline unsigned KTAmplitudeDistributor::GetMaxBin() const
    {
        return fMaxBin;
    }

    inline void KTAmplitudeDistributor::SetMaxBin(unsigned bin)
    {
        fMaxBin = bin;
        fCalculateMaxBin = false;
        return;
    }

    inline unsigned KTAmplitudeDistributor::GetDistNBins() const
    {
        return fDistNBins;
    }

    inline void KTAmplitudeDistributor::SetDistNBins(unsigned nBins)
    {
        fDistNBins = nBins;
        return;
    }

    inline Bool_t KTAmplitudeDistributor::GetUseBuffer() const
    {
        return fUseBuffer;
    }

    inline void KTAmplitudeDistributor::SetUseBuffer(Bool_t useBuffer)
    {
        fUseBuffer = useBuffer;
        return;
    }

    inline unsigned KTAmplitudeDistributor::GetBufferSize() const
    {
        return fBufferSize;
    }

    inline void KTAmplitudeDistributor::SetBufferSize(unsigned buffer)
    {
        fBufferSize = buffer;
        return;
    }

    inline double KTAmplitudeDistributor::GetDistMin() const
    {
        return fDistMin;
    }

    inline void KTAmplitudeDistributor::SetDistMin(double min)
    {
        fDistMin = min;
        fInvDistBinWidth = double (fDistNBins) / (fDistMax - fDistMin);
        return;
    }

    inline double KTAmplitudeDistributor::GetDistMax() const
    {
        return fDistMax;
    }

    inline void KTAmplitudeDistributor::SetDistMax(double max)
    {
        fDistMax = max;
        fInvDistBinWidth = double (fDistNBins) / (fDistMax - fDistMin);
        return;
    }

} /* namespace Katydid */
#endif /* KTAMPLITUDEDISTRIBUTOR_HH_ */
