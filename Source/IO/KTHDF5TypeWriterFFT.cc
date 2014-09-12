/*
 * KTHDF5TypeWriterFFT.cc
 *
 *  Created on: 9/12/2014
 *      Author: J.N. Kofron
 */

#include <string>
#include <sstream>

#include "KTHDF5TypeWriterFFT.hh"
#include "KTTIFactory.hh"
#include "KTLogger.hh"
#include "KTFrequencySpectrumPolar.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTSliceHeader.hh"
#include "KTMultiFSDataPolar.hh"
#include "KTMultiFSDataFFTW.hh"
#include "KTPowerSpectrum.hh"
#include "KTPowerSpectrumData.hh"
#include "KTTimeFrequencyDataPolar.hh"
#include "KTTimeFrequencyPolar.hh"


namespace Katydid {
    KTLOGGER(publog, "KTHDF5TypeWriterFFT");

    static KTTIRegistrar<KTHDF5TypeWriter, KTHDF5TypeWriterFFT> sH5TWFFTReg;

    KTHDF5TypeWriterFFT::KTHDF5TypeWriterFFT() :
        KTHDF5TypeWriter(),
        polar_fft_buffer(NULL),
        polar_fft_dspace(NULL)
    {}

    KTHDF5TypeWriterFFT::~KTHDF5TypeWriterFFT()
    {}

    void KTHDF5TypeWriterFFT::ProcessEggHeader(KTEggHeader* header)
    {   

        if(header != NULL) {
            KTDEBUG(publog, "Configuring from Egg header...");
            this->n_components = (header->GetNChannels());
            this->slice_size = (header->GetSliceSize());

            /*
             * Polar FFT preparation.  
             * Each component gets two rows - one for abs, and one for arg.
             * Each row is slice_size/2 + 1 long.  
             * Our array is therefore 2N*(slice_size/2 + 1).
             */
            this->polar_fft_size = (this->slice_size >> 1) + 1;
            this->polar_fft_buffer = new boost::multi_array<double,2>(boost::extents[2*n_components][this->polar_fft_size]);
            KTDEBUG(publog, "Done.");
            this->fft_group = fWriter->AddGroup("/frequency_spectra");
        }

        this->CreateDataspaces();
    }

    void KTHDF5TypeWriterFFT::CreateDataspaces() {
        /*
        If the dataspaces have already been created, this is a no-op.  
        Create dataspaces for:
            - Polar FFT data.  This is 2XN where N == slice_len/2 + 1
        */
        if(this->polar_fft_dspace == NULL) {
            KTDEBUG(publog, "Creating H5::DataSpaces for Polar FFT");
            hsize_t polar_fft_dims[] = {2*this->n_components, 
                                        this->polar_fft_size};

            this->polar_fft_dspace = new H5::DataSpace(2, polar_fft_dims);
            KTDEBUG(publog, "Done.");
        }
    }

     H5::DataSet* KTHDF5TypeWriterFFT::CreatePolarFFTDSet(const std::string& name) {
        H5::Group* grp = this->fft_group;
        H5::DSetCreatPropList plist;
        unsigned default_value = 0.0;
        plist.setFillValue(H5::PredType::NATIVE_DOUBLE, &default_value);
        KTDEBUG(publog, "Creating dataset.");
        KTDEBUG(publog, grp);
        H5::DataSet* dset = new H5::DataSet(grp->createDataSet(name.c_str(),
                                                               H5::PredType::NATIVE_DOUBLE,
                                                               *(this->polar_fft_dspace),
                                                               plist));
        KTDEBUG("Done.");
        return dset;
    }


