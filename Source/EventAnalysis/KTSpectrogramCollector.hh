/*
 * KTSpectrogramCollector.hh
 *
 *  Created on: Oct 9, 2015
 *      Author: ezayas
 */

#ifndef KTSPECTROGRAMCOLLECTOR_HH_
#define KTSPECTROGRAMCOLLECTOR_HH_

#include "KTProcessor.hh"

#include "KTSlot.hh"
#include "KTLogger.hh"

#include "KTSpectrumCollectionData.hh"

#include <set>


namespace Katydid
{
    
    KTLOGGER(avlog_hh, "KTDataAccumulator.hh");

    class KTPowerSpectrumData;
    class KTProcessedTrackData;
    class KTSliceHeader;

    /*
     @class KTSpectrogramCollector
     @author E. Zayas

     @brief Collects the spectra that pertain to a particular track or event

     @details
     Supports an arbitrary number of tracks to collect simultaneously. Collection begins when a spectrum is received which matches the timestamp
     of the beginning of a track. A signal is emitted when the spectrum matches the end time.

     Configuration name: "spectrogram-collector"

     Available configuration values:
     - "min-frequency": double -- minimum frequency
     - "max-frequency": double -- maximum frequency
     - "min-bin": unsigned -- minimum frequency by bin
     - "max-bin": unsigned -- maximum frequency by bin
     - "lead-time": double -- time to collect before the beginning of the track
     - "trail-time": double -- time to collect after the end of the track

     Slots:
     - "track": void (Nymph::KTDataPtr) -- Adds a track to the list of active spectrogram collections; Requires KTProcessedTrackData; Adds nothing
     - "ps": void (Nymph::KTDataPtr) -- Adds a power spectrum to the appropriate spectrogram(s), if any; Requires KTPowerSpectrumData and KTSliceHeader; Adds nothing

     Signals:
     - "waterfall": void (Nymph::KTDataPtr) -- Emitted upon completion of a spectrogram (waterfall plot); Guarantees KTPSCollectionData
    */

    class KTSpectrogramCollector : public Nymph::KTProcessor
    {
        public:
            KTSpectrogramCollector(const std::string& name = "spectrogram-collector");
            virtual ~KTSpectrogramCollector();

            bool Configure(const scarab::param_node* node);

            double GetMinFrequency() const;
            void SetMinFrequency(double freq);

            double GetMaxFrequency() const;
            void SetMaxFrequency(double freq);

            unsigned GetMinBin() const;
            void SetMinBin(unsigned bin);

            unsigned GetMaxBin() const;
            void SetMaxBin(unsigned bin);

            double GetLeadTime() const;
            void SetLeadTime(double t);

            double GetTrailTime() const;
            void SetTrailTime(double t);

        private:
            double fMinFrequency;
            double fMaxFrequency;
            unsigned fMinBin;
            unsigned fMaxBin;
            bool fCalculateMinBin;
            bool fCalculateMaxBin;
            double fLeadTime;
            double fTrailTime;

        public:
            bool AddTrack(KTProcessedTrackData& trackData, unsigned component);
            bool ConsiderSpectrum(KTPowerSpectrum& ps, KTSliceHeader& slice, unsigned component, bool forceEmit);
            bool ReceiveTrack(KTProcessedTrackData& data);
            bool ReceiveSpectrum(KTPowerSpectrumData& data, KTSliceHeader& sliceData, bool forceEmit);
            void FinishSC( Nymph::KTDataPtr data );

            struct KTTrackCompare
            {
                bool operator() (const std::pair< Nymph::KTDataPtr, KTPSCollectionData* > lhs, const std::pair< Nymph::KTDataPtr, KTPSCollectionData* > rhs) const
                {
                    return lhs.second->GetStartTime() < rhs.second->GetStartTime();
                }
            };

            // The spectrograms are stored in a vector of sets of pairs of Nymph::KTDataPtr and KTPSCollectionData. The levels to this hierarchy are:
            //      Vector - each element corresponds to a component
            //      Set    - each element corresponds to a track
            //      Pair   - the Nymph::KTDataPtr which contains the spectrogram, and a pointer to the spectrogram
            // It is necessary to store the Nymph::KTDataPtr because the signal must contain this object when it emits, and each spectrogram must have a
            // unique associated Nymph::KTDataPtr

            std::vector< std::set< std::pair< Nymph::KTDataPtr, KTPSCollectionData* >, KTTrackCompare > > fWaterfallSets;

        private:

            //***************
            // Signals
            //***************

        private:
            Nymph::KTSignalData fWaterfallSignal;

            //***************
            // Slots
            //***************

        private:
            Nymph::KTSlotDataOneType< KTProcessedTrackData > fTrackSlot;
            void SlotFunctionPSData( Nymph::KTDataPtr data );

    };

    inline double KTSpectrogramCollector::GetMinFrequency() const
    {
        return fMinFrequency;
    }

    inline void KTSpectrogramCollector::SetMinFrequency(double freq)
    {
        fMinFrequency = freq;
        fCalculateMinBin = true;
        return;
    }

    inline double KTSpectrogramCollector::GetMaxFrequency() const
    {
        return fMaxFrequency;
    }

    inline void KTSpectrogramCollector::SetMaxFrequency(double freq)
    {
        fMaxFrequency = freq;
        fCalculateMaxBin = true;
        return;
    }

    inline unsigned KTSpectrogramCollector::GetMinBin() const
    {
        return fMinBin;
    }

    inline void KTSpectrogramCollector::SetMinBin(unsigned bin)
    {
        fMinBin = bin;
        fCalculateMinBin = false;
        return;
    }

    inline unsigned KTSpectrogramCollector::GetMaxBin() const
    {
        return fMaxBin;
    }

    inline void KTSpectrogramCollector::SetMaxBin(unsigned bin)
    {
        fMaxBin = bin;
        fCalculateMaxBin = false;
        return;
    }

    inline double KTSpectrogramCollector::GetLeadTime() const
    {
        return fLeadTime;
    }

    inline void KTSpectrogramCollector::SetLeadTime(double t)
    {
        fLeadTime = t;
        return;
    }

    inline double KTSpectrogramCollector::GetTrailTime() const
    {
        return fTrailTime;
    }

    inline void KTSpectrogramCollector::SetTrailTime(double t)
    {
        fTrailTime = t;
        return;
    }

    void KTSpectrogramCollector::SlotFunctionPSData( Nymph::KTDataPtr data )
    {
        // Standard data slot pattern:
        // Check to ensure that the required data types are present
        if (! data->Has< KTPowerSpectrumData >())
        {
            KTERROR(avlog_hh, "Data not found with type < KTPowerSpectrumData >!");
            return;
        }
        if (! data->Has< KTSliceHeader >())
        {
            KTERROR(avlog_hh, "Data not found with type < KTSliceHeader >!");
            return;
        }

        // If the slice is the last, set a flag to force a signal emit
        bool force = data->GetLastData();
        if (force)
        {
            KTDEBUG(avlog_hh, "Reached last-data, forcing emit");
        }

        // Call the function
        if (! ReceiveSpectrum(data->Of< KTPowerSpectrumData >(), data->Of< KTSliceHeader >(), force))
        {
            KTERROR(avlog_hh, "Something went wrong while analyzing data with type < KTPSCollectionData >");
            return;
        }
    
        return;
    }
}

#endif /* KTTRACKSPECTROGRAMCOLLECTOR_HH_ */