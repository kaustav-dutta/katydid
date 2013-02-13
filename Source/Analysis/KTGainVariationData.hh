/*
 * KTGainVariationData.hh
 *
 *  Created on: Dec 10, 2012
 *      Author: nsoblath
 */

#ifndef KTGAINVARIATIONDATA_HH_
#define KTGAINVARIATIONDATA_HH_

#include "KTWriteableData.hh"

#include "KTGainVariationProcessor.hh"
#include "KTSpline.hh"

#ifdef ROOT_FOUND
#include "TH1.h"
#endif

#include <vector>

namespace Katydid
{

    class KTGainVariationData : public KTWriteableData
    {
        public:
            //typedef KTGainVariationProcessor::GainVariation GainVariation;

        protected:
            struct PerChannelData
            {
                KTSpline* fSpline;
                //GainVariation* fGainVar;
            };

        public:
            KTGainVariationData(UInt_t nChannels=1);
            virtual ~KTGainVariationData();

            //const GainVariation* GetGainVariation(UInt_t component = 0) const;
            //GainVariation* GetGainVariation(UInt_t component = 0);
            const KTSpline* GetSpline(UInt_t component = 0) const;
            KTSpline* GetSpline(UInt_t component = 0);
            UInt_t GetNComponents() const;

            //void SetGainVariation(GainVariation* record, UInt_t component = 0);
            void SetSpline(KTSpline* spline, UInt_t component = 0);
            void SetNComponents(UInt_t channels);

            void Accept(KTWriter* writer) const;

        protected:
            static std::string fDefaultName;

            std::vector< PerChannelData > fChannelData;

#ifdef ROOT_FOUND
        public:
            virtual TH1D* CreateGainVariationHistogram(UInt_t nBins, UInt_t component = 0, const std::string& name = "hGainVariation") const;
#endif
    };
/*
    inline const KTGainVariationData::GainVariation* KTGainVariationData::GetGainVariation(UInt_t component) const
    {
        return fChannelData[component].fGainVar;
    }

    inline KTGainVariationData::GainVariation* KTGainVariationData::GetGainVariation(UInt_t component)
    {
        return fChannelData[component].fGainVar;
    }
*/
    inline const KTSpline* KTGainVariationData::GetSpline(UInt_t component) const
    {
        return fChannelData[component].fSpline;
    }

    inline KTSpline* KTGainVariationData::GetSpline(UInt_t component)
    {
        return fChannelData[component].fSpline;
    }

    inline UInt_t KTGainVariationData::GetNComponents() const
    {
        return UInt_t(fChannelData.size());
    }
/*
    inline void KTGainVariationData::SetGainVariation(GainVariation* record, UInt_t component)
    {
        if (component >= fChannelData.size()) fChannelData.resize(component+1);
        fChannelData[component].fGainVar = record;
    }
*/
    inline void KTGainVariationData::SetSpline(KTSpline* spline, UInt_t component)
    {
        if (component >= fChannelData.size()) fChannelData.resize(component+1);
        fChannelData[component].fSpline = spline;
    }

    inline void KTGainVariationData::SetNComponents(UInt_t channels)
    {
        fChannelData.resize(channels);
        return;
    }


} /* namespace Katydid */

#endif /* KTGAINVARIATIONDATA_HH_ */
