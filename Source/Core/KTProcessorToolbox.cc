/*
 * KTProcessorToolbox.cc
 *
 *  Created on: Sep 27, 2012
 *      Author: nsoblath
 */

#include "KTProcessorToolbox.hh"

#include "KTLogger.hh"
#include "KTPrimaryProcessor.hh"
#include "KTPStoreNode.hh"

#ifndef SINGLETHREADED
#include <boost/thread.hpp>
#endif

#include <vector>

using std::deque;
using std::set;
using std::string;
using std::vector;

namespace Katydid
{
    KTLOGGER(proclog, "KTProcessorToolbox");

    KTProcessorToolbox::KTProcessorToolbox(const std::string& name) :
            KTConfigurable(name),
            fProcFactory(KTNOFactory< KTProcessor >::GetInstance()),
            fRunQueue(),
            fProcMap()
    {
    }

    KTProcessorToolbox::~KTProcessorToolbox()
    {
        ClearProcessors();
    }

    bool KTProcessorToolbox::Configure(const KTPStoreNode* node)
    {
        KTPROG(proclog, "Configuring . . .");
        // Deal with "processor" blocks first
        KTPStoreNode::csi_pair itPair = node->EqualRange("processor");
        for (KTPStoreNode::const_sorted_iterator it = itPair.first; it != itPair.second; it++)
        {
            KTPStoreNode subNode = KTPStoreNode(&(it->second));
            if (! subNode.HasData("type"))
            {
                KTERROR(proclog, "Unable to create processor: no processor type given");
                return false;
            }
            string procType = subNode.GetData("type");

            string procName;
            if (! subNode.HasData("name"))
            {
                KTINFO(proclog, "No name given for processor of type <" << procType << ">; using type as name.");
                procName = procType;
            }
            else
            {
                procName = subNode.GetData("name");
            }
            KTProcessor* newProc = fProcFactory->CreateNamed(procType);
            if (newProc == NULL)
            {
                KTERROR(proclog, "Unable to create processor of type <" << procType << ">");
                return false;
            }

            bool isTopLevel = false;
            if (subNode.HasData("is-top-level"))
            {
                isTopLevel = subNode.GetData< bool >("is-top-level");
            }

            if (! AddProcessor(procName, newProc))
            {
                KTERROR(proclog, "Unable to add processor <" << procName << ">");
                delete newProc;
                return false;
            }
        }

        // Then deal with "connection blocks"
        itPair = node->EqualRange("connection");
        for (KTPStoreNode::const_sorted_iterator it = itPair.first; it != itPair.second; it++)
        {
            KTPStoreNode subNode = KTPStoreNode(&(it->second));
            if (! subNode.HasData("signal-processor") || ! subNode.HasData("signal-name") ||
                    ! subNode.HasData("slot-processor") || ! subNode.HasData("slot-name"))
            {
                KTERROR(proclog, "Signal/Slot connection information is incomplete!");
                if (subNode.HasData("signal-processor"))
                {
                    KTWARN(proclog, "signal-processor = " << subNode.GetData<string>("signal-processor"));
                }
                else
                {
                    KTERROR(proclog, "signal-processor = MISSING");
                }
                if (subNode.HasData("signal-name"))
                {
                    KTWARN(proclog, "signal-name = " << subNode.GetData<string>("signal-name"));
                }
                else
                {
                    KTERROR(proclog, "signal-name = MISSING");
                }
                if (subNode.HasData("slot-processor"))
                {
                    KTWARN(proclog, "slot-processor = " << subNode.GetData<string>("slot-processor"));
                }
                else
                {
                    KTERROR(proclog, "slot-processor = MISSING");
                }
                if (subNode.HasData("slot-name"))
                {
                    KTWARN(proclog, "slot-name = " << subNode.GetData<string>("slot-name"));
                }
                else
                {
                    KTERROR(proclog, "slot-namer = MISSING");
                }
                return false;
            }

            KTProcessor* signalProc = GetProcessor(subNode.GetData("signal-processor"));
            KTProcessor* slotProc = GetProcessor(subNode.GetData("slot-processor"));

            if (signalProc == NULL)
            {
                KTERROR(proclog, "Processor named <" << subNode.GetData("signal-processor") << "> was not found!");
                return false;
            }
            if (slotProc == NULL)
            {
                KTERROR(proclog, "Processor named <" << subNode.GetData("slot-processor") << "> was not found!");
                return false;
            }

            string signalName = subNode.GetData("signal-name");
            string slotName = subNode.GetData("slot-name");

            bool useGroupOrdering = false;
            int groupOrder = 0;
            if (subNode.HasData("group-order"))
            {
                useGroupOrdering = true;
                groupOrder = subNode.GetData< int >("group-order");
            }

            try
            {
                if (useGroupOrdering)
                {
                    signalProc->ConnectASlot(signalName, slotProc, slotName, groupOrder);
                }
                else
                {
                    signalProc->ConnectASlot(signalName, slotProc, slotName);
                }
            }
            catch (std::exception& e)
            {
                KTERROR(proclog, "An error occurred while connecting signals and slots:\n"
                        << "\tSignal " << signalName << " from processor " << subNode.GetData("signal-processor") << " (a.k.a. " << signalProc->GetConfigName() << ")" << '\n'
                        << "\tSlot " << slotName << " from processor " << subNode.GetData("slot-processor") << " (a.k.a. " << slotProc->GetConfigName() << ")" << '\n'
                        << '\t' << e.what());
                return false;
            }
            KTINFO(proclog, "Signal <" << subNode.GetData("signal-processor") << ":" << signalName << "> connected to slot <" << subNode.GetData("slot-processor") << ":" << signalName << ">");
        }

        // Finally, deal with processor-run specifications
        // In the current implementation there is only one ThreadGroup, and all processors specified will be run in separate threads in parallel.
        // In single threaded mode all threads will be run sequentially in the order they were specified.
        const KTPStoreNode subNode = node->GetChild("run-queue");
        if (subNode.IsValid())
        {
            ThreadGroup threadGroup;
            for (KTPStoreNode::const_iterator iter = subNode.Begin(); iter != subNode.End(); iter++)
            {
                KTPStoreNode subSubNode = KTPStoreNode(&(iter->second));
                string procName = subSubNode.GetValue< string >();
                KTProcessor* procForRunQueue = GetProcessor(procName);
                KTDEBUG(proclog, "Adding processor of type " << procName << " to the run queue");
                if (procForRunQueue == NULL)
                {
                    KTERROR(proclog, "Unable to find processor <" << procName << "> requested for the run queue");
                    return false;
                }

                KTPrimaryProcessor* primaryProc = dynamic_cast< KTPrimaryProcessor* >(procForRunQueue);
                if (primaryProc == NULL)
                {
                    KTERROR(proclog, "Processor <" << procName << "> is not a primary processor.");
                    return false;
                }

                threadGroup.insert(primaryProc);
            }
            fRunQueue.push_back(threadGroup);
        }
        else
        {
            KTWARN(proclog, "No run queue was specified during configuration.");
        }

        return true;
    }

