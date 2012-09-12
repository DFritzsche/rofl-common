/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COFMATCH_H
#define COFMATCH_H 1

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <endian.h>
#include <stdio.h>
#include <string.h>
#include "openflow12.h"
#include <endian.h>
#ifndef be32toh
#include "../endian_conversion.h"
#endif

#ifdef __cplusplus
}
#endif

#include "../cmemory.h"
#include "../caddress.h"
#include "../cerror.h"
#include "../cmacaddr.h"
#include "../cvastring.h"

#include "rofl/platform/unix/csyslog.h"
#include "coxmlist.h"

class eOFmatchBase : public cerror {}; // error base class cofmatch
class eOFmatchType : public eOFmatchBase {};
class eOFmatchInval : public eOFmatchBase {};
class eOFmatchNotFound : public eOFmatchBase {};
class eOFmatchInvalBadValue : public eOFmatchInval {};




class cofmatch :
	public csyslog
{
public: // data structures

	struct ofp_match match;		// struct ofp_match header (including oxm_fields)
	coxmlist oxmlist;			// list of all oxms

private: // data structures

	std::string info; 	// info string

public: // methods

	/** constructor
	 *
	 */
	cofmatch(
			uint16_t type = OFPMT_OXM);


	/** constructor
	 *
	 */
	cofmatch(
		struct ofp_match *__match,
		size_t __matchlen);


	/** copy constructor
	 *
	 */
	cofmatch(const cofmatch &m) 
	{
		*this = m;
	};


	/** destructor
	 *
	 */
	virtual 
	~cofmatch();


	/** assignment operator
	 */
	cofmatch&
	operator= (const cofmatch& m);
	

	/** dump cofmatch instance
	 */
	const char*
	c_str();


	/** validate match structure
	 *
	 */
	void
	validate() throw (eOFmatchInval);


	/** return required length for packed cofmatch (includes padding to 64bit)
	 */
	size_t
	length();

private:
	size_t
	length_internal();


public:
	/** copy internal struct ofp_match into specified ofp_match ptr 'm'
	 * @return pointer 'm'
	 *
	 */
	struct ofp_match*
	pack(
			struct ofp_match* m,
			size_t mlen) throw (eOFmatchInval);


	/** copy ofp_match structure pointed to by 'm' into internal struct ofp_match
	 *
	 */
	void
	unpack(
			struct ofp_match* m,
			size_t mlen) throw (eOFmatchInval);


	/** check for an overlap between two ofp_match structures 
	 * (strict and non-strict)
	 */
	bool 
	overlaps(
		cofmatch const& m,
		bool strict = false);

	
	/** check for an identical match between two ofp_match structures
	 */
	bool 
	operator== (
		cofmatch& m);


	/** reset structure
	 *
	 */
	void
	reset();


	/**
	 *
	 */
	void
	set_type(uint16_t type);


	/**
	 *
	 */
	uint32_t
	get_in_port()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_in_port(
			uint32_t in_port);


	/**
	 *
	 */
	uint32_t
	get_in_phy_port()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_in_phy_port(
			uint32_t in_phy_port);


	/**
	 *
	 */
	uint64_t
	get_metadata()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_metadata(uint64_t metadata);


	/**
	 *
	 */
	cmacaddr
	get_eth_dst()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_eth_dst(
			cmacaddr const& maddr);


	/**
	 *
	 */
	cmacaddr
	get_eth_src()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_eth_src(
			cmacaddr const& maddr);


	/**
	 *
	 */
	uint16_t
	get_eth_type()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_eth_type(
			uint16_t dl_type);


	/**
	 *
	 */
	uint32_t
	get_mpls_label()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_mpls_label(
			uint32_t label);


	/**
	 *
	 */
	uint8_t
	get_mpls_tc()
		throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_mpls_tc(
			uint8_t tc);




	/**
	 *
	 */
	uint8_t
	get_ip_proto();


	/**
	 *
	 */
	void
	set_ip_proto(
			uint8_t proto);


	/**
	 *
	 */
	uint16_t
	get_udp_src()
			throw (eOFmatchNotFound);



	/**
	 *
	 */
	void
	set_udp_src(
			uint16_t src_port);


	/**
	 *
	 */
	uint16_t
	get_udp_dst()
			throw (eOFmatchNotFound);


	/**
	 *
	 */
	void
	set_udp_dst(
			uint16_t dst_port);


#ifndef NDEBUG
	/**
	 *
	 */
	static void
	test();
#endif
};

#endif
