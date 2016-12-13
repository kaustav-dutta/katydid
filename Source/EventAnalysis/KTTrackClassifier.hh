/*
 * KTTrackClassifier.hh
 *
 *  Created on: Dec 13, 2016
 *      Author: ezayas
 */

#ifndef KTTRACKCLASSIFIER_HH_
#define KTTRACKCLASSIFIER_HH_

#include "KTProcessor.hh"
#include "KTData.hh"

#include "KTProcessedTrackData.hh"
#include "KTPowerFitData.hh"

#include "KTSlot.hh"
#include "KTLogger.hh"

#include "TMVA/Reader.h"


namespace Katydid
{
    
    KTLOGGER(avlog_hh, "KTTrackClassifier.hh");

    /*
     @class KTTrackClassifier
     @author E. Zayas
     @brief Classifies tracks as signal or background (i.e. sidebands)
     @details
     Reads the output of a machine-learning training algorithm to assign a classifier value to a track. The file to read is generated by TMVA and must exist at runtime.

     Available configuration values:
     - "mva-file": std::string -- location of the XML file produced by TMVA to read
     - "algortihm": std::string -- machine-learning algorithm used to generated the XML file
     - "mva-cut": double -- threshold of the classifier value to label events as a signal

     Slots:
     - "power-fit": void (Nymph::KTDataPtr) -- Performs MVA analysis with the rotated-and-projected parameters; Requires KTProcessedTrackData and KTPowerFitData; Adds nothing

     Signals:
     - "classify": void (Nymph::KTDataPtr) -- Emitted upon successful classification; Guarantees KTProcessedTrackData and KTPowerFitData
    */

    class KTTrackClassifier : public Nymph::KTProcessor
    {
        public:
            KTTrackClassifier(const std::string& name = "track-classifier");
            virtual ~KTTrackClassifier();

            bool Configure(const scarab::param_node* node);

            std::string GetMVAFile() const;
            void SetMVAFile(std::string fileName);

            std::string GetAlgorithm() const;
            void SetAlgorithm(std::string alg);

            double GetMVACut() const;
            void SetMVACut(double value);

        private:
            std::string fMVAFile;
            std::string fAlgorithm;
            double fMVACut;

            // MVA Reader
            TMVA::Reader* reader;

            // Variables for power-fit slot
            float fAverage;
            float fRMS;
            float fSkewness;
            float fKurtosis;
            float fNormCentral;
            float fMeanCentral;
            float fSigmaCentral;
            float fNPeaks;
            float fCentralPowerRatio;
            float fRMSAwayFromCentral;

        public:
            bool ClassifyTrack( KTProcessedTrackData& trackData, double mva );

            //***************
            // Signals
            //***************

        private:
            Nymph::KTSignalData fClassifySignal;

            //***************
            // Slots
            //***************

        private:
            void SlotFunctionPowerFitData( Nymph::KTDataPtr data );

    };

    inline std::string KTTrackClassifier::GetMVAFile() const
    {
        return fMVAFile;
    }

    inline void KTTrackClassifier::SetMVAFile(std::string fileName)
    {
        fMVAFile = fileName;
        return;
    }

    inline std::string KTTrackClassifier::GetAlgorithm() const
    {
        return fAlgorithm;
    }

    inline void KTTrackClassifier::SetAlgorithm(std::string alg)
    {
        fAlgorithm = alg;
        return;
    }

    inline double KTTrackClassifier::GetMVACut() const
    {
        return fMVACut;
    }

    inline void KTTrackClassifier::SetMVACut(double value)
    {
        fMVACut = value;
        return;
    }

    void KTTrackClassifier::SlotFunctionPowerFitData( Nymph::KTDataPtr data )
    {
        // Standard data slot pattern:
        // Check to ensure that the required data types are present

        if (! data->Has< KTProcessedTrackData >())
        {
            KTERROR(avlog_hh, "Data not found with type < KTProcessedTrackData >!");
            return;
        }

        if (! data->Has< KTPowerFitData >())
        {
            KTERROR(avlog_hh, "Data not found with type < KTPowerFitData >!");
            return;
        }

        // Set up reader

        reader = new TMVA::Reader();

        reader->AddVariable( "Average", &fAverage );
        reader->AddVariable( "RMS", &fRMS );
        reader->AddVariable( "Skewness", &fSkewness );
        reader->AddVariable( "Kurtosis", &fKurtosis );
        reader->AddVariable( "NormCentral", &fNormCentral );
        reader->AddVariable( "MeanCentral", &fMeanCentral );
        reader->AddVariable( "SigmaCentral", &fSigmaCentral );
        reader->AddVariable( "NPeaks", &fNPeaks );
        reader->AddVariable( "CentralPowerRatio", &fCentralPowerRatio );
        reader->AddVariable( "RMSAwayFromCentral", &fRMSAwayFromCentral );

        try
        {
            KTDEBUG(avlog_hh, "Algorithm = " << fAlgorithm);
            KTDEBUG(avlog_hh, "MVA File = " << fMVAFile);

            reader->BookMVA( fAlgorithm, fMVAFile );
        }
        catch(...)
        {
            KTERROR(avlog_hh, "Invalid reader configuration; please make sure the algorithm is correct and the file exists. Aborting");
            return;
        }

        KTINFO(avlog_hh, "Successfully set up TMVA reader");

        KTPowerFitData& pfData = data->Of< KTPowerFitData >();

        // Assign variables
        fAverage = (float)(pfData.GetAverage());
        fRMS = (float)(pfData.GetRMS());
        fSkewness = (float)(pfData.GetSkewness());
        fKurtosis = (float)(pfData.GetKurtosis());
        fNormCentral = (float)(pfData.GetNormCentral());
        fMeanCentral = (float)(pfData.GetMeanCentral());
        fSigmaCentral = (float)(pfData.GetSigmaCentral());
        fNPeaks = (float)(pfData.GetNPeaks());
        fCentralPowerRatio = (float)(pfData.GetCentralPowerRatio());
        fRMSAwayFromCentral = (float)(pfData.GetRMSAwayFromCentral());

        double mvaValue = reader->EvaluateMVA( fAlgorithm );
        KTDEBUG(avlog_hh, "Evaluated MVA classifier = " << mvaValue);

        // Call function
        if( !ClassifyTrack( data->Of< KTProcessedTrackData >(), mvaValue ) )
        {
            KTERROR(avlog_hh, "Something went wrong analyzing data of type < KTProcessedTrackData >");
            return;
        }

        KTINFO(avlog_hh, "Classification finished!");

        // Emit signal
        fClassifySignal( data );
    
        return;
    }
}

#endif /* KTTRACKCLASSIFIER_HH_ */
