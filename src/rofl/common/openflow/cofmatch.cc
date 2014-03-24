/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cofmatch.h"

using namespace rofl::openflow;

cofmatch::cofmatch(
		uint8_t of_version,
		uint16_t type) :
				of_version(of_version),
				type(type)
{

}


cofmatch::~cofmatch()
{

}


cofmatch::cofmatch(
		cofmatch const& match)
{
	*this = match;
}


cofmatch&
cofmatch::operator= (
		const cofmatch& match)
{
	if (this == &match)
		return *this;

	of_version		= match.of_version;
	oxmtlvs			= match.oxmtlvs;
	type			= match.type;

	return *this;
}


bool
cofmatch::operator== (
		const cofmatch& match)
{
	return (
		(of_version == match.of_version) &&
		(oxmtlvs 	== match.oxmtlvs) &&
		(type 	    == match.type));
}


size_t
cofmatch::length_with_padding()
{
	switch (of_version) {
	case rofl::openflow10::OFP_VERSION: {
		return openflow10::OFP_MATCH_STATIC_LEN;
	} break;
	case rofl::openflow12::OFP_VERSION:
	case rofl::openflow13::OFP_VERSION: {
		/*
		 * returns length of struct ofp_match including padding
		 */
		size_t match_len = 2 * sizeof(uint16_t); // first two 16bit fields in struct ofp_match

		match_len += oxmtlvs.length();

		if (0 != (match_len % sizeof(uint64_t))) {
			match_len = ((match_len / sizeof(uint64_t)) + 1) * sizeof(uint64_t);
		}
		return match_len;
	} break;
	default:
		throw eBadVersion();
	}
	return 0;
}


size_t
cofmatch::length() const
{
	switch (of_version) {
	case rofl::openflow10::OFP_VERSION: {
		return sizeof(struct rofl::openflow10::ofp_match);
	} break;
	case rofl::openflow12::OFP_VERSION:
	case rofl::openflow13::OFP_VERSION: {
		size_t total_length = 2*sizeof(uint16_t) + oxmtlvs.length(); // type-field + length-field + OXM-TLV list

		size_t pad = (0x7 & total_length);
		/* append padding if not a multiple of 8 */
		if (pad) {
			total_length += 8 - pad;
		}
		return total_length;
	}
	default:
		throw eBadVersion();
	}
	return 0;
}


void
cofmatch::pack(uint8_t* buf, size_t buflen)
{
	switch (of_version) {
	case rofl::openflow10::OFP_VERSION: return pack_of10(buf, buflen); break;
	case rofl::openflow12::OFP_VERSION:
	case rofl::openflow13::OFP_VERSION: return pack_of13(buf, buflen); break;
	default: throw eBadVersion();
	}
}


void
cofmatch::unpack(uint8_t *buf, size_t buflen)
{
	switch (of_version) {
	case rofl::openflow10::OFP_VERSION: unpack_of10(buf, buflen); break;
	case rofl::openflow12::OFP_VERSION:
	case rofl::openflow13::OFP_VERSION: unpack_of13(buf, buflen); break;
	default: throw eBadVersion();
	}
}


