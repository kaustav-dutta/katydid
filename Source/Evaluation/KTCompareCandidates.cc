/*
 * KTCompareCandidates.cc
 *
 *  Created on: Apr 9, 2013
 *      Author: nsoblath
 */

#include "KTCompareCandidates.hh"

#include "KTCCResults.hh"
#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTPStoreNode.hh"

#include <sstream>
#include <vector>

using boost::shared_ptr;

using std::string;
using std::vector;

namespace Katydid
{
    static KTDerivedNORegistrar< KTProcessor, KTCompareCandidates > sCompCandRegistrar("compare-candidates");

    KTLOGGER(cclog, "katydid.evaluate");

    KTCompareCandidates::KTCompareCandidates(const string& name) :
            KTProcessor(name),
            fAssumeSparseCandidates(false),
            fTruthAndAnalysisSlot("truth-vs-analysis", this, &KTCompareCandidates::CompareTruthAndAnalysis),
            fCCResultsSignal("cc-results", this)
    {
    }

    KTCompareCandidates::~KTCompareCandidates()
    {
    }

    Bool_t KTCompareCandidates::Configure(const KTPStoreNode* node)
    {
        if (node == NULL) return false;

        SetAssumeSparseCandidates(node->GetData<Bool_t>("assume-sparse-candidates", fAssumeSparseCandidates));

        return true;
    }

