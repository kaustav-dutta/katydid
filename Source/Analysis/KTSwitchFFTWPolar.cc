/*
 * KTSwitchFFTWPolar.cc
 *
 *  Created on: Mar 19, 2013
 *      Author: nsoblath
 */

#include "KTSwitchFFTWPolar.hh"

#include "KTFactory.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTFrequencySpectrumFFTW.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTNormalizedFSData.hh"
#include "KTLogger.hh"
#include "KTNormalizedFSData.hh"
#include "KTPStoreNode.hh"
#include "KTWignerVilleData.hh"

using std::string;
using boost::shared_ptr;

namespace Katydid
{
    KTLOGGER(gnlog, "katydid.analysis");

    static KTDerivedRegistrar< KTProcessor, KTSwitchFFTWPolar > sSwitchFFTWPolar("switch-fftw-polar");

    KTSwitchFFTWPolar::KTSwitchFFTWPolar(const std::string& name) :
            KTProcessor(name),
            fFSPolarSignal("fs-polar", this),
            //fFSFFTWSignal("fs-fftw", this),
            fFSFFTWSlot("fs-fftw", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal),
            fNormFSFFTWSlot("norm-fs-fftw", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal),
            fWignerVilleSlot("wv", this, &KTSwitchFFTWPolar::SwitchToPolar, &fFSPolarSignal)
    {
    }

    KTSwitchFFTWPolar::~KTSwitchFFTWPolar()
    {
    }

    Bool_t KTSwitchFFTWPolar::Configure(const KTPStoreNode* node)
    {
        return true;
    }


    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTFrequencySpectrumDataFFTW& fsData)
    {
        UInt_t nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (UInt_t iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar();
            if (newSpectrum == NULL)
            {
                KTERROR(gnlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(gnlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTNormalizedFSDataFFTW& fsData)
    {
        UInt_t nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (UInt_t iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar();
            if (newSpectrum == NULL)
            {
                KTERROR(gnlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(gnlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

    Bool_t KTSwitchFFTWPolar::SwitchToPolar(KTWignerVilleData& fsData)
    {
        UInt_t nComponents = fsData.GetNComponents();

        KTFrequencySpectrumDataPolar& newData = fsData.Of< KTFrequencySpectrumDataPolar >().SetNComponents(nComponents);

        for (UInt_t iComponent=0; iComponent<nComponents; iComponent++)
        {
            KTFrequencySpectrumPolar* newSpectrum = fsData.GetSpectrumFFTW(iComponent)->CreateFrequencySpectrumPolar();
            if (newSpectrum == NULL)
            {
                KTERROR(gnlog, "Switch of spectrum " << iComponent << " (fftw->polar) failed for some reason. Continuing processing.");
                continue;
            }
            newData.SetSpectrum(newSpectrum, iComponent);
        }
        KTINFO(gnlog, "Completed switch (fftw->polar) of " << nComponents << " frequency spectra (polar)");

        return true;
    }

} /* namespace Katydid */
