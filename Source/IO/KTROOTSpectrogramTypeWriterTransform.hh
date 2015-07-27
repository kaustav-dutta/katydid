/*
 * KTROOTSpectrogramTypeWriterTransform.hh
 *
 *  Created on: Jun 18, 2015
 *      Author: nsoblath
 */

#ifndef KTROOTSPECTROGRAMTYPEWRITERTRANSFORM_HH_
#define KTROOTSPECTROGRAMTYPEWRITERTRANSFORM_HH_

#include "KTROOTSpectrogramWriter.hh"

namespace Katydid
{
    class KTROOTSpectrogramTypeWriterTransform : public KTROOTSpectrogramTypeWriter//, public KTTypeWriterTransform
    {
        public:
            KTROOTSpectrogramTypeWriterTransform();
            virtual ~KTROOTSpectrogramTypeWriterTransform();

            void RegisterSlots();

        public:

            void AddFrequencySpectrumDataPolar(KTDataPtr data);
            void OutputFrequencySpectrumDataPolar();

            void AddFrequencySpectrumDataFFTW(KTDataPtr data);
            void OutputFrequencySpectrumDataFFTW();

            void AddPowerSpectrumData(KTDataPtr data);
            void OutputPowerSpectrumData();

            void AddPSDData(KTDataPtr data);
            void OutputPSDData();

            void OutputSpectrograms();

            void ClearSpectrograms();

        private:
            void OutputASpectrogramSet(std::vector< SpectrogramData >& aSpectSet);
            void ClearASpectrogramSet(std::vector< SpectrogramData >& aSpectSet);

            std::vector< SpectrogramData > fFSPolarSpectrograms;
            std::vector< SpectrogramData > fFSFFTWSpectrograms;
            std::vector< SpectrogramData > fPowerSpectrograms;
            std::vector< SpectrogramData > fPSDSpectrograms;
    };


} /* namespace Katydid */
#endif /* KTROOTSPECTROGRAMTYPEWRITERTRANSFORM_HH_ */
