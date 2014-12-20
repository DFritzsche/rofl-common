/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CROFDPT_H
#define CROFDPT_H 1

#include <map>
#include <set>
#include <algorithm>
#include <inttypes.h>
#include <stdio.h>
#include <strings.h>

#include "rofl/common/croflexception.h"
#include "rofl/common/csocket.h"
#include "rofl/common/cmemory.h"
#include "rofl/common/cdptid.h"
#include "rofl/common/openflow/cofports.h"
#include "rofl/common/openflow/coftables.h"
#include "rofl/common/openflow/openflow.h"
#include "rofl/common/openflow/messages/cofmsg.h"
#include "rofl/common/openflow/cofflowmod.h"
#include "rofl/common/openflow/cofgroupmod.h"
#include "rofl/common/crofchan.h"
#include "rofl/common/openflow/cofhelloelemversionbitmap.h"
#include "rofl/common/openflow/cofasyncconfig.h"
#include "rofl/common/openflow/cofrole.h"
#include "rofl/common/openflow/cofmeterstats.h"
#include "rofl/common/openflow/cofmeterconfig.h"
#include "rofl/common/openflow/cofmeterfeatures.h"
#include "rofl/common/cauxid.h"
#include "rofl/common/cdpid.h"

#include "rofl/common/ciosrv.h"
#include "rofl/common/cmemory.h"
#include "rofl/common/protocols/fetherframe.h"
#include "rofl/common/logging.h"
#include "rofl/common/crofdpt.h"
#include "rofl/common/openflow/cofhelloelemversionbitmap.h"
#include "rofl/common/crofchan.h"
#include "rofl/common/ctransactions.h"
#include "rofl/common/openflow/cofpacketqueue.h"
#include "rofl/common/openflow/cofmeterbands.h"


namespace rofl {

/* error classes */
class eRofDptBase 					: public RoflException {};
class eRofDptNotFound 				: public eRofDptBase {};

class crofdpt;

class crofdpt_env {
	friend class crofdpt;
	static std::set<crofdpt_env*> rofdpt_envs;
public:

	crofdpt_env()
	{ crofdpt_env::rofdpt_envs.insert(this); };

	virtual
	~crofdpt_env()
	{ crofdpt_env::rofdpt_envs.erase(this); };

public:

	/**
	 *
	 */
	virtual void
	handle_dpt_attached(crofdpt& dpt)
	{};

	/**
	 *
	 */
	virtual void
	handle_dpt_detached(crofdpt& dpt)
	{};

	/**
	 * @brief Called upon establishment of a control connection within the control channel
	 */
	virtual void
	handle_conn_established(
			crofdpt& dpt,
			const rofl::cauxid& auxid)
	{};

	/**
	 * @brief Called upon a peer initiated termination of a control connection within the control channel
	 */
	virtual void
	handle_conn_terminated(
			crofdpt& dpt,
			const rofl::cauxid& auxid)
	{};

	/**
	 * @brief Called in the event of a connection refused
	 */
	virtual void
	handle_conn_refused(
			crofdpt& dpt,
			const rofl::cauxid& auxid)
	{};

	/**
	 * @brief Called in the event of a connection failed (except refused)
	 */
	virtual void
	handle_conn_failed(
			crofdpt& dpt,
			const rofl::cauxid& auxid)
	{};

public:

	/**
	 * @name Event handlers
	 *
	 * The following methods should be overwritten by a derived class
	 * in order to get reception notifications for the various OF
	 * packets. While crofbase handles most of the lower layer details,
	 * a derived class must provide higher layer functionality.
	 */

	/**@{*/

	/**
	 *
	 * @param ctl
	 * @param auxid
	 */
	virtual void
	handle_write(rofl::crofdpt& dpt, const rofl::cauxid& auxid) {};

	/**
	 * @brief	Called once a FEATURES.reply message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt Pointer to cofdpt instance from which the FEATURES.reply was received
	 * @param msg Pointer to rofl::openflow::cofmsg_features_reply message containing the received message
	 */
	virtual void
	handle_features_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_features_reply& msg) {};

