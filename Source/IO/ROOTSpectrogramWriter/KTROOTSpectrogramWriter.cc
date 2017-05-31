/*
 * KTROOTSpectrogramWriter.cc
 *
 *  Created on: Jun 18, 2015
 *      Author: nsoblath
 */

#include "KTROOTSpectrogramWriter.hh"

#include "KTROOTWriterFileManager.hh"

#include "KTCommandLineOption.hh"

#include <algorithm>

using std::string;
using std::stringstream;

namespace Katydid
{
    KTLOGGER(publog, "KTROOTSpectrogramWriter");


    KT_REGISTER_WRITER(KTROOTSpectrogramWriter, "root-spectrogram-writer");
    KT_REGISTER_PROCESSOR(KTROOTSpectrogramWriter, "root-spectrogram-writer");

    static Nymph::KTCommandLineOption< string > sRSWFilenameCLO("ROOT Spectrogram Writer", "ROOT spectrogram writer filename", "rsw-file");

    KTROOTSpectrogramWriter::KTROOTSpectrogramWriter(const std::string& name) :
            KTWriterWithTypists< KTROOTSpectrogramWriter, KTROOTSpectrogramTypeWriter >(name),
            fFilename("spect_output.root"),
            fFileFlag("recreate"),
            fMinTime(0.),
            fMaxTime(0.),
            fMinFreq(0.),
            fMaxFreq(0.),
            fBufferFreq(0.),
            fBufferTime(0.),
            fFile(NULL),
            fFileManager(KTROOTWriterFileManager::get_instance()),
            fWriteFileSlot("write-file", this, &KTROOTSpectrogramWriter::WriteFile)
    {
    }

    KTROOTSpectrogramWriter::~KTROOTSpectrogramWriter()
    {
        CloseFile();
    }

    bool KTROOTSpectrogramWriter::Configure(const scarab::param_node* node)
    {
        // Config-file settings
        if (node != NULL)
        {
            Filename() = node->get_value("output-file", fFilename);
            FileFlag() = node->get_value("file-flag", fFileFlag);
            SetMinFreq(node->get_value("min-freq", fMinFreq));
            SetMaxFreq(node->get_value("max-freq", fMaxFreq));
            SetMinTime(node->get_value("min-time", fMinTime));
            SetMaxTime(node->get_value("max-time", fMaxTime));
            SetBufferFreq(node->get_value("buffer-freq", fBufferFreq));
            SetBufferTime(node->get_value("buffer-time", fBufferTime));
        }

        // Command-line settings
        Filename() = fCLHandler->GetCommandLineValue< string >("rsw-file", fFilename);

        return true;
    }

    TFile* KTROOTSpectrogramWriter::OpenFile(const std::string& filename, const std::string& flag)
    {
        CloseFile();
        fFile = fFileManager->OpenFile(this, filename.c_str(), flag.c_str());
        return fFile;
    }

    void KTROOTSpectrogramWriter::CloseFile()
    {
        if (fFile != NULL)
        {
            fFileManager->FinishFile(this, fFilename);
            fFile = NULL;
        }
        return;
    }

    bool KTROOTSpectrogramWriter::OpenAndVerifyFile()
    {
        if (fFile == NULL)
        {
            fFile = fFileManager->OpenFile(this, fFilename.c_str(), fFileFlag.c_str());
        }
        if (! fFile->IsOpen())
        {
            fFileManager->DiscardFile(this, fFilename);
            fFile = NULL;
            KTERROR(publog, "Output file <" << fFilename << "> did not open!");
            return false;
        }
        fFile->cd();
        return true;
    }

    void KTROOTSpectrogramWriter::WriteFile()
    {
        KTDEBUG("In write-file slot")
        for (TypeWriterMap::iterator thisTypeWriter = fTypeWriters.begin(); thisTypeWriter != fTypeWriters.end(); ++thisTypeWriter)
        {
            thisTypeWriter->second->OutputSpectrograms();
        }
    }


