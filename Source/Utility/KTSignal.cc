/*
 * KTSignalWrapper.cc
 *
 *  Created on: Aug 8, 2012
 *      Author: nsoblath
 */

#include "KTSignal.hh"

namespace Katydid
{
    SignalException::SignalException (std::string const& why)
      : std::logic_error(why)
    {}

    KTSignalWrapper::KTSignalWrapper() :
            fSignalWrapper(NULL)
    {
    }

    KTSignalWrapper::~KTSignalWrapper()
    {
        delete fSignalWrapper;
    }

} /* namespace Katydid */
