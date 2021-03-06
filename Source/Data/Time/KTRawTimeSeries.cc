/*
 * KTRawTimeSeries.cc
 *
 *  Created on: Dec 23, 2013
 *      Author: nsoblath
 */

#include "KTRawTimeSeries.hh"

namespace Katydid
{

    KTRawTimeSeries::KTRawTimeSeries() :
            KTVarTypePhysicalArray< uint64_t >(),
            fSampleSize(1)
    {
        SetAt(0., 0);
    }

    KTRawTimeSeries::KTRawTimeSeries(size_t dataTypeSize, uint32_t dataFormat, size_t nBins, double rangeMin, double rangeMax) :
            KTVarTypePhysicalArray< uint64_t >(dataTypeSize, dataFormat, nBins, rangeMin, rangeMax),
            fSampleSize(1)
    {
        for (unsigned iBin = 0; iBin < nBins; ++iBin)
        {
            SetAt(0., iBin);
        }
    }

    KTRawTimeSeries::KTRawTimeSeries(const KTRawTimeSeries& orig) :
            KTVarTypePhysicalArray< uint64_t >(orig),
            fSampleSize(orig.fSampleSize)
    {
    }

    KTRawTimeSeries::~KTRawTimeSeries()
    {
    }

    KTRawTimeSeries& KTRawTimeSeries::operator=(const KTRawTimeSeries& rhs)
    {
        KTVarTypePhysicalArray< uint64_t >::operator=(rhs);
        fSampleSize = rhs.fSampleSize;
        return *this;
    }

} /* namespace Katydid */