    Bool_t KTCompareCandidates::CompareTruthAndAnalysis(KTMCTruthEvents& mcEventData, KTAnalysisCandidates& candidateData)
    {
        const KTMCTruthEvents::EventSet& events = mcEventData.GetEvents();
        const KTAnalysisCandidates::CandidateSet& candidates = candidateData.GetCandidates();

        const UInt_t eventRecordSize = mcEventData.GetRecordSize();
        const UInt_t candidateRecordSize = candidateData.GetRecordSize();

        const UInt_t nRecords = mcEventData.GetNRecords();
        if (nRecords != candidateData.GetNRecords())
        {
            KTERROR(cclog, "The number of records simulated (" << nRecords << ") does not match the number of records analyzed (" << candidateData.GetNRecords() << ")");
            return false;
        }

        // eventMatches: each position in the vector represents one of the events; this vector records how many candidates matched each event.
        vector< UInt_t > eventMatches(events.size());
        // candidateMatches: each position in the vector represents one of the candidates; this vector records how many events matched each candidate.
        vector< UInt_t > candidateMatches(candidates.size());

        //ULong64_t candidateSampleSum = 0;
        //ULong64_t eventSampleSum = 0;
        //ULong64_t candidateCorrectSampleSum = 0;

        UInt_t eventCounter = 0;
        Bool_t continueEventLoop = true;
        UInt_t candidateCounter = 0;
        KTAnalysisCandidates::CandidateSet::const_iterator candStartHere = candidates.begin();
        UInt_t candidateCounterStart = 0;
        Int_t comparison = 0;
        for (KTMCTruthEvents::EventSet::const_iterator truthIt = events.begin(); truthIt != events.end() && continueEventLoop; truthIt++)
        {
            candidateCounter = candidateCounterStart;
            for (KTAnalysisCandidates::CandidateSet::const_iterator candIt = candStartHere; candIt != candidates.end(); candIt++)
            {
                comparison = CompareAnEventToACandidate(*truthIt, *candIt, eventRecordSize, candidateRecordSize);
                if (comparison == 0)
                {
                    eventMatches[eventCounter] = eventMatches[eventCounter] + 1;
                    candidateMatches[candidateCounter] = candidateMatches[candidateCounter] + 1;
                    KTDEBUG(cclog, "Match found:\n" <<
                            "\tEvent " << eventCounter << "): " << truthIt->fStartRecord << ", " << truthIt->fStartSample << ", " << truthIt->fEndRecord << ", " << truthIt->fEndSample << '\n' <<
                            "\tCandidate " << candidateCounter << "): " << candIt->fStartRecord << ", " << candIt->fStartSample << ", " << candIt->fEndRecord << ", " << candIt->fEndSample << '\n');
                }
                else if (comparison < 0)
                {
                    KTDEBUG(cclog, "Candidate before event:\n" <<
                            "\tEvent: " << truthIt->fStartRecord << ", " << truthIt->fStartSample << ", " << truthIt->fEndRecord << ", " << truthIt->fEndSample << '\n' <<
                            "\tCandidate: " << candIt->fStartRecord << ", " << candIt->fStartSample << ", " << candIt->fEndRecord << ", " << candIt->fEndSample << '\n');
                    if (fAssumeSparseCandidates)
                    {
                        // this candidate is earlier than any remaining events, so it can be skipped for following events
                        candStartHere = candIt;
                        candidateCounterStart = candidateCounter;
                        // move up to the next event as the place to start
                        candStartHere++;
                        candidateCounterStart++;
                        // if this is the last candidate, than there's no use in doing any more comparisons to events
                        if (candStartHere == candidates.end())
                        {
                            continueEventLoop = false;
                            // if we're here, the candidate loop will complete after this cycle
                        }
                    }
                }
                else // comparison > 0
                {
                    KTDEBUG(cclog, "Candidate after event:\n" <<
                            "\tEvent: " << truthIt->fStartRecord << ", " << truthIt->fStartSample << ", " << truthIt->fEndRecord << ", " << truthIt->fEndSample << '\n' <<
                            "\tCandidate: " << candIt->fStartRecord << ", " << candIt->fStartSample << ", " << candIt->fEndRecord << ", " << candIt->fEndSample << '\n');
                    // all remaining candidates are after the current truth event, so they can be skipped
                    break;
                }
                candidateCounter++;
            }
            eventCounter++;
        }

        shared_ptr< KTData > dataPtr(new KTData());
        dataPtr->fLastData = true;
        KTCCResults& ccrData = dataPtr->Of< KTCCResults >();

        ccrData.SetNEvents(events.size());
        ccrData.SetNCandidates(candidates.size());

        // iterate through eventMatches and candidateMatches to collect interesting statistics
        UInt_t largestNumberOfMatches = 0;
        UInt_t nEventsWithAtLeastOneCandidateMatch = 0;
        ccrData.ResizeNEventsWithXCandidateMatches(candidates.size()); // the largest size this should be is the number of candidates
        for (UInt_t iEvent = 0; iEvent < eventMatches.size(); iEvent++)
        {
            if (eventMatches[iEvent] > largestNumberOfMatches)
            {
                largestNumberOfMatches = eventMatches[iEvent];
            }
            if (eventMatches[iEvent] > 0)
            {
                nEventsWithAtLeastOneCandidateMatch++;
            }
            ccrData.IncrementNEventsWithXCandidateMatches(eventMatches[iEvent]);
            //nEventsWithCandidateMatches[eventMatches[iEvent]] = nEventsWithCandidateMatches[eventMatches[iEvent]] + 1;
        }
        //nEventsWithCandidateMatches.resize(largestNumberOfMatches + 1);
        ccrData.ResizeNEventsWithXCandidateMatches(largestNumberOfMatches + 1);

        KTPROG(cclog, "Number of events: " << ccrData.GetNEvents());
        KTPROG(cclog, "Largest number of candidates matching an event: " << largestNumberOfMatches);
        std::stringstream textHist1;
        const vector< UInt_t >& nEventsWithXCandidateMatches = ccrData.GetNEventsWithXCandidateMatches();
        for (UInt_t iNEvents = 0; iNEvents < nEventsWithXCandidateMatches.size(); iNEvents++)
        {
            textHist1 << iNEvents << ": " << nEventsWithXCandidateMatches[iNEvents] << '\n';
        }
        KTPROG(cclog, "Number of events (y axis) with a given number of candidate matches (x axis):\n" << textHist1.str());

        ccrData.SetEfficiency(Double_t(nEventsWithAtLeastOneCandidateMatch) / Double_t(events.size()));
        KTPROG(cclog, "Detection efficiency (# events with at least 1 match / # events): " << ccrData.GetEfficiency());


        largestNumberOfMatches = 0;
        ccrData.ResizeNCandidatesWithXEventMatches(events.size()); //  the largest size this should be is the number of events
        if (ccrData.GetNCandidatesWithXEventMatches().size() > 0)
        {
            for (UInt_t iCandidate = 0; iCandidate < candidateMatches.size(); iCandidate++)
            {
                if (candidateMatches[iCandidate] > largestNumberOfMatches)
                {
                    largestNumberOfMatches = candidateMatches[iCandidate];
                }
                ccrData.IncrementNCandidatesWithXEventMatches(candidateMatches[iCandidate]);
            }
            ccrData.ResizeNCandidatesWithXEventMatches(largestNumberOfMatches + 1);
        }
        KTPROG(cclog, "Number of candidates: " << ccrData.GetNCandidates());
        KTPROG(cclog, "Largest number of events matching a candidate: " << largestNumberOfMatches);
        std::stringstream textHist2;
        const vector< UInt_t >& nCandidatesWithXEventMatches = ccrData.GetNCandidatesWithXEventMatches();
        for (UInt_t iNCandidates = 0; iNCandidates < nCandidatesWithXEventMatches.size(); iNCandidates++)
        {
            textHist2 << iNCandidates << ": " << nCandidatesWithXEventMatches[iNCandidates] << '\n';
        }
        KTPROG(cclog, "Number of candidates (y axis) with a given number of event matches (x axis):\n" << textHist2.str());

        ccrData.SetFalseRate(1.e6 * Double_t(nCandidatesWithXEventMatches[0]) / (Double_t(nRecords) * Double_t(eventRecordSize)));
        KTPROG(cclog, "False rate (10^6 * # candidates not matching events / # of samples simulated): " << ccrData.GetFalseRate());

        fCCResultsSignal(dataPtr);

        return true;
    }

    Int_t KTCompareCandidates::CompareAnEventToACandidate(const KTMCTruthEvents::Event& event, const KTAnalysisCandidates::Candidate& candidate, UInt_t eventRecordSize, UInt_t candidateRecordSize) const
    {
        /* more readable version
        ULong64_t candidateSampleStart = candidate.fStartRecord * candidateRecordSize + candidate.fStartSample;
        ULong64_t eventSampleEnd = event.fEndRecord * eventRecordSize + event.fEndSample;
        if (candidateSampleStart > eventSampleEnd) return 1;

        ULong64_t candidateSampleEnd = candidate.fEndRecord * candidateRecordSize + candidate.fEndSample;
        ULong64_t eventSampleStart = event.fStartRecord * eventRecordSize + event.fStartSample;
        if (candidateSampleEnd < eventSampleStart) return -1;
        */

        if (candidate.fStartRecord * candidateRecordSize + candidate.fStartSample >
                event.fEndRecord * eventRecordSize + event.fEndSample)
            return 1;

        if (candidate.fEndRecord * candidateRecordSize + candidate.fEndSample <
                event.fStartRecord * eventRecordSize + event.fStartSample)
            return -1;

        return 0;
    }


} /* namespace Katydid */
