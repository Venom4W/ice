// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Functional.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <Freeze/Initialize.h>
#include <algorithm>


using namespace IceStorm;
using namespace std;


namespace IceStorm
{

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class PublisherProxyI : public Ice::Blobject
{
public:

    PublisherProxyI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~PublisherProxyI()
    {
    }

    virtual bool ice_invoke(const vector< Ice::Byte>&, vector< Ice::Byte>&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    IceStorm::TopicSubscribersPtr _subscribers;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TopicLinkI : public TopicLink
{
public:

    TopicLinkI(const IceStorm::TopicSubscribersPtr& s) :
	_subscribers(s)
    {
    }

    ~TopicLinkI()
    {
    }

    virtual void forward(const string&, ::Ice::OperationMode, const ByteSeq&, const ContextData&, const Ice::Current&);

private:

    //
    // Set of associated subscribers
    //
    IceStorm::TopicSubscribersPtr _subscribers;
};

} // End namespace IceStorm

IceStorm::TopicSubscribers::TopicSubscribers(const TraceLevelsPtr& traceLevels) :
    _traceLevels(traceLevels)
{
}

IceStorm::TopicSubscribers::~TopicSubscribers()
{
}

void
IceStorm::TopicSubscribers::add(const SubscriberPtr& subscriber)
{
    Ice::Identity id = subscriber->id();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    
    //
    // If a subscriber with this identity is already subscribed
    // then mark the subscriber as replaced.
    //
    // Note that this doesn't actually remove the subscriber from
    // the list of subscribers - it marks the subscriber as
    // replaced, and it's removed on the next event publish.
    //
    for(SubscriberList::iterator i = _subscribers.begin() ; i != _subscribers.end(); ++i)
    {
	if((*i)->id() == id)
	{
	    //
	    // This marks the subscriber as invalid. It will be
	    // removed on the next event publish.
	    //
	    (*i)->replace();
	    break;
	}
    }

    //
    // Add to the set of subscribers.
    //
    _subscribers.push_back(subscriber);
}

//
// Unsubscribe the subscriber with the given identity. Note that
// this doesn't remove the subscriber from the list of subscribers
// - it marks the subscriber as unsubscribed, and it's removed on
// the next event publish.
//
void
IceStorm::TopicSubscribers::remove(const Ice::ObjectPrx& obj)
{
    Ice::Identity id = obj->ice_getIdentity();
    
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    
    for(SubscriberList::iterator i = _subscribers.begin() ; i != _subscribers.end(); ++i)
    {
	if((*i)->id() == id)
	{
	    //
	    // This marks the subscriber as invalid. It will be
	    // removed on the next event publish.
	    //
	    (*i)->unsubscribe();
	    return;
	}
    }
    
    //
    // If the subscriber was not found then display a diagnostic.
    //
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << id << ": not subscribed.";
    }
}

//
// TODO: Optimize
//
// It's not strictly necessary to clear the error'd subscribers on
// every publish iteration (if the subscriber validates the state
// before attempting publishing the event). This means more mutex
// locks (due to the state check in the subscriber) - but with the
// advantage that publishes can occur in parallel and less
// subscriber list iterations.
//
void
IceStorm::TopicSubscribers::publish(const EventPtr& event)
{
    //
    // Copy of the subscriber list so that event publishing can
    // occur in parallel.
    //
    // TODO: Find out whether this is a false optimization - how
    // expensive is the cost of copying vs. lack of parallelism?
    //
    SubscriberList copy;
    
    {
	IceUtil::Mutex::Lock sync(_subscribersMutex);
	
	//
	// Copy of the subscribers that are in error.
	//
	SubscriberList e;

	//
	// Erase the inactive subscribers from the _subscribers
	// list. Copy the subscribers in error to the error list.
	//
	SubscriberList::iterator p = _subscribers.begin();
	while(p != _subscribers.end())
	{
	    if((*p)->inactive())
	    {
		if((*p)->error())
		{
		    e.push_back(*p);
		}
		
		SubscriberList::iterator tmp = p;
		++p;
		_subscribers.erase(tmp);
	    }
	    else
	    {
		copy.push_back(*p);
		++p;
	    }
	}
	
	if(!e.empty())
	{
	    IceUtil::Mutex::Lock errorSync(_errorMutex);
	    _error.splice(_error.begin(), e);
	}
    }
    
    for(SubscriberList::iterator p = copy.begin(); p != copy.end(); ++p)
    {
	(*p)->publish(event);
    }
}

//
// Clear & return the set of subscribers that are in error.
//
SubscriberList
IceStorm::TopicSubscribers::clearErrorList()
{
    //
    // Uses splice for efficiency
    //
    IceUtil::Mutex::Lock errorSync(_errorMutex);
    SubscriberList c;
    c.splice(c.begin(), _error);
    return c;
}

//
// Incoming events from publishers.
//
bool
PublisherProxyI::ice_invoke(const vector< Ice::Byte>& inParams, vector< Ice::Byte>& outParam,
                            const Ice::Current& current)
{
    const Ice::Context& context = current.ctx;

    EventPtr event = new Event;
    event->forwarded = false;
    Ice::Context::const_iterator p = context.find("cost");
    if(p != context.end())
    {
        event->cost = atoi(p->second.c_str());
    }
    else
    {
        event->cost = 0; // TODO: Default comes from property?
    }
    event->op = current.operation;
    event->mode = current.mode;
    event->data = inParams;
    event->context = context;

    _subscribers->publish(event);

    return true;
}

//
// Incoming events from linked topics.
//
void
TopicLinkI::forward(const string& op, Ice::OperationMode mode, const ByteSeq& data, const ContextData& context,
                    const Ice::Current& current)
{
    EventPtr event = new Event;
    event->forwarded = true;
    event->cost = 0;
    event->op = op;
    event->mode = mode;
    event->data = data;
    event->context = context;

    _subscribers->publish(event);
}

TopicI::TopicI(const Ice::ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels, const string& name,
	       const SubscriberFactoryPtr& factory, 
	       const string& envName, const string& dbName, bool createDb) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _name(name),
    _factory(factory),
    _destroyed(false),
    _connection(Freeze::createConnection(adapter->getCommunicator(), envName)),
    _links(_connection, dbName, createDb)
{
    _subscribers = new TopicSubscribers(_traceLevels);

    //
    // Create a servant per topic to receive event data. The servant's
    // identity is category=<topicname>, name=publish. Activate the
    // object and save a reference to give to publishers.
    //
    _publisher = new PublisherProxyI(_subscribers);
    
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";
    _publisherPrx = _adapter->add(_publisher, id);

    //
    // Create a servant per topic to receive linked event data. The
    // servant's identity is category=<topicname>, name=link. Activate
    // the object and save a reference to give to linked topics.
    //
    _link = new TopicLinkI(_subscribers);

    id.name = "link";
    _linkPrx = TopicLinkPrx::uncheckedCast(_adapter->add(_link, id));

    //
    // Run through link database re-establishing linked subscribers.
    //
    for(IdentityLinkDict::const_iterator p = _links.begin(); p != _links.end(); ++p)
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << _name << " relink " << p->first;
	}
	
