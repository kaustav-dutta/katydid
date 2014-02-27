/*
 * KTConfigurator.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "KTConfigurator.hh"

#include "mt_parser.hh"
#include "KTLogger.hh"

using std::string;

namespace Katydid
{
    KTLOGGER( conflog, "KTConfigurator" );

    KTConfigurator::KTConfigurator() :
            fMasterConfig( new KTParamNode() ),
            fParamBuffer( NULL ),
            fStringBuffer()
    {
        parser t_parser( an_argc, an_argv );
        //std::cout << "options parsed" << std::endl;
        //cout << t_parser );

        // first configuration: defaults
        if ( a_default != NULL )
        {
            fMasterConfig->Merge(a_default);
        }

        //std::cout << "first configuration complete" << std::endl;
        //cout << fMasterConfig );
        //cout << t_parser );

        string t_name_config("config");
        string t_name_json("json");

        // second configuration: config file
        if( t_parser.has( t_name_config ) )
        {
            string t_config_filename = t_parser.ValueAt( t_name_config )->Get();
            if( ! t_config_filename.empty() )
            {
                KTParamNode* t_config_from_file = KTParamInputJSON::ReadFile( t_config_filename );
                if( t_config_from_file == NULL )
                {
                    throw KTException() << "[KTConfigurator] error parsing config file";
                }
                fMasterConfig->Merge( t_config_from_file );
                delete t_config_from_file;
            }
        }

        //std::cout << "second configuration complete" << std::endl;
        //cout << fMasterConfig );
        //cout << t_parser );

        // third configuration: command line json
        if( t_parser.Has( t_name_json ) )
        {
            string t_config_json = t_parser.ValueAt( t_name_json )->Get();
            if( ! t_config_json.empty() )
            {
                KTParamNode* t_config_from_json = KTParamInputJSON::ReadString( t_config_json );
                fMasterConfig->Merge( t_config_from_json );
                delete t_config_from_json;
            }
        }

        //std::cout << "third configuration complete" << std::endl;
        //cout << fMasterConfig );
        //cout << t_parser );

        // fourth configuration: command line arguments
        t_parser.Erase( t_name_config );
        t_parser.Erase( t_name_json );

        //std::cout << "removed config and json from parsed options" << std::endl;
        //cout << t_parser );
        fMasterConfig->Merge( &t_parser );

        //std::cout << "fourth configuration complete" << std::endl;
        KTINFO( conflog, "final configuration:\n" << *fMasterConfig );
    }

    KTConfigurator::~KTConfigurator()
    {
        delete fMasterConfig;
    }

    void KTConfigurator::Merge(const KTParamNode& aNode)
    {
        fMasterConfig->Merge(aNode);
        return;
    }

    KTParamNode* KTConfigurator::Config()
    {
        return fMasterConfig;
    }

    const KTParamNode* KTConfigurator::Config() const
    {
        return fMasterConfig;
    }

} /* namespace Katydid */
