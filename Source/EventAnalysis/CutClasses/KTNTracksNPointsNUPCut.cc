/*
 * KTEventNTracksFirstTrackNPointsNUPCut.cc
 *
 *  Created on: February 18, 2019
 *      Author: Yuhao
 */

#include "KTNTracksNPointsNUPCut.hh"

#include "KTMultiTrackEventData.hh"

#include "KTLogger.hh"

using std::vector;

namespace Katydid
{
    KTLOGGER(ecnuplog, "KTNTracksNPointsNUPCut");

    const std::string KTNTracksNPointsNUPCut::Result::sName = "event-ntracks-first-track-npoints-nup-cut-nso";

    KT_REGISTER_CUT(KTNTracksNPointsNUPCut);

    KTNTracksNPointsNUPCut::KTNTracksNPointsNUPCut(const std::string& name) :
                 KTCutOneArg(name),
                 fThresholds(),
                 fWideOrNarrow( WideOrNarrow::wide ),
                 fTimeOrBinAverage( TimeOrBinAvg::time )
    {}

    KTNTracksNPointsNUPCut::~KTNTracksNPointsNUPCut()
    {}

    bool KTNTracksNPointsNUPCut::Configure(const scarab::param_node* node)
    {
        if (node == NULL) return false;

        if (! node->has("parameters") || ! node->at("parameters")->is_array())
        {
            KTERROR(ecnuplog, "No cut parameters were provided, or \"parameters\" was not an array");
            return false;
        }

        const scarab::param_array* parameters = node->array_at("parameters");
        unsigned nParamSets = parameters->size();

        // Create the vector that will temporarily hold the parameter sets after the first loop
        struct extendedThresholds : thresholds
        {
            unsigned fFTNPoints;
            unsigned fNTracks;
        };
        vector< extendedThresholds > tempThresholds(nParamSets);

        // First loop: extract all parameters, and the max dimensions of the thresholds array
        unsigned maxFTNPointsConfig = 0;
        unsigned maxNTracksConfig = 0;
        unsigned tempThreshPos = 0;
        for (auto paramIt = parameters->begin(); paramIt != parameters->end(); ++paramIt)
        {
            if (! (*paramIt)->is_node())
            {
                KTERROR(ecnuplog, "Invalid set of parameters");
                return false;
            }
            const scarab::param_node& oneSetOfParams = (*paramIt)->as_node();

            try
            {
                unsigned ftNPoints = oneSetOfParams.get_value< unsigned >("ft-npoints");
                unsigned nTracks = oneSetOfParams.get_value< unsigned >("ntracks");
                if (ftNPoints > maxFTNPointsConfig) maxFTNPointsConfig = ftNPoints;
                if (nTracks > maxNTracksConfig) maxNTracksConfig = nTracks;

                tempThresholds[tempThreshPos].fFTNPoints = ftNPoints;
                tempThresholds[tempThreshPos].fNTracks = nTracks;

                tempThresholds[tempThreshPos].fMinTotalNUP = oneSetOfParams.get_value< double >("min-total-nup");
                tempThresholds[tempThreshPos].fMinAverageNUP = oneSetOfParams.get_value< double >("min-average-nup");
                tempThresholds[tempThreshPos].fMinMaxNUP = oneSetOfParams.get_value< double >("min-max-track-nup");
            }
            catch( scarab::error& e )
            {
                // this will catch scarab::errors from param_node::get_value in the case that a parameter is missing
                KTERROR(ecnuplog, "An incomplete set of parameters was found: " << oneSetOfParams);
                return false;
            }

            ++tempThreshPos;
        }

        // Create the 2D thresholds array
        // Dimensions are are maxNTracksConfig + 1 rows by maxFTNPointsConfig + 1 columns
        // Positions in the array for zero n-tracks and zero ft-npoints are kept to make later indexing of fThresholds simpler
        ++maxNTracksConfig;
        ++maxFTNPointsConfig;
        KTDEBUG(ecnuplog, "maxNTracksConfig = " << maxNTracksConfig << "  " << "maxFTNPointsConfig = " << maxFTNPointsConfig);
        fThresholds.clear();
        fThresholds.resize(maxNTracksConfig);
        // skip the first row; there will never be 0 tracks
        for (unsigned iRow = 1; iRow < maxNTracksConfig; ++iRow)
        {
            fThresholds[iRow].resize(maxFTNPointsConfig);
        }

        // Second loop: fill in the 2D array
        for (auto oneParamSet : tempThresholds)
        {
            fThresholds[oneParamSet.fNTracks][oneParamSet.fFTNPoints].fMinTotalNUP = oneParamSet.fMinTotalNUP;
            fThresholds[oneParamSet.fNTracks][oneParamSet.fFTNPoints].fMinAverageNUP = oneParamSet.fMinAverageNUP;
            fThresholds[oneParamSet.fNTracks][oneParamSet.fFTNPoints].fMinMaxNUP = oneParamSet.fMinMaxNUP;
            fThresholds[oneParamSet.fNTracks][oneParamSet.fFTNPoints].fFilled = true;
        }

#ifndef NDEBUG
        {
            std::stringstream arrayStream;
            for (auto oneRow : fThresholds)
            {
                arrayStream << "[ ";
                for (auto oneParamSet : oneRow)
                {
                    arrayStream << "[" << oneParamSet.fMinTotalNUP << ", " << oneParamSet.fMinAverageNUP << ", " << oneParamSet.fMinMaxNUP << "] ";
                }
                arrayStream << "]\n";
            }
            KTDEBUG(ecnuplog, "Thresholds prior to filling:\n" << arrayStream.str());
        }
#endif

        // Now fill in any gaps in rows
#ifndef NDEBUG
        unsigned iRow = 0;
#endif
        for (auto oneRowPtr = fThresholds.begin(); oneRowPtr != fThresholds.end(); ++oneRowPtr)
        {
            if (oneRowPtr->empty()) continue;

            unsigned iFilledPos = 0;
            // find the first non-zero position from the left, and fill that value to the left
            for (iFilledPos = 0; ! (*oneRowPtr)[iFilledPos].fFilled && iFilledPos != oneRowPtr->size(); ++iFilledPos) {}
            KTDEBUG(ecnuplog, "Row " << iRow << ": first filled position is " << iFilledPos << " or it ranged out at " << oneRowPtr->size());
            if (iFilledPos == oneRowPtr->size())
            {
                KTWARN(ecnuplog, "Empty threshold row found");
                continue;
            }
            for (unsigned iPos = 1; iPos < iFilledPos; ++iPos)
            {
                (*oneRowPtr)[iPos] = (*oneRowPtr)[iFilledPos];
            }

            // find the first non-zero position from the right, and fill that value to the right
            // there's no risk of finding an unfilled row, since we would have caught that in the previous section
            for (iFilledPos = oneRowPtr->size()-1; ! (*oneRowPtr)[iFilledPos].fFilled; --iFilledPos) {}
            KTDEBUG(ecnuplog, "Row " << iRow << ": last filled position is " << iFilledPos);
            for (unsigned iPos = oneRowPtr->size()-1; iPos > iFilledPos; --iPos)
            {
                (*oneRowPtr)[iPos] = (*oneRowPtr)[iFilledPos];
            }

            // fill in any holes from left to right
            // there are no completely unfilled rows
            // rows will have a minimum size of 2
            // for a given position, if it's unfilled, then fill from the position to the left
            for (unsigned iPos = 1; iPos < oneRowPtr->size(); ++iPos)
            {
                if (! (*oneRowPtr)[iPos].fFilled)
                {
                    (*oneRowPtr)[iPos] = (*oneRowPtr)[iPos-1];
                }
            }

#ifndef NDEBUG
            ++iRow;
#endif
        }

#ifndef NDEBUG
        {
            std::stringstream arrayStream;
            for (auto oneRow : fThresholds)
            {
                arrayStream << "[ ";
                for (auto oneParamSet : oneRow)
                {
                    arrayStream << "[" << oneParamSet.fMinTotalNUP << ", " << oneParamSet.fMinAverageNUP << ", " << oneParamSet.fMinMaxNUP << "] ";
                }
                arrayStream << "]\n";
            }
            KTDEBUG(ecnuplog, "Thresholds prior to filling:\n" << arrayStream.str());
        }
#endif

/*
     parameters:
      - ft-npoints: 3
        ntracks: 1
        min-total-nup: 0
        min-average-nup: 13
        min-max-track-nup: 0
      - ft-npoints: 3
        ntracks: 2
        min-total-nup: 0
        min-average-nup: 11
        min-max-track-nup: 0
      - ft-npoints: 3
        ntracks: 3
        min-total-nup: 0
        min-average-nup: 7.8
        min-max-track-nup: 0
      - ft-npoints: 4
        ntracks: 1
        min-total-nup: 0
        min-average-nup: 10
        min-max-track-nup: 0
      - ft-npoints: 4
        ntracks: 2
        min-total-nup: 0
        min-average-nup: 8.5
        min-max-track-nup: 0
      - ft-npoints: 5
        ntracks: 1
        min-total-nup: 0
        min-average-nup: 7.8
        min-max-track-nup: 0
      - ft-npoints: 5
        ntracks: 2
        min-total-nup: 0
        min-average-nup: 7.8
        min-max-track-nup: 0
      - ft-npoints: 6
        ntracks: 1
        min-total-nup: 0
        min-average-nup: 8.5
        min-max-track-nup: 0
      - ft-npoints: 7
        ntracks: 1
        min-total-nup: 0
        min-average-nup: 7.3
        min-max-track-nup: 0

        To fill in the cut parameters:
        1. scan array to get dimensions
        2. create 2D array with those dimensions
        3. store offset and maximum of both dimensions
        3. fill 2D array with -1
        4. fill in values given
        5. for each row:
            1. scan horizontally from left to find first >=0 value, then fill back in to the left side
            2. scan horizontally from the right to find the first >=0 value, then fill back in to the right side
            3. check for holes in between and fill from the left

        Need to work out function to retrieve the cut parameters with appropriate bounding

         */

        if (node->has("wide-or-narrow"))
        {
            if (node->get_value("wide-or-narrow") == "wide")
            {
                SetWideOrNarrow(WideOrNarrow::wide);
            }
            else if (node->get_value("wide-or-narrow") == "narrow")
            {
                SetWideOrNarrow(WideOrNarrow::narrow);
            }
            else
            {
                KTERROR(ecnuplog, "Invalid string for fWideOrNarrow");
                return false;
            }
        }
        if (node->has("time-or-bin-average"))
        {
            if (node->get_value("time-or-bin-average") == "time")
            {
                SetTimeOrBinAverage(TimeOrBinAvg:: time);
            }
            else if (node->get_value("time-or-bin-average") == "bin")
            {
                SetTimeOrBinAverage(TimeOrBinAvg::bin);
            }
            else
            {
                KTERROR(ecnuplog, "Invalid string for fTimeOrBinAverage");
                return false;
            }
        }

        return true;
    }

