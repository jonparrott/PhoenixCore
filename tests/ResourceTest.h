/*

Copyright (c) 2009, Jonathan Wayne Parrott

Please see the license.txt file included with this source
distribution for more information.

*/


#include "Phoenix.h"
#include <boost/thread.hpp>
#include <iostream>

using namespace phoenix;
using namespace std;


//! Test resource
class TestResource
    : public Resource
{
protected:

    TestResource( ResourceManager& r )
        : Resource(r)
    {
        ++resourcecount;
    }

public:

    virtual ~TestResource()
    {
        resourcecount--;
    }

    virtual void step()
    {
        cout<<"Resource "<<this<<" says hey!"<<endl;
    }

    static int resourcecount;
    friend class Resource;
};

int TestResource::resourcecount = 0;

class ResourceTest
{
    public:

        ResourceTest() : rmanager(), irmanager()
        {
        }

        virtual ~ResourceTest()
        {
        }

        /*!
            This actually runs this test. It verifies:
            *) That Resources can be added and removed from a resource manager.
            *) The the iterative resource manager behaves correctly.
        */
        int run()
        {

            //! Add 10 resources to rmanager (Basic resource manager).
            for(int i = 0; i<10; ++i)
            {
                TestResource::create<TestResource>(rmanager);
                cout<<"Test Resource Count: "<<TestResource::resourcecount<<endl;
            }
            cout<<endl;

            //! Now Remove one.
            rmanager.getResource(0)->drop();
            cout<<"Test Resource Count: "<<TestResource::resourcecount<<endl<<endl;

            //! Now delete them all.
            rmanager.clearResourceList();

            cout<<"Test Resource Count: "<<TestResource::resourcecount<<endl;
            cout<<"Resource Manager Resource Count: "<<rmanager.getResourceCount()<<endl;

            //! Now we'll do iteration.

            //! Add 10 resources to irmanager (Iterative resource manager).
            for(int i = 0; i<10; ++i)
            {
                TestResource::create<TestResource>(irmanager);
                cout<<"Test Resource Count: "<<TestResource::resourcecount<<endl;
            }

            //! Now iterate, and remove 5.
            for( unsigned int i = 0; i < irmanager.getResourceCount(); ++i )
            {
                if( i==5 )
                    irmanager.getResource(i)->drop();

                boost::shared_ptr<TestResource> resource = irmanager.getResource(i)->grab<TestResource>();

                if( resource )
                    resource->step();
			}

			cout<<"Test Resource Count: "<<TestResource::resourcecount<<endl;

            cin.get();

            return 0;


        }// Run

    protected:
        ResourceManager rmanager;
        ResourceManager irmanager;
    private:
};