    bool KTProcessorToolbox::ConfigureProcessors(const KTPStoreNode* node)
    {
        for (ProcMapIt iter = fProcMap.begin(); iter != fProcMap.end(); iter++)
        {
            KTDEBUG(proclog, "Attempting to configure processor <" << iter->first << ">");
            string procName = iter->first;
            string nameUsed;
            const KTPStoreNode subNode = node->GetChild(procName);
            if (! subNode.IsValid())
            {
                string configName = iter->second.fProc->GetConfigName();
                KTWARN(proclog, "Did not find a PSToreNode <" << procName << ">\n"
                        "\tWill check using the generic name of the processor, <" << configName << ">.");
                const KTPStoreNode subNode2 = node->GetChild(configName);
                if (! subNode2.IsValid())
                {
                    KTWARN(proclog, "Did not find a PStoreNode <" << configName << ">\n"
                            "\tProcessor <" << iter->first << "> was not configured.");
                    continue;
                }
                nameUsed = configName;
            }
            else
            {
                nameUsed = procName;
            }
            if (! iter->second.fProc->Configure(&subNode))
            {
                KTERROR(proclog, "An error occurred while configuring processor <" << iter->first << "> with PStoreNode <" << nameUsed << ">");
                return false;
            }
        }
        return true;
    }

