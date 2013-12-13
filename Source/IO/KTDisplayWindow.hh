/*
 * KTDisplayWindow.hh
 *
 *  Created on: Dec 13, 2013
 *      Author: nsoblath
 */

#ifndef KTDISPLAYWINDOW_HH_
#define KTDISPLAYWINDOW_HH_

#include "KTLogger.hh"

#include <TQObject.h>
#include <RQ_OBJECT.h>

class TApplication;
class TGMainFrame;
class TH1;
class TH2;
class TRootEmbeddedCanvas;

//#include <boost/thread.hpp>
#ifndef __CINT__
#include <pthread.h>
#else
struct pthread_cond_t;
struct pthread_mutex_t;
#endif


namespace Katydid
{
    KTLOGGER(ddlog, "katydid.io");

    class KTDisplayWindow
    {
        RQ_OBJECT("KTDataDisplay")

        public:
            KTDisplayWindow(UInt_t width = 200, UInt_t height = 200);
            virtual ~KTDisplayWindow();

            void Run();

            void Continue();

            void Draw(TH1* hist);
            void Draw(TH2* hist);

        private:
            void Wait();

            TApplication* fApp;
            TGMainFrame* fMain;
            TRootEmbeddedCanvas* fEcanvas;

            //boost::condition_variable fDisplayCondition;
            //boost::mutex fMutex;
            pthread_cond_t fDisplayCondition;
            pthread_mutex_t fMutex;

            ClassDef(KTDisplayWindow, 0);
    };

} /* namespace Katydid */
#endif /* KTDISPLAYWINDOW_HH_ */
