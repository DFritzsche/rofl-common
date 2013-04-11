/*
 * crofbase.h
 *
 *  Created on: 25.10.2012
 *      Author: andreas
 */

#ifndef CROFBASE_H
#define CROFBASE_H 1

#include <map>
#include <set>
#include <list>
#include <vector>
#include <bitset>
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif
#include "openflow/openflow12.h"
#include "openflow/openflow_rofl.h"
#include <endian.h>
#include <string.h>
#include <time.h>

#ifndef htobe16
#include "endian_conversion.h"
#endif

#ifdef __cplusplus
}
#endif

#include "ciosrv.h"
#include "cerror.h"
#include "cvastring.h"
#include "croflexp.h"
#include "cphyport.h"
#include "csocket.h"
#include "thread_helper.h"

//#include "rofl/experimental/crib.h"
#include "rofl/platform/unix/crandom.h"

#include "openflow/cofport.h"
#include "openflow/cofdpt.h"
#include "openflow/cofctl.h"
#include "openflow/cofinst.h"
#include "openflow/cofinlist.h"
#include "openflow/cofaction.h"
#include "openflow/cofaclist.h"
#include "openflow/cofmatch.h"
#include "openflow/cflowentry.h"
#include "openflow/cgroupentry.h"
#include "openflow/cofstats.h"
#include "openflow/extensions/cfsptable.h"
#include "openflow/openflow_rofl_exceptions.h"
#include "openflow/messages/cofmsg.h"
#include "openflow/messages/cofmsg_hello.h"
#include "openflow/messages/cofmsg_echo.h"
#include "openflow/messages/cofmsg_error.h"
#include "openflow/messages/cofmsg_features.h"
#include "openflow/messages/cofmsg_config.h"
#include "openflow/messages/cofmsg_packet_out.h"
#include "openflow/messages/cofmsg_packet_in.h"
#include "openflow/messages/cofmsg_flow_mod.h"
#include "openflow/messages/cofmsg_flow_removed.h"
#include "openflow/messages/cofmsg_group_mod.h"
#include "openflow/messages/cofmsg_table_mod.h"
#include "openflow/messages/cofmsg_port_mod.h"
#include "openflow/messages/cofmsg_port_status.h"
#include "openflow/messages/cofmsg_stats.h"
#include "openflow/messages/cofmsg_desc_stats.h"
#include "openflow/messages/cofmsg_flow_stats.h"
#include "openflow/messages/cofmsg_aggr_stats.h"
#include "openflow/messages/cofmsg_table_stats.h"
#include "openflow/messages/cofmsg_port_stats.h"
#include "openflow/messages/cofmsg_queue_stats.h"
#include "openflow/messages/cofmsg_group_stats.h"
#include "openflow/messages/cofmsg_group_desc_stats.h"
#include "openflow/messages/cofmsg_group_features_stats.h"
#include "openflow/messages/cofmsg_barrier.h"
#include "openflow/messages/cofmsg_queue_get_config.h"
#include "openflow/messages/cofmsg_role.h"
#include "openflow/messages/cofmsg_experimenter.h"