	//
	// Create the subscriber object and add it to the set of
	// subscribers.
	//
	SubscriberPtr subscriber = _factory->createLinkSubscriber(p->second.obj, p->second.info.cost);
	_subscribers->add(subscriber);
    }
}

TopicI::~TopicI()
{
}

string
TopicI::getName(const Ice::Current&) const
{
    // Immutable
    return _name;
}

Ice::ObjectPrx
TopicI::getPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

void
TopicI::destroy(const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    //
    // See the comment in the constructor for the format of the identities.
    //
    Ice::Identity id;
    id.category = _name;
    id.name = "publish";

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "destroying " << id;
    }

    _adapter->remove(id);

    id.name = "link";

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "destroying " << id;
    }

    _adapter->remove(id);

    _links.destroy();
}

void
TopicI::subscribe(const QoS& qos, const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    Ice::Identity ident = subscriber->ice_getIdentity();
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << "Subscribe: " << Ice::identityToString(ident);
	if(_traceLevels->topic > 1)
	{
	    out << " QoS: ";
	    for(QoS::const_iterator qi = qos.begin(); qi != qos.end() ; ++qi)
	    {
		if(qi != qos.begin())
		{
		    out << ',';
		}
		out << '[' << qi->first << "," << qi->second << ']';
	    }
	}
    }

    reap();

    //
    // Add this subscriber to the set of subscribers.
    //
    SubscriberPtr sub = _factory->createSubscriber(qos, subscriber);
    _subscribers->add(sub);
}