	/**
	 * @brief	Called once a timer expires for a FEATURES.reply message.
	 *
	 * Default behaviour: deletes cofdpt instance, thus effectively closing the control connection.
	 *
	 * @param dpt pointer to cofdpt instance
	 */
	virtual void
	handle_features_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a GET-CONFIG.reply message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt Pointer to cofdpt instance from which the GET-CONFIG.reply was received
	 * @param msg Pointer to rofl::openflow::cofmsg_get_config_reply message containing the received message
	 */
	virtual void
	handle_get_config_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_get_config_reply& msg) {};

	/**
	 * @brief	Called once a timer expires for a GET-CONFIG.reply message.
	 *
	 * Default behaviour: deletes cofdpt instance, thus effectively closing the control connection.
	 *
	 * @param dpt pointer to cofdpt instance
	 */
	virtual void
	handle_get_config_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a STATS.reply message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_stats_reply message containing the received message
	 */
	virtual void
	handle_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_stats_reply& msg) {};

	/**
	 * @brief	Called once a timer has expired for a STATS.reply message.
	 *
	 * To be overwritten by derived class. Default behaviour: ignores event.
	 *
	 * @param dpt pointer to cofdpt instance where the timeout occured.
	 * @param xid transaction ID of STATS.request previously sent to data path element.
	 */
	virtual void
	handle_multipart_reply_timeout(rofl::crofdpt& dpt, uint32_t xid, uint8_t msg_sub_type) {};

	/**
	 * @brief	Called once a DESC-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the DESC-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_desc_stats_reply message containing the received message
	 */
	virtual void
	handle_desc_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_desc_stats_reply& msg) {};

	/**
	 * @brief	Called once a DESC-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_desc_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a TABLE-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the TABLE-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_table_stats_reply message containing the received message
	 */
	virtual void
	handle_table_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_table_stats_reply& msg) {};

	/**
	 * @brief	Called once a TABLE-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_table_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a PORT-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the PORT-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_port_stats_reply message containing the received message
	 */
	virtual void
	handle_port_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_port_stats_reply& msg) {};

	/**
	 * @brief	Called once a PORT-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_port_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a FLOW-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the FLOW-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_flow_stats_reply message containing the received message
	 */
	virtual void
	handle_flow_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_flow_stats_reply& msg) {};

	/**
	 * @brief	Called once a FLOW-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_flow_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once an AGGREGATE-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the AGGREGATE-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_aggregate_stats_reply message containing the received message

	 */
	virtual void
	handle_aggregate_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_aggr_stats_reply& msg) {};

	/**
	 * @brief	Called once a AGGREGATE-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_aggregate_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a QUEUE-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the QUEUE-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_queue_stats_reply message containing the received message
	 */
	virtual void
	handle_queue_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_queue_stats_reply& msg) {};

	/**
	 * @brief	Called once a QUEUE-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_queue_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a GROUP-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the GROUP-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_group_stats_reply message containing the received message
	 */
	virtual void
	handle_group_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_group_stats_reply& msg) {};

	/**
	 * @brief	Called once a GROUP-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_group_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a GROUP-DESC-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the GROUP-DESC-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_group_desc_stats_reply message containing the received message
	 */
	virtual void
	handle_group_desc_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_group_desc_stats_reply& msg) {};

	/**
	 * @brief	Called once a GROUP-DESC-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_group_desc_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a GROUP-FEATURES-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the GROUP-FEATURES-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_group_features_stats_reply message containing the received message
	 */
	virtual void
	handle_group_features_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_group_features_stats_reply& msg) {};

	/**
	 * @brief	Called once a GROUP-FEATURES-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_group_features_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a METER-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the METER-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_meter_stats_reply message containing the received message
	 */
	virtual void
	handle_meter_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_meter_stats_reply& msg) {};

	/**
	 * @brief	Called once a METER-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_meter_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a METER-CONFIG-STATS.reply message was received.
	 *
	 * To be overwritten by derived class.
	 *
	 * @param dpt pointer to cofdpt instance from which the METER-CONFIG-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_meter_config_stats_reply message containing the received message
	 */
	virtual void
	handle_meter_config_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_meter_config_stats_reply& msg) {};

	/**
	 * @brief	Called once a METER-CONFIG-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_meter_config_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a METER-FEATURES-STATS.reply message was received.
	 *
	 * To be overwritten by derived class.
	 *
	 * @param dpt pointer to cofdpt instance from which the METER-FEATURES-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_meter_features_stats_reply message containing the received message
	 */
	virtual void
	handle_meter_features_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_meter_features_stats_reply& msg) {};

	/**
	 * @brief	Called once a METER-FEATURES-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_meter_features_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a TABLE-FEATURES-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the TABLE-FEATURES-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_table_features_reply message containing the received message
	 */
	virtual void
	handle_table_features_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_table_features_stats_reply& msg) {};

	/**
	 * @brief	Called once a TABLE-FEATURES-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_table_features_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a PORT-DESC-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the PORT-DESC-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_port_desc_stats_reply message containing the received message
	 */
	virtual void
	handle_port_desc_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_port_desc_stats_reply& msg) {};

	/**
	 * @brief	Called once a PORT-DESC-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_port_desc_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once an EXPERIMENTER-STATS.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the EXPERIMENTER-STATS.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_experimenter_stats_reply message containing the received message
	 */
	virtual void
	handle_experimenter_stats_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_experimenter_stats_reply& msg) {};

	/**
	 * @brief	Called once an EXPERIMENTER-STATS.request expires.
	 *
	 * @param dpt data path handle
	 * @param xid transaction id of associated request
	 */
	virtual void
	handle_experimenter_stats_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a PACKET-IN.message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the PACKET-IN.message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_packet_in message containing the received message
	 */
	virtual void
	handle_packet_in(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_packet_in& msg) {};

	/**
	 * @brief	Called once a BARRIER.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the BARRIER.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_barrier_reply message containing the received message
	 */
	virtual void
	handle_barrier_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_barrier_reply& msg) {};

	/**
	 * @brief	Called once a timer has expired for a BARRIER.reply message.
	 *
	 * To be overwritten by derived class. Default behaviour: ignores event.
	 *
	 * @param dpt pointer to cofdpt instance where the timeout occured.
	 * @param xid transaction ID of BARRIER.request previously sent to data path element.
	 */
	virtual void
	handle_barrier_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a FLOW-REMOVED.message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the FLOW-REMOVED.message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_flow_removed message containing the received message
	 */
	virtual void
	handle_flow_removed(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_flow_removed& msg) {};

	/**
	 * @brief	Called once a PORT-STATUS.message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the PORT-STATUS.message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_port_status message containing the received message
	 */
	virtual void
	handle_port_status(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_port_status& msg) {};

	/**
	 * @brief	Called once a QUEUE-GET-CONFIG.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the QUEUE-GET-CONFIG.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_queue_get_config_reply message containing the received message
	 */
	virtual void
	handle_queue_get_config_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_queue_get_config_reply& msg) {};

	/**
	 * @brief	Called once a timer expires for a GET-CONFIG.reply message.
	 *
	 * Default behaviour: deletes cofdpt instance, thus effectively closing the control connection.
	 *
	 * @param dpt pointer to cofdpt instance
	 */
	virtual void
	handle_queue_get_config_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once an EXPERIMENTER.message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the EXPERIMENTER.message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_experimenter message containing the received message
	 */
	virtual void
	handle_experimenter_message(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_experimenter& msg) {};

	/**
	 * @brief	Called once an ERROR.message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the EXPERIMENTER.message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_experimenter message containing the received message
	 */
	virtual void
	handle_error_message(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_error& msg) {};

	/**
	 * @brief	Called once a timer expires for a GET-CONFIG.reply message.
	 *
	 * Default behaviour: deletes cofdpt instance, thus effectively closing the control connection.
	 *
	 * @param dpt pointer to cofdpt instance
	 */
	virtual void
	handle_experimenter_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a timer has expired for an EXPERIMENTER.message.
	 *
	 * To be overwritten by derived class. Default behaviour: ignores event.
	 *
	 * @param dpt pointer to cofdpt instance where the timeout occured.
	 * @param xid transaction ID of EXPERIMENTER.message previously sent to data path element.
	 */
	virtual void
	handle_get_fsp_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a ROLE.reply message was received.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt pointer to cofdpt instance from which the ROLE.reply message was received.
	 * @param msg pointer to rofl::openflow::cofmsg_role_reply message containing the received message
	 */
	virtual void
	handle_role_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_role_reply& msg) {};

	/**
	 * @brief	Called once a timer has expired for a ROLE.reply.
	 *
	 * To be overwritten by derived class. Default behaviour: ignores event.
	 *
	 * @param dpt pointer to cofdpt instance where the timeout occured.
	 * @param xid transaction ID of ROLE.reply message previously sent to data path element.
	 */
	virtual void
	handle_role_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**
	 * @brief	Called once a GET-ASYNC-CONFIG.reply message was received from a data path element.
	 *
	 * To be overwritten by derived class. Default behavior: removes msg from heap.
	 *
	 * @param dpt Pointer to cofdpt instance from which the GET-ASYNC-CONFIG.reply was received
	 * @param msg Pointer to rofl::openflow::cofmsg_get_async_config_reply message containing the received message
	 */
	virtual void
	handle_get_async_config_reply(rofl::crofdpt& dpt, const rofl::cauxid& auxid, rofl::openflow::cofmsg_get_async_config_reply& msg) {};

	/**
	 * @brief	Called once a timer expires for a GET-ASYNC-CONFIG.reply message.
	 *
	 * Default behaviour: deletes cofdpt instance, thus effectively closing the control connection.
	 *
	 * @param dpt pointer to cofdpt instance
	 */
	virtual void
	handle_get_async_config_reply_timeout(rofl::crofdpt& dpt, uint32_t xid) {};

	/**@}*/
};