    //****************************
    // KTROOTSpectrogramTypeWriter
    //****************************

    KTROOTSpectrogramTypeWriter::KTROOTSpectrogramTypeWriter() :
        KTDerivedTypeWriter< KTROOTSpectrogramWriter >()
    {
    }

    KTROOTSpectrogramTypeWriter::~KTROOTSpectrogramTypeWriter()
    {
    }

    void KTROOTSpectrogramTypeWriter::CreateNewSpectrograms(const KTFrequencyDomainArrayData& data, unsigned nComponents, double startTime, double sliceLength, std::vector< SpectrogramData >& spectrograms, string histNameBase)
    {
        if (spectrograms.size() < nComponents)
        {
            // Yes, we do need to resize the vector of histograms
            // Get number of components and resize the vector of histograms
            unsigned currentSize = spectrograms.size();
            spectrograms.resize(nComponents);

            // calculate the properties of the time axis
            unsigned nSlices = unsigned((fWriter->GetMaxTime() - startTime) / sliceLength) + 1; // the +1 is so that the end time is the first slice ending outside the max time.
            double endTime = startTime + sliceLength * (double)nSlices;
            std::cout << fWriter->GetMaxTime() << "  " << startTime << "  " << sliceLength << "  " << nSlices << "  " << endTime << std::endl;
            for (unsigned iComponent = currentSize; iComponent < nComponents; ++iComponent)
            {
                // calculate the properties of the frequency axis
                double freqBinWidth = data.GetArray(iComponent)->GetAxis().GetBinWidth();
                const KTAxisProperties< 1 >& axis = data.GetArray(iComponent)->GetAxis();
                spectrograms[iComponent].fFirstFreqBin = std::max< unsigned >(0, axis.FindBin(fWriter->GetMinFreq()));
                spectrograms[iComponent].fLastFreqBin = std::min< unsigned >(axis.GetNBins()-1, axis.FindBin(fWriter->GetMaxFreq()));
                //spectrograms[iComponent].fFirstFreqBin = unsigned((fWriter->GetMinFreq() - data.GetArray(iComponent)->GetAxis().GetBinLowEdge(0)) / freqBinWidth);
                //spectrograms[iComponent].fLastFreqBin = unsigned((fWriter->GetMaxFreq() - data.GetArray(iComponent)->GetAxis().GetBinLowEdge(0)) / freqBinWidth) + 1;
                unsigned nFreqBins = spectrograms[iComponent].fLastFreqBin - spectrograms[iComponent].fFirstFreqBin + 1;
                //double startFreq = spectrograms[iComponent].fFirstFreqBin * freqBinWidth;
                //double endFreq = spectrograms[iComponent].fLastFreqBin * freqBinWidth;
                double startFreq = axis.GetBinLowEdge(spectrograms[iComponent].fFirstFreqBin);
                double endFreq = axis.GetBinLowEdge(spectrograms[iComponent].fLastFreqBin) + freqBinWidth;
                // form the histogram name
                stringstream conv;
                conv << iComponent;
                string histName = histNameBase + conv.str();
                KTDEBUG(publog, "Creating new spectrogram histogram for component " << iComponent << ": " << histName << ", " << nSlices << ", " << startTime << ", " << endTime << ", " << nFreqBins << ", " << startFreq << ", " << endFreq);
                spectrograms[iComponent].fSpectrogram = new TH2D(histName.c_str(), "Spectrogram", nSlices, startTime, endTime, nFreqBins, startFreq, endFreq );
                spectrograms[iComponent].fSpectrogram->SetXTitle("Time (s)");
                spectrograms[iComponent].fSpectrogram->SetYTitle(axis.GetAxisLabel().c_str());
                spectrograms[iComponent].fSpectrogram->SetZTitle(data.GetArray(iComponent)->GetOrdinateLabel().c_str());
            }
        } // done initializing new spectrograms
    }


} /* namespace Katydid */
