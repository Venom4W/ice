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

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestCommon.h>
#include <ByteIntMap.h>

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Freeze;


// #define SHOW_EXCEPTIONS 1

#ifdef __SUNPRO_CC
extern
#else
static 
#endif
Byte alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";

vector<Byte> alphabet;

// The extern in the following function is due to a Sun C++ 5.4 template bug
//
extern void
ForEachTest(const pair<const Byte, const Int>&)
{
}

extern bool
FindIfTest(const pair<const Byte, const Int>& p)
{
    return p.first == 'b';
}

extern bool
FindFirstOfTest(const pair<const Byte, const Int>& p, Byte q)
{
    return p.first == q;
}


void
populateDB(const ConnectionPtr& connection, ByteIntMap& m)
{
    alphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars) - 1);
    size_t length = alphabet.size();
    
    for(;;)
    {
	try
	{
	    TransactionHolder txHolder(connection);
	    for(size_t j = 0; j < length; ++j)
	    {
		m.put(ByteIntMap::value_type(alphabet[j], static_cast<Int>(j)));
	    }
	    txHolder.commit();
	    break;
	}
	catch(const DBDeadlockException&)
	{
#ifdef SHOW_EXCEPTIONS
	    cerr << "t" << flush;
#endif

	    length = length / 2;
	    //
	    // Try again
	    //
	}
    }
}

class ReadThread : public IceUtil::Thread
{
public:

    ReadThread(const CommunicatorPtr& communicator, const string& envName, const string& dbName) :
	_connection(createConnection(communicator, envName)),
	_map(_connection, dbName)
    {
    }

    virtual void
    run()
    {
	for(int i = 0; i < 10; ++i)
	{
	    for(;;)
	    {
		try
		{
		    for(ByteIntMap::iterator p = _map.begin(); p != _map.end(); ++p)
		    {
			test(p->first == p->second + 'a');
			IceUtil::ThreadControl::yield();
		    }
		    break; // for(;;)
		}
		catch(const DBDeadlockException&)
		{
#ifdef SHOW_EXCEPTIONS
		    cerr << "r" << flush;
#endif
		    //
		    // Try again
		    //
		}
		catch(const DBInvalidPositionException&)
		{
#ifdef SHOW_EXCEPTIONS
		    cerr << "i" << flush;
#endif
		    break;
		}
	    }
	}
    }

private:

    ConnectionPtr _connection;
    ByteIntMap  _map;
};


class WriteThread : public IceUtil::Thread
{
public:

    WriteThread(const CommunicatorPtr& communicator, const string& envName, const string& dbName) :
	_connection(createConnection(communicator, envName)),
	_map(_connection, dbName)
    {
    }

    virtual void
    run()
    {
	//
	// Delete an recreate each object
	//
	for(int i = 0; i < 4; ++i)
	{
	    for(;;)
	    {
		try
		{
		    for(ByteIntMap::iterator p = _map.begin(); p != _map.end(); ++p)
		    {
			p.set(p->second + 1);
			_map.erase(p);
		    }
		    break; // for(;;)
		}
		catch(const DBDeadlockException&)
		{
#ifdef SHOW_EXCEPTIONS
		    cerr << "w" << flush;
#endif
		    //
		    // Try again
		    //
		}
		catch(const DBInvalidPositionException&)
		{
#ifdef SHOW_EXCEPTIONS
		    cerr << "I" << flush;
#endif
		    break;
		}
	    }
	    populateDB(_connection, _map);
	}
    }

private:

    ConnectionPtr _connection;
    ByteIntMap  _map;
};


