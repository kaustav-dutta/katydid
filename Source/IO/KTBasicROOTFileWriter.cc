/*
 * KTBasicROOTFileWriter.cc
 *
 *  Created on: Aug 24, 2012
 *      Author: nsoblath
 */

#include "KTBasicROOTFileWriter.hh"

#include "KTNOFactory.hh"
#include "KTLogger.hh"
#include "KTParam.hh"

using std::string;

namespace Katydid
{
    KTLOGGER(publog, "KTBasicROOTFileWriter");


    static KTNORegistrar< KTWriter, KTBasicROOTFileWriter > sBRFWriterRegistrar("basic-root-writer");
    static KTNORegistrar< KTProcessor, KTBasicROOTFileWriter > sBRFWProcRegistrar("basic-root-writer");

    KTBasicROOTFileWriter::KTBasicROOTFileWriter(const std::string& name) :
            KTWriterWithTypists< KTBasicROOTFileWriter >(name),
            fFilename("basic_output.root"),
            fFileFlag("recreate"),
            fFile(NULL)
    {
    }

    KTBasicROOTFileWriter::~KTBasicROOTFileWriter()
    {
        CloseFile();
    }

    bool KTBasicROOTFileWriter::Configure(const KTParamNode* node)
    {
        // Config-file settings
        if (node != NULL)
        {
            SetFilename(node->GetValue< string >("output-file", fFilename));
            SetFileFlag(node->GetValue< string >("file-flag", fFileFlag));
        }

        return true;
    }

    bool KTBasicROOTFileWriter::OpenAndVerifyFile()
    {
        if (fFile == NULL)
        {
            fFile = new TFile(fFilename.c_str(), fFileFlag.c_str());
        }
        if (! fFile->IsOpen())
        {
            delete fFile;
            fFile = NULL;
            KTERROR(publog, "Output file <" << fFilename << "> did not open!");
            return false;
        }
        fFile->cd();
        return true;
    }

} /* namespace Katydid */