/**
 * \class	rofl::openflow::cofdpt
 *
 * This class stores state for an attached data path element
 * including its ports (@see rofl::openflow::cofport). It is tightly bound to
 * class crofbase (@see crofbase). Created upon reception of an
 * OpenFlow HELLO message from the data path element, rofl::openflow::cofdpath
 * acquires all state by sending FEATURES-request, GET-CONFIG-request,
 * and TABLE-STATS-request. It also provides a number of convenience
 * methods for controlling the datapath, e.g. clearing all flowtable
 * or grouptable entries.
 *
 */
class crofdpt :
		public rofl::crofchan_env,
		public rofl::ctransactions_env,
		public ciosrv
{
private: // data structures

		enum crofdpt_impl_timer_t {
			TIMER_SIGNAL_DISCONNECT						= 0,
		};

		enum crofdpt_impl_state_t {
			STATE_INIT 									= 0,
			STATE_DISCONNECTED							= 1,
			STATE_CONNECTED								= 2,
			STATE_FEATURES_RCVD 						= 3,
			STATE_GET_CONFIG_RCVD						= 4,
			STATE_TABLE_FEATURES_RCVD					= 5,
			STATE_ESTABLISHED							= 6,
		};

		enum crofdpt_event_t {
			EVENT_NONE									= 0,
			EVENT_DISCONNECTED							= 1,
			EVENT_CONNECTED								= 2,
			EVENT_FEATURES_REPLY_RCVD					= 3,
			EVENT_FEATURES_REQUEST_EXPIRED				= 4,
			EVENT_GET_CONFIG_REPLY_RCVD					= 5,
			EVENT_GET_CONFIG_REQUEST_EXPIRED			= 6,
			EVENT_TABLE_STATS_REPLY_RCVD				= 7,
			EVENT_TABLE_STATS_REQUEST_EXPIRED			= 8,
			EVENT_TABLE_FEATURES_STATS_REPLY_RCVD		= 9,
			EVENT_TABLE_FEATURES_STATS_REQUEST_EXPIRED	= 10,
			EVENT_PORT_DESC_STATS_REPLY_RCVD			= 11,
			EVENT_PORT_DESC_STATS_REQUEST_EXPIRED		= 12,
		};

public: // static

	/**
	 * @brief	Returns reference to crofdpt instance identified by cdptid object.
	 *
	 * @param	dptid - cdptid handle
	 * @throw	eRofDptNotFound
	 * @return	Reference to crofdpt instance
	 */
	static crofdpt&
	get_dpt(const cdptid& dptid);

	/**
	 * @brief	Returns reference to crofdpt instance identified by OpenFlow dpid (uint64_t).
	 *
	 * @param	dpid - OpenFlow dpid
	 * @throw	eRofDptNotFound
	 * @return	Reference to crofdpt instance
	 */
	static crofdpt&
	get_dpt(const cdpid& dpid);

public:

	/**
	 * @brief 	Creates new crofdpt instance.
	 *
	 */
	crofdpt(
			crofdpt_env* env,
			const cdptid& dptid,
			bool remove_on_channel_close,
			const rofl::openflow::cofhello_elem_versionbitmap& versionbitmap) :
				env(env),
				dptid(dptid),
				remove_on_channel_close(remove_on_channel_close),
				dpid(0),
				rofchan(this, versionbitmap),
				hwaddr(cmacaddr("00:00:00:00:00:00")),
				n_buffers(0),
				n_tables(0),
				capabilities(0),
				config(0),
				miss_send_len(0),
				transactions(this),
				state(STATE_INIT) {
		crofdpt::rofdpts[dptid] = this;
		rofl::logging::debug << "[rofl-common][crofdpt] "
				<< "instance created, dptid: " << dptid.str() << std::endl;
	};


	/**
	 * @brief	Destroys crofdpt instance.
	 *
	 */
	virtual
	~crofdpt() {
		rofl::logging::debug << "[rofl-common][crofdpt] "
				<< "instance destroyed, dptid: " << dptid.str() << std::endl;
		crofdpt::rofdpts.erase(dptid);
		rofchan.close();
		transactions.clear();
	};




	/**
	 * @brief	Returns cdptid handle for this crofdpt instance.
	 *
	 * @return dpid
	 */
	const cdptid&
	get_dptid() const
	{ return dptid; };


	/**
	 * @brief	Returns OpenFlow dpid for this crofdpt instance.
	 *
	 * @return dpid
	 */
	const cdpid&
	get_dpid() const
	{ return dpid; };

public:

	/**
	 *
	 */
	virtual std::list<rofl::cauxid>
	get_conn_index() const
	{ return rofchan.get_conn_index(); };

	/**
	 *
	 */
	virtual void
	connect(
			const rofl::cauxid& auxid,
			enum rofl::csocket::socket_type_t socket_type,
			const cparams& socket_params) {
		rofchan.close();
		transactions.clear();
		tables.clear();
		ports.clear();
		state = STATE_DISCONNECTED;
		/* establish main connection */
		rofchan.add_conn(auxid, socket_type, socket_params);
	};

	/**
	 *
	 */
	virtual void
	disconnect(
			const rofl::cauxid& auxid)
	{ rofchan.drop_conn(auxid); };

	/**
	 *
	 */
	virtual void
	reconnect(
			const rofl::cauxid& auxid)  {
		rofchan.set_conn(auxid).close();
		rofchan.set_conn(auxid).reconnect(true);
	};

	/**
	 *
	 */
	virtual void
	add_connection(
			crofconn* conn)  {
		if (NULL == conn) {
			return;
		}
		rofchan.add_conn(conn->get_aux_id(), conn);
	};

public:

	/**
	 * @brief	Returns true, when the control handshake (HELLO) has been completed.
	 */
	virtual bool
	is_established() const
	{ return rofchan.is_established(); };

	/**
	 * @brief 	Returns the OpenFlow protocol version used for this control connection.
	 *
	 * @return OpenFlow version used for this control connection
	 */
	virtual uint8_t
	get_version_negotiated() const
	{ return rofchan.get_version(); };

	/**
	 *
	 * @return
	 */
	virtual const rofl::openflow::cofhello_elem_versionbitmap&
	get_versions_available() const
	{ return rofchan.get_versionbitmap(); };

	/**
	 * @brief	Returns true, when this instance should be destroyed when its crofchan has closed
	 */
	bool
	remove_upon_channel_termination() const
	{ return remove_on_channel_close; };

	/**
	 * @brief	Returns caddress of connected remote entity.
	 *
	 * @return caddress object obtained from this->socket
	 */
	virtual caddress
	get_peer_addr(
			const rofl::cauxid& auxid) const
	{ return rofchan.get_conn(auxid).get_rofsocket().get_socket().get_raddr(); };

	/**
	 *
	 */
	uint32_t
	get_next_idle_group_id() {
		uint32_t group_id = 1;
		while (groupids.find(group_id) != groupids.end()) {
			group_id++;
		}
		groupids.insert(group_id);
		return group_id;
	};

	/**
	 *
	 */
	void
	release_group_id(uint32_t group_id)
	{ groupids.erase(group_id); };

	/**
	 *
	 */
	void
	clear_group_ids()
	{ groupids.clear(); };

	/**
	 * @brief	Returns the data path element's hardware address.
	 *
	 * @return hwaddr
	 */
	virtual cmacaddr
	get_hwaddr() const
	{ return hwaddr; };

	/**
	 * @brief	Returns the data path element's number of buffers for storing data packets.
	 *
	 * @return n_buffers
	 */
	virtual uint32_t
	get_n_buffers() const
	{ return n_buffers; };

	/**
	 * @brief	Returns the data path element's number of tables in the OpenFlow pipeline.
	 *
	 * @return n_tables
	 */
	virtual uint8_t
	get_n_tables() const
	{ return n_tables; };

	/**
	 * @brief	Returns the data path element's capabilities.
	 *
	 * @return capabilities
	 */
	virtual uint32_t
	get_capabilities() const
	{ return capabilities; };

	/**
	 * @brief	Returns the data path element's configuration.
	 *
	 * @return config
	 */
	virtual uint16_t
	get_config() const
	{ return config; };

	/**
	 * @brief	Returns the data path element's current miss_send_len parameter.
	 *
	 * @return miss_send_len
	 */
	virtual uint16_t
	get_miss_send_len() const
	{ return miss_send_len; };

	/**
	 * @brief	Returns reference to the data path element's rofl::openflow::cofport list.
	 *
	 * @return ports
	 */
	virtual rofl::openflow::cofports&
	set_ports()
	{ return ports; };

	/**
	 * @brief	Returns reference to the data path element's rofl::openflow::cofport list.
	 *
	 * @return ports
	 */
	virtual rofl::openflow::cofports const&
	get_ports() const
	{ return ports; };

	/**
	 * @brief	Returns reference to the data path element's rofl::openflow::coftable_stats_reply list.
	 *
	 * @return tables
	 */
	virtual rofl::openflow::coftables&
	set_tables()
	{ return tables; };

	/**
	 * @brief	Returns reference to the data path element's rofl::openflow::coftable_stats_reply list.
	 *
	 * @return tables
	 */
	virtual rofl::openflow::coftables const&
	get_tables() const
	{ return tables; };

public:

	/**
	 * @name	FlowMod management methods
	 *
	 * These methods provide a simple to use interface for managing FlowMod
	 * entries.
	 *
	 * Please note: these methods are subject to change in future revisions.
	 */

	/**@{*/

	/**
	 * @brief	Removes all flowtable entries from the attached datapath element.
	 *
	 * Sends a FlowMod-Delete message to the attached datapath element for removing
	 * all flowtable entries.
	 */
	virtual void
	flow_mod_reset();

	/**
	 * @brief	Removes all grouptable entries from the attached datapath element.
	 *
	 * Sends a GroupMod-Delete message to the attached datapath element for removing
	 * all grouptable entries.
	 */
	virtual void
	group_mod_reset();

	/**
	 * @brief	Drops packet identified by buffer-id
	 *
	 * Drops a packet stored on the data path in the buffer identified by buffer-id
	 */
	virtual void
	drop_buffer(
			const rofl::cauxid& auxid,
			uint32_t buffer_id,
			uint32_t inport = rofl::openflow::OFPP_CONTROLLER) {
		rofl::openflow::cofactions actions(get_version_negotiated());
		send_packet_out_message(auxid, buffer_id, inport, actions, NULL, 0);
	};

	/**@}*/

public:


	/**
	 * @name	Methods for sending OpenFlow messages
	 *
	 * These methods may be called by a derived class for sending
	 * a specific OF message.
	 */

	/**@{*/

	/**
	 * @brief	Sends a FEATURES.request to a data path element.
	 *
	 * @return transaction ID assigned to this request
	 */
	virtual uint32_t
	send_features_request(
			const cauxid& aux_id);

	/**
	 * @brief	Sends a GET-CONFIG.request to a data path element.
	 *
	 * @return transaction ID assigned to this request
	 */
	virtual uint32_t
	send_get_config_request(
			const cauxid& aux_id);

	/**
	 * @brief	Sends a TABLE-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this TABLE-STATS.request
	 */
	virtual uint32_t
	send_table_features_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags);

	/**
	 * @brief	Sends a STATS.request to a data path element.
	 *
	 * @param stats_type one of the OFPMP_* constants
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param body body of STATS.request
	 * @param bodylen length of STATS.request body
	 * @return transaction ID for this STATS.request
	 */
	virtual uint32_t
	send_stats_request(
			const cauxid& aux_id,
			uint16_t stats_type,
			uint16_t stats_flags,
			uint8_t *body = NULL,
			size_t bodylen = 0);

	/**
	 * @brief	Sends a DESC-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this DESC-STATS.request
	 */
	virtual uint32_t
	send_desc_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags);

	/**
	 * @brief	Sends a FLOW-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param flow_stats_request body of a FLOW-STATS.request
	 * @return transaction ID for this FLOW-STATS.request
	 */
	virtual uint32_t
	send_flow_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			rofl::openflow::cofflow_stats_request const& flow_stats_request);

	/**
	 * @brief	Sends a AGGREGATE-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param aggr_stats_request body of an AGGREGATE-STATS.request
	 * @return transaction ID for this AGGREGATE-STATS.request
	 */
	virtual uint32_t
	send_aggr_stats_request(
			const cauxid& aux_id,
			uint16_t flags,
			rofl::openflow::cofaggr_stats_request const& aggr_stats_request);


	/**
	 * @brief	Sends a TABLE-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this TABLE-STATS.request
	 */
	virtual uint32_t
	send_table_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags = 0);

	/**
	 * @brief	Sends a FLOW-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param port_stats_request body of a PORT-STATS.request
	 * @return transaction ID for this FLOW-STATS.request
	 */
	virtual uint32_t
	send_port_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			rofl::openflow::cofport_stats_request const& port_stats_request);

	/**
	 * @brief	Sends a QUEUE-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param queue_stats_request body of a QUEUE-STATS.request
	 * @return transaction ID for this QUEUE-STATS.request
	 */
	virtual uint32_t
	send_queue_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			rofl::openflow::cofqueue_stats_request const& queue_stats_request);

	/**
	 * @brief	Sends a GROUP-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param queue_stats_request body of a GROUP-STATS.request
	 * @return transaction ID for this GROUP-STATS.request
	 */
	virtual uint32_t
	send_group_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			rofl::openflow::cofgroup_stats_request const& group_stats_request);

	/**
	 * @brief	Sends a GROUP-DESC-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this AGGREGATE-STATS.request
	 */
	virtual uint32_t
	send_group_desc_stats_request(
			const cauxid& aux_id,
			uint16_t flags = 0);

	/**
	 * @brief	Sends a GROUP-FEATURES-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this GROUP-FEATURES-STATS.request
	 */
	virtual uint32_t
	send_group_features_stats_request(
			const cauxid& aux_id,
			uint16_t flags);

	/**
	 * @brief	Sends a PORT-DESC-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this PORT-DESC-STATS.request
	 */
	virtual uint32_t
	send_port_desc_stats_request(
			const cauxid& aux_id,
			uint16_t flags);

	/**
	 * @brief	Sends an EXPERIMENTER-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param exp_id experimenter ID
	 * @param exp_type user defined type
	 * @param body user defined body
	 * @return transaction ID for this EXPERIMENTER-STATS.request
	 */
	virtual uint32_t
	send_experimenter_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			uint32_t exp_id,
			uint32_t exp_type,
			cmemory const& body);

	/**
	 * @brief	Sends a METER-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param mstats meter multipart request
	 * @return transaction ID for this METER-STATS.request
	 */
	virtual uint32_t
	send_meter_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			const rofl::openflow::cofmeter_stats_request& mstats);

	/**
	 * @brief	Sends a METER-CONFIG-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @param mstats meter multipart request
	 * @return transaction ID for this METER-CONFIG-STATS.request
	 */
	virtual uint32_t
	send_meter_config_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags,
			const rofl::openflow::cofmeter_config_request& mstats);

	/**
	 * @brief	Sends a METER-FEATURES-STATS.request to a data path element.
	 *
	 * @param stats_flags a bitfield with OFPSF_REQ_* flags
	 * @return transaction ID for this METER-FEATURES-STATS.request
	 */
	virtual uint32_t
	send_meter_features_stats_request(
			const cauxid& aux_id,
			uint16_t stats_flags);

	/**
	 * @brief	Sends a PACKET-OUT.message to a data path element.
	 *
	 * @param buffer_id buffer ID assigned by datapath (-1 if none) in host byte order
	 * @param in_port packet’s in-port (OFPP_NONE if none) in host byte order
	 * @param aclist OpenFlow ActionList
	 * @param data data packet to be sent out (optional)
	 * @param datalen length of data packet (optional)
	 * @result transaction ID assigned to this request
	 */
	virtual uint32_t
	send_packet_out_message(
			const cauxid& aux_id,
			uint32_t buffer_id,
			uint32_t in_port,
			rofl::openflow::cofactions& aclist,
			uint8_t *data = NULL,
			size_t datalen = 0);

	/**
	 * @brief	Sends a BARRIER.request to a data path element.
	 *
	 * @result transaction ID assigned to this request
	 */
	virtual uint32_t
	send_barrier_request(
			const cauxid& aux_id);

	/**
	 * @brief	Sends a ROLE.request to a data path element.
	 *
	 * @param role role as defined by OpenFlow
	 * @param generation_id gen_id as defined by OpenFlow
	 */
	virtual uint32_t
	send_role_request(
			const cauxid& aux_id,
			rofl::openflow::cofrole const& role);

	/**
	 * @brief 	Sends a FLOW-MOD.message to a data path element.
	 *
	 * @param flowentry FlowMod entry
	 */
	virtual uint32_t
	send_flow_mod_message(
			const cauxid& aux_id,
			rofl::openflow::cofflowmod const& flowentry);

	/**
	 * @brief 	Sends a GROUP-MOD.message to a data path element.
	 *
	 * @param groupentry GroupMod entry
	 */
	virtual uint32_t
	send_group_mod_message(
			const cauxid& aux_id,
			rofl::openflow::cofgroupmod const& groupentry);

	/**
	 * @brief	Sends a TABLE-MOD.message to a data path element.
	 *
	 * @param table_id ID of table to be reconfigured
	 * @param config new configuration for table
	 */
	virtual uint32_t
	send_table_mod_message(
			const cauxid& aux_id,
			uint8_t table_id,
			uint32_t config);

	/**
	 * @brief	Sends a PORT-MOD.message to a data path element.
	 *
	 * @param port_no number of port to be updated
	 * @param hwaddr MAC address assigned to port
	 * @param config OpenFlow config parameter
	 * * @param mask OpenFlow mask parameter
	 * * @param advertise OpenFlow advertise parameter
	 */
	virtual uint32_t
	send_port_mod_message(
			const cauxid& aux_id,
			uint32_t port_no,
			cmacaddr const& hwaddr,
			uint32_t config,
			uint32_t mask,
			uint32_t advertise);

	/**
	 * @brief	Sends a SET-CONFIG.message to a data path element.
	 *
	 * @param flags field of OpenFlow's OFPC_* flags
	 * @param miss_send_len OpenFlow' miss_send_len parameter
	 */
	virtual uint32_t
	send_set_config_message(
			const cauxid& aux_id,
			uint16_t flags,
			uint16_t miss_send_len);

	/**
	 * @brief	Sends a QUEUE-GET-CONFIG.request to a data path element.
	 *
	 * @param port port to be queried. Should refer to a valid physical port (i.e. < OFPP_MAX)
	 * @result transaction ID assigned to this request
	 */
	virtual uint32_t
	send_queue_get_config_request(
			const cauxid& aux_id,
			uint32_t port);

	/**
	 * @brief	Sends a GET-ASYNC-CONFIG.request to a data path element.
	 *
	 * @return transaction ID assigned to this request
	 */
	virtual uint32_t
	send_get_async_config_request(
			const cauxid& aux_id);

	/**
	 * @brief	Sends a SET-ASYNC-CONFIG.message to a data path element.
	 *
	 */
	virtual uint32_t
	send_set_async_config_message(
			const cauxid& aux_id,
			rofl::openflow::cofasync_config const& async_config);

	/**
	 * @brief	Sends a METER-MOD.message to a data path element.
	 *
	 * @param table_id ID of table to be reconfigured
	 * @param config new configuration for table
	 */
	virtual uint32_t
	send_meter_mod_message(
			const cauxid& auxid,
			uint16_t command,
			uint16_t flags,
			uint32_t meter_id,
			const rofl::openflow::cofmeter_bands& meter_bands);

	/**
	 * @brief	Sends an ERROR.message to a data path element.
	 *
	 * These messages are used for failed HELLO negotiations and
	 * experimental extensions.
	 *
	 * @param xid transaction ID of reply/notification that generated this error message
	 * @param type one of OpenFlow's OFPET_* flags
	 * @param code one of OpenFlow's error codes
	 * @param data first (at least 64) bytes of failed reply/notification
	 * @param datalen length of failed reply/notification appended to error message
	 */
	virtual void
	send_error_message(
			const cauxid& aux_id,
			uint32_t xid,
			uint16_t type,
			uint16_t code,
			uint8_t* data = NULL,
			size_t datalen = 0);

	/**
	 * @brief 	Sends an EXPERIMENTER.message to a data path element.
	 *
	 * @param experimenter_id exp_id as assigned by ONF
	 * @param exp_type exp_type as defined by the ONF member
	 * @param body pointer to opaque experimenter message body (optional)
	 * @param bodylen length of body (optional)
	 * @result transaction ID assigned to this request
	 */
	virtual uint32_t
	send_experimenter_message(
			const cauxid& aux_id,
			uint32_t experimenter_id,
			uint32_t exp_type,
			uint8_t *body = NULL,
			size_t bodylen = 0);

	/**@}*/

