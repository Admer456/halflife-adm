//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "netadr.h"

struct net_response_t;

#define NETAPI_REQUEST_SERVERLIST (0) // Doesn't need a remote address
#define NETAPI_REQUEST_PING (1)
#define NETAPI_REQUEST_RULES (2)
#define NETAPI_REQUEST_PLAYERS (3)
#define NETAPI_REQUEST_DETAILS (4)

// Set this flag for things like broadcast requests, etc. where the engine should not
//  kill the request hook after receiving the first response
#define FNETAPI_MULTIPLE_RESPONSE (1 << 0)

typedef void (*net_api_response_func_t)(net_response_t* response);

#define NET_SUCCESS (0)
#define NET_ERROR_TIMEOUT (1 << 0)
#define NET_ERROR_PROTO_UNSUPPORTED (1 << 1)
#define NET_ERROR_UNDEFINED (1 << 2)

struct net_adrlist_t
{
	net_adrlist_t* next;
	netadr_t remote_address;
};

struct net_response_t
{
	// NET_SUCCESS or an error code
	int error;

	// Context ID
	int context;
	// Type
	int type;

	// Server that is responding to the request
	netadr_t remote_address;

	// Response RTT ping time
	double ping;
	// Key/Value pair string ( separated by backlash \ characters )
	// WARNING:  You must copy this buffer in the callback function, because it is freed
	//  by the engine right after the call!!!!
	// ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	void* response;
};

struct net_status_t
{
	// Connected to remote server?  1 == yes, 0 otherwise
	int connected;
	// Client's IP address
	netadr_t local_address;
	// Address of remote server
	netadr_t remote_address;
	// Packet Loss ( as a percentage )
	int packet_loss;
	// Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	double latency;
	// Connection time, in seconds
	double connection_time;
	// Rate setting ( for incoming data )
	double rate;
};

struct net_api_t
{
	// APIs
	void (*InitNetworking)();
	void (*Status)(net_status_t* status);
	void (*SendRequest)(int context, int request, int flags, double timeout, netadr_t* remote_address, net_api_response_func_t response);
	void (*CancelRequest)(int context);
	void (*CancelAllRequests)();
	char* (*AdrToString)(netadr_t* a);
	int (*CompareAdr)(netadr_t* a, netadr_t* b);
	int (*StringToAdr)(char* s, netadr_t* a);
	const char* (*ValueForKey)(const char* s, const char* key);
	void (*RemoveKey)(char* s, const char* key);
	void (*SetValueForKey)(char* s, const char* key, const char* value, int maxsize);
};
