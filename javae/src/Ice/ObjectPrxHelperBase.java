// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class ObjectPrxHelperBase implements ObjectPrx
{
    public final int
    hashCode()
    {
        return _reference.hashCode();
    }

    public final int
    ice_hash()
    {
        return _reference.hashCode();
    }

    public final Communicator ice_communicator()
    {
        return _reference.getCommunicator();
    }

    public final String toString()
    {
        return _reference.toString();
    }

    public final String ice_toString()
    {
        return toString();
    }

    public final boolean
    ice_isA(String __id)
    {
        return ice_isA(__id, _reference.getContext());
    }

    public final boolean
    ice_isA(String __id, java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
	        __checkTwowayOnly("ice_isA");
		Connection __connection = ice_connection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_isA", OperationMode.Nonmutating,
                                                                     __context);
                try
                {
                    try
                    {
                        IceInternal.BasicStream __os = __og.os();
                        __os.writeString(__id);
                    }
                    catch(Ice.LocalException __ex)
                    {
                        __og.abort(__ex);
                    }
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.is();
                        if(!__ok)
                        {
                            __is.throwException();
                        }
                        return __is.readBool();
                    }
                    catch(UserException __ex)
                    {
                        throw new Ice.UnknownUserException(__ex.ice_name());
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.NonRepeatable(__ex);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final void
    ice_ping()
    {
        ice_ping(_reference.getContext());
    }

    public final void
    ice_ping(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
	        __checkTwowayOnly("ice_ping");
		Connection __connection = ice_connection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_ping", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.is();
                        if(!__ok)
                        {
                            __is.throwException();
                        }
                    }
                    catch(UserException __ex)
                    {
                        throw new Ice.UnknownUserException(__ex.ice_name());
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.NonRepeatable(__ex);
                    }
                }
                finally
                {
                   __connection.reclaimOutgoing(__og);
                }
                return;
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public String[]
    ice_ids()
    {
        return ice_ids(_reference.getContext());
    }

    public String[]
    ice_ids(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
	        __checkTwowayOnly("ice_ids");
	        Connection __connection = ice_connection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_ids", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.is();
                        if(!__ok)
                        {
                            __is.throwException();
                        }
                        return __is.readStringSeq();
                    }
                    catch(UserException __ex)
                    {
                        throw new Ice.UnknownUserException(__ex.ice_name());
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.NonRepeatable(__ex);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public String
    ice_id()
    {
        return ice_id(_reference.getContext());
    }

    public String
    ice_id(java.util.Hashtable __context)
    {
        int __cnt = 0;
        while(true)
        {
            try
            {
	        __checkTwowayOnly("ice_id");
	        Connection __connection = ice_connection();
                IceInternal.Outgoing __og = __connection.getOutgoing(_reference, "ice_id", OperationMode.Nonmutating,
                                                                    __context);
                try
                {
                    boolean __ok = __og.invoke();
                    try
                    {
                        IceInternal.BasicStream __is = __og.is();
                        if(!__ok)
                        {
                            __is.throwException();
                        }
                        return __is.readString();
                    }
                    catch(UserException __ex)
                    {
                        throw new Ice.UnknownUserException(__ex.ice_name());
                    }
                    catch(LocalException __ex)
                    {
                        throw new IceInternal.NonRepeatable(__ex);
                    }
                }
                finally
                {
                    __connection.reclaimOutgoing(__og);
                }
            }
            catch(IceInternal.NonRepeatable __ex)
            {
                __cnt = __handleException(__ex.get(), __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__ex, __cnt);
            }
        }
    }

    public final Identity
    ice_getIdentity()
    {
        return _reference.getIdentity();
    }

    public final ObjectPrx
    ice_newIdentity(Identity newIdentity)
    {
        if(newIdentity.equals(_reference.getIdentity()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    public final java.util.Hashtable
    ice_getContext()
    {
        return _reference.getContext();
    }

    public final ObjectPrx
    ice_newContext(java.util.Hashtable newContext)
    {
	ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
	proxy.setup(_reference.changeContext(newContext));
	return proxy;
    }

    public final ObjectPrx
    ice_defaultContext()
    {
	ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
	proxy.setup(_reference.defaultContext());
	return proxy;
    }

    public final String
    ice_getFacet()
    {
        return _reference.getFacet();
    }

    public final ObjectPrx
    ice_newFacet(String newFacet)
    {
        if(newFacet == null)
        {
            newFacet = "";
        }

        if(newFacet.equals(_reference.getFacet()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeFacet(newFacet));
            return proxy;
        }
    }

    public final ObjectPrx
    ice_twoway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeTwoway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isTwoway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeTwoway;
    }

    public final ObjectPrx
    ice_oneway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeOneway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isOneway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeOneway;
    }

    public final ObjectPrx
    ice_batchOneway()
    {
        IceInternal.Reference ref = _reference.changeMode(IceInternal.Reference.ModeBatchOneway);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final boolean
    ice_isBatchOneway()
    {
	return _reference.getMode() == IceInternal.Reference.ModeBatchOneway;
    }

    public final ObjectPrx
    ice_timeout(int t)
    {
        IceInternal.Reference ref = _reference.changeTimeout(t);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_router(Ice.RouterPrx router)
    {
        IceInternal.Reference ref = _reference.changeRouter(router);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public final ObjectPrx
    ice_locator(Ice.LocatorPrx locator)
    {
        IceInternal.Reference ref = _reference.changeLocator(locator);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(ref);
            return proxy;
        }
    }

    public synchronized final Connection
    ice_connection()
    {
	if(_connection == null)
	{
	    _connection = _reference.getConnection();

            //
            // If this proxy is for a non-local object, and we are
            // using a router, then add this proxy to the router info
            // object.
            //
	    try
	    {
	        IceInternal.RoutableReference rr = (IceInternal.RoutableReference)_reference;
	        if(rr != null && rr.getRouterInfo() != null)
	        {
	            rr.getRouterInfo().addProxy(this);
	        }
	    }
	    catch(ClassCastException e)
	    {
	    }
	}
	return _connection;
    }

    public final boolean
    equals(java.lang.Object r)
    {
        ObjectPrxHelperBase rhs = (ObjectPrxHelperBase)r;
        return _reference.equals(rhs._reference);
    }

    public final IceInternal.Reference
    __reference()
    {
        return _reference;
    }

    public final void
    __copyFrom(ObjectPrx from)
    {
        ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
        IceInternal.Reference ref = null;
        Connection con = null;

        synchronized(from)
        {
            ref = h._reference;
            con = h._connection;
        }

        //
        // No need to synchronize "*this", as this operation is only
        // called upon initialization.
        //

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference == null);
	    IceUtil.Debug.Assert(_connection == null);
	}

        _reference = ref;
	_connection = con;
    }

    public final int
    __handleException(LocalException ex, int cnt)
    {
	//
	// Only _connection needs to be mutex protected here.
	//
	synchronized(this)
	{
	    _connection = null;
	}

	IceInternal.ProxyFactory proxyFactory = _reference.getInstance().proxyFactory();
	if(proxyFactory != null)
	{
	    return proxyFactory.checkRetryAfterException(ex, _reference, cnt);
	}
	else
	{
	    //
            // The communicator is already destroyed, so we cannot
            // retry.
	    //
	    throw ex;
	}
    }

    public final synchronized void
    __rethrowException(LocalException ex)
    {
        _connection = null;
        throw ex;
    }

    public final void
    __checkTwowayOnly(String name)
    {
	//
	// No mutex lock necessary, there is nothing mutable in this
	// operation.
	//

        if(!ice_isTwoway())
	{
	    TwowayOnlyException ex = new TwowayOnlyException();
	    ex.operation = name;
	    throw ex;
	}
    }

    protected java.util.Hashtable
    __defaultContext()
    {
        return _reference.getContext();
    }

    //
    // Only for use by IceInternal.ProxyFactory
    //
    public final void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initial initialization.
        //

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference == null);
	    IceUtil.Debug.Assert(_connection == null);
	}

        _reference = ref;
    }

    protected IceInternal.Reference _reference;
    private Connection _connection;
}