public:

	/**
	 *
	 */
	class crofdpt_find_by_dptid {
		cdptid dptid;
	public:
		crofdpt_find_by_dptid(const cdptid& dptid) : dptid(dptid) {};
		bool operator() (const crofdpt* rofdpt) {
			return (rofdpt->get_dptid() == dptid);
		};
	};

	/**
	 *
	 */
	class crofdpt_find_by_dpid {
		uint64_t dpid;
	public:
		crofdpt_find_by_dpid(uint64_t dpid) : dpid(dpid) {};
		bool operator() (const std::pair<cdptid, crofdpt*>& p) {
			return (p.second->get_dpid().get_uint64_t() == dpid);
		};
	};

protected:

	/**
	 *
	 */
	void
	set_dpid(
			const cdpid& dpid) {
		this->dpid = dpid;
	};

protected:

	crofdpt_env&
	call_env() {
		if (crofdpt_env::rofdpt_envs.find(env) == crofdpt_env::rofdpt_envs.end()) {
			throw eRofDptNotFound();
		}
		return *env;
	};

public:

	friend std::ostream&
	operator<< (std::ostream& os, const crofdpt& dpt) {
		os << indent(0) << "<cofdptImpl >";
		{ rofl::indent i(2); os << dpt.get_dptid(); }
		{ rofl::indent i(2); os << dpt.rofchan; }
		switch (dpt.state) {
		case STATE_INIT: {
			os << indent(2) << "<state: -INIT- >" << std::endl;
		} break;
		case STATE_CONNECTED: {
			os << indent(2) << "<state: -CONNECTED- >" << std::endl;
		} break;
		case STATE_DISCONNECTED: {
			os << indent(2) << "<state: -DISCONNECTED- >" << std::endl;
		} break;
		case STATE_ESTABLISHED: {
			os << indent(2) << "<state: -ESTABLISHED- >" << std::endl;
		} break;
		case STATE_FEATURES_RCVD: {
			os << indent(2) << "<state: -FEATURES-RCVD- >" << std::endl;
		} break;
		case STATE_GET_CONFIG_RCVD: {
			os << indent(2) << "<state: -GET-CONFIG-RCVD- >" << std::endl;
		} break;
		case STATE_TABLE_FEATURES_RCVD: {
			os << indent(2) << "<state: -TABLE-FEATURES-RCVD- >" << std::endl;
		} break;
		default: {
			os << indent(2) << "<state: -UNKNOWN- >" << std::endl;
		} break;
		}
		os << indent(2) << "<hwaddr: " << dpt.hwaddr.str() << " >" << std::endl;
		os << indent(2) << "<#buffers: " << (int)dpt.n_buffers << " >" << std::endl;
		os << indent(2) << "<#tables: " << (int)dpt.n_tables << " >" << std::endl;
		os << indent(2) << "<capabilities: " << std::hex << (int)dpt.capabilities << std::dec << " >" << std::endl;
		os << indent(2) << "<config: " << std::hex << (int)dpt.config << std::dec << " >" << std::endl;
		os << indent(2) << "<miss-send-len: " << (int)dpt.miss_send_len << " >" << std::endl;
		rofl::indent j(2);
		os << dpt.tables;
		os << dpt.ports;
		os << dpt.transactions;
		return os;
	};

	std::string
	str() const {
		std::stringstream ss;
		ss << "dpid: " << get_dpid().str() << " ";
		switch (state) {
		case STATE_INIT: {
			ss << indent(2) << "state: -init- " << std::endl;
		} break;
		case STATE_CONNECTED: {
			ss << indent(2) << "state: -connected- >" << std::endl;
		} break;
		case STATE_DISCONNECTED: {
			ss << indent(2) << "state: -disconnected- " << std::endl;
		} break;
		case STATE_ESTABLISHED: {
			ss << indent(2) << "state: -established- " << std::endl;
		} break;
		case STATE_FEATURES_RCVD: {
			ss << indent(2) << "state: -features-rcvd- " << std::endl;
		} break;
		case STATE_GET_CONFIG_RCVD: {
			ss << indent(2) << "state: -get-config-rcvd- " << std::endl;
		} break;
		case STATE_TABLE_FEATURES_RCVD: {
			ss << indent(2) << "state: -table-features-rcvd- " << std::endl;
		} break;
		default: {
			ss << indent(2) << "state: -unknown- " << std::endl;
		} break;
		}
		return ss.str();
	};

