/*
 * KTTIFactory.hh
 *
 *  Created on: Jan 2, 2013
 *      Author: nsoblath
 *
 *      Type-Indexed Factory and Registrars
 */

#ifndef KTTIFACTORY_HH_
#define KTTIFACTORY_HH_

#include "KTSingleton.hh"
#include "KTLogger.hh"

#include <map>
#include <typeinfo>

namespace Katydid
{
    KTLOGGER(utillog_ti_factory, "katydid.utility");

    template< class XBaseType >
    class KTTIFactory;

    template< class XBaseType >
    class KTTIRegistrar
    {
        public:
            KTTIRegistrar() {}
            virtual ~KTTIRegistrar() {}

        public:
            friend class KTTIFactory< XBaseType >;

        protected:
            virtual XBaseType* Create() const = 0;

    };

    template< class XBaseType, class XDerivedType >
    class KTDerivedTIRegistrar : public KTTIRegistrar< XBaseType >
    {
        public:
            KTDerivedTIRegistrar();
            virtual ~KTDerivedTIRegistrar();

        protected:
            void Register() const;

            XBaseType* Create() const;

    };


    template< class XBaseType >
    class KTTIFactory : public KTSingleton< KTTIFactory< XBaseType > >
    {
        public:
            struct CompareTypeInfo
            {
                bool operator() (const std::type_info* lhs, const std::type_info* rhs)
                {
                    return lhs->before(*rhs);
                }
            };

        public:
            typedef std::map< const std::type_info*, const KTTIRegistrar< XBaseType >* > FactoryMap;
            typedef typename FactoryMap::value_type FactoryEntry;
            typedef typename FactoryMap::iterator FactoryIt;
            typedef typename FactoryMap::const_iterator FactoryCIt;

        public:
            template< class XDerivedType >
            XBaseType* Create();

            XBaseType* Create(const FactoryCIt& iter);

            template< class XDerivedType >
            void Register(const KTTIRegistrar< XBaseType >* registrar);

            FactoryCIt GetFactoryMapBegin() const;
            FactoryCIt GetFactoryMapEnd() const;

        protected:
            FactoryMap* fMap;


        protected:
            friend class KTSingleton< KTTIFactory >;
            friend class KTDestroyer< KTTIFactory >;
            KTTIFactory();
            ~KTTIFactory();
    };

    template< class XBaseType >
    template< class XDerivedType >
    XBaseType* KTTIFactory< XBaseType >::Create()
    {
        FactoryCIt it = fMap->find(&typeid(XDerivedType));
        if (it == fMap->end())
        {
            KTERROR(utillog_ti_factory, "Did not find factory with type <" << typeid(XDerivedType).name() << ">.");
            return NULL;
        }

        return it->second->Create();
    }

    template< class XBaseType >
    XBaseType* KTTIFactory< XBaseType >::Create(const FactoryCIt& iter)
    {
        return iter->second->Create();
    }

    template< class XBaseType >
    template< class XDerivedType >
    void KTTIFactory< XBaseType >::Register(const KTTIRegistrar< XBaseType >* registrar)
    {
        FactoryCIt it = fMap->find(&typeid(XDerivedType));
        if (it != fMap->end())
        {
            KTERROR(utillog_ti_factory, "Already have factory registered for type <" << typeid(XDerivedType).name() << ">.");
            return;
        }
        fMap->insert(std::pair< const std::type_info*, const KTTIRegistrar< XBaseType >* >(&typeid(XDerivedType), registrar));
        KTDEBUG(utillog_ti_factory, "Registered a factory for class type " << typeid(XDerivedType).name() << ", factory #" << fMap->size()-1);
    }

    template< class XBaseType >
    KTTIFactory< XBaseType >::KTTIFactory() :
        fMap(new FactoryMap())
    {}

    template< class XBaseType >
    KTTIFactory< XBaseType >::~KTTIFactory()
    {
        delete fMap;
    }

    template< class XBaseType >
    typename KTTIFactory< XBaseType >::FactoryCIt KTTIFactory< XBaseType >::GetFactoryMapBegin() const
    {
        return fMap->begin();
    }

    template< class XBaseType >
    typename KTTIFactory< XBaseType >::FactoryCIt KTTIFactory< XBaseType >::GetFactoryMapEnd() const
    {
        return fMap->end();
    }




    template< class XBaseType, class XDerivedType >
    KTDerivedTIRegistrar< XBaseType, XDerivedType >::KTDerivedTIRegistrar() :
            KTTIRegistrar< XBaseType >()
    {
        Register();
    }

    template< class XBaseType, class XDerivedType >
    KTDerivedTIRegistrar< XBaseType, XDerivedType >::~KTDerivedTIRegistrar()
    {}

    template< class XBaseType, class XDerivedType >
    void KTDerivedTIRegistrar< XBaseType, XDerivedType >::Register() const
    {
        KTTIFactory< XBaseType >::GetInstance()->template Register<XDerivedType>(this);
        return;
    }

    template< class XBaseType, class XDerivedType >
    XBaseType* KTDerivedTIRegistrar< XBaseType, XDerivedType >::Create() const
    {
        return dynamic_cast< XBaseType* >(new XDerivedType());
    }


} /* namespace Katydid */
#endif /* KTTIFACTORY_HH_ */