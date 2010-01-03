/*

Copyright (c) 2009, Jonathan Wayne Parrott

Please see the license.txt file included with this source
distribution for more information.

*/

#ifndef __PHRESOURCEMANAGER_H__
#define __PHRESOURCEMANAGER_H__

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "AbstractGarbageCollector.h"
#include "Timer.h"


namespace phoenix
{


    //forward decl of Resource
    class Resource;

    //! Garbage-Collecting Resource Manager.
    /*!
    	A class for automatic garbage collection. Any classes that management resources should
    	inherit from this class and their resources should inherit from Resource. This class
		can also be used directly for management of simple resources. The concept behind Phoenix's
		resource management is rather simple: Anything that is dynamically allocated and needs to 
		be used a lot should be kept track of, and that's where this class comes in. It also takes
		it further by keeping track of which objects should be deleted and safely deleting those object.
		This class is the implementation of the concepts of AbstractGarbageCollector and Droppable.
		Dropped objects are considered deleted and should be skipped during iteration of the resources.
		Dropped objects are removed by the garabge collector when it is safe. You can drop a resource
		whenever you'd like.
    */
    class ResourceManager
		: public AbstractGarbageCollector
    {
    public:

        //! Constructor
        ResourceManager( )
			: resourcelist(), recyclelist(), AbstractGarbageCollector()
		{
            setGarbageCollectionFunction( boost::bind( &ResourceManager::garbageCollect, this ) );
		}

        //! Destructor
        /*!
            Destructing the resource manager lets go of all the references to its resouces,
            therefore you can expect those resources to be deleted as soon as their last
            reference goes out of scope.
        */
        virtual ~ResourceManager()
        {
			clearResourceList();
        }

        //! Adds a resource to the list.
        /*!
        	This should be called when a resource is constructed.
        	\sa removeResource
        */
        inline virtual void addResource( boost::shared_ptr<Resource> rc )
        {
			boost::recursive_mutex::scoped_lock l( getMutex() );
            resourcelist.push_back( rc );
        }

        //! Adds a resource to the list of resources to be be garbage-collected.
        /*!
        	This should be called in the drop() function of any resources.
        	\sa addResource
        */
        inline virtual void removeResource( boost::shared_ptr<Resource> rc )
        {
			boost::recursive_mutex::scoped_lock l( getMutex() );
			recyclelist.push_back( rc );
        }

        //! Clear list
        /*!
        	Releases the reference to every resource in the list by clearing the resource list.
        */
        inline virtual void clearResourceList()
        {
			boost::recursive_mutex::scoped_lock l( getMutex()  );
			recyclelist.clear();
            resourcelist.clear();
        }

        //! Gets the resource at the given index.
        inline virtual boost::shared_ptr<Resource> getResource( const unsigned int index )
        {
			boost::recursive_mutex::scoped_lock l( getMutex() );
            if ( index < resourcelist.size() )
            {
                std::list< boost::shared_ptr<Resource> >::const_iterator it = resourcelist.begin();
                std::advance(it, index);
                return (*it);
            }
            return boost::shared_ptr<Resource>();
        }

        //! The number of resources.
        inline const unsigned int getResourceCount()
        {
			boost::recursive_mutex::scoped_lock l( getMutex() );
            return resourcelist.size();
        }

        //! Finds the resource with the given name.
        virtual boost::shared_ptr<Resource> findResource( const std::string& name );

        //! Get resource list
		/*!
			Returns a reference to the resource list.
			\note This not <b>not</b> thread-safe! If you do any operations on the list
			you must call lock() before and unlock() after. If you do not, prepare for a crash
			when the garbage collector comes around.
		*/
        inline std::list< boost::shared_ptr<Resource> >& getResourceList()
        {
            return resourcelist;
        }

		

    protected:

		//! list of resources
        std::list< boost::shared_ptr<Resource> > resourcelist;

		//! list of resources to be recycled
		std::vector< boost::shared_ptr<Resource> > recyclelist;

		//! Garbage collection function
		void garbageCollect();

    private:
    };

}//namespace phoenix

#include "Resource.h"

#endif // __PHRESOURCEMANAGER_H__