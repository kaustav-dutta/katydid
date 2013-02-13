/*
 * KTGainVariationProcessor.cc
 *
 *  Created on: Dec 10, 2012
 *      Author: nsoblath
 */

#include "KTGainVariationProcessor.hh"

#include "KTBundle.hh"
#include "KTFactory.hh"
#include "KTFrequencySpectrumDataPolar.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTGainVariationData.hh"
#include "KTLogger.hh"
#include "KTPStoreNode.hh"
#include "KTSpline.hh"

#include <cmath>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#endif

using std::string;
using std::vector;
using boost::shared_ptr;

namespace Katydid
{
    KTLOGGER(gvlog, "katydid.analysis");

    static KTDerivedRegistrar< KTProcessor, KTGainVariationProcessor > sGainVarRegistrar("gain-variation");

    KTGainVariationProcessor::KTGainVariationProcessor() :
            KTProcessor(),
            fMinFrequency(0.),
            fMaxFrequency(1.),
            fMinBin(0),
            fMaxBin(1),
            fCalculateMinBin(true),
            fCalculateMaxBin(true),
            fNFitPoints(5),
            fInputDataName("frequency-spectrum"),
            fOutputDataName("gain-variation")
    {
        fConfigName = "gain-variation";

        RegisterSignal("gain-var", &fGainVarSignal, "void (const KTGainVariationData*)");

        RegisterSlot("bundle", this, &KTGainVariationProcessor::ProcessBundle, "void (shared_ptr<KTBundle>)");
        RegisterSlot("fsdata", this, &KTGainVariationProcessor::ProcessFrequencySpectrumData, "void (const KTFrequencySpectrumDataPolar*)");
        RegisterSlot("fsdata-fftw", this, &KTGainVariationProcessor::ProcessFrequencySpectrumDataFFTW, "void (const KTFrequencySpectrumDataFFTW*)");
    }

    KTGainVariationProcessor::~KTGainVariationProcessor()
    {
    }

    Bool_t KTGainVariationProcessor::Configure(const KTPStoreNode* node)
    {
        if (node == NULL) return false;

        if (node->HasData("min-frequency"))
        {
            SetMinFrequency(node->GetData< Double_t >("min-frequency"));
        }
        if (node->HasData("max-frequency"))
        {
            SetMaxFrequency(node->GetData< Double_t >("max-frequency"));
        }

        if (node->HasData("min-bin"))
        {
            SetMinBin(node->GetData< UInt_t >("min-bin"));
        }
        if (node->HasData("max-bin"))
        {
            SetMaxBin(node->GetData< UInt_t >("max-bin"));
        }

        SetNFitPoints(node->GetData< UInt_t >("fit-points", fNFitPoints));

        SetInputDataName(node->GetData< string >("input-data-name", fInputDataName));
        SetOutputDataName(node->GetData< string >("output-data-name", fOutputDataName));

        return true;
    }

