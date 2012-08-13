/*
 * KTRectangularWindow.cc
 *
 *  Created on: Sep 18, 2011
 *      Author: nsoblath
 */

#include "KTRectangularWindow.hh"

#include "TArrayD.h"
#include <cmath>

ClassImp(Katydid::KTRectangularWindow);

namespace Katydid
{

    KTRectangularWindow::KTRectangularWindow() :
            KTEventWindowFunction()
    {
    }

    KTRectangularWindow::KTRectangularWindow(const KTEvent* event) :
            KTEventWindowFunction(event)
    {
    }

    KTRectangularWindow::~KTRectangularWindow()
    {
    }

    Double_t KTRectangularWindow::GetWeight(Double_t time) const
    {
        if (fabs(time) <= fLength/2.) return 1.;
        return 0.;
    }

    Double_t KTRectangularWindow::GetWeight(UInt_t bin) const
    {
        if (bin < fSize) return 1.;
        return 0.;
    }

    void KTRectangularWindow::RebuildWindowFunction()
    {
        fWindowFunction.resize(fSize);
        for (Int_t iBin=0; iBin < fSize; iBin++)
        {
            fWindowFunction[iBin] = 1.;
        }
        return;
    }


} /* namespace Katydid */
