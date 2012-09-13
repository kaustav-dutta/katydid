/*
 * KTTestProcessor.cc
 *
 *  Created on: Aug 15, 2012
 *      Author: nsoblath
 */

#include "KTTestProcessor.hh"

#include "KTLogger.hh"

namespace Katydid
{
    KTLOGGER(testsiglog, "katydid.verification.testsig")

    KTTestProcessorA::KTTestProcessorA() :
            fTheSignal()
    {
        RegisterSignal("the_signal", &fTheSignal, "void (int)");
    }

    KTTestProcessorA::~KTTestProcessorA()
    {
    }

    void KTTestProcessorA::EmitSignals(int value)
    {
        fTheSignal(value);
        return;
    }




    KTTestProcessorB::KTTestProcessorB()
    {
        RegisterSlot("first_slot", this, &KTTestProcessorB::Slot1, "void (int)");
        RegisterSlot("second_slot", this, &KTTestProcessorB::Slot2, "void (int)");
    }

    KTTestProcessorB::~KTTestProcessorB()
    {
    }

    void KTTestProcessorB::Slot1(int input)
    {
        KTINFO(testsiglog, "Slot1: input is " << input);
        return;
    }

    void KTTestProcessorB::Slot2(int input)
    {
        KTINFO(testsiglog, "Slot2: twice input is " << 2*input);
        return;
    }



} /* namespace Katydid */