    KTGainVariationData* KTGainVariationProcessor::CalculateGainVariation(const KTFrequencySpectrumDataPolar* data)
    {
        if (fCalculateMinBin) SetMinBin(data->GetSpectrumPolar(0)->FindBin(fMinFrequency));
        if (fCalculateMaxBin) SetMaxBin(data->GetSpectrumPolar(0)->FindBin(fMaxFrequency));

        UInt_t nTotalBins = fMaxBin - fMinBin;
        UInt_t nBinsPerFitPoint = nTotalBins / fNFitPoints; // integer division rounds down; there may be bins leftover unused

        KTINFO(gvlog, "Performing gain variation fits with " << fNFitPoints << " points, and " << nBinsPerFitPoint << " bins averaged per fit point.");

        UInt_t nChannels = data->GetNComponents();

        KTGainVariationData* newData = new KTGainVariationData(nChannels);

        Double_t sigmaNorm = 1. / Double_t(nBinsPerFitPoint - 1);
        for (UInt_t iChannel=0; iChannel<nChannels; iChannel++)
        {
            const KTFrequencySpectrumPolar* spectrum = data->GetSpectrumPolar(iChannel);

            Double_t* xVals = new Double_t[fNFitPoints];
            Double_t* yVals = new Double_t[fNFitPoints];

            // Calculate fit points
#pragma omp parallel for default(shared)
            for (UInt_t iFitPoint=0; iFitPoint < fNFitPoints; iFitPoint++)
            {
                UInt_t fitPointStartBin = iFitPoint * nBinsPerFitPoint + fMinBin;
                UInt_t fitPointEndBin = fitPointStartBin + nBinsPerFitPoint;

                Double_t leftEdge = spectrum->GetBinLowEdge(fitPointStartBin);
                Double_t rightEdge = spectrum->GetBinLowEdge(fitPointEndBin);
                xVals[iFitPoint] = leftEdge + 0.5 * (rightEdge - leftEdge);

                Double_t mean = 0.;
                for (UInt_t iBin=fitPointStartBin; iBin<fitPointEndBin; iBin++)
                {
                    mean += (*spectrum)(iBin).abs();
                }
                mean /= (Double_t)nBinsPerFitPoint;
                yVals[iFitPoint] = mean;

                KTDEBUG(gvlog, "Fit point " << iFitPoint << "  " << xVals[iFitPoint] << "  " << yVals[iFitPoint]);
            }

            // Normalize the fit points to 1
            Double_t minYVal = yVals[0];
            for (UInt_t iFitPoint=1; iFitPoint < fNFitPoints; iFitPoint++)
            {
                if (yVals[iFitPoint] < minYVal) minYVal = yVals[iFitPoint];
            }
            for (UInt_t iFitPoint=0; iFitPoint < fNFitPoints; iFitPoint++)
            {
                yVals[iFitPoint] = yVals[iFitPoint] / minYVal;
            }

            KTSpline* spline = new KTSpline(xVals, yVals, fNFitPoints);
            //GainVariation* gainVarResult = CreateGainVariation(spline, spectrum->GetNBins(), spectrum->GetRangeMin(), spectrum->GetRangeMax());

            newData->SetSpline(spline, iChannel);
            //newData->SetGainVariation(gainVarResult, iChannel);
        }

        newData->SetName(fOutputDataName);
        newData->SetBundle(data->GetBundle());

        fGainVarSignal(newData);

        return newData;
    }

    KTGainVariationData* KTGainVariationProcessor::CalculateGainVariation(const KTFrequencySpectrumDataFFTW* data)
    {
        // Frequency spectra include negative and positive frequencies; this algorithm operates only on the positive frequencies.
        if (fCalculateMinBin) SetMinBin(data->GetSpectrumFFTW(0)->FindBin(fMinFrequency));
        if (fCalculateMaxBin) SetMaxBin(data->GetSpectrumFFTW(0)->FindBin(fMaxFrequency));
        KTDEBUG(gvlog, fMinFrequency << "  " << fMaxFrequency << "  " << fMinBin << "  " << fMaxBin << "  " << data->GetSpectrumFFTW(0)->GetRangeMin() << "  " << data->GetSpectrumFFTW(0)->GetRangeMax());

        UInt_t nTotalBins = fMaxBin - fMinBin;
        UInt_t nBinsPerFitPoint = nTotalBins / fNFitPoints; // integer division rounds down; there may be bins leftover unused

        KTINFO(gvlog, "Performing gain variation fit with " << fNFitPoints << " points, and " << nBinsPerFitPoint << " bins averaged per fit point.");

        UInt_t nChannels = data->GetNComponents();

        KTGainVariationData* newData = new KTGainVariationData(nChannels);

        Double_t sigmaNorm = 1. / Double_t(nBinsPerFitPoint - 1);
        for (UInt_t iChannel=0; iChannel<nChannels; iChannel++)
        {
            const KTFrequencySpectrumFFTW* spectrum = data->GetSpectrumFFTW(iChannel);

            Double_t* xVals = new Double_t[fNFitPoints];
            Double_t* yVals = new Double_t[fNFitPoints];

            // Calculate fit points
#pragma omp parallel for default(shared)
            for (UInt_t iFitPoint=0; iFitPoint < fNFitPoints; iFitPoint++)
            {
                UInt_t fitPointStartBin = iFitPoint * nBinsPerFitPoint + fMinBin;
                UInt_t fitPointEndBin = fitPointStartBin + nBinsPerFitPoint;

                Double_t leftEdge = spectrum->GetBinLowEdge(fitPointStartBin);
                Double_t rightEdge = spectrum->GetBinLowEdge(fitPointEndBin);
                xVals[iFitPoint] = leftEdge + 0.5 * (rightEdge - leftEdge);

                Double_t mean = 0.;
                for (UInt_t iBin=fitPointStartBin; iBin<fitPointEndBin; iBin++)
                {
                    mean += sqrt((*spectrum)(iBin)[0] * (*spectrum)(iBin)[0] + (*spectrum)(iBin)[1] * (*spectrum)(iBin)[1]);
                }
                mean /= (Double_t)nBinsPerFitPoint;
                yVals[iFitPoint] = mean;

                KTDEBUG(gvlog, "Fit point " << iFitPoint << "  " << xVals[iFitPoint] << "  " << yVals[iFitPoint]);
            }

            // Normalize the fit points to 1
            Double_t minYVal = yVals[0];
            for (UInt_t iFitPoint=1; iFitPoint < fNFitPoints; iFitPoint++)
            {
                if (yVals[iFitPoint] < minYVal) minYVal = yVals[iFitPoint];
            }
            for (UInt_t iFitPoint=0; iFitPoint < fNFitPoints; iFitPoint++)
            {
                yVals[iFitPoint] = yVals[iFitPoint] / minYVal;
            }


            KTSpline* spline = new KTSpline(xVals, yVals, fNFitPoints);
            //GainVariation* gainVarResult = CreateGainVariation(spline, spectrum->GetNBins(), spectrum->GetRangeMin(), spectrum->GetRangeMax());

            newData->SetSpline(spline, iChannel);
            //newData->SetGainVariation(gainVarResult, iChannel);
        }

        newData->SetName(fOutputDataName);
        newData->SetBundle(data->GetBundle());

        fGainVarSignal(newData);

        return newData;
    }