void
cofmatch::pack_of10(uint8_t* buf, size_t buflen)
{
	if (buflen < length()) {
		throw eOFmatchInval();
	}

	uint32_t wildcards = 0;

	memset(buf, 0, buflen);

	struct rofl::openflow10::ofp_match *m = (struct rofl::openflow10::ofp_match*)buf;

	// in_port
	if (oxmtlvs.has_match(OXM_TLV_BASIC_IN_PORT)) {
		m->in_port = htobe16((uint16_t)(oxmtlvs.get_match(OXM_TLV_BASIC_IN_PORT).get_u32value() && 0x0000ffff));
	} else {
		wildcards |= rofl::openflow10::OFPFW_IN_PORT;
	}

	// dl_src
	if (oxmtlvs.has_match(OXM_TLV_BASIC_ETH_SRC)) {
		memcpy(m->dl_src, oxmtlvs.get_match(OXM_TLV_BASIC_ETH_SRC).get_u48value().somem(), OFP_ETH_ALEN);
	} else {
		wildcards |= rofl::openflow10::OFPFW_DL_SRC;
	}

	// dl_dst
	if (oxmtlvs.has_match(OXM_TLV_BASIC_ETH_DST)) {
		memcpy(m->dl_dst, oxmtlvs.get_match(OXM_TLV_BASIC_ETH_DST).get_u48value().somem(), OFP_ETH_ALEN);
	} else {
		wildcards |= rofl::openflow10::OFPFW_DL_DST;
	}

	// dl_vlan
	if (oxmtlvs.has_match(OXM_TLV_BASIC_VLAN_VID)) {
		m->dl_vlan = htobe16(oxmtlvs.get_match(OXM_TLV_BASIC_VLAN_VID).get_u16value());
	} else {
		wildcards |= rofl::openflow10::OFPFW_DL_VLAN;
	}

	// dl_vlan_pcp
	if (oxmtlvs.has_match(OXM_TLV_BASIC_VLAN_PCP)) {
		m->dl_vlan_pcp = oxmtlvs.get_match(OXM_TLV_BASIC_VLAN_PCP).get_u8value();
	} else {
		wildcards |= rofl::openflow10::OFPFW_DL_VLAN_PCP;
	}

	// dl_type
	if (oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) {
		m->dl_type = htobe16(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value());
	} else {
		wildcards |= rofl::openflow10::OFPFW_DL_TYPE;
	}

	// nw_tos
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_TOS)) {
		m->nw_tos = oxmtlvs.get_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_TOS).get_u8value();
	} else {
		wildcards |= rofl::openflow10::OFPFW_NW_TOS;
	}

	// nw_proto
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_PROTO)) {
		m->nw_proto = oxmtlvs.get_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_PROTO).get_u8value();
	} else {
		wildcards |= rofl::openflow10::OFPFW_NW_PROTO;
	}

	// nw_src
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_SRC)) {
		coxmatch const& oxm = oxmtlvs.get_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_SRC);
		m->nw_src = htobe32(oxm.get_u32value());
		if (oxm.get_oxm_hasmask()) {
			std::bitset<32> mask(oxm.get_u32mask());
			wildcards |= ((32 - mask.count()) << rofl::openflow10::OFPFW_NW_SRC_SHIFT) & rofl::openflow10::OFPFW_NW_SRC_MASK;
		}
	} else {
		wildcards |= rofl::openflow10::OFPFW_NW_SRC_ALL;
	}


	// nw_dst
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_NW_DST)) {
		coxmatch const& oxm = oxmtlvs.get_match(experimental::OXM_TLV_EXPR_NW_DST);
		m->nw_dst = htobe32(oxm.get_u32value());
		if (oxm.get_oxm_hasmask()) {
			std::bitset<32> mask(oxm.get_u32mask());
			wildcards |= ((32 - mask.count()) << rofl::openflow10::OFPFW_NW_DST_SHIFT) & rofl::openflow10::OFPFW_NW_DST_MASK;
		}
	} else {
		wildcards |= rofl::openflow10::OFPFW_NW_DST_ALL;
	}

	// tp_src
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_TP_SRC)) {
		m->tp_src = htobe16(oxmtlvs.get_match(rofl::openflow::experimental::OXM_TLV_EXPR_TP_SRC).get_u16value());
	} else {
		wildcards |= rofl::openflow10::OFPFW_TP_SRC;
	}

	// tp_dst
	if (oxmtlvs.has_match(rofl::openflow::experimental::OXM_TLV_EXPR_TP_DST)) {
		m->tp_dst = htobe16(oxmtlvs.get_match(rofl::openflow::experimental::OXM_TLV_EXPR_TP_DST).get_u16value());
	} else {
		wildcards |= rofl::openflow10::OFPFW_TP_DST;
	}


	m->wildcards = htobe32(wildcards);
}