    bool KTNTracksNPointsNUPCut::Apply( Nymph::KTData& data, KTMultiTrackEventData& eventData )
    {        
        bool isCut = false;
        unsigned nTracksIndex = std::min(eventData.GetTotalEventSequences(), (unsigned)fThresholds.size() - 1);
        unsigned ftNPointsIndex = std::min(eventData.GetFirstTrackNTrackBins(), (int)fThresholds[nTracksIndex].size() - 1);

        KTDEBUG(ecnuplog, "Applying n-tracks/n-points/nup cut; (" << eventData.GetTotalEventSequences() << ", " << eventData.GetFirstTrackNTrackBins() << ")");
        KTDEBUG(ecnuplog, "Using indices: (" << nTracksIndex << ", " << ftNPointsIndex << ")");
        KTDEBUG(ecnuplog, "Cut thresholds: " << fThresholds[nTracksIndex][ftNPointsIndex].fMinTotalNUP << ", " << fThresholds[nTracksIndex][ftNPointsIndex].fMinAverageNUP << ", " << fThresholds[nTracksIndex][ftNPointsIndex].fMinMaxNUP)

        double ftTotalNUP = eventData.GetFirstTrackTotalNUP();
        if (fWideOrNarrow == WideOrNarrow::wide)
        {
            ftTotalNUP = eventData.GetFirstTrackTotalWideNUP();
        }

        if( ftTotalNUP < fThresholds[nTracksIndex][ftNPointsIndex].fMinTotalNUP )
        {
            KTDEBUG(ecnuplog, "Event is cut based on total NUP: " << ftTotalNUP << " < " << fThresholds[nTracksIndex][ftNPointsIndex].fMinTotalNUP);
            isCut = true;
        }
        else
        {
            double divisor = eventData.GetFirstTrackTimeLength();
            if (fTimeOrBinAverage == TimeOrBinAvg::bin)
            {
                divisor = (double)eventData.GetFirstTrackNTrackBins();
            }

            if( ftTotalNUP / divisor < fThresholds[nTracksIndex][ftNPointsIndex].fMinAverageNUP )
            {
                KTDEBUG(ecnuplog, "Event is cut based on average NUP: " << ftTotalNUP / divisor << " < " <<fThresholds[nTracksIndex][ftNPointsIndex].fMinAverageNUP);
                isCut = true;
            }
            else
            {
                if( eventData.GetFirstTrackMaxNUP() < fThresholds[nTracksIndex][ftNPointsIndex].fMinMaxNUP )
                {
                    KTDEBUG(ecnuplog, "Event is cut based on max NUP: " << eventData.GetFirstTrackMaxNUP() << " < " << fThresholds[nTracksIndex][ftNPointsIndex].fMinMaxNUP);
                    isCut = true;
                }
            }
        }

        data.GetCutStatus().AddCutResult< KTNTracksNPointsNUPCut::Result >(isCut);

        return isCut;
    }
    
    

} // namespace Katydid
