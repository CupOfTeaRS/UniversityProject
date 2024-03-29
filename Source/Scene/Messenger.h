/*******************************************
	Messenger.h

	Entity messenger class definitions
********************************************/

#pragma once

#include <map>
using namespace std;

#include "Defines.h"
#include "Entity.h"

namespace gen
{

/////////////////////////////////////
//	Public types

// Some basic message types
enum EMessageType
{
	Msg_Dmg,
	Msg_Knockback,
	Msg_Victory,
};
// A message contains a type, then one of a selection of structures. The "union" structure
// holds several sub-structures or types - all occupying the same memory (on top of each other)
// So only one of the sub-structures can be used by any particular message. E.g. A Msg_Goto
// message suggests that the {pt,distPt} structure should be used - a target point & range.
// This isn't enforced by the language - use of the union is up to the programmer
struct SMessage
{
	EMessageType type;
	TEntityUID   from;
	TUInt32 dmg;
	TUInt32 effect;
	TUInt32 knockbackVel;
	TUInt32 knockUpVel;
	bool isStoppingTime;
	bool isTheWorld ;
	bool isKnockbackedRight ;
	bool isThrow = false;
};


// Messenger class allows the sending and receipt of messages between entities - addressed
// by UID
class CMessenger
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Default constructor
	CMessenger() {}

	// No destructor needed

private:
	// Disallow use of copy constructor and assignment operator (private and not defined)
	CMessenger( const CMessenger& );
	CMessenger& operator=( const CMessenger& );
	typedef pair<TEntityUID, SMessage> UIDMsgPair; // The type stored by the multimap

/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	// Message sending/receiving

	// Send the given message to a particular UID, does not check if the UID exists
	void SendMessage( TEntityUID to, const SMessage& msg);

	// Fetch the next available message for the given UID, returns the message through the given 
	// pointer. Returns false if there are no messages for this UID
	bool FetchMessage( TEntityUID to, SMessage* msg );


/////////////////////////////////////
//	Private interface
private:

	// A multimap has properties similar to a hash map - mapping a key to a value. Here we
	// have the key as an entity UID and the value as a message for that UID. The stored
	// key/value pairs in a multimap are sorted by key, which means all the messages for a
	// particular UID are together. Key look-up is somewhat slower than for a hash map though
	// Define some types to make usage easier
	typedef multimap<TEntityUID, SMessage> TMessages;
	typedef TMessages::iterator TMessageIter;
    

	TMessages m_Messages;
};


} // namespace gen