    bool KTProcessorToolbox::Run()
    {
        KTPROG(proclog, "Beginning processing . . .");
#ifndef SINGLETHREADED
        unsigned iGroup = 0;
#endif
        for (RunQueue::const_iterator rqIter = fRunQueue.begin(); rqIter != fRunQueue.end(); rqIter++)
        {
#ifdef SINGLETHREADED
            for (ThreadGroup::const_iterator tgIter = rqIter->begin(); tgIter != rqIter->end(); tgIter++)
            {
                if (! (*tgIter)->Run())
                {
                    return false;
                }
            }
#else
            KTDEBUG(proclog, "Starting thread group " << iGroup);
            boost::thread_group parallelThreads;
            unsigned iThread = 0;
            for (ThreadGroup::const_iterator tgIter = rqIter->begin(); tgIter != rqIter->end(); tgIter++)
            {
                // create a boost::thread object to launch the thread
                // use boost::ref to avoid copying the processor
                KTDEBUG(proclog, "Starting thread " << iThread);
                parallelThreads.create_thread(boost::ref(**tgIter));
                iThread++;
            }
            // wait for execution to complete
            parallelThreads.join_all();
            iGroup++;
#endif
        }
        KTPROG(proclog, ". . . processing complete.");
        return true;
    }

    KTProcessor* KTProcessorToolbox::GetProcessor(const std::string& procName)
    {
        ProcMapIt it = fProcMap.find(procName);
        if (it == fProcMap.end())
        {
            KTWARN(proclog, "Processor <" << procName << "> was not found.");
            return NULL;
        }
        return it->second.fProc;
    }

    const KTProcessor* KTProcessorToolbox::GetProcessor(const std::string& procName) const
    {
        ProcMapCIt it = fProcMap.find(procName);
        if (it == fProcMap.end())
        {
            KTWARN(proclog, "Processor <" << procName << "> was not found.");
            return NULL;
        }
        return it->second.fProc;
    }

    bool KTProcessorToolbox::AddProcessor(const std::string& procName, KTProcessor* proc)
    {
        ProcMapIt it = fProcMap.find(procName);
        if (it == fProcMap.end())
        {
            ProcessorInfo pInfo;
            pInfo.fProc = proc;
            fProcMap.insert(ProcMapValue(procName, pInfo));
            KTDEBUG(proclog, "Added processor <" << procName << "> (a.k.a. " << proc->GetConfigName() << ")");
            return true;
        }
        KTWARN(proclog, "Processor <" << procName << "> already exists; new processor was not added.");
        return false;
    }

    bool KTProcessorToolbox::RemoveProcessor(const std::string& procName)
    {
        KTProcessor* procToRemove = ReleaseProcessor(procName);
        if (procToRemove == NULL)
        {
            return false;
        }
        delete procToRemove;
        KTDEBUG(proclog, "Processor <" << procName << "> deleted.");
        return true;
    }

    KTProcessor* KTProcessorToolbox::ReleaseProcessor(const std::string& procName)
    {
        ProcMapIt it = fProcMap.find(procName);
        if (it == fProcMap.end())
        {
            KTWARN(proclog, "Processor <" << procName << "> was not found.");
            return NULL;
        }
        KTProcessor* procToRelease = it->second.fProc;
        fProcMap.erase(it);
        return procToRelease;
    }

    void KTProcessorToolbox::ClearProcessors()
    {
        for (ProcMapIt it = fProcMap.begin(); it != fProcMap.end(); it++)
        {
            delete it->second.fProc;
        }
        fProcMap.clear();
        return;
    }

} /* namespace Katydid */