void
TopicI::unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    Ice::Identity ident = subscriber->ice_getIdentity();

    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);

	out << "Unsubscribe: " << Ice::identityToString(ident);
    }

    reap();

    //
    // Unsubscribe the subscriber with this identity.
    //
    _subscribers->remove(subscriber);
}

void
TopicI::link(const TopicPrx& topic, Ice::Int cost, const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    string name = topic->getName();
    if(_traceLevels->topic > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	out << _name << " link " << name << " cost " << cost;
    }

    //
    // Retrieve the TopicLink.
    //
    TopicInternalPrx internal = TopicInternalPrx::checkedCast(topic);
    TopicLinkPrx link = internal->getLinkProxy();
    Ice::Identity ident = link->ice_getIdentity();

    //
    // Create the LinkDB record.
    //
    LinkDB dbInfo;
    dbInfo.obj = link;
    dbInfo.info.theTopic = topic;
    dbInfo.info.name = name;
    dbInfo.info.cost = cost;
    _links.put(pair<const Ice::Identity, const LinkDB>(ident, dbInfo));

    //
    // Create the subscriber object and add it to the set of subscribers.
    //
    SubscriberPtr subscriber = _factory->createLinkSubscriber(dbInfo.obj, dbInfo.info.cost);
    _subscribers->add(subscriber);
}

void
TopicI::unlink(const TopicPrx& topic, const Ice::Current&)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    reap();

    TopicInternalPrx internal = TopicInternalPrx::checkedCast(topic);
    Ice::ObjectPrx link = internal->getLinkProxy();

    if(_links.erase(link->ice_getIdentity()) > 0)
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << _name << " unlink " << topic->getName();
	}
	_subscribers->remove(link);
    }
    else
    {
	if(_traceLevels->topic > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
	    out << _name << " unlink " << topic->getName() << " failed - not linked";
	}
    }
}

LinkInfoSeq
TopicI::getLinkInfoSeq(const Ice::Current&) const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    TopicI* const This = const_cast<TopicI* const>(this);
    This->reap();

    LinkInfoSeq seq;

    for(IdentityLinkDict::const_iterator p = _links.begin(); p != _links.end(); ++p)
    {
	LinkInfo info = p->second.info;
	seq.push_back(info);
    }

    return seq;
}

TopicLinkPrx
TopicI::getLinkProxy(const Ice::Current&)
{
    // Immutable
    return _linkPrx;
}

bool
TopicI::destroyed() const
{
    IceUtil::RecMutex::Lock sync(*this);
    return _destroyed;
}

void
TopicI::reap()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	return;
    }

    //
    // Run through all invalid subscribers and remove them from the
    // database.
    //
    SubscriberList error = _subscribers->clearErrorList();
    for(SubscriberList::iterator p = error.begin(); p != error.end(); ++p)
    {
	SubscriberPtr subscriber = *p;
	assert(subscriber->error());
	if(subscriber->persistent())
	{
	    if(_links.erase(subscriber->id()) > 0)
	    {
		if(_traceLevels->topic > 0)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
		    out << "reaping " << subscriber->id();
		}
	    }
	    else
	    {
		if(_traceLevels->topic > 0)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->topicCat);
		    out << "reaping " << subscriber->id() << " failed - not in database";
		}
	    }
	}
    }
}