void
cofmatch::unpack_of10(uint8_t* buf, size_t buflen)
{
	oxmtlvs.clear();

	if (buflen < sizeof(struct rofl::openflow10::ofp_match)) {
		throw eOFmatchInval();
	}

	struct rofl::openflow10::ofp_match *m = (struct rofl::openflow10::ofp_match*)buf;

	uint32_t wildcards = be32toh(m->wildcards);

	// in_port
	if (!(wildcards & rofl::openflow10::OFPFW_IN_PORT)) {
		oxmtlvs.add_match(coxmatch_ofb_in_port(be16toh(m->in_port)));
	}

	// dl_src
	if (!(wildcards & rofl::openflow10::OFPFW_DL_SRC)) {
		oxmtlvs.add_match(coxmatch_ofb_eth_src(rofl::cmacaddr(m->dl_src, OFP_ETH_ALEN)));
	}

	// dl_dst
	if (!(wildcards & rofl::openflow10::OFPFW_DL_DST)) {
		oxmtlvs.add_match(coxmatch_ofb_eth_dst(rofl::cmacaddr(m->dl_dst, OFP_ETH_ALEN)));
	}

	// dl_vlan
	if (!(wildcards & rofl::openflow10::OFPFW_DL_VLAN)) {
		if (m->dl_vlan != rofl::openflow10::OFPVID_NONE) {
			oxmtlvs.add_match(coxmatch_ofb_vlan_vid(be16toh(m->dl_vlan)));
		} else {
			oxmtlvs.add_match(coxmatch_ofb_vlan_untagged());
		}
	}

	// dl_vlan_pcp
	if (!(wildcards & rofl::openflow10::OFPFW_DL_VLAN_PCP) && m->dl_vlan != 0xffff) { //0xFFFF value is used to indicate that no VLAN id eas set.
		oxmtlvs.add_match(coxmatch_ofb_vlan_pcp(m->dl_vlan_pcp));
	}

	// dl_type
	if (!(wildcards & rofl::openflow10::OFPFW_DL_TYPE)) {
		oxmtlvs.add_match(coxmatch_ofb_eth_type(be16toh(m->dl_type)));
	}

	// nw_tos
	if (!(wildcards & rofl::openflow10::OFPFW_NW_TOS)) {
		oxmtlvs.add_match(coxmatch_ofx_nw_tos(m->nw_tos));
	}

	// nw_proto
	if (!(wildcards & rofl::openflow10::OFPFW_NW_PROTO)) {
		oxmtlvs.add_match(coxmatch_ofx_nw_proto(m->nw_proto));
	}

	// nw_src
	{
		uint64_t num_of_bits = (wildcards & rofl::openflow10::OFPFW_NW_SRC_MASK) >> openflow10::OFPFW_NW_SRC_SHIFT;
		if(num_of_bits > 32)
			num_of_bits = 32;
		uint64_t u_mask = ~((1UL << num_of_bits) - 1UL);
		rofl::caddress addr(AF_INET, "0.0.0.0");
		rofl::caddress mask(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = m->nw_src;
		mask.ca_s4addr->sin_addr.s_addr = htobe32((uint32_t)u_mask);
		if (num_of_bits < 32) {
			oxmtlvs.add_match(coxmatch_ofx_nw_src(addr, mask));
		}
#ifdef FALSCH
		if (num_of_bits > 0) {
			set_nw_src(addr, mask);
		} else {
			set_nw_src(addr);
		}
#endif
	}

	// nw_dst
	{
		uint64_t num_of_bits = (wildcards & rofl::openflow10::OFPFW_NW_DST_MASK) >> openflow10::OFPFW_NW_DST_SHIFT;
		if(num_of_bits > 32)
			num_of_bits = 32;
		uint64_t u_mask = ~((1UL << num_of_bits) - 1UL);
		rofl::caddress addr(AF_INET, "0.0.0.0");
		rofl::caddress mask(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = m->nw_dst;
		mask.ca_s4addr->sin_addr.s_addr = htobe32((uint32_t)u_mask);
		if (num_of_bits < 32) {
			oxmtlvs.add_match(coxmatch_ofx_nw_dst(addr, mask));
		}
#ifdef FALSCH
		if (num_of_bits > 0) {
			set_nw_dst(addr, mask);
		} else {
			set_nw_dst(addr);
		}
#endif
	}

	// tp_src
	if (!(wildcards & rofl::openflow10::OFPFW_TP_SRC)) {
		oxmtlvs.add_match(coxmatch_ofx_tp_src(be16toh(m->tp_src)));
	}

	// tp_dst
	if (!(wildcards & rofl::openflow10::OFPFW_TP_DST)) {
		oxmtlvs.add_match(coxmatch_ofx_tp_dst(be16toh(m->tp_dst)));
	}
}


void
cofmatch::pack_of13(uint8_t* buf, size_t buflen)
{
	if (buflen < length()) {
		throw eOFmatchInval();
	}

	struct rofl::openflow13::ofp_match* m = (struct rofl::openflow13::ofp_match*)buf;

	m->type		= htobe16(type);
	m->length 	= htobe16(2 * sizeof(uint16_t) + oxmtlvs.length()); // real length without padding

	oxmtlvs.pack(m->oxm_fields, oxmtlvs.length());
}



void
cofmatch::unpack_of13(uint8_t* buf, size_t buflen)
{
	oxmtlvs.clear();

	if (buflen < length()) {
		throw eOFmatchInval();
	}

	struct rofl::openflow13::ofp_match* m = (struct rofl::openflow13::ofp_match*)buf;

	type = be16toh(m->type);

	if (rofl::openflow13::OFPMT_OXM != type) {
		throw eBadMatchBadType();
	}

	buflen -= 2 * sizeof(uint16_t);

	if (buflen > 0) {
		oxmtlvs.unpack(m->oxm_fields, buflen);
	}
}






void
cofmatch::check_prerequisites() const
{
	switch (of_version) {
	case rofl::openflow12::OFP_VERSION:
	case rofl::openflow13::OFP_VERSION: {

		/*
		 * these are generic prerequisites as defined in OF 1.3, section 7.2.3.6, page 53 ff.
		 */
		if (oxmtlvs.has_match(OXM_TLV_BASIC_IN_PHY_PORT)) {
			if (not oxmtlvs.has_match(OXM_TLV_BASIC_IN_PORT)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IN-PHY-PORT defined while no IN-PORT is present" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_VLAN_PCP)) {
			if (openflow::OFPVID_NONE == oxmtlvs.get_match(OXM_TLV_BASIC_VLAN_VID).get_u16value()) {
				logging::warn << "[rofl][match] rejecting ofp_match: VLAN-PCP defined while VID is set to OFPVID-NONE" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IP_DSCP)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0800) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd))) {
				logging::warn << "[rofl][match] rejecting ofp_match: IP-DSCP defined while ETH-TYPE is not IPv4/IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IP_ECN)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0800) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd))) {
				logging::warn << "[rofl][match] rejecting ofp_match: IP-ECN defined while ETH-TYPE is not IPv4/IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0800) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd))) {
				logging::warn << "[rofl][match] rejecting ofp_match: IP-PROTO defined while ETH-TYPE is not IPv4/IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV4_SRC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0800)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV4-SRC defined while ETH-TYPE is not IPv4" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV4_DST)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0800)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV4-DST defined while ETH-TYPE is not IPv4" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_TCP_SRC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 6)) {
				logging::warn << "[rofl][match] rejecting ofp_match: TCP-SRC defined while IP-PROTO is not TCP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_TCP_DST)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 6)) {
				logging::warn << "[rofl][match] rejecting ofp_match: TCP-DST defined while IP-PROTO is not TCP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_UDP_SRC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 17)) {
				logging::warn << "[rofl][match] rejecting ofp_match: UDP-SRC defined while IP-PROTO is not UDP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_UDP_DST)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 17)) {
				logging::warn << "[rofl][match] rejecting ofp_match: UDP-DST defined while IP-PROTO is not UDP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_SCTP_SRC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 132)) {
				logging::warn << "[rofl][match] rejecting ofp_match: SCTP-SRC defined while IP-PROTO is not SCTP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_SCTP_DST)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 132)) {
				logging::warn << "[rofl][match] rejecting ofp_match: SCTP-DST defined while IP-PROTO is not SCTP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV4_TYPE)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 1)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ICMPV4-TYPE defined while IP-PROTO is not ICMPV4" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV4_CODE)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 1)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ICMPV4-CODE defined while IP-PROTO is not ICMPV4" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ARP_OP)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0806)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ARP-OPCODE defined while ETH-TYPE is not ARP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ARP_SPA)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0806)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ARP-SPA defined while ETH-TYPE is not ARP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ARP_TPA)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0806)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ARP-TPA defined while ETH-TYPE is not ARP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ARP_SHA)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0806)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ARP-SHA defined while ETH-TYPE is not ARP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ARP_THA)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x0806)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ARP-THA defined while ETH-TYPE is not ARP" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_SRC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV6-SRC defined while ETH-TYPE is not IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_DST)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV6-DST defined while ETH-TYPE is not IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_FLABEL)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV6-FLABEL defined while ETH-TYPE is not IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV6_TYPE)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 58)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ICMPV6-TYPE defined while IP-PROTO is not ICMPV6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV6_CODE)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_IP_PROTO)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_IP_PROTO).get_u8value() != 58)) {
				logging::warn << "[rofl][match] rejecting ofp_match: ICMPV6-CODE defined while IP-PROTO is not ICMPV6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_ND_TARGET)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV6_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ICMPV6_TYPE).get_u8value() != 135) && (oxmtlvs.get_match(OXM_TLV_BASIC_ICMPV6_TYPE).get_u8value() != 136))) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPv6-ND-TARGET defined while ICMPV6-TYPE is not ND-SOLICITATION or ND-ADVERTISEMENT" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_ND_SLL)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV6_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ICMPV6_TYPE).get_u8value() != 135)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPv6-ND-SLL defined while ICMPV6-TYPE is not ND-SOLICITATION" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_ND_TLL)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ICMPV6_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ICMPV6_TYPE).get_u8value() != 136)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPv6-ND-TLL defined while ICMPV6-TYPE is not ND-ADVERTISEMENT" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_MPLS_LABEL)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8847) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8848))) {
				logging::warn << "[rofl][match] rejecting ofp_match: MPLS-LABEL defined while ETH-TYPE is not MPLS/MPLS-UPSTREAM" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_MPLS_TC)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8847) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8848))) {
				logging::warn << "[rofl][match] rejecting ofp_match: MPLS-TC defined while ETH-TYPE is not MPLS/MPLS-UPSTREAM" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_MPLS_BOS)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					((oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8847) && (oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x8848))) {
				logging::warn << "[rofl][match] rejecting ofp_match: MPLS-BOS defined while ETH-TYPE is not MPLS/MPLS-UPSTREAM" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_PBB_ISID)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x88e7)) {
				logging::warn << "[rofl][match] rejecting ofp_match: PBB-ISID defined while ETH-TYPE is not PBB" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

		if (oxmtlvs.has_match(OXM_TLV_BASIC_IPV6_EXTHDR)) {
			if ((not oxmtlvs.has_match(OXM_TLV_BASIC_ETH_TYPE)) ||
					(oxmtlvs.get_match(OXM_TLV_BASIC_ETH_TYPE).get_u16value() != 0x86dd)) {
				logging::warn << "[rofl][match] rejecting ofp_match: IPV6-EXTHDR defined while ETH-TYPE is not IPv6" << std::endl << oxmtlvs;
				throw eBadMatchBadPrereq();
			}
		}

	} break;
	default: {
		// do nothing
	};
	}
}