    /*
    KTGainVariationProcessor::GainVariation* KTGainVariationProcessor::CreateGainVariation(TSpline* spline, UInt_t nBins, Double_t rangeMin, Double_t rangeMax) const
    {
        GainVariation* newGainVar = new GainVariation(nBins, rangeMin, rangeMax);

        // The fit region: [fMinBin, fMaxBin)
        // Keep track of the minimum value so we can shift it down to 1
        Double_t minVal = spline->Eval(newGainVar->GetBinCenter(fMinBin));
        for (UInt_t iBin = fMinBin; iBin < fMaxBin; iBin++)
        {
            (*newGainVar)(iBin) = spline->Eval(newGainVar->GetBinCenter(iBin));
            if ((*newGainVar)(iBin) < minVal)
            {
                minVal = (*newGainVar)(iBin);
            }
        }
        for (UInt_t iBin = fMinBin; iBin < fMaxBin; iBin++)
        {
            (*newGainVar)(iBin) = (*newGainVar)(iBin) / minVal;
        }

        // Before the fit region: [0, fMinBin)
        for (UInt_t iBin = 0; iBin < fMinBin; iBin++)
        {
            (*newGainVar)(iBin) = 1.;
        }

        // After the fit region: [fMaxBin, nBins)
        for (UInt_t iBin = fMaxBin; iBin < nBins; iBin++)
        {
            (*newGainVar)(iBin) = 1.;
        }

        return newGainVar;
    }
    */

    void KTGainVariationProcessor::ProcessBundle(shared_ptr<KTBundle> bundle)
    {
        const KTFrequencySpectrumDataPolar* fsData = bundle->GetData< KTFrequencySpectrumDataPolar >(fInputDataName);
        if (fsData != NULL)
        {
            KTGainVariationData* newData = CalculateGainVariation(fsData);
            bundle->AddData(newData);
            return;
        }

        const KTFrequencySpectrumDataFFTW* fsDataFFTW = bundle->GetData< KTFrequencySpectrumDataFFTW >(fInputDataName);
        if (fsDataFFTW != NULL)
        {
            KTGainVariationData* newData = CalculateGainVariation(fsDataFFTW);
            bundle->AddData(newData);
            return;
        }

        KTWARN(gvlog, "No time series data named <" << fInputDataName << "> was available in the bundle");
        return;
    }

    void KTGainVariationProcessor::ProcessFrequencySpectrumData(const KTFrequencySpectrumDataPolar* data)
    {
        KTGainVariationData* newData = CalculateGainVariation(data);
        if (data->GetBundle() != NULL)
            data->GetBundle()->AddData(newData);
        return;
    }
    void KTGainVariationProcessor::ProcessFrequencySpectrumDataFFTW(const KTFrequencySpectrumDataFFTW* data)
    {
        KTGainVariationData* newData = CalculateGainVariation(data);
        if (data->GetBundle() != NULL)
            data->GetBundle()->AddData(newData);
        return;
    }

} /* namespace Katydid */
