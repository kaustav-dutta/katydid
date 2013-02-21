/*
 * KTFrequencyCandidateIdentifier.hh
 *
 *  Created on: Dec 17, 2012
 *      Author: nsoblath
 */

#ifndef KTFREQUENCYCANDIDATEIDENTIFIER_HH_
#define KTFREQUENCYCANDIDATEIDENTIFIER_HH_

#include "KTProcessor.hh"

#include "KTCluster1DData.hh"
#include "KTFrequencyCandidateData.hh"

#include <boost/shared_ptr.hpp>


namespace Katydid
{
    class KTCorrelationData;
    class KTFrequencySpectrumPolar;
    class KTFrequencySpectrumDataPolar;
    class KTFrequencySpectrumDataFFTW;
    class KTFrequencySpectrumFFTW;

    class KTFrequencyCandidateIdentifier : public KTProcessor
    {
        protected:
            typedef KTSignal< void (boost::shared_ptr< KTData >) >::signal FCSignal;

        public:
            KTFrequencyCandidateIdentifier();
            virtual ~KTFrequencyCandidateIdentifier();

            Bool_t Configure(const KTPStoreNode* node);

        protected:


        public:
            Bool_t IdentifyCandidates(KTCluster1DData& clusterData, const KTFrequencySpectrumDataPolar& fsData);
            Bool_t IdentifyCandidates(KTCluster1DData& clusterData, const KTFrequencySpectrumDataFFTW& fsData);
            Bool_t IdentifyCandidates(KTCluster1DData& clusterData, const KTCorrelationData& fsData);

            KTFrequencyCandidateData::Candidates IdentifyCandidates(const KTCluster1DData::SetOfClusters& clusters, const KTFrequencySpectrumPolar* freqSpec);
            KTFrequencyCandidateData::Candidates IdentifyCandidates(const KTCluster1DData::SetOfClusters& clusters, const KTFrequencySpectrumFFTW* freqSpec);


            //***************
            // Signals
            //***************

        private:
            FCSignal fFCSignal;

            //***************
            // Slots
            //***************

        public:
            void ProcessClusterAndFSPolarData(boost::shared_ptr< KTData >);
            void ProcessClusterAndFSFFTWData(boost::shared_ptr< KTData >);
            void ProcessClusterAndCorrelationData(boost::shared_ptr< KTData >);

    };

} /* namespace Katydid */
#endif /* KTFREQUENCYCANDIDATEIDENTIFIER_HH_ */
