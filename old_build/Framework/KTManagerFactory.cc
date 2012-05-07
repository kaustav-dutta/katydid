// $Id$

/*
 Class: KTManagerFactory
 Author: N. Oblath

 For full class documentation: KTManagerFactory.hh

 Revision History
 Date         Name          Brief description
 -----------------------------------------------
 08/04/2010   N. Oblath     First version
 31/01/2011   N. Oblath     Made this the base class for KTManagerFactoryPrototype

 */

#include "KTManagerFactory.hh"
#include "KTManagerTable.hh"

#include "KTFrameworkMessage.hh"

namespace Katydid
{

    KTManagerFactory::KTManagerFactory( const string& aType ) :
        fInstance( NULL ),
        fTypeName( aType ),
        fStatus( eBad )
    {
        if( KTManagerTable::GetInstance()->RegisterManagerFactory( aType, this ) )
        {
#ifdef DEBUG_VERBOSE
            fwmsg < "KTManagerFactory::constructor";
            fwmsg(eDebug) << "Successfully registered a factory for manager <" << aType << ">" << eom;
#endif
            fStatus = eNoManager;
        }
    }

    KTManagerFactory::~KTManagerFactory()
    {
#ifdef DEBUG_VERBOSE
        fwmsg < "KTManagerFactory(" < fTypeName < ")::~KTManagerFactory";
        fwmsg(eDebug) << "Removing factory from the ManagerTable, and deleting the manager" << eom;
#endif
        if (fInstance != NULL) fInstance->Abort();
        KTManagerTable::GetInstance()->RemoveManagerFactory(fTypeName);
        delete fInstance;
        fInstance = NULL;
        fStatus = eNoManager;
    }

    KTManager* KTManagerFactory::GetManager()
    {
        if( fStatus == eBad )
        {
            fwmsg < "KTManagerFactory::GetManager";
            fwmsg( eWarning ) << "Status of manager factory <" << fTypeName << "> is bad!" << eom;
            return NULL;
        }
        if( fInstance == NULL)
        {
            this->CreateManager();
            fInstance->fManagerFactory = this;
            fStatus = eManagerExists;
#ifdef DEBUG_VERBOSE
            fwmsg < "KTManagerFactory::GetManager";
            fwmsg(eDebug) << "Creating manager <" << fTypeName << ">: " << fInstance << eom;
#endif
        }
        return fInstance;
    }

    const string& KTManagerFactory::GetTypeName() const
    {
        return fTypeName;
    }

    KTManagerFactory::FactoryStatus KTManagerFactory::GetStatus() const
    {
        return fStatus;
    }

} // end Katydid namespace