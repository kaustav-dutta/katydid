/*
 * KTImageTypeWriterTransform.cc
 *
 *  Created on: Apr 12, 2017
 *      Author: nsoblath
 */

#include "KTImageTypeWriterTransform.hh"

#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTLogger.hh"
#include "KTPowerSpectrumData.hh"

#include "path.hh"

using std::vector;

namespace Katydid
{
    KTLOGGER(publog, "KTImageTypeWriterTransform");

    static Nymph::KTTIRegistrar< KTImageTypeWriter, KTImageTypeWriterTransform > sRSTWTRegistrar;

    KTImageTypeWriterTransform::KTImageTypeWriterTransform() :
            KTImageTypeWriter(),
            //KTTypeWriterTransform()
            fFSPolarSpectrograms(),
            fFSPolarIndicator("fsp"),
            fFSFFTWSpectrograms(),
            fFSFFTWIndicator("fsf"),
            fPowerSpectrograms(),
            fPowerIndicator("ps"),
            fPSDSpectrograms(),
            fPSDIndicator("psd")
    {
    }

    KTImageTypeWriterTransform::~KTImageTypeWriterTransform()
    {
    }

    void KTImageTypeWriterTransform::OutputASpectrogramSet(vector< SpectrogramData >& aSpectrogramSet, const std::string& aDataTypeIndicator)
    {
        // this function does not check the root file; it's assumed to be opened and verified already
        while (! aSpectrogramSet.empty())
        {
            std::stringstream filenameStream;
            filenameStream << fWriter->GetFilenameBase() << '_' << aDataTypeIndicator << '_' << aSpectrogramSet.size() - 1 << '.' << fWriter->GetFileExtension();
            scarab::path filename = scarab::expand_path(filenameStream.str());
            aSpectrogramSet.back().CreateSpectrogram().write(filename.native());
            aSpectrogramSet.pop_back();
        }
        return;
    }

    void KTImageTypeWriterTransform::ClearASpectrogramSet(vector< SpectrogramData >& aSpectrogramSet)
    {
        while (! aSpectrogramSet.empty())
        {
            delete aSpectrogramSet.back().fSpectrogram;
            aSpectrogramSet.pop_back();
        }
        return;
    }

    void KTImageTypeWriterTransform::OutputSpectrograms()
    {
        KTDEBUG(publog, "calling output each spectrogram set")
        OutputASpectrogramSet(fFSPolarSpectrograms, fFSPolarIndicator);
        OutputASpectrogramSet(fFSFFTWSpectrograms, fFSFFTWIndicator);
        OutputASpectrogramSet(fPowerSpectrograms, fPowerIndicator);
        OutputASpectrogramSet(fPSDSpectrograms, fPSDIndicator);

        return;
    }

    void KTImageTypeWriterTransform::ClearSpectrograms()
    {
        ClearASpectrogramSet(fFSPolarSpectrograms);
        ClearASpectrogramSet(fFSFFTWSpectrograms);
        ClearASpectrogramSet(fPowerSpectrograms);
        ClearASpectrogramSet(fPSDSpectrograms);
        return;
    }

    void KTImageTypeWriterTransform::RegisterSlots()
    {
        fWriter->RegisterSlot("fs-polar", this, &KTImageTypeWriterTransform::AddFrequencySpectrumDataPolar);
        fWriter->RegisterSlot("fs-fftw", this, &KTImageTypeWriterTransform::AddFrequencySpectrumDataFFTW);
        fWriter->RegisterSlot("ps", this, &KTImageTypeWriterTransform::AddPowerSpectrumData);
        fWriter->RegisterSlot("psd", this, &KTImageTypeWriterTransform::AddPSDData);
        return;
    }


    //************************
    // Frequency Spectrum Data
    //************************

    void KTImageTypeWriterTransform::AddFrequencySpectrumDataPolar(Nymph::KTDataPtr data)
    {
        AddFrequencySpectrumDataHelper< KTFrequencySpectrumDataPolar >(data, fFSPolarSpectrograms, "FSPolarSpectrogram_");
        return;
    }

    void KTImageTypeWriterTransform::AddFrequencySpectrumDataFFTW(Nymph::KTDataPtr data)
    {
        AddFrequencySpectrumDataHelper< KTFrequencySpectrumDataFFTW >(data, fFSFFTWSpectrograms, "FSFFTWSpectrogram_");
        return;
    }

    //********************
    // Power Spectrum Data
    //********************

    void KTImageTypeWriterTransform::AddPowerSpectrumData(Nymph::KTDataPtr data)
    {
        AddPowerSpectrumDataCoreHelper< KTPowerSpectrumData >(data, fPowerSpectrograms, "PowerSpectrogram_");
        return;
    }

    void KTImageTypeWriterTransform::AddPSDData(Nymph::KTDataPtr data)
    {
        AddPowerSpectralDensityDataCoreHelper< KTPowerSpectrumData >(data, fPSDSpectrograms, "PSDSpectrogram_");
        return;
    }


} /* namespace Katydid */