protected:

	/**
	 * @brief Called upon establishment of a control connection within the control channel
	 */
	virtual void
	handle_conn_established(
			crofchan& chan,
			const rofl::cauxid& auxid) {
		rofl::logging::info << "[rofl-common][crofdpt] dptid:0x" << dptid.str()
						<< " control connection established, auxid: " << auxid.str() << std::endl;
		call_env().handle_conn_established(*this, auxid);
		if (auxid == rofl::cauxid(0)) {
			rofl::logging::info << "[rofl-common][crofdpt] dpid:" << std::hex << get_dpid().str() << std::dec
					<< " OFP control channel established, " << chan.str() << std::endl;
			run_engine(EVENT_CONNECTED);
		}
	};

	virtual void
	handle_conn_terminated(
			crofchan& chan,
			const rofl::cauxid& auxid) {
		rofl::logging::info << "[rofl-common][crofdpt] dptid:0x" << dptid.str()
						<< " control connection terminated, auxid: " << auxid.str() << std::endl;
		call_env().handle_conn_terminated(*this, auxid);
		if (auxid == rofl::cauxid(0)) {
			rofl::logging::info << "[rofl-common][crofdpt] dptid:" << dptid.str()
					<< " OFP control channel terminated, " << chan.str() << std::endl;
			transactions.clear();
			run_engine(EVENT_DISCONNECTED);
		}
	};

	/**
	 * @brief Called in the event of a connection refused
	 */
	virtual void
	handle_conn_refused(
			crofchan& chan,
			const rofl::cauxid& auxid)
	{ call_env().handle_conn_refused(*this, auxid);	};

	/**
	 * @brief Called in the event of a connection failed (except refused)
	 */
	virtual void
	handle_conn_failed(
			crofchan& chan,
			const rofl::cauxid& auxid)
	{ call_env().handle_conn_failed(*this, auxid); };

	virtual void
	handle_write(crofchan& chan, const cauxid& auxid)
	{ call_env().handle_write(*this, auxid); };

	virtual void
	recv_message(crofchan& chan, const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	virtual uint32_t
	get_async_xid(crofchan& chan)
	{ return transactions.get_async_xid(); };

	virtual uint32_t
	get_sync_xid(crofchan& chan, uint8_t msg_type = 0, uint16_t msg_sub_type = 0)
	{ return transactions.add_ta(cclock(/*secs=*/5), msg_type, msg_sub_type); };

	virtual void
	release_sync_xid(crofchan& chan, uint32_t xid)
	{ return transactions.drop_ta(xid); };

protected:

	virtual void
	ta_expired(rofl::ctransactions& tas, rofl::ctransaction& ta);

protected:

	virtual void
	handle_timeout(
		int opaque, void *data = (void*)0);

private:

	void
	run_engine(
			enum crofdpt_event_t state = EVENT_NONE);

	void
	event_disconnected();

	void
	event_connected();

	void
	event_features_reply_rcvd();

	void
	event_features_request_expired(
			uint32_t xid = 0);

	void
	event_get_config_reply_rcvd();

	void
	event_get_config_request_expired(
			uint32_t xid = 0);

	void
	event_table_stats_reply_rcvd();

	void
	event_table_stats_request_expired(
			uint32_t xid = 0);

	void
	event_table_features_stats_reply_rcvd();

	void
	event_table_features_stats_request_expired(
			uint32_t xid = 0);

	void
	event_port_desc_reply_rcvd();

	void
	event_port_desc_request_expired(
			uint32_t xid = 0);

private:

	void
	experimenter_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	error_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	features_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	get_config_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	multipart_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	desc_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	table_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	port_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	flow_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	aggregate_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	queue_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	group_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	group_desc_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	group_features_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	meter_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	meter_config_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	meter_features_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	table_features_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	port_desc_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	experimenter_stats_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	barrier_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	port_mod_sent(
			rofl::openflow::cofmsg *pack);

	void
	packet_in_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	flow_removed_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	port_status_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	role_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	queue_get_config_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

	void
	get_async_config_reply_rcvd(
			const cauxid& aux_id, rofl::openflow::cofmsg *msg);

private:

	static std::map<cdptid, crofdpt*>
							rofdpts;

	crofdpt_env*			env;
	cdptid   				dptid;			// handle for this crofdpt instance
	bool					remove_on_channel_close;
	cdpid 					dpid;			// datapath id
	std::set<uint32_t>		groupids;		// allocated groupids on datapath

	rofl::crofchan			rofchan;		// OFP control channel

	cmacaddr 				hwaddr;			// datapath mac address
	uint32_t 				n_buffers; 		// number of buffer lines
	uint8_t 				n_tables;		// number of tables
	uint32_t 				capabilities;	// capabilities flags

	rofl::openflow::coftables
							tables;			// list of tables
	rofl::openflow::cofports
							ports;			// list of ports
	uint16_t				config;
	uint16_t 				miss_send_len; 	// length of bytes sent to controller
	ctransactions			transactions;	// pending OFP transactions

	unsigned int			state;
	std::deque<enum crofdpt_event_t>
							events;
};

}; // end of namespace

#endif
