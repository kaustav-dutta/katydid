/**
 @file KTSpectrumDiscriminator.hh
 @brief Contains KTSpectrumDiscriminator
 @details Discriminates Spectrum
 @author: N. S. Oblath
 @date: Dec 12, 2012
 */

#ifndef KTSPECTRUMDISCRIMINATOR_HH_
#define KTSPECTRUMDISCRIMINATOR_HH_

#include "KTProcessor.hh"

#include "KTSlot.hh"


namespace Katydid
{
    class KTCorrelationData;
    class KTDiscriminatedPoints1DData;
    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumDataFFTWCore;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumDataPolarCore;
    class KTNormalizedFSDataFFTW;
    class KTNormalizedFSDataPolar;
    class KTWignerVilleData;


    /*!
     @class KTSpectrumDiscriminator
     @author N. S. Oblath

     @brief .

     @details
  
     Configuration name: "spectrum-discriminator"

     Available configuration values:
     - "snr-threshold-power": double -- Provide and use an SNR threshold, assuming the input data is in units of power
     - "snr-threshold-amplitude": double -- Provide and use an SNR threshold, assuming the input data is in units of amplitude
     - "sigma-threshold": double -- Provide and use a number-of-sigma threshold
     - "min-frequency": double -- minimum frequency
     - "max-frequency": double -- maximum frequency
     - "min-bin": unsigned -- minimum frequency by bin
     - "max-bin": unsigned -- maximum frequency by bin

     Slots:
     - "fs-polar": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTFrequencySpectrumDataPolar; Adds KTDiscrimiantedPoints1DData
     - "fs-fftw": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTFrequencySpectrumDataFFTW; Adds KTDiscrimiantedPoints1DData
     - "norm-fs-polar": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTFrequencySpectrumDataPolar; Adds KTDiscrimiantedPoints1DData
     - "norm-fs-fftw": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTNormalizedFSDataFFTW; Adds KTDiscrimiantedPoints1DData
     - "corr": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTCorrelationData; Adds KTDiscrimiantedPoints1DData
     - "wv": void (KTDataPtr) -- Discriminates points above a threshold; Requires KTWignerVilleData; Adds KTDistributedPoints1DData

     Signals:
     - "disc-1d": void (KTDataPtr) Emitted upon performance of a discrimination; Guarantees KTDiscriminatedPoints1DData-->
    */
    class KTSpectrumDiscriminator : public KTProcessor
    {
        private:
            enum ThresholdMode
            {
                eSNR_Amplitude,
                eSNR_Power,
                eSigma
            };

        public:
            KTSpectrumDiscriminator(const std::string& name = "spectrum-discriminator");
            virtual ~KTSpectrumDiscriminator();

            Bool_t Configure(const KTPStoreNode* node);

            double GetSNRThreshold() const;
            void SetSNRAmplitudeThreshold(double thresh);
            void SetSNRPowerThreshold(double thresh);

            double GetSigmaThreshold() const;
            void SetSigmaThreshold(double thresh);

            double GetMinFrequency() const;
            void SetMinFrequency(double freq);

            double GetMaxFrequency() const;
            void SetMaxFrequency(double freq);

            unsigned GetMinBin() const;
            void SetMinBin(unsigned bin);

            unsigned GetMaxBin() const;
            void SetMaxBin(unsigned bin);

        private:

            double fSNRThreshold;
            double fSigmaThreshold;
            ThresholdMode fThresholdMode;

            double fMinFrequency;
            double fMaxFrequency;
            unsigned fMinBin;
            unsigned fMaxBin;
            Bool_t fCalculateMinBin;
            Bool_t fCalculateMaxBin;

        public:
            Bool_t Discriminate(KTFrequencySpectrumDataPolar& data);
            Bool_t Discriminate(KTFrequencySpectrumDataFFTW& data);
            Bool_t Discriminate(KTNormalizedFSDataPolar& data);
            Bool_t Discriminate(KTNormalizedFSDataFFTW& data);
            Bool_t Discriminate(KTCorrelationData& data);
            Bool_t Discriminate(KTWignerVilleData& data);

        private:
            Bool_t CoreDiscriminate(KTFrequencySpectrumDataPolarCore& data, KTDiscriminatedPoints1DData& newData);
            Bool_t CoreDiscriminate(KTFrequencySpectrumDataFFTWCore& data, KTDiscriminatedPoints1DData& newData);


            //***************
            // Signals
            //***************

        private:
            KTSignalData fDiscrim1DSignal;

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

    };

    inline double KTSpectrumDiscriminator::GetSNRThreshold() const
    {
        return fSNRThreshold;
    }

    inline void KTSpectrumDiscriminator::SetSNRAmplitudeThreshold(double thresh)
    {
        fSNRThreshold = thresh;
        fThresholdMode = eSNR_Amplitude;
        return;
    }

    inline void KTSpectrumDiscriminator::SetSNRPowerThreshold(double thresh)
    {
        fSNRThreshold = thresh;
        fThresholdMode = eSNR_Power;
        return;
    }

    inline double KTSpectrumDiscriminator::GetSigmaThreshold() const
    {
        return fSigmaThreshold;
    }

    inline void KTSpectrumDiscriminator::SetSigmaThreshold(double thresh)
    {
        fSigmaThreshold = thresh;
        fThresholdMode = eSigma;
        return;
    }

    inline double KTSpectrumDiscriminator::GetMinFrequency() const
    {
        return fMinFrequency;
    }

    inline void KTSpectrumDiscriminator::SetMinFrequency(double freq)
    {
        fMinFrequency = freq;
        fCalculateMinBin = true;
        return;
    }

    inline double KTSpectrumDiscriminator::GetMaxFrequency() const
    {
        return fMaxFrequency;
    }

    inline void KTSpectrumDiscriminator::SetMaxFrequency(double freq)
    {
        fMaxFrequency = freq;
        fCalculateMaxBin = true;
        return;
    }

    inline unsigned KTSpectrumDiscriminator::GetMinBin() const
    {
        return fMinBin;
    }

    inline void KTSpectrumDiscriminator::SetMinBin(unsigned bin)
    {
        fMinBin = bin;
        fCalculateMinBin = false;
        return;
    }

    inline unsigned KTSpectrumDiscriminator::GetMaxBin() const
    {
        return fMaxBin;
    }

    inline void KTSpectrumDiscriminator::SetMaxBin(unsigned bin)
    {
        fMaxBin = bin;
        fCalculateMaxBin = false;
        return;
    }
} /* namespace Katydid */
#endif /* KTSPECTRUMDISCRIMINATOR_HH_ */