int
run(const CommunicatorPtr& communicator, const string& envName, const string&dbName)
{
    ConnectionPtr connection = createConnection(communicator, envName);
    ByteIntMap m(connection, dbName);
    
    //
    // Populate the database with the alphabet
    //
    populateDB(connection, m);

    vector<Byte>::const_iterator j;
    ByteIntMap::iterator p;
    ByteIntMap::const_iterator cp;

    cout << "  testing populate... ";
    //
    // First try non-const iterator
    //
    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
	p = m.find(*j);
	test(p != m.end());
	test(p->first == *j && p->second == j - alphabet.begin());
    }
    //
    // Next try const iterator
    //
    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
	cp = m.find(*j);
	test(cp != m.end());
	test(cp->first == *j && cp->second == j - alphabet.begin());
    }
   
    test(!m.empty());
    test(m.size() == alphabet.size());
    cout << "ok" << endl;

    cout << "  testing map::find... ";
    j = find(alphabet.begin(), alphabet.end(), 'n');
    
    cp = m.find(*j);
    test(cp != m.end());
    test(cp->first == 'n' && cp->second == j - alphabet.begin());
    cout << "ok" << endl;

    cout << "  testing erase... ";

    //
    // erase first offset characters (first offset characters is
    // important for later verification of the correct second value in
    // the map).
    //
    int offset = 3;
    vector<Byte> bytes;
    bytes.push_back('a');
    bytes.push_back('b');
    bytes.push_back('c');
    for(j = bytes.begin(); j != bytes.end(); ++j)
    {
	p = m.find(*j);
	test(p != m.end());
	m.erase(p);
	
	//
	// Release locks to avoid self deadlock
	//
	p = m.end();
	
	p = m.find(*j);
	test(p == m.end());
	vector<Byte>::iterator r = find(alphabet.begin(), alphabet.end(), *j);
	test(r != alphabet.end());
	alphabet.erase(r);
    }

    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
	cp = m.find(*j);
	test(cp != m.end());
	test(cp->first == *j && cp->second == (j - alphabet.begin()) + offset);
    }

    cout << "ok" << endl;

    //
    // Get an iterator for the deleted element - this should fail.
    //
    cout << "  testing map::find (again)... ";
    cp = m.find('a');
    test(cp == m.end());
    cout << "ok" << endl;

    cout << "  testing iterators... ";
    p = m.begin();
    ByteIntMap::iterator p2 = p;

    //
    // Verify both iterators point at the same element, and that
    // element is in the map.
    //
    test(p == p2);
    test(p->first == p2->first && p->second == p2->second);
    test(find(alphabet.begin(), alphabet.end(), p->first) != alphabet.end());

    //
    // Create iterator that points at 'n'
    //
    p = m.find('n');
    p2 = p;

    //
    // Verify both iterators point at 'n'
    //
    test(p == p2);
    test(p->first == 'n' && p->second == 13);
    test(p2->first == 'n' && p2->second == 13);

    //
    // Create cursor that points at 'n'
    //
    p = m.find('n');
    test(p->first == 'n' && p->second == 13);
    ++p;

    p2 = p;

    //
    // Verify cloned cursors are independent
    //
    test(p->first != 'n' && p->second != 13);
    pair<const Byte, const Int> data = *p;
    ++p;

    test(p->first != data.first && p->second != data.second);
    ++p;

    test(p2->first == data.first && p2->second == data.second);
  
    p = m.find('n');
    p2 = ++p;
    test(p2->first == p->first);

    char c = p2->first;
    p2 = p++;
    test(c == p2->first); // p2 should still be the same
    test(p2->first != p->first && (++p2)->first == p->first);
    
    cout << "ok" << endl;

    //
    // Test writing into an iterator.
    //
    cout << "  testing iterator.set... ";

    p = m.find('d');
    test(p != m.end() && p->second == 3);

    test(m.find('a') == m.end());
    ByteIntMap::value_type i1('a', 1);

    m.put(i1);
    //
    // Note: VC++ won't accept this
    //
    //m.put(ByteIntMap::value_type('a', 1));

    p = m.find('a');
    test(p != m.end() && p->second == 1);

    ByteIntMap::value_type i2('a', 0);
    m.put(i2);
    //
    // Note: VC++ won't accept this
    //
    //m.put(ByteIntMap::value_type('a', 0));
    
    p = m.find('a');
    test(p != m.end() && p->second == 0);
    //
    // Test inserts
    // 
    
    ByteIntMap::value_type i3('a', 7);
    pair<ByteIntMap::iterator, bool> insertResult = m.insert(i3);

    test(insertResult.first == m.find('a'));
    test(insertResult.first->second == 0);
    test(insertResult.second == false);
    insertResult.first = m.end();
    
    p = m.insert(m.end(), i3);
    test(p == m.find('a'));
    test(p->second == 0);

    ByteIntMap::value_type i4('b', 7);
    
    insertResult = m.insert(i4);
    test(insertResult.first == m.find('b'));
    test(insertResult.first->second == 7);
    test(insertResult.second == true);
    insertResult.first = m.end();
    
    ByteIntMap::value_type i5('c', 8);
    
    p = m.insert(m.end(), i5);
    test(p == m.find('c'));
    test(p->second == 8);

    p = m.find('a');
    test(p != m.end() && p->second == 0);
    p.set(1);
    test(p != m.end() && p->second == 1);

    //
    // This is necessary to release the locks held
    // by p and avoid a self-deadlock
    //
    p = m.end();
    
    p = m.find('a');
    test(p != m.end() && p->second == 1);
    cout << "ok" << endl;
    
    //
    // Re-populate
    //
    populateDB(connection, m);

    cout << "  testing algorithms... ";

    for_each(m.begin(), m.end(), ForEachTest);

    //
    // Inefficient, but this is just a test. Ensure that both forms of
    // operator== & != are tested.
    //
    ByteIntMap::value_type toFind('n', 13);
    
    p = find(m.begin(), m.end(), toFind);
    test(p != m.end());
    test(*p == toFind);
    test(toFind == *p);
    test(!(*p != toFind));
    test(!(toFind != *p));

    p = find_if(m.begin(), m.end(), FindIfTest);
    test(p->first == 'b');

    //
    // find_first_of. First construct a map with keys n, o, p,
    // q. The test must find one of the types (it doesn't matter
    // which since the container doesn't have to maintain sorted
    // order).
    //
    j = find(alphabet.begin(), alphabet.end(), 'n');
    map<Byte, const Int> pairs;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));

    p = find_first_of(m.begin(), m.end(), pairs.begin(), pairs.end());
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    j = find(alphabet.begin(), alphabet.end(), 'n');
    p = find_first_of(m.begin(), m.end(), j, j + 4, FindFirstOfTest);
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    pairs.clear();
    for(p = m.begin(); p != m.end(); ++p)
    {
        pairs.insert(pair<const Byte, const Int>(p->first, p->second));
    }
    test(pairs.size() == m.size());

    map<Byte, const Int>::const_iterator pit;
    for(pit = pairs.begin(); pit != pairs.end(); ++pit)
    {
	p = m.find(pit->first);
	test(p != m.end());
    }
    cout << "ok" << endl;

    cout << "  testing concurrent access... " << flush;
    m.clear();
    populateDB(connection, m);

    vector<IceUtil::ThreadControl> controls;
    for(int i = 0; i < 5; ++i)
    {
	IceUtil::ThreadPtr rt = new ReadThread(communicator, envName, dbName);
	controls.push_back(rt->start());
	IceUtil::ThreadPtr wt = new WriteThread(communicator, envName, dbName);
	controls.push_back(wt->start());
    }
    for(vector<IceUtil::ThreadControl>::iterator q = controls.begin(); q != controls.end(); ++q)
    {
	q->join();
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
 
    string envName = "db";

    try
    {
	communicator = Ice::initialize(argc, argv);
	if(argc != 1)
	{
	    envName = argv[1];
	    envName += "/";
	    envName += "db";
	}
       
	cout << "testing encoding..." << endl;
	status = run(communicator, envName, "binary");
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    try
    {
	communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    return status;
}