    void KTHDF5TypeWriterFFT::RegisterSlots() {
        fWriter->RegisterSlot("setup-from-header", this, &KTHDF5TypeWriterFFT::ProcessEggHeader);
        fWriter->RegisterSlot("fs-polar", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolar);
        fWriter->RegisterSlot("fs-fftw", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTW);
        fWriter->RegisterSlot("fs-polar-phase", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPhase);
        fWriter->RegisterSlot("fs-fftw-phase", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPhase);
        fWriter->RegisterSlot("fs-polar-power", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPower);
        fWriter->RegisterSlot("fs-fftw-power", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPower);
        fWriter->RegisterSlot("fs-polar-mag-dist", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarMagnitudeDistribution);
        fWriter->RegisterSlot("fs-fftw-mag-dist", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWMagnitudeDistribution);
        fWriter->RegisterSlot("fs-polar-power-dist", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPowerDistribution);
        fWriter->RegisterSlot("fs-fftw-power-dist", this, &KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPowerDistribution);
        fWriter->RegisterSlot("ps", this, &KTHDF5TypeWriterFFT::WritePowerSpectrum);
        fWriter->RegisterSlot("psd", this, &KTHDF5TypeWriterFFT::WritePowerSpectralDensity);
        fWriter->RegisterSlot("ps-dist", this, &KTHDF5TypeWriterFFT::WritePowerSpectrumDistribution);
        fWriter->RegisterSlot("psd-dist", this, &KTHDF5TypeWriterFFT::WritePowerSpectralDensityDistribution);
        fWriter->RegisterSlot("tf-polar", this, &KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolar);
        fWriter->RegisterSlot("tf-polar-phase", this, &KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolarPhase);
        fWriter->RegisterSlot("tf-polar-power", this, &KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolarPower);
        fWriter->RegisterSlot("multi-fs-polar", this, &KTHDF5TypeWriterFFT::WriteMultiFSDataPolar);
        fWriter->RegisterSlot("multi-fs-fftw", this, &KTHDF5TypeWriterFFT::WriteMultiFSDataFFTW);
    }

    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolar(KTDataPtr data) {
        if (!data) return;

        KTDEBUG(publog, "Creating spectrum and dataset...");
        std::string spectrum_name;
        std::stringstream name_builder;

        uint64_t sliceN = data->Of<KTSliceHeader>().GetSliceNumber();
        name_builder << "FSpolar_" << sliceN;
        name_builder >> spectrum_name;

        H5::DataSet* dset = this->CreatePolarFFTDSet(spectrum_name);
        KTDEBUG(publog, "Done.");

        KTFrequencySpectrumDataPolar& fsData = data->Of<KTFrequencySpectrumDataPolar>();
        unsigned nComp = fsData.GetNComponents();

        if( !fWriter->OpenAndVerifyFile() ) return;

        KTDEBUG(publog, "Writing Polar FFT data to HDF5 file.");
        for (unsigned iC = 0; iC < nComp; iC++) {
            const KTFrequencySpectrumPolar* spec = fsData.GetSpectrumPolar(iC);
            if (spec != NULL) {
                for(int f=0; f < spec[0].size(); f++) {
                    (*this->polar_fft_buffer)[iC][f] = spec[0].GetAbs(f);
                    (*this->polar_fft_buffer)[iC+1][f] = spec[0].GetArg(f);
                }
                
            }
        }
        dset->write(this->polar_fft_buffer->data(), H5::PredType::NATIVE_DOUBLE);
        KTDEBUG(publog, "Done.");
    }
  
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTW(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPhase(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPhase(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPower(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPower(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarMagnitudeDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWMagnitudeDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataPolarPowerDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteFrequencySpectrumDataFFTWPowerDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WritePowerSpectrum(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WritePowerSpectralDensity(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WritePowerSpectrumDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WritePowerSpectralDensityDistribution(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolar(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolarPhase(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteTimeFrequencyDataPolarPower(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteMultiFSDataPolar(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
    void KTHDF5TypeWriterFFT::WriteMultiFSDataFFTW(KTDataPtr data) {
        KTDEBUG(publog, "NOT IMPLEMENTED");
    }
}


