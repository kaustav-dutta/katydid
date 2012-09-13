/*
 * KTFrequencySpectrumData.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTFrequencySpectrumData.hh"

#include "KTFrequencySpectrum.hh"
#include "KTPublisher.hh"

namespace Katydid
{
    std::string KTFrequencySpectrumData::fName("frequency-spectrum");

    const std::string& KTFrequencySpectrumData::StaticGetName()
    {
        return fName;
    }

    KTFrequencySpectrumData::KTFrequencySpectrumData(unsigned nChannels) :
            KTWriteableData(),
            fSpectra(nChannels)
    {
    }

    KTFrequencySpectrumData::~KTFrequencySpectrumData()
    {
        while (! fSpectra.empty())
        {
            delete fSpectra.back();
            fSpectra.pop_back();
        }
    }

    void KTFrequencySpectrumData::Accept(KTPublisher* publisher) const
    {
        publisher->Write(this);
        return;
    }

} /* namespace Katydid */
