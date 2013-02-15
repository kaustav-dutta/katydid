/*
 * KTMultiBundleROOTTypeWriterEgg.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTMultiBundleROOTTypeWriterEgg.hh"

#include "KTEggHeader.hh"
#include "KTTIFactory.hh"
#include "KTLogger.hh"
#include "KTTimeSeries.hh"
#include "KTTimeSeriesData.hh"

#include "TCanvas.h"
#include "TH1.h"
#include "TStyle.h"

#include <sstream>

using boost::shared_ptr;

using std::stringstream;
using std::string;
using std::vector;

namespace Katydid
{
    KTLOGGER(publog, "katydid.output");

    static KTDerivedTIRegistrar< KTMultiBundleROOTTypeWriter, KTMultiBundleROOTTypeWriterEgg > sMERTWERegistrar;

    KTMultiBundleROOTTypeWriterEgg::KTMultiBundleROOTTypeWriterEgg() :
            KTMEROOTTypeWriterBase(),
            //KTTypeWriterEgg()
            fTSHists()
    {
    }

    KTMultiBundleROOTTypeWriterEgg::~KTMultiBundleROOTTypeWriterEgg()
    {
        ClearHistograms();
    }

    void KTMultiBundleROOTTypeWriterEgg::StartNewHistograms()
    {
        ClearHistograms();
        // At this point the vector is size 0
        return;
    }

    void KTMultiBundleROOTTypeWriterEgg::FinishHistograms()
    {
        OutputHistograms();
        ClearHistograms();
        return;
    }

    void KTMultiBundleROOTTypeWriterEgg::OutputHistograms()
    {
        if (! fWriter->OpenAndVerifyFile()) return;

        gStyle->SetOptStat(0);
        for (UInt_t iComponent=0; iComponent < fTSHists.size(); iComponent++)
        {
            // Printing to graphics files
            stringstream conv;
            conv << "_ts_" << iComponent << "." << fWriter->GetGraphicsFileType();
            string fileName = fWriter->GetGraphicsFilenameBase() + conv.str();
            if (! fWriter->GetGraphicsFilePath().empty()) fileName = fWriter->GetGraphicsFilePath() + '/' + fileName;

            TCanvas* cPrint = new TCanvas("cPrint", "cPrint");
            cPrint->SetLogy(1);
            fTSHists[iComponent]->Draw();

            cPrint->Print(fileName.c_str(), fWriter->GetGraphicsFileType().c_str());
            KTINFO(publog, "Printed file " << fileName);
            delete cPrint;

            // Writing to ROOT file
            fTSHists[iComponent]->SetDirectory(fWriter->GetFile());
            fTSHists[iComponent]->Write();
        }

        return;
    }

    void KTMultiBundleROOTTypeWriterEgg::ClearHistograms()
    {
        for (vector<TH1D*>::iterator it=fTSHists.begin(); it != fTSHists.end(); it++)
        {
            delete *it;
        }
        fTSHists.clear();
        return;
    }

    void KTMultiBundleROOTTypeWriterEgg::RegisterSlots()
    {
        fWriter->RegisterSlot("start-by-header", this, &KTMultiBundleROOTTypeWriterEgg::StartByHeader, "void (const KTEggHeader*)");

        fWriter->RegisterSlot("ts-data", this, &KTMultiBundleROOTTypeWriterEgg::AddTimeSeriesData, "void (shared_ptr<KTData>)");
        return;
    }


    void KTMultiBundleROOTTypeWriterEgg::StartByHeader(const KTEggHeader* header)
    {
        fWriter->Start();
        return;
    }


    //*****************
    // Time Series Data
    //*****************

    void KTMultiBundleROOTTypeWriterEgg::AddTimeSeriesData(shared_ptr<KTData> data)
    {
        KTTimeSeriesData& tsData = data->Of<KTTimeSeriesData>();
        if (fTSHists.size() == 0)
        {
            fTSHists.resize(tsData.GetNComponents());

            std::string histNameBase("PowerSpectrum");
            for (UInt_t iComponent=0; iComponent < tsData.GetNComponents(); iComponent++)
            {
                std::stringstream conv;
                conv << iComponent;
                std::string histName = histNameBase + conv.str();
                TH1D* newPS = tsData.GetTimeSeries(iComponent)->CreateHistogram(histName);
                fTSHists[iComponent] = newPS;
            }
        }
        else
        {
            for (UInt_t iComponent=0; iComponent < tsData.GetNComponents(); iComponent++)
            {
                TH1D* newTS = tsData.GetTimeSeries(iComponent)->CreateHistogram();
                fTSHists[iComponent]->Add(newTS);
                delete newTS;
            }
        }
        return;
    }

} /* namespace Katydid */