namespace rofl
{


/* error classes */
class eRofBase						: public cerror {};   // base error class crofbase
class eRofBaseIsBusy 				: public eRofBase {}; // this FwdElem is already controlled
class eRofBaseNotImpl 				: public eRofBase {}; // this FwdElem's method is not implemented
class eRofBaseNoCtrl 				: public eRofBase {}; // no controlling entity attached to this FwdElem
class eRofBaseNotFound 				: public eRofBase {}; // internal entity not found
class eRofBaseInval					: public eRofBase {}; // invalid parameter (e.g. invalid packet type)
class eRofBaseNotAttached 			: public eRofBase {}; // received command from entity being not attached
class eRofBaseNoRequest 			: public eRofBase {}; // no request packet found for session
class eRofBaseXidInval	 			: public eRofBase {}; // invalid xid in session exchange
class eRofBaseExists				: public eRofBase {}; // fwdelem with either this dpid or dpname already exists
class eRofBaseOFportNotFound 		: public eRofBase {}; // cofport instance not found
class eRofBaseTableNotFound 		: public eRofBase {}; // flow-table not found (e.g. unknown table_id in flow_mod)
class eRofBaseGotoTableNotFound 	: public eRofBase {}; // table-id specified in OFPIT_GOTO_TABLE invalid
class eRofBaseFspSupportDisabled 	: public eRofBase {};




class cofctl;
class cofdpt;



/**
 * \class crofbase
 * \brief Revised OpenFlow Library core class.
 *
 * This class provides functionality for creating OpenFlow
 * endpoints for data path elements or controllers. crofbase
 * provides functions for opening listening sockets and
 * creates instances of \see{ cofctl } and \see{ cofdpt }
 * for handling individual sessions.
 */
class crofbase :
	public ciosrv,
	public csocket_owner,
	public cfsm
{
private:

	enum crofbase_flag_t {
		NSP_ENABLED = 0x01,
	};

	std::bitset<32> 					fe_flags;
	std::string 						info;			/**< info string */
	pthread_rwlock_t					xidlock;		/**< rwlock variable for transaction ids
															stored in ta_pending_reqs */

	/*
	 * data structures
	 */

	std::map<uint32_t, uint8_t> 		ta_pending_reqs; 	// list of pending requests
	std::set<uint32_t>	 				xids_used;			// list of recently used xids
	size_t 								xid_used_max; 		// reusing xids: max number of currently blocked xid entries stored
	uint32_t 							xid_start; 			// start value xid
#define CPCP_DEFAULT_XID_USED_MAX       16


protected: // data structures

	std::set<cofctl*>			ofctl_set;		/**< set of active controller connections */
	std::set<cofdpt*>			ofdpt_set;		/**< set of active data path connections */
	cfsptable 					fsptable; 		/**< flowspace registrations table */

	/** \enum crofbase::crofbase_event_t
	 *
	 * events defined by crofbase
	 */
	enum crofbase_event_t {
		CROFBASE_EVENT_WAKEUP	= 1, /**< wakeup event used in method \see{ wakeup } */
	};

	/** \enum crofbase::crofbase_timer_t
	 *
	 * timers defined by crofbase
	 */
	enum crofbase_timer_t {
		TIMER_FE_BASE = (0x0020 << 16),	/**< random number for base timer */
		TIMER_FE_DUMP_OFPACKETS,		/**< dumps periodically all existing cofmsg instances */
		CROFBASE_TIMER_WAKEUP,			/**< timer used for waking up via crofbase::wakeup() */
	};

	/** \enum crofbase::crofbase_rpc_t
	 *
	 * crofbase supports both controller and data path role and is
	 * capable of hosting an arbitrary number of listening sockets
	 * for ctl and dpt role.
	 *
	 * \see rpc
	 */
	enum crofbase_rpc_t { // for cofrpc *rpc[2]; (see below)
		RPC_CTL = 0,	/**< index for std::set<csocket*> in \see{ rpc } for ctls */
		RPC_DPT = 1,	/**< index for std::set<csocket*> in \see{ rpc } for dpts */
	};

	std::set<csocket*>			rpc[2];	/**< two sets of listening sockets for ctl and dpt */


public:


	friend class cport;
	static std::set<crofbase*> rofbases; 		/**< set of all active crofbase instances */


public:

	/**
	 * @name	crofbase
	 * @brief	Constructor for crofbase
	 *
	 * Initializes structures for transaction identifiers. xidlock is the rwlock
	 * for manipulating the transaction id maps. xid_start defines the first
	 * transaction id used by crofbase. xid_start is incremented by one for each new
	 * transaction.
	 *
	 * \see xidlock
	 * \see xid_start
	 */
	crofbase();


	/**
	 * @name	~crofbase
	 * @brief	Destructor for crofbase
	 *
	 * The destructor shuts down all active connections and listening sockets.
	 *
	 */
	virtual
	~crofbase();



	/**
	 * @name	rpc_listen_for_dpts
	 * @brief	Opens a listening socket for accepting connection requests from data path elements
	 *
	 * @param addr Address to bind for listening
	 * @param domain Socket domain
	 * @param type Socket type
	 * @param protocol Socket protocol
	 * @param backlog Maximum number of concurrent pending connection requests
	 */
	void
	rpc_listen_for_dpts(
			caddress const& addr = caddress(AF_INET, "0.0.0.0", 6633),
			int domain = PF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP,
			int backlog = 10);


	/**
	 * @name	rpc_listen_for_ctls
	 * @brief	Opens a listening socket for accepting connection requests from controllers
	 *
	 * @param addr Address to bind for listening
	 * @param domain Socket domain
	 * @param type Socket type
	 * @param protocol Socket protocol
	 * @param backlog Maximum number of concurrent pending connection requests
	 */
	void
	rpc_listen_for_ctls(
			caddress const& addr = caddress(AF_INET, "0.0.0.0", 6644),
			int domain = PF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP,
			int backlog = 10);


	/**
	 * @name 	rpc_connect_to_ctl
	 * @brief	Connects to a remote controller in data path role.
	 *
	 * Establishes a socket connection to a remote controller entity.
	 * When the connection is successfully established, crofbase calls
	 * method crofbase::handle_ctrl_open().
	 *
	 * \see{ handle_ctrl_open() }
	 *
	 * @param ra Address to connect to
	 * @param domain Socket domain (default: PF_INET)
	 * @param type Socket type (default: SOCK_STREAM)
	 * @param protocol Socket protocol (default: IPPROTO_TCP)
	 */
	void
	rpc_connect_to_ctl(
			caddress const& ra,
			int domain = PF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP);


	/**
	 * @name	rpc_disconnect_from_ctl
	 * @brief 	Closes a connection to a controller entity with a proper shutdown.
	 *
	 * \see{ handle_ctrl_close() }
	 *
	 * @param ctl cofctl instance to be disconnected
	 */
	void
	rpc_disconnect_from_ctl(
			cofctl *ctl);



	/**
	 * @name 	rpc_connect_to_dpt
	 * @brief	Connects to a remote data path in controller role.
	 *
	 * Establishes a socket connection to a remote data path element.
	 * When the connection is successfully established, crofbase calls
	 * method crofbase::handle_dpath_open().
	 *
	 * \see{ handle_dpath_open() }
	 *
	 * @param ra Address to connect to
	 * @param domain Socket domain (default: PF_INET)
	 * @param type Socket type (default: SOCK_STREAM)
	 * @param protocol Socket protocol (default: IPPROTO_TCP)
	 */
	void
	rpc_connect_to_dpt(
			caddress const& ra,
			int domain = PF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP);


	/**
	 * @name	rpc_disconnect_from_dpt
	 * @brief 	Closes a connection to a data path entity with a proper shutdown.
	 *
	 * \see{ handle_dpath_close() }
	 *
	 * @param dpt cofdpt instance to be disconnected
	 */
	void
	rpc_disconnect_from_dpt(
			cofdpt *dpath);


	/**
	 * @name	rpc_close_all
	 * @brief	Closes all open cofctl, cofdpt and listening socket instances.
	 *
	 */
	void
	rpc_close_all();


	/**
	 * @name	wakeup
	 * @brief	Method for waking up this thread from another thread.
	 *
	 */
	void
	wakeup();


	// cofdpt related methods
	//

	/** find cofswitch instance
	 */
	cofdpt*
	dpt_find(
		uint64_t dpid) throw (eRofBaseNotFound);

	cofdpt*
	dpt_find(
		std::string s_dpid) throw (eRofBaseNotFound);

	cofdpt*
	dpt_find(
		cmacaddr dl_dpid) throw (eRofBaseNotFound);

	cofdpt*
	dpt_find(
			cofdpt* dpt) throw (eRofBaseNotFound);


	// COFCTRL related methods
	//


	/** find cofctrl instance
	 */
	cofctl*
	ctl_find(
			cofctl* entity) throw (eRofBaseNotFound);




	/** dump info string
	 */
	virtual const char*
	c_str();


	/** enable/disable namespace support
	 *
	 */
	void
	nsp_enable(bool enable = true);








protected:


	/**
	 * @name	cofctl_factory
	 * @brief	creates a new cofctl instance for an existing socket with sockfd newsd.
	 *
	 * This method constructs a new instance of class cofctl for managing a single connection
	 * to a controller. This class is supposed to be overwritten, if a class derived from crofbase
	 * intends to overwrite cofctl and add additional functionality. When the initial HELLO message
	 * exchange in OpenFlow succeeds, method crofbase::handle_ctrl_open() will be called.
	 *
	 * @param owner Pointer to this crofbase instance for callbacks used by the cofctl instance
	 * @param newsd socket descriptor of new created socket for cofctl instance
	 * @param ra Remote address of peer entity connected via socket referenced by newsd
	 * @param domain socket domain (see man 2 socket for details)
	 * @param type socket type (see man 2 socket for details)
	 * @param protocol socket protocol (see man 2 socket for details)
	 *
	 */
	virtual cofctl*
	cofctl_factory(
			crofbase* owner,
			int newsd,
			caddress const& ra,
			int domain,
			int type,
			int protocol);


	/**
	 * @name	cofctl_factory
	 * @brief	Creates a new cofctl instance and tries to connect to a remote controller entity.
	 *
	 * This method constructs a new instance of class cofctl for actively establishing a single connection
	 * to a controller. This class is supposed to be overwritten, if a class derived from crofbase
	 * intends to overwrite cofctl and add additional functionality. cofctl will indefinitely attempt
	 * to connect to the peer entity unless it is removed by calling crofbase::rpf_disconnect_from_ctl().
	 * When connection setup and the initial HELLO message exchange in OpenFlow succeeds, method
	 * crofbase::handle_ctrl_open() will be called.
	 *
	 * @param owner Pointer to this crofbase instance for callbacks used by the cofctl instance
	 * @param ra Remote address to connect to
	 * @param domain socket domain (see man 2 socket for details)
	 * @param type socket type (see man 2 socket for details)
	 * @param protocol socket protocol (see man 2 socket for details)
	 *
	 */
	virtual cofctl*
	cofctl_factory(
			crofbase* owner,
			caddress const& ra,
			int domain,
			int type,
			int protocol);


	/**
	 * @name	cofdpt_factory
	 * @brief	creates a new cofdpt instance for an existing socket with sockfd newsd.
	 *
	 * This method constructs a new instance of class cofdpt for managing a single connection
	 * to a data path element. This class is supposed to be overwritten, if a class derived from crofbase
	 * intends to overwrite cofdpt and add additional functionality. When the initial handshake in OpenFlow
	 * succeeds (FEATURES.request/reply, GET-CONFIG.request/reply, TABLE-STATS.request/reply), method
	 * crofbase::handle_ctrl_open() will be called.
	 *
	 * @param owner Pointer to this crofbase instance for callbacks used by the cofdpt instance
	 * @param newsd socket descriptor of new created socket for cofdpt instance
	 * @param ra Remote address of peer entity connected via socket referenced by newsd
	 * @param domain socket domain (see man 2 socket for details)
	 * @param type socket type (see man 2 socket for details)
	 * @param protocol socket protocol (see man 2 socket for details)
	 *
	 */
	virtual cofdpt*
	cofdpt_factory(
			crofbase* owner,
			int newsd,
			caddress const& ra,
			int domain,
			int type,
			int protocol);


	/**
	 * @name	cofdpt_factory
	 * @brief	Creates a new cofdpt instance and tries to connect to a remote data path element.
	 *
	 * This method constructs a new instance of class cofdpt for actively establishing a single connection
	 * to a data path element. This class is supposed to be overwritten, if a class derived from crofbase
	 * intends to overwrite cofdpt and add additional functionality. cofdpt will indefinitely attempt
	 * to connect to the peer entity unless it is removed by calling crofbase::rpf_disconnect_from_dpt().
	 * When the initial handshake in OpenFlow succeeds (FEATURES.request/reply, GET-CONFIG.request/reply,
	 * TABLE-STATS.request/reply), method crofbase::handle_dpath_open() will be called.
	 *
	 * @param owner Pointer to this crofbase instance for callbacks used by the cofdpt instance
	 * @param ra Remote address to connect to
	 * @param domain socket domain (see man 2 socket for details)
	 * @param type socket type (see man 2 socket for details)
	 * @param protocol socket protocol (see man 2 socket for details)
	 *
	 */
	virtual cofdpt*
	cofdpt_factory(
			crofbase* owner,
			caddress const& ra,
			int domain,
			int type,
			int protocol);



protected:

	/*
	 * The following methods should be overwritten by a derived class
	 * in order to get reception notifications for the various OF
	 * packets. While crofbase handles most of the lower layer details,
	 * a derived class must provide higher layer functionality.
	 */

	/**
	 * @name	handle_features_request
	 * @brief	Called once a FEATURES.request message was received from a controller entity.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param ctl Pointer to cofctl instance from which the FEATURES.request was received
	 * @param msg Pointer to cofmsg_features_request message containing the received message
	 */
	virtual void
	handle_features_request(cofctl *ctl, cofmsg_features_request *msg) { delete msg; };


	/**
	 * @name	handle_features_reply
	 * @brief	Called once a FEATURES.reply message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt Pointer to cofdpt instance from which the FEATURES.reply was received
	 * @param msg Pointer to cofmsg_features_reply message containing the received message
	 */
	virtual void
	handle_features_reply(cofdpt *dpt, cofmsg_features_reply *msg) { delete msg; };



	/** Handle OF features reply timeout. To be overwritten by derived class.
	 *
	 * This method is called when TIMER_SEND_FEATURES_REPLY expires, i.e.
	 * the FEATURES.reply of a datapath element was lost. This method
	 * should be overwritten by a derived class.
	 *
	 * @param sw The cofswitch instance holding all parameters from the attached
	 * datapath.
	 */
	virtual void
	handle_features_reply_timeout(cofdpt *dpt);


	/** Handle OF get-config request. To be overwritten by derived class.
	 *
	 * Called from within crofbase::fe_down_get_config_request().
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param ctrl cofdpath instance from whom the GET-CONFIG.request was received.
	 * @pack OF GET-CONFIG.request packet received from controller
	 */
	virtual void
	handle_get_config_request(cofctl *ctl, cofmsg_get_config_request *msg) { delete msg; };

	/** Handle OF get-config reply. To be overwritten by derived class.
	 *
	 * Called from within crofbase::fe_up_get_config_reply().
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom the GET-CONFIG.reply was received.
	 * @pack OF GET-CONFIG.reply packet received from datapath
	 */
	virtual void
	handle_get_config_reply(cofdpt *dpt, cofmsg_get_config_reply *msg) { delete msg; };

	/** Handle OF get-config reply timeout. To be overwritten by derived class.
	 *
	 * Called upon expiration of TIMER_FE_SEND_GET_CONFIG_REPLY.
	 *
	 * @param sw cotswitch instance from whom a GET-CONFIG.reply was expected.
	 */
	virtual void
	handle_get_config_reply_timeout(cofdpt *dpt);

	/** Handle OF stats request. NOT to be overwritten by derived class.
	 *
	 * Called upon reception of a STATS.request from the controlling entity.
	 *
	 * @param pack STATS.request packet received from controller.
	 */
	void
	handle_stats_request(cofctl *ctl, cofmsg_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_desc_stats_request(cofctl *ctl, cofmsg_desc_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_table_stats_request(cofctl *ctl, cofmsg_table_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_port_stats_request(cofctl *ctl, cofmsg_port_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_flow_stats_request(cofctl *ctl, cofmsg_flow_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_aggregate_stats_request(cofctl *ctl, cofmsg_aggr_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_queue_stats_request(cofctl *ctl, cofmsg_queue_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_group_stats_request(cofctl *ctl, cofmsg_group_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_group_desc_stats_request(cofctl *ctl, cofmsg_group_desc_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_group_features_stats_request(cofctl *ctl, cofmsg_group_features_stats_request *msg) { throw eBadRequestBadStat(); };


	/**
	 *
	 */
	virtual void
	handle_experimenter_stats_request(cofctl *ctl, cofmsg_stats_request *msg) { throw eBadRequestBadStat(); };


	/** Handle OF stats reply. To be overwritten by derived class.
	 *
	 * Called upon reception of a STATS.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a STATS.reply was received
	 * @param pack STATS.reply packet received from datapath
	 */
	virtual void
	handle_stats_reply(cofdpt *dpt, cofmsg_stats_reply *msg) { delete msg; };

	/** Handle OF stats reply timeout. To be overwritten by derived class.
	 *
	 * Called upon expiration of TIMER_FE_SEND_STATS_REPLY.
	 *
	 * @param sw cotswitch instance from whom a GET-CONFIG.reply was expected.
	 */
	virtual void
	handle_stats_reply_timeout(cofdpt *dpt, uint32_t xid) {};


	/**
	 *
	 */
	virtual void
	handle_desc_stats_reply(cofdpt *dpt, cofmsg_desc_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_table_stats_reply(cofdpt *dpt, cofmsg_table_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_port_stats_reply(cofdpt *dpt, cofmsg_port_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_flow_stats_reply(cofdpt *dpt, cofmsg_flow_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_aggregate_stats_reply(cofdpt *dpt, cofmsg_aggr_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_queue_stats_reply(cofdpt *dpt, cofmsg_queue_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_group_stats_reply(cofdpt *dpt, cofmsg_group_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_group_desc_stats_reply(cofdpt *dpt, cofmsg_group_desc_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_group_features_stats_reply(cofdpt *dpt, cofmsg_group_features_stats_reply *msg) { delete msg; };


	/**
	 *
	 */
	virtual void
	handle_experimenter_stats_reply(cofdpt *dpt, cofmsg_stats_reply *msg) { delete msg; };


	/** Handle OF packet-out messages. To be overwritten by derived class.
	 *
	 * Called upon reception of a PACKET-OUT.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack PACKET-OUT.message packet received from controller.
	 */
	virtual void
	handle_packet_out(cofctl *ctl, cofmsg_packet_out *msg) { delete msg; };

	/** Handle OF packet-in messages. To be overwritten by derived class.
	 *
	 * Called upon reception of a PACKET-IN.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a PACKET-IN.message was received
	 * @param pack PACKET-IN.message packet received from datapath
	 */
	virtual void
	handle_packet_in(cofdpt *dpt, cofmsg_packet_in *msg) { delete msg; };

	/** Handle OF barrier request. To be overwritten by derived class.
	 *
	 * Called upon reception of a BARRIER.request from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack BARRIER.request packet received from controller.
	 */
	virtual void
	handle_barrier_request(cofctl *ctl, cofmsg_barrier_request *msg) { delete msg; };

	/** Handle OF barrier reply. To be overwritten by derived class.
	 *
	 * Called upon reception of a BARRIER.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a BARRIER.reply was received
	 * @param pack BARRIER.reply packet received from datapath
	 */
	virtual void
	handle_barrier_reply(cofdpt *dpt, cofmsg_barrier_request *msg) { delete msg; };

	/** Handle OF barrier reply timeout. To be overwritten by derived class.
	 *
	 * Called upon expiration of TIMER_FE_SEND_BARRIER_REPLY.
	 *
	 * @param sw cotswitch instance from whom a BARRIER.reply was expected.
	 */
	virtual void
	handle_barrier_reply_timeout(cofdpt *dpt, uint32_t xid) {};

	/** Handle OF error message. To be overwritten by derived class.
	 *
	 * Called upon reception of an ERROR.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom an ERROR.message was received
	 * @param pack ERROR.message packet received from datapath
	 */
	virtual void
	handle_error(cofdpt *dpt, cofmsg_error *msg) { delete msg; };

	/** Handle OF flow-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a FLOW-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack FLOW-MOD.message packet received from controller.
	 */
	virtual void
	handle_flow_mod(cofctl *ctl, cofmsg_flow_mod *msg) { delete msg; };

	/** Handle OF group-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a GROUP-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack GROUP-MOD.message packet received from controller.
	 */
	virtual void
	handle_group_mod(cofctl *ctl, cofmsg_group_mod *msg) { delete msg; };

	/** Handle OF table-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a TABLE-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack TABLE-MOD.message packet received from controller.
	 */
	virtual void
	handle_table_mod(cofctl *ctl, cofmsg_table_mod *msg) { delete msg; };

	/** Handle OF port-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a PORT-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack PORT-MOD.message packet received from controller.
	 */
	virtual void
	handle_port_mod(cofctl *ctl, cofmsg_port_mod *msg) { delete msg; };

	/** Handle OF flow-removed message. To be overwritten by derived class.
	 *
	 * Called upon reception of a FLOW-REMOVED.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a FLOW-REMOVED.message was received
	 * @param pack FLOW-REMOVED.message packet received from datapath
	 */
	virtual void
	handle_flow_removed(cofdpt *dpt, cofmsg_flow_removed *msg) { delete msg; };

	/** Handle OF port-status message. To be overwritten by derived class.
	 *
	 * Called upon reception of a PORT-STATUS.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a PORT-STATUS.message was received
	 * @param pack PORT-STATUS.message packet received from datapath
	 */
	virtual void
	handle_port_status(cofdpt *dpt, cofmsg_port_status *msg) { delete msg; };

	/** Handle OF set-config message. To be overwritten by derived class.
	 *
	 * Called upon reception of a SET-CONFIG.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack SET-CONFIG.message packet received from controller.
	 */
	virtual void
	handle_set_config(cofctl *ctl, cofmsg_set_config *msg) { delete msg; };

	/** Handle OF queue-get-config request. To be overwritten by derived class.
 	 *
	 * Called upon reception of a QUEUE-GET-CONFIG.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a QUEUE-GET-CONFIG.reply was received
	 * @param pack QUEUE-GET-CONFIG.reply packet received from datapath
	 */
	virtual void
	handle_queue_get_config_request(cofctl *ctl, cofmsg_queue_get_config_request *msg) { delete msg; };

	/** Handle OF queue-get-config reply. To be overwritten by derived class.
 	 *
	 * Called upon reception of a QUEUE-GET-CONFIG.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a QUEUE-GET-CONFIG.reply was received
	 * @param pack QUEUE-GET-CONFIG.reply packet received from datapath
	 */
	virtual void
	handle_queue_get_config_reply(cofdpt *dpt, cofmsg_queue_get_config_reply *msg) { delete msg; };

	/** Handle OF experimenter message. To be overwritten by derived class.
	 *
	 * Called upon reception of a VENDOR.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack VENDOR.message packet received from controller.
	 */
	virtual void
	handle_experimenter_message(cofctl *ctl, cofmsg_experimenter *msg);

	/** Handle OF experimenter message. To be overwritten by derived class.
	 *
	 * Called upon reception of a VENDOR.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a VENDOR.message was received
	 * @param pack VENDOR.message packet received from datapath
	 */
	virtual void
	handle_experimenter_message(cofdpt *dpt, cofmsg_experimenter *msg) { delete msg; };

	/** Handle new dpath
	 *
	 * Called upon creation of a new cofswitch instance.
	 *
	 * @param sw new cofswitch instance
	 */
	virtual void
	handle_dpath_open(cofdpt *dpt) {};

	/** Handle close event on dpath
	 *
	 * Called upon deletion of a cofswitch instance
	 *
	 * @param sw cofswitch instance to be deleted
	 */
	virtual void
	handle_dpath_close(cofdpt *dpt) {};

	/** Handle new ctrl
	 *
	 * Called upon creation of a new cofctrl instance.
	 *
	 * @param ctrl new cofctrl instance
	 */
	virtual void
	handle_ctrl_open(cofctl *ctl) {};

	/** Handle close event on ctrl
	 *
	 * Called upon deletion of a cofctrl instance
	 *
	 * @param ctrl cofctrl instance to be deleted
	 */
	virtual void
	handle_ctrl_close(cofctl *ctl) {};

	/** Handle timeout for GET-FSP request
	 *
	 */
	virtual void
	handle_get_fsp_reply_timeout(cofdpt *dpt) {};

	/** Handle OF role request. To be overwritten by derived class.
	 *
	 * Called upon reception of a ROLE.request from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack ROLE.request packet received from controller.
	 */
	virtual void
	handle_role_request(cofctl *ctl, cofmsg_role_request *msg) { delete msg; };

	/** Handle OF role reply. To be overwritten by derived class.
	 *
	 * Called upon reception of a ROLE.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a ROLE.reply was received
	 * @param pack ROLE.reply packet received from datapath
	 */
	virtual void
	handle_role_reply(cofdpt *dpt, cofmsg_role_reply *msg) { delete msg; };

	/** Handle OF role reply timeout. To be overwritten by derived class.
	 *
	 * Called upon expiration of TIMER_FE_SEND_ROLE_REPLY.
	 *
	 * @param sw cotswitch instance from whom a ROLE.reply was expected.
	 */
	virtual void
	handle_role_reply_timeout(cofdpt *dpt) {};





	// overloaded from ciosrv

	/** Handle timeout events. This method is overwritten from ciosrv.
	 *
	 * @param opaque expired timer type
	 */
	virtual void
	handle_timeout(
		int opaque);


	/**
	 * Handle cioctl events received
	 */
	virtual void
	handle_event(cevent const& ev);


public:

	// allow class cadaptor access to these methods
	friend class chandler;

	friend class bcm_port;
	// allow class cgttable access to error method
	friend class cgttable;

	/* These methods may be called by a derived class to send
	 * a specific OF packet.
	 *
	 */

	// HELLO messages
	//

	/** Send a OF HELLO.message to data path.
	 *
	 */
	virtual void
	send_hello_message(
			cofdpt *dpt,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);

	/** Send a OF HELLO.message to controller.
	 *
	 */
	virtual void
	send_hello_message(
			cofctl *ctl,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);

	// ECHO request/reply
	//

	/**
	 *
	 */
	virtual void
	send_echo_request(
			cofdpt *dpt,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);


	/**
	 *
	 */
	virtual void
	send_echo_reply(
			cofdpt *dpt,
			uint32_t xid,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);


	/**
	 *
	 */
	virtual void
	send_echo_request(
			cofctl *ctl,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);


	/**
	 *
	 */
	virtual void
	send_echo_reply(
			cofctl *ctl,
			uint32_t xid,
			uint8_t *body = (uint8_t*)0, size_t bodylen = 0);


	// FEATURES request/reply
	//

	/** Send OF FEATURES.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 */
	virtual void
	send_features_request(
		cofdpt *sw);

	/** Send OF FEATURES.reply to controlling entity.
	 *
	 */
	virtual void
	send_features_reply(
			cofctl *ctl,
			uint32_t xid,
			uint64_t dpid,
			uint32_t n_buffers,
			uint8_t n_tables,
			uint32_t capabilities,
			uint8_t of13_auxiliary_id = 0,
			uint32_t of10_actions_bitmap = 0,
			cofportlist const& portlist = cofportlist());

	// GET-CONFIG request/reply
	//

	/** Send OF GET-CONFIG.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 */
	virtual void
	send_get_config_request(
		cofdpt *sw);

	/** Send OF GET-CONFIG.reply to controlling entity.
	 *
	 */
	virtual void
	send_get_config_reply(
			cofctl *ctl,
			uint32_t xid,
			uint16_t flags,
			uint16_t miss_send_len);

	// STATS request/reply
	//

	/** Send OF STATS.request to data path entity.
	 *
	 * @param dpt cofswitch instance representing the data path to be addressed
	 * @param type One of the OFPST_* constants
	 * @param flags OFPSF_REQ_* flags
	 * @param body Body of a STATS request
	 * @param bodylen length of STATS request body
	 */
	virtual uint32_t
	send_stats_request(
		cofdpt *dpt,
		uint16_t stats_type,
		uint16_t stats_flags,
		uint8_t *body = NULL,
		size_t bodylen = 0);

	/** Send OF FLOW STATS.request to data path entity.
	 *
	 * @param dpt cofswitch instance representing the data path to be addressed
	 * @param type One of the OFPST_* constants
	 * @param flags OFPSF_REQ_* flags
	 * @param body Body of a STATS request
	 * @param bodylen length of STATS request body
	 */
	virtual uint32_t
	send_flow_stats_request(
		cofdpt *dpt,
		uint16_t flags,
		cofflow_stats_request const& flow_stats_request);

	/** Send OF AGGR STATS.request to data path entity.
	 *
	 * @param dpt cofswitch instance representing the data path to be addressed
	 * @param type One of the OFPST_* constants
	 * @param flags OFPSF_REQ_* flags
	 * @param body Body of a STATS request
	 * @param bodylen length of STATS request body
	 */
	virtual uint32_t
	send_aggr_stats_request(
		cofdpt *dpt,
		uint16_t flags,
		cofaggr_stats_request const& aggr_stats_request);



private:

	/** Helper method for handling DESCription STATS.requests.
	 * Only used within crofbase internally.
	 */
	void
	send_stats_reply_local();

public:

	/** Send OF STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		uint16_t stats_type,
		uint8_t *body = NULL,
		size_t bodylen = 0,
		bool more = false);

	/** Send OF DESC STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_desc_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		cofdesc_stats_reply const& desc_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_table_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		std::vector<coftable_stats_reply> const& table_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_port_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		std::vector<cofport_stats_reply> const& port_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_flow_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		std::vector<cofflow_stats_reply> const& flow_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_aggr_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		cofaggr_stats_reply const& aggr_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_group_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		std::vector<cofgroup_stats_reply> const& group_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_group_desc_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		std::vector<cofgroup_desc_stats_reply> const& group_desc_stats,
		bool more = false);

	/** Send OF TABLE STATS.reply to controlling entity.
	 *
	 * @param xid transaction ID used in STATS request
	 * @param stats_type One of the OFPST_* constants
	 * @param body Body of a STATS reply
	 * @param bodylen length of STATS reply body
	 * @param more flag if multiple STATS replies will be sent
	 */
	virtual void
	send_group_features_stats_reply(
		cofctl *ctl,
		uint32_t xid,
		cofgroup_features_stats_reply const& group_features_stats,
		bool more = false);

	// PACKET-OUT message
	//

	/** Send OF PACKET-OUT.message to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param buffer_id buffer ID assigned by datapath (-1 if none) in host byte order
	 * @param in_port Packet’s input port (OFPP_NONE if none) in host byte order
	 * @param acvec vector containing cofaction instances
	 * @param data optional packet data to be sent out
	 * @param datalen
	 */
	virtual void
	send_packet_out_message(
		cofdpt *sw,
		uint32_t buffer_id,
		uint32_t in_port,
		cofaclist& aclist,
		uint8_t *data = NULL,
		size_t datalen = 0);


	// PACKET-IN message
	//

	/** Send OF PACKET-IN.message to controlling entity.
	 *
	 * @param buffer_id buffer ID assigned by datapath in host byte order
	 * @param total_len Full length of frame
	 * @param in_port Port on which frame was received
	 * @param reason Reason packet is being sent (one of OFPR_*)
	 * @param data Ethernet frame
	 * @param datalen Ethernet frame length
	 */
	virtual void
	send_packet_in_message(
		uint32_t buffer_id,
		uint16_t total_len,
		uint8_t reason,
		uint8_t table_id,
		uint64_t cookie,
		uint16_t in_port, // for OF1.0
		cofmatch &match,
		uint8_t *data,
		size_t datalen) throw(eRofBaseNoCtrl);

	// BARRIER request/reply
	//

	/** Send OF BARRIER.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 */
	virtual uint32_t
	send_barrier_request(
		cofdpt *sw);

	/** Send OF BARRIER.reply to controlling entity.
	 *
	 */
	virtual void
	send_barrier_reply(
			cofctl *ctl,
			uint32_t xid);

	// ERROR message
	//

	/** Send OF ERROR.message to controlling entity.
	 *
	 * @param type One of OFPET_*
	 * @param code error code
	 * @param data failed request
	 * @param datalen length of failed request
	 */
	virtual void
	send_error_message(
		cofctl *ctl,
		uint32_t xid,
		uint16_t type,
		uint16_t code,
		uint8_t* data = NULL,
		size_t datalen = 0);

	/** Send OF ERROR.message to datapath entity.
	 *
	 * @param type One of OFPET_*
	 * @param code error code
	 * @param data failed request
	 * @param datalen length of failed request
	 */
	virtual void
	send_error_message(
		cofdpt *dpt,
		uint32_t xid,
		uint16_t type,
		uint16_t code,
		uint8_t* data = NULL,
		size_t datalen = 0);

	// FLOW-MOD message
	//

	/** Send OF FLOW-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param cookie Opaque controller-issued identifier
	 * @param command One of OFPFC_*
	 * @param idle_timeout Idle time before discarding (seconds)
	 * @param hard_timeout Max time before discarding (seconds)
	 * @param priority Priority level of flow entry
	 * @param buffer_id Buffered packet to apply to (or -1)
	 * @param out_port For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction
	 * @param flags One of OFPFF_*
	 * @param acvec Vector of cofaction instances
	 */
	virtual void
	send_flow_mod_message(
		cofdpt *sw,
		cofmatch& ofmatch,
		uint64_t cookie,
		uint64_t cookie_mask,
		uint8_t table_id,
		uint8_t command,
		uint16_t idle_timeout,
		uint16_t hard_timeout,
		uint16_t priority,
		uint32_t buffer_id,
		uint32_t out_port,
		uint32_t out_group,
		uint16_t flags,
		cofinlist& inlist);

	virtual void
	send_flow_mod_message(
			cofdpt *sw,
			cflowentry& flowentry);

	// GROUP-MOD message
	//

	/** Send OF GROUP-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param cookie Opaque controller-issued identifier
	 * @param command One of OFPFC_*
	 * @param idle_timeout Idle time before discarding (seconds)
	 * @param hard_timeout Max time before discarding (seconds)
	 * @param priority Priority level of flow entry
	 * @param buffer_id Buffered packet to apply to (or -1)
	 * @param out_port For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction
	 * @param flags One of OFPFF_*
	 * @param acvec Vector of cofaction instances
	 */
#if 0
	void
	send_group_mod_message(
		cofdpt *sw,
		uint16_t command,
		uint8_t type,
		uint32_t group_id,
		std::vector<cofbucket*>& buckets);
#endif

	virtual void
	send_group_mod_message(
			cofdpt *sw,
			cgroupentry& groupentry);

	// TABLE-MOD message
	//

	/** Send OF GROUP-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param table_id the table id according to OF1.1
	 * @param config table configuration, see openflow.h for OFPTC_TABLE_*
	 */
	virtual void
	send_table_mod_message(
		cofdpt *sw,
		uint8_t table_id,
		uint32_t config);

	// PORT-MOD message
	//

	/** Send OF PORT-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param port_no Number of port to modify
	 * @param hw_addr Memory area containing the ports hardware address
	 * @param hw_addr_len Length of memory area
	 * @param config Bitmap of OFPPC_* flags
	 * @param mask Bitmap of OFPPC_* flags to be changed
	 * @param advertise Bitmap of "ofp_port_features"s. Zero all bits to prevent any action taking place.
	 */
	virtual void
	send_port_mod_message(
		cofdpt *sw,
		uint32_t port_no,
		cmacaddr const& hwaddr,
		uint32_t config,
		uint32_t mask,
		uint32_t advertise);

	// FLOW-REMOVED message
	//

	/** Send OF FLOW-REMOVED.message to controlling entity.
	 *
	 * All values in host byte order.
	 *
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param cookie Opaque controller-issued identifier
	 * @param priority Priority level of flow entry
	 * @param reason One of OFPRR_*
	 * @param duration_sec Time flow was alive in seconds
	 * @param duration_nsec Time flow was alive in nanoseconds beyond duration_sec
	 * @param idle_timeout Idle timeout from original flow mod
	 * @param packet_count number of packets handled by this flow mod
	 * @param byte_count number of bytes handled by this flow mod
	 */
	virtual void
	send_flow_removed_message(
		cofctl *ctl,
		cofmatch& ofmatch,
		uint64_t cookie,
		uint16_t priority,
		uint8_t reason,
		uint8_t table_id,
		uint32_t duration_sec,
		uint32_t duration_nsec,
		uint16_t idle_timeout,
		uint16_t hard_timeout,
		uint64_t packet_count,
		uint64_t byte_count);

	// PORT-STATUS message
	//

	/** Send OF PORT-STATUS.message to controlling entity.
	 *
	 * @param reason One of OFPPR_*
	 * @param port cofport instance that changed its status
	 */
	virtual void
	send_port_status_message(
		uint8_t reason,
		cofport const& port);

	// SET-CONFIG message
	//

	/** Send OF SET-CONFIG.message to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param flags OFPC_* flags
	 * @param miss_send_len Max bytes of new flow that datapath should send to the controller
	 */
	virtual void
	send_set_config_message(
		cofdpt *sw,
		uint16_t flags,
		uint16_t miss_send_len);

	// QUEUE-GET-CONFIG request/reply
	//

	/** Send OF QUEUE-GET-CONFIG.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param port Port to be queried. Should refer to a valid physical port (i.e. < OFPP_MAX)
	 */
	virtual void
	send_queue_get_config_request(
		cofdpt *sw,
		uint32_t port);

	/** Send OF QUEUE-GET-CONFIG.reply to controlling entity.
	 *
	 */
	virtual void
	send_queue_get_config_reply(
			cofctl *ctl,
			uint32_t xid,
			uint32_t portno);

	/** Send OF experimenter message to data path
	 *
	 */
	virtual void
	send_experimenter_message(
			cofdpt *sw,
			uint32_t experimenter_id,
			uint32_t exp_type,
			uint8_t *body = NULL,
			size_t bodylen = 0);

	/** Send OF experimenter message to controller
	 *
	 */
	virtual void
	send_experimenter_message(
			cofctl *ctrl,
			uint32_t experimenter_id,
			uint32_t exp_type,
			uint8_t *body = NULL,
			size_t bodylen = 0);


	// ROLE request/reply
	//

	/** Send OF ROLE.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 */
	virtual void
	send_role_request(
		cofdpt *sw,
		uint32_t role,
		uint64_t generation_id);

	/** Send OF ROLE.reply to controlling entity.
	 *
	 */
	virtual void
	send_role_reply(
			cofctl *ctl,
			uint32_t xid,
			uint32_t role,
			uint64_t generation_id);




public:


	/**
	 *
	 */
	virtual void
	handle_accepted(
			csocket *socket,
			int newsd,
			caddress const& ra);


	/**
	 *
	 */
	virtual void
	handle_connected(
			csocket *socket,
			int sd);


	/**
	 *
	 */
	virtual void
	handle_connect_refused(
			csocket *socket,
			int sd);


	/**
	 *
	 */
	virtual void
	handle_read(
			csocket *socket,
			int sd);


	/**
	 *
	 */
	virtual void
	handle_closed(
			csocket *socket,
			int sd);




protected:

	// allow class cofswitch access to these methods
	friend class cofdpt;
	// allow class cofctrl access to these methods
	friend class cofctl;

	/** for use by cofdpt
	 *
	 */
	void
	handle_dpt_open(cofdpt *dpt);


	/** for use by cofdpt
	 *
	 */
	void
	handle_dpt_close(cofdpt *dpt);


	/** for use by cofctl
	 *
	 */
	void
	handle_ctl_open(cofctl *ctl);


	/** for use by cofctl
	 *
	 */
	void
	handle_ctl_close(cofctl *ctl);


protected:


	/** add pending request to transaction queue
	 * - allocates new xid not in xid_used
	 * - adds xid to xid_used
	 * - adds pair(type, xid) to ta_pending_requests
	 */
	uint32_t
	ta_add_request(
			uint8_t type);


	/** remove pending request from transaction queue
	 */
	void
	ta_rem_request(
			uint32_t xid);


	/** return boolean flag for pending request of type x
	 */
	bool
	ta_pending(
			uint32_t xid,
			uint8_t type);


	/** return new xid for asynchronous calls
	 * - adds xid to xid_used
	 * - does not add xid to ta_pending_requests
	 */
	uint32_t
	ta_new_async_xid();


	/** validate incoming reply for transaction
	 * checks for existing type and associated xid
	 * removes request from ta_pending_reqs, if found
	 */
	bool
	ta_validate(
			uint32_t xid,
			uint8_t type) throw (eRofBaseXidInval);


	/** validate a cofpacket, calls ta_validate(xid, type)
	 */
	bool
	ta_validate(
			cofmsg *pack);


	/** returns true if a xid is used by a pending
	 * transaction
	 */
	bool
	ta_active_xid(
			uint32_t xid);


private: // methods for attaching/detaching other cofbase instances

	/** attach data path
	 */
	void
	dpath_attach(cofbase* dp);

	/** detach data path
	 */
	void
	dpath_detach(cofbase* dp);

	/** attach controlling entity
	 */
	void
	ctrl_attach(cofbase* dp) throw (eRofBaseFspSupportDisabled);

	/** detach controlling entity
	 */
	void
	ctrl_detach(cofbase* dp);




};

}; // end of namespace

#endif


