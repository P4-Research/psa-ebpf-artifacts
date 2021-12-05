#include "ebpf_kernel.h"

#include <stdbool.h>
#include <linux/if_ether.h>
#include "psa.h"

#define EBPF_MASK(t, w) ((((t)(1)) << (w)) - (t)1)
#define BYTES(w) ((w) / 8)
#define write_partial(a, w, s, v) do { *((u8*)a) = ((*((u8*)a)) & ~(EBPF_MASK(u8, w) << s)) | (v << s) ; } while (0)
#define write_byte(base, offset, v) do { *(u8*)((base) + (offset)) = (v); } while (0)
#define bpf_trace_message(fmt, ...)

#define CLONE_MAX_PORTS 64
#define CLONE_MAX_INSTANCES 1
#define CLONE_MAX_CLONES (CLONE_MAX_PORTS * CLONE_MAX_INSTANCES)
#define CLONE_MAX_SESSIONS 1024
#define DEVMAP_SIZE 256

#ifndef PSA_PORT_RECIRCULATE
#error "PSA_PORT_RECIRCULATE not specified, please use -DPSA_PORT_RECIRCULATE=n option to specify index of recirculation interface (see the result of command 'ip link')"
#endif
#define P4C_PSA_PORT_RECIRCULATE 0xfffffffa

struct internal_metadata {
    __u16 pkt_ether_type;
} __attribute__((aligned(4)));

struct list_key_t {
    __u32 port;
    __u16 instance;
};
typedef struct list_key_t elem_t;

struct element {
    struct clone_session_entry entry;
    elem_t next_id;
} __attribute__((aligned(4)));


struct ethernet_t {
    u64 dst_addr; /* mac_addr_t */
    u64 src_addr; /* mac_addr_t */
    u8 ebpf_valid;
};
struct eth_type_t {
    u16 value; /* bit<16> */
    u8 ebpf_valid;
};
struct vlan_tag_t {
    u16 eth_type; /* bit<16> */
    u8 pri; /* bit<3> */
    u8 cfi; /* bit<1> */
    u16 vlan_id; /* vlan_id_t */
    u8 ebpf_valid;
};
struct mpls_t {
    u32 label; /* bit<20> */
    u8 tc; /* bit<3> */
    u8 bos; /* bit<1> */
    u8 ttl; /* bit<8> */
    u8 ebpf_valid;
};
struct pppoe_t {
    u8 version; /* bit<4> */
    u8 type_id; /* bit<4> */
    u8 code; /* bit<8> */
    u16 session_id; /* bit<16> */
    u16 length; /* bit<16> */
    u16 protocol; /* bit<16> */
    u8 ebpf_valid;
};
struct ipv4_t {
    u8 version; /* bit<4> */
    u8 ihl; /* bit<4> */
    u8 dscp; /* bit<6> */
    u8 ecn; /* bit<2> */
    u16 total_len; /* bit<16> */
    u16 identification; /* bit<16> */
    u8 flags; /* bit<3> */
    u16 frag_offset; /* bit<13> */
    u8 ttl; /* bit<8> */
    u8 protocol; /* bit<8> */
    u16 hdr_checksum; /* bit<16> */
    u32 src_addr; /* bit<32> */
    u32 dst_addr; /* bit<32> */
    u8 ebpf_valid;
};
struct bridged_metadata_t {
    u32 line_id; /* bit<32> */
    u16 pppoe_session_id; /* bit<16> */
    u16 vlan_id; /* bit<12> */
    u8 bng_type; /* bit<8> */
    u8 fwd_type; /* bit<8> */
    u8 push_double_vlan; /* bit<8> */
    u16 inner_vlan_id; /* bit<12> */
    u8 ebpf_valid;
};
struct empty_metadata_t {
};
struct bng_meta_t {
    u8 type; /* bit<2> */
    u32 line_id; /* bit<32> */
    u16 pppoe_session_id; /* bit<16> */
    u32 ds_meter_result; /* bit<32> */
    u16 s_tag; /* vlan_id_t */
    u16 c_tag; /* vlan_id_t */
};
struct local_metadata_t {
    u16 ip_eth_type; /* bit<16> */
    u16 vlan_id; /* vlan_id_t */
    u8 vlan_pri; /* bit<3> */
    u8 vlan_cfi; /* bit<1> */
    u8 push_double_vlan; /* bool */
    u16 inner_vlan_id; /* vlan_id_t */
    u8 inner_vlan_pri; /* bit<3> */
    u8 inner_vlan_cfi; /* bit<1> */
    u32 mpls_label; /* mpls_label_t */
    u8 mpls_ttl; /* bit<8> */
    u8 skip_forwarding; /* bool */
    u8 fwd_type; /* fwd_type_t */
    u8 ip_proto; /* bit<8> */
    u16 l4_sport; /* bit<16> */
    u16 l4_dport; /* bit<16> */
    u32 ipv4_src_addr; /* bit<32> */
    u32 ipv4_dst_addr; /* bit<32> */
    struct bng_meta_t bng; /* bng_meta_t */
    u8 port_type; /* port_type_t */
};
struct headers_t {
    struct bridged_metadata_t bmd; /* bridged_metadata_t */
    struct ethernet_t ethernet; /* ethernet_t */
    struct vlan_tag_t vlan_tag; /* vlan_tag_t */
    struct vlan_tag_t inner_vlan_tag; /* vlan_tag_t */
    struct eth_type_t eth_type; /* eth_type_t */
    struct pppoe_t pppoe; /* pppoe_t */
    struct mpls_t mpls; /* mpls_t */
    struct ipv4_t ipv4; /* ipv4_t */
__u32 __helper_variable;
};
struct tuple_0 {
    u8 f0; /* bit<8> */
    u8 f1; /* bit<8> */
};
struct meter_value {
    u64 pir_period; /* bit<64> */
    u64 pir_unit_per_period; /* bit<64> */
    u64 cir_period; /* bit<64> */
    u64 cir_unit_per_period; /* bit<64> */
    u64 pbs; /* bit<64> */
    u64 cbs; /* bit<64> */
    u64 pbs_left; /* bit<64> */
    u64 cbs_left; /* bit<64> */
    u64 time_p; /* bit<64> */
    u64 time_c; /* bit<64> */
};
struct hdr_md {
    struct headers_t cpumap_hdr;
    struct local_metadata_t cpumap_usermeta;
    __u8 __hook;
};
struct xdp2tc_metadata {
    struct headers_t headers;
    struct psa_ingress_output_metadata_t ostd;
    __u32 packetOffsetInBits;
    __u16 pkt_ether_type;
} __attribute__((aligned(4)));


struct ingress_fwd_classifier_key {
    u64 field0; /* hdr.ethernet.dst_addr */
    u32 field1; /* standard_metadata.ingress_port */
    u16 field2; /* hdr.eth_type.value */
    u16 field3; /* local_metadata.ip_eth_type */
} __attribute__((aligned(8)));
#define MAX_INGRESS_FWD_CLASSIFIER_KEY_MASKS 3
struct ingress_fwd_classifier_key_mask {
    __u8 mask[sizeof(struct ingress_fwd_classifier_key)];
} __attribute__((aligned(8)));
#define INGRESS_FWD_CLASSIFIER_ACT_INGRESS_SET_FORWARDING_TYPE 1
struct ingress_fwd_classifier_value {
    unsigned int action;
    __u32 priority;
    union {
        struct {
        } _NoAction;
        struct {
            u8 fwd_type;
        } ingress_set_forwarding_type;
    } u;
};
struct ingress_fwd_classifier_value_mask {
    __u32 tuple_id;
    struct ingress_fwd_classifier_key_mask next_tuple_mask;
    __u8 has_next;
};
struct ingress_fwd_classifier_value_cache {
    struct ingress_fwd_classifier_value value;
    u8 hit;
};
struct ingress_ingress_port_vlan_key {
    u32 field0; /* standard_metadata.ingress_port */
    u16 field1; /* hdr.vlan_tag.vlan_id */
    u16 field2; /* hdr.inner_vlan_tag.vlan_id */
    u8 field3; /*     hdr.vlan_tag.ebpf_valid */
} __attribute__((aligned(4)));
#define MAX_INGRESS_INGRESS_PORT_VLAN_KEY_MASKS 3
struct ingress_ingress_port_vlan_key_mask {
    __u8 mask[sizeof(struct ingress_ingress_port_vlan_key)];
} __attribute__((aligned(4)));
#define INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_DENY 1
#define INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_PERMIT 2
#define INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_PERMIT_WITH_INTERNAL_VLAN 3
struct ingress_ingress_port_vlan_value {
    unsigned int action;
    __u32 priority;
    union {
        struct {
        } _NoAction;
        struct {
        } ingress_deny;
        struct {
            u8 port_type;
        } ingress_permit;
        struct {
            u16 vlan_id;
            u8 port_type;
        } ingress_permit_with_internal_vlan;
    } u;
};
struct ingress_ingress_port_vlan_value_mask {
    __u32 tuple_id;
    struct ingress_ingress_port_vlan_key_mask next_tuple_mask;
    __u8 has_next;
};
struct ingress_ingress_port_vlan_value_cache {
    struct ingress_ingress_port_vlan_value value;
    u8 hit;
};
struct ingress_next_vlan_key {
    u32 field0; /* ostd.egress_port */
} __attribute__((aligned(4)));
#define INGRESS_NEXT_VLAN_ACT_INGRESS_SET_VLAN 1
#define INGRESS_NEXT_VLAN_ACT_INGRESS_SET_DOUBLE_VLAN 2
struct ingress_next_vlan_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
            u16 vlan_id;
        } ingress_set_vlan;
        struct {
            u16 outer_vlan_id;
            u16 inner_vlan_id;
        } ingress_set_double_vlan;
    } u;
};
struct ingress_routing_v4_key {
    u32 prefixlen;
    u32 field0; /* local_metadata.ipv4_dst_addr */
} __attribute__((aligned(4)));
#define INGRESS_ROUTING_V4_ACT_INGRESS_ROUTE 1
struct ingress_routing_v4_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
            u32 port_num;
            u64 smac;
            u64 dmac;
        } ingress_route;
    } u;
};
struct ingress_routing_v4_value_cache {
    struct ingress_routing_v4_value value;
    u8 hit;
};
struct ingress_t_line_map_key {
    u16 field0; /* local_metadata.bng.s_tag */
    u16 field1; /* local_metadata.bng.c_tag */
} __attribute__((aligned(4)));
#define INGRESS_T_LINE_MAP_ACT_INGRESS_SET_LINE 1
struct ingress_t_line_map_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
            u32 line_id;
        } ingress_set_line;
    } u;
};
struct ingress_t_line_session_map_key {
    u32 field0; /* local_metadata.bng.line_id */
} __attribute__((aligned(4)));
#define INGRESS_T_LINE_SESSION_MAP_ACT_INGRESS_SET_SESSION 1
#define INGRESS_T_LINE_SESSION_MAP_ACT_INGRESS_DROP 2
struct ingress_t_line_session_map_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
            u16 pppoe_session_id;
        } ingress_set_session;
        struct {
        } ingress_drop;
    } u;
};
struct ingress_t_pppoe_cp_key {
    u8 field0; /* hdr.pppoe.code */
} __attribute__((aligned(4)));
#define INGRESS_T_PPPOE_CP_ACT_INGRESS_PUNT_TO_CPU 1
struct ingress_t_pppoe_cp_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
        } ingress_punt_to_cpu;
    } u;
};
struct ingress_t_pppoe_term_v4_key {
    u32 field0; /* local_metadata.bng.line_id */
    u32 field1; /* hdr.ipv4.src_addr */
    u16 field2; /* hdr.pppoe.session_id */
} __attribute__((aligned(4)));
#define INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_ENABLED_V4 1
#define INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_DISABLED 2
struct ingress_t_pppoe_term_v4_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
        } ingress_term_enabled_v4;
        struct {
        } ingress_term_disabled;
    } u;
};
struct ingress_t_qos_v4_key {
    u32 field0; /* local_metadata.bng.line_id */
    u32 field1; /* hdr.ipv4.src_addr */
    u8 field2; /* hdr.ipv4.dscp */
    u8 field3; /* hdr.ipv4.ecn */
} __attribute__((aligned(4)));
#define MAX_INGRESS_T_QOS_V4_KEY_MASKS 3
struct ingress_t_qos_v4_key_mask {
    __u8 mask[sizeof(struct ingress_t_qos_v4_key)];
} __attribute__((aligned(4)));
#define INGRESS_T_QOS_V4_ACT_INGRESS_QOS_PRIO 1
#define INGRESS_T_QOS_V4_ACT_INGRESS_QOS_BESTEFF 2
struct ingress_t_qos_v4_value {
    unsigned int action;
    __u32 priority;
    union {
        struct {
        } _NoAction;
        struct {
        } ingress_qos_prio;
        struct {
        } ingress_qos_besteff;
    } u;
};
struct ingress_t_qos_v4_value_mask {
    __u32 tuple_id;
    struct ingress_t_qos_v4_key_mask next_tuple_mask;
    __u8 has_next;
};
struct ingress_t_qos_v4_value_cache {
    struct ingress_t_qos_v4_value value;
    u8 hit;
};
typedef u32 ingress_c_control_key;
typedef struct {
    u32 packets;
} ingress_c_control_value;
typedef u32 ingress_c_dropped_key;
typedef struct {
    u32 bytes;
} ingress_c_dropped_value;
typedef u32 ingress_c_line_rx_key;
typedef struct {
    u32 bytes;
} ingress_c_line_rx_value;
typedef u32 ingress_c_terminated_key;
typedef struct {
    u32 bytes;
} ingress_c_terminated_value;
typedef u32 ingress_m_besteff_key;
typedef u32 ingress_m_prio_key;
struct indirect_meter {
    struct meter_value value; /* struct meter_value */
    struct bpf_spin_lock lock; /* struct bpf_spin_lock */
};
struct egress_egress_vlan_key {
    u16 field0; /* hdr.bmd.vlan_id */
    u32 field1; /* istd.egress_port */
} __attribute__((aligned(4)));
#define EGRESS_EGRESS_VLAN_ACT_EGRESS_PUSH_VLAN 1
#define EGRESS_EGRESS_VLAN_ACT_EGRESS_POP_VLAN 2
#define EGRESS_EGRESS_VLAN_ACT_EGRESS_DROP 3
struct egress_egress_vlan_value {
    unsigned int action;
    union {
        struct {
        } _NoAction;
        struct {
        } egress_push_vlan;
        struct {
        } egress_pop_vlan;
        struct {
        } egress_drop;
    } u;
};
typedef u32 egress_c_line_tx_key;
typedef struct {
    u32 bytes;
} egress_c_line_tx_value;

struct bpf_map_def SEC("maps") tx_port = {
    .type          = BPF_MAP_TYPE_DEVMAP,
    .key_size      = sizeof(int),
    .value_size    = sizeof(struct bpf_devmap_val),
    .max_entries   = DEVMAP_SIZE,
};

REGISTER_START()
REGISTER_TABLE_INNER(clone_session_tbl_inner, BPF_MAP_TYPE_HASH, elem_t, struct element, 64, 1, 1)
BPF_ANNOTATE_KV_PAIR(clone_session_tbl_inner, elem_t, struct element)
REGISTER_TABLE_OUTER(clone_session_tbl, BPF_MAP_TYPE_ARRAY_OF_MAPS, __u32, __u32, 1024, 1, clone_session_tbl_inner)
BPF_ANNOTATE_KV_PAIR(clone_session_tbl, __u32, __u32)
REGISTER_TABLE_INNER(multicast_grp_tbl_inner, BPF_MAP_TYPE_HASH, elem_t, struct element, 64, 2, 2)
BPF_ANNOTATE_KV_PAIR(multicast_grp_tbl_inner, elem_t, struct element)
REGISTER_TABLE_OUTER(multicast_grp_tbl, BPF_MAP_TYPE_ARRAY_OF_MAPS, __u32, __u32, 1024, 2, multicast_grp_tbl_inner)
BPF_ANNOTATE_KV_PAIR(multicast_grp_tbl, __u32, __u32)
REGISTER_TABLE(ingress_fwd_classifier_prefixes, BPF_MAP_TYPE_HASH, struct ingress_fwd_classifier_key_mask, struct ingress_fwd_classifier_value_mask, 1024)
BPF_ANNOTATE_KV_PAIR(ingress_fwd_classifier_prefixes, struct ingress_fwd_classifier_key_mask, struct ingress_fwd_classifier_value_mask)
REGISTER_TABLE_INNER(ingress_fwd_classifier_tuple, BPF_MAP_TYPE_HASH, struct ingress_fwd_classifier_key, struct ingress_fwd_classifier_value, 1024, 3, 3)
BPF_ANNOTATE_KV_PAIR(ingress_fwd_classifier_tuple, struct ingress_fwd_classifier_key, struct ingress_fwd_classifier_value)
REGISTER_TABLE_OUTER(ingress_fwd_classifier_tuples_map, BPF_MAP_TYPE_ARRAY_OF_MAPS, __u32, __u32, 1024, 3, ingress_fwd_classifier_tuple)
BPF_ANNOTATE_KV_PAIR(ingress_fwd_classifier_tuples_map, __u32, __u32)
REGISTER_TABLE(ingress_fwd_classifier_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_fwd_classifier_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_fwd_classifier_defaultAction, u32, struct ingress_fwd_classifier_value)
REGISTER_TABLE(ingress_fwd_classifier_cache, BPF_MAP_TYPE_LRU_HASH, struct ingress_fwd_classifier_key, struct ingress_fwd_classifier_value_cache, 512)
BPF_ANNOTATE_KV_PAIR(ingress_fwd_classifier_cache, struct ingress_fwd_classifier_key, struct ingress_fwd_classifier_value_cache)
REGISTER_TABLE(ingress_ingress_port_vlan_prefixes, BPF_MAP_TYPE_HASH, struct ingress_ingress_port_vlan_key_mask, struct ingress_ingress_port_vlan_value_mask, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_ingress_port_vlan_prefixes, struct ingress_ingress_port_vlan_key_mask, struct ingress_ingress_port_vlan_value_mask)
REGISTER_TABLE_INNER(ingress_ingress_port_vlan_tuple, BPF_MAP_TYPE_HASH, struct ingress_ingress_port_vlan_key, struct ingress_ingress_port_vlan_value, 8192, 4, 4)
BPF_ANNOTATE_KV_PAIR(ingress_ingress_port_vlan_tuple, struct ingress_ingress_port_vlan_key, struct ingress_ingress_port_vlan_value)
REGISTER_TABLE_OUTER(ingress_ingress_port_vlan_tuples_map, BPF_MAP_TYPE_ARRAY_OF_MAPS, __u32, __u32, 8192, 4, ingress_ingress_port_vlan_tuple)
BPF_ANNOTATE_KV_PAIR(ingress_ingress_port_vlan_tuples_map, __u32, __u32)
REGISTER_TABLE(ingress_ingress_port_vlan_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_ingress_port_vlan_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_ingress_port_vlan_defaultAction, u32, struct ingress_ingress_port_vlan_value)
REGISTER_TABLE(ingress_ingress_port_vlan_cache, BPF_MAP_TYPE_LRU_HASH, struct ingress_ingress_port_vlan_key, struct ingress_ingress_port_vlan_value_cache, 4096)
BPF_ANNOTATE_KV_PAIR(ingress_ingress_port_vlan_cache, struct ingress_ingress_port_vlan_key, struct ingress_ingress_port_vlan_value_cache)
REGISTER_TABLE(ingress_next_vlan, BPF_MAP_TYPE_HASH, struct ingress_next_vlan_key, struct ingress_next_vlan_value, 1024)
BPF_ANNOTATE_KV_PAIR(ingress_next_vlan, struct ingress_next_vlan_key, struct ingress_next_vlan_value)
REGISTER_TABLE(ingress_next_vlan_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_next_vlan_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_next_vlan_defaultAction, u32, struct ingress_next_vlan_value)
REGISTER_TABLE_FLAGS(ingress_routing_v4, BPF_MAP_TYPE_LPM_TRIE, struct ingress_routing_v4_key, struct ingress_routing_v4_value, 1024, BPF_F_NO_PREALLOC)
BPF_ANNOTATE_KV_PAIR(ingress_routing_v4, struct ingress_routing_v4_key, struct ingress_routing_v4_value)
REGISTER_TABLE(ingress_routing_v4_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_routing_v4_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_routing_v4_defaultAction, u32, struct ingress_routing_v4_value)
REGISTER_TABLE(ingress_routing_v4_cache, BPF_MAP_TYPE_LRU_HASH, struct ingress_routing_v4_key, struct ingress_routing_v4_value_cache, 512)
BPF_ANNOTATE_KV_PAIR(ingress_routing_v4_cache, struct ingress_routing_v4_key, struct ingress_routing_v4_value_cache)
REGISTER_TABLE(ingress_t_line_map, BPF_MAP_TYPE_HASH, struct ingress_t_line_map_key, struct ingress_t_line_map_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_t_line_map, struct ingress_t_line_map_key, struct ingress_t_line_map_value)
REGISTER_TABLE(ingress_t_line_map_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_t_line_map_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_t_line_map_defaultAction, u32, struct ingress_t_line_map_value)
REGISTER_TABLE(ingress_t_line_session_map, BPF_MAP_TYPE_HASH, struct ingress_t_line_session_map_key, struct ingress_t_line_session_map_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_t_line_session_map, struct ingress_t_line_session_map_key, struct ingress_t_line_session_map_value)
REGISTER_TABLE(ingress_t_line_session_map_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_t_line_session_map_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_t_line_session_map_defaultAction, u32, struct ingress_t_line_session_map_value)
REGISTER_TABLE(ingress_t_pppoe_cp, BPF_MAP_TYPE_HASH, struct ingress_t_pppoe_cp_key, struct ingress_t_pppoe_cp_value, 16)
BPF_ANNOTATE_KV_PAIR(ingress_t_pppoe_cp, struct ingress_t_pppoe_cp_key, struct ingress_t_pppoe_cp_value)
REGISTER_TABLE(ingress_t_pppoe_cp_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_t_pppoe_cp_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_t_pppoe_cp_defaultAction, u32, struct ingress_t_pppoe_cp_value)
REGISTER_TABLE(ingress_t_pppoe_term_v4, BPF_MAP_TYPE_HASH, struct ingress_t_pppoe_term_v4_key, struct ingress_t_pppoe_term_v4_value, 32768)
BPF_ANNOTATE_KV_PAIR(ingress_t_pppoe_term_v4, struct ingress_t_pppoe_term_v4_key, struct ingress_t_pppoe_term_v4_value)
REGISTER_TABLE(ingress_t_pppoe_term_v4_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_t_pppoe_term_v4_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_t_pppoe_term_v4_defaultAction, u32, struct ingress_t_pppoe_term_v4_value)
REGISTER_TABLE(ingress_t_qos_v4_prefixes, BPF_MAP_TYPE_HASH, struct ingress_t_qos_v4_key_mask, struct ingress_t_qos_v4_value_mask, 256)
BPF_ANNOTATE_KV_PAIR(ingress_t_qos_v4_prefixes, struct ingress_t_qos_v4_key_mask, struct ingress_t_qos_v4_value_mask)
REGISTER_TABLE_INNER(ingress_t_qos_v4_tuple, BPF_MAP_TYPE_HASH, struct ingress_t_qos_v4_key, struct ingress_t_qos_v4_value, 256, 5, 5)
BPF_ANNOTATE_KV_PAIR(ingress_t_qos_v4_tuple, struct ingress_t_qos_v4_key, struct ingress_t_qos_v4_value)
REGISTER_TABLE_OUTER(ingress_t_qos_v4_tuples_map, BPF_MAP_TYPE_ARRAY_OF_MAPS, __u32, __u32, 256, 5, ingress_t_qos_v4_tuple)
BPF_ANNOTATE_KV_PAIR(ingress_t_qos_v4_tuples_map, __u32, __u32)
REGISTER_TABLE(ingress_t_qos_v4_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct ingress_t_qos_v4_value, 1)
BPF_ANNOTATE_KV_PAIR(ingress_t_qos_v4_defaultAction, u32, struct ingress_t_qos_v4_value)
REGISTER_TABLE(ingress_t_qos_v4_cache, BPF_MAP_TYPE_LRU_HASH, struct ingress_t_qos_v4_key, struct ingress_t_qos_v4_value_cache, 128)
BPF_ANNOTATE_KV_PAIR(ingress_t_qos_v4_cache, struct ingress_t_qos_v4_key, struct ingress_t_qos_v4_value_cache)
REGISTER_TABLE(ingress_c_control, BPF_MAP_TYPE_ARRAY, u32, ingress_c_control_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_c_control, u32, ingress_c_control_value)
REGISTER_TABLE(ingress_c_dropped, BPF_MAP_TYPE_ARRAY, u32, ingress_c_dropped_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_c_dropped, u32, ingress_c_dropped_value)
REGISTER_TABLE(ingress_c_line_rx, BPF_MAP_TYPE_ARRAY, u32, ingress_c_line_rx_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_c_line_rx, u32, ingress_c_line_rx_value)
REGISTER_TABLE(ingress_c_terminated, BPF_MAP_TYPE_ARRAY, u32, ingress_c_terminated_value, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_c_terminated, u32, ingress_c_terminated_value)
REGISTER_TABLE(ingress_m_besteff, BPF_MAP_TYPE_HASH, ingress_m_besteff_key, struct indirect_meter, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_m_besteff, ingress_m_besteff_key, struct indirect_meter)
REGISTER_TABLE(ingress_m_prio, BPF_MAP_TYPE_HASH, ingress_m_prio_key, struct indirect_meter, 8192)
BPF_ANNOTATE_KV_PAIR(ingress_m_prio, ingress_m_prio_key, struct indirect_meter)
REGISTER_TABLE(egress_egress_vlan, BPF_MAP_TYPE_HASH, struct egress_egress_vlan_key, struct egress_egress_vlan_value, 1024)
BPF_ANNOTATE_KV_PAIR(egress_egress_vlan, struct egress_egress_vlan_key, struct egress_egress_vlan_value)
REGISTER_TABLE(egress_egress_vlan_defaultAction, BPF_MAP_TYPE_ARRAY, u32, struct egress_egress_vlan_value, 1)
BPF_ANNOTATE_KV_PAIR(egress_egress_vlan_defaultAction, u32, struct egress_egress_vlan_value)
REGISTER_TABLE(egress_c_line_tx, BPF_MAP_TYPE_ARRAY, u32, egress_c_line_tx_value, 8192)
BPF_ANNOTATE_KV_PAIR(egress_c_line_tx, u32, egress_c_line_tx_value)
REGISTER_TABLE(xdp2tc_shared_map, BPF_MAP_TYPE_PERCPU_ARRAY, u32, struct xdp2tc_metadata, 1)
BPF_ANNOTATE_KV_PAIR(xdp2tc_shared_map, u32, struct xdp2tc_metadata)
REGISTER_TABLE(bridged_headers, BPF_MAP_TYPE_PERCPU_ARRAY, u32, struct headers_t, 1)
BPF_ANNOTATE_KV_PAIR(bridged_headers, u32, struct headers_t)
REGISTER_TABLE(egress_progs_table, BPF_MAP_TYPE_PROG_ARRAY, u32, u32, 1)
BPF_ANNOTATE_KV_PAIR(egress_progs_table, u32, u32)
REGISTER_TABLE(hdr_md_cpumap, BPF_MAP_TYPE_PERCPU_ARRAY, u32, struct hdr_md, 2)
BPF_ANNOTATE_KV_PAIR(hdr_md_cpumap, u32, struct hdr_md)
REGISTER_END()

static __always_inline
void crc16_update(u16 * reg, const u8 * data, u16 data_size, const u16 poly) {
    data += data_size - 1;
    for (u16 i = 0; i < data_size; i++) {
        bpf_trace_message("CRC16: data byte: %x\n", *data);
        *reg ^= *data;
        for (u8 bit = 0; bit < 8; bit++) {
            *reg = (*reg) & 1 ? ((*reg) >> 1) ^ poly : (*reg) >> 1;
        }
        data--;
    }
}
static __always_inline u16 crc16_finalize(u16 reg, const u16 poly) {
    return reg;
}
static __always_inline
void crc32_update(u32 * reg, const u8 * data, u16 data_size, const u32 poly) {
    data += data_size - 1;
    for (u16 i = 0; i < data_size; i++) {
        bpf_trace_message("CRC32: data byte: %x\n", *data);
        *reg ^= *data;
        for (u8 bit = 0; bit < 8; bit++) {
            *reg = (*reg) & 1 ? ((*reg) >> 1) ^ poly : (*reg) >> 1;
        }
        data--;
    }
}
static __always_inline u32 crc32_finalize(u32 reg, const u32 poly) {
    return reg ^ 0xFFFFFFFF;
}
inline u16 csum16_add(u16 csum, u16 addend) {
    u16 res = csum;
    res += addend;
    return (res + (res < addend));
}
inline u16 csum16_sub(u16 csum, u16 addend) {
    return csum16_add(csum, ~addend);
}
static __always_inline
int do_for_each(SK_BUFF *skb, void *map, unsigned int max_iter, void (*a)(SK_BUFF *, void *))
{
    elem_t head_idx = {0, 0};
    struct element *elem = bpf_map_lookup_elem(map, &head_idx);
    if (!elem) {
        return -1;
    }
    if (elem->next_id.port == 0 && elem->next_id.instance == 0) {
               return 0;
    }
    elem_t next_id = elem->next_id;
    for (unsigned int i = 0; i < max_iter; i++) {
        struct element *elem = bpf_map_lookup_elem(map, &next_id);
        if (!elem) {
            break;
        }
        a(skb, &elem->entry);
        if (elem->next_id.port == 0 && elem->next_id.instance == 0) {
            break;
        }
        next_id = elem->next_id;
    }
    return 0;
}

static __always_inline
void do_clone(SK_BUFF *skb, void *data)
{
    struct clone_session_entry *entry = (struct clone_session_entry *) data;
    bpf_clone_redirect(skb, entry->egress_port, 0);
}

static __always_inline
int do_packet_clones(SK_BUFF * skb, void * map, __u32 session_id, PSA_PacketPath_t new_pkt_path, __u8 caller_id)
{
    struct psa_global_metadata * meta = (struct psa_global_metadata *) skb->cb;
    void * inner_map;
    inner_map = bpf_map_lookup_elem(map, &session_id);
    if (inner_map != NULL) {
        PSA_PacketPath_t original_pkt_path = meta->packet_path;
        meta->packet_path = new_pkt_path;
        if (do_for_each(skb, inner_map, CLONE_MAX_CLONES, &do_clone) < 0) {
            return -1;
        }
        meta->packet_path = original_pkt_path;
    } else {
    }
    return 0;
 }

static __always_inline
enum PSA_MeterColor_t meter_execute(struct meter_value *value, void *lock, u32 *packet_len, u64 *time_ns) {
    if (value != NULL && value->pir_period != 0) {
        u64 delta_p, delta_c;
        u64 n_periods_p, n_periods_c, tokens_pbs, tokens_cbs;
        bpf_spin_lock(lock);
        delta_p = *time_ns - value->time_p;
        delta_c = *time_ns - value->time_c;

        n_periods_p = delta_p / value->pir_period;
        n_periods_c = delta_c / value->cir_period;

        value->time_p += n_periods_p * value->pir_period;
        value->time_c += n_periods_c * value->cir_period;

        tokens_pbs = value->pbs_left + n_periods_p * value->pir_unit_per_period;
        if (tokens_pbs > value->pbs) {
            tokens_pbs = value->pbs;
        }
        tokens_cbs = value->cbs_left + n_periods_c * value->cir_unit_per_period;
        if (tokens_cbs > value->cbs) {
            tokens_cbs = value->cbs;
        }

        if (*packet_len > tokens_pbs) {
            value->pbs_left = tokens_pbs;
            value->cbs_left = tokens_cbs;
            bpf_spin_unlock(lock);
            return RED;
        }

        if (*packet_len > tokens_cbs) {
            value->pbs_left = tokens_pbs - *packet_len;
            value->cbs_left = tokens_cbs;
            bpf_spin_unlock(lock);
            return YELLOW;
        }

        value->pbs_left = tokens_pbs - *packet_len;
        value->cbs_left = tokens_cbs - *packet_len;
        bpf_spin_unlock(lock);
        return GREEN;
    } else {
        // From P4Runtime spec. No value - return default GREEN.
        return GREEN;
    }
}

static __always_inline
enum PSA_MeterColor_t meter_execute_bytes_value(void *value, void *lock, u32 *packet_len, u64 *time_ns) {
    return meter_execute(value, lock, packet_len, time_ns);
}

static __always_inline
enum PSA_MeterColor_t meter_execute_bytes(void *map, u32 *packet_len, void *key, u64 *time_ns) {
    struct meter_value *value = BPF_MAP_LOOKUP_ELEM(*map, key);
    return meter_execute_bytes_value(value, ((void *)value) + sizeof(struct meter_value), packet_len, time_ns);
}

static __always_inline
enum PSA_MeterColor_t meter_execute_packets_value(void *value, void *lock, u64 *time_ns) {
    u32 len = 1;
    return meter_execute(value, lock, &len, time_ns);
}

static __always_inline
enum PSA_MeterColor_t meter_execute_packets(void *map, void *key, u64 *time_ns) {
    struct meter_value *value = BPF_MAP_LOOKUP_ELEM(*map, key);
    return meter_execute_packets_value(value, ((void *)value) + sizeof(struct meter_value), time_ns);
}


SEC("xdp/map-initializer")
int map_initialize() {
    u32 ebpf_zero = 0;
    struct ingress_fwd_classifier_value value_0 = {
        .action = INGRESS_FWD_CLASSIFIER_ACT_INGRESS_SET_FORWARDING_TYPE,
        .u = {.ingress_set_forwarding_type = {0,}},
    };
    int ret = BPF_MAP_UPDATE_ELEM(ingress_fwd_classifier_defaultAction, &ebpf_zero, &value_0, BPF_ANY);
    if (ret) {
    } else {
    }
    struct ingress_ingress_port_vlan_value value_1 = {
        .action = INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_DENY,
        .u = {.ingress_deny = {}},
    };
    int ret_0 = BPF_MAP_UPDATE_ELEM(ingress_ingress_port_vlan_defaultAction, &ebpf_zero, &value_1, BPF_ANY);
    if (ret_0) {
    } else {
    }
    struct ingress_t_line_map_value value_2 = {
        .action = INGRESS_T_LINE_MAP_ACT_INGRESS_SET_LINE,
        .u = {.ingress_set_line = {0,}},
    };
    int ret_1 = BPF_MAP_UPDATE_ELEM(ingress_t_line_map_defaultAction, &ebpf_zero, &value_2, BPF_ANY);
    if (ret_1) {
    } else {
    }
    struct ingress_t_pppoe_term_v4_value value_3 = {
        .action = INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_DISABLED,
        .u = {.ingress_term_disabled = {}},
    };
    int ret_2 = BPF_MAP_UPDATE_ELEM(ingress_t_pppoe_term_v4_defaultAction, &ebpf_zero, &value_3, BPF_ANY);
    if (ret_2) {
    } else {
    }
    struct ingress_t_qos_v4_value value_4 = {
        .action = INGRESS_T_QOS_V4_ACT_INGRESS_QOS_BESTEFF,
        .u = {.ingress_qos_besteff = {}},
    };
    int ret_3 = BPF_MAP_UPDATE_ELEM(ingress_t_qos_v4_defaultAction, &ebpf_zero, &value_4, BPF_ANY);
    if (ret_3) {
    } else {
    }
    struct egress_egress_vlan_value value_5 = {
        .action = EGRESS_EGRESS_VLAN_ACT_EGRESS_DROP,
        .u = {.egress_drop = {}},
    };
    int ret_4 = BPF_MAP_UPDATE_ELEM(egress_egress_vlan_defaultAction, &ebpf_zero, &value_5, BPF_ANY);
    if (ret_4) {
    } else {
    }

    return 0;
}

SEC("xdp_ingress/xdp-ingress")
int xdp_ingress_func(struct xdp_md *skb) {
    struct empty_metadata_t resubmit_meta;

    struct hdr_md *hdrMd;
    struct headers_t *hdr;
    struct local_metadata_t *local_metadata;

    unsigned ebpf_packetOffsetInBits = 0;
    unsigned ebpf_packetOffsetInBits_save = 0;
    ParserError_t ebpf_errorCode = NoError;
    void* pkt = ((void*)(long)skb->data);
    void* ebpf_packetEnd = ((void*)(long)skb->data_end);
    u32 ebpf_zero = 0;
    u32 ebpf_one = 1;
    unsigned char ebpf_byte;
    u32 pkt_len = skb->data_end - skb->data;
    u64 tstamp = bpf_ktime_get_ns();
    hdrMd = BPF_MAP_LOOKUP_ELEM(hdr_md_cpumap, &ebpf_zero);
    if (!hdrMd)
        return XDP_DROP;
    __builtin_memset(hdrMd, 0, sizeof(struct hdr_md));

    hdr = &(hdrMd->cpumap_hdr);
    local_metadata = &(hdrMd->cpumap_usermeta);
    struct psa_ingress_output_metadata_t ostd = {
            .drop = true,
    };

    struct eth_type_t eth_type_0;
    struct eth_type_t eth_type_1;
    start: {
/* extract(hdr->ethernet) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96 + 16)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->ethernet.dst_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
        ebpf_packetOffsetInBits += 48;

        hdr->ethernet.src_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
        ebpf_packetOffsetInBits += 48;

        hdr->ethernet.ebpf_valid = 1;

local_metadata->vlan_id = 4094;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_0.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_0.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_0.value) {
            case 34984: goto parse_vlan_tag;
            case 37120: goto parse_vlan_tag;
            case 33024: goto parse_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_vlan_tag: {
/* extract(hdr->vlan_tag) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->vlan_tag.ebpf_valid = 1;

local_metadata->bng.s_tag = hdr->vlan_tag.vlan_id;local_metadata->vlan_id = hdr->vlan_tag.vlan_id;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_1.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_1.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_1.value) {
            case 33024: goto parse_inner_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_inner_vlan_tag: {
/* extract(hdr->inner_vlan_tag) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->inner_vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->inner_vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->inner_vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->inner_vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->inner_vlan_tag.ebpf_valid = 1;

local_metadata->vlan_id = hdr->inner_vlan_tag.vlan_id;local_metadata->bng.c_tag = hdr->inner_vlan_tag.vlan_id;        goto parse_eth_type;
    }
    parse_eth_type: {
/* extract(hdr->eth_type) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->eth_type.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->eth_type.ebpf_valid = 1;

        switch (hdr->eth_type.value) {
            case 34887: goto parse_mpls;
            case 2048: goto parse_ipv4;
            case 34915: goto parse_pppoe;
            case 34916: goto parse_pppoe;
            default: goto accept;
        }
    }
    parse_pppoe: {
/* extract(hdr->pppoe) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 64 + 0)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->pppoe.version = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->pppoe.type_id = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->pppoe.code = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->pppoe.session_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->pppoe.length = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->pppoe.protocol = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->pppoe.ebpf_valid = 1;

        switch (hdr->pppoe.protocol) {
            case 33: goto parse_ipv4;
            default: goto accept;
        }
    }
    parse_mpls: {
/* extract(hdr->mpls) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 12)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->mpls.label = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits)) >> 12) & EBPF_MASK(u32, 20));
        ebpf_packetOffsetInBits += 20;

        hdr->mpls.tc = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 1) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->mpls.bos = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->mpls.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->mpls.ebpf_valid = 1;

local_metadata->mpls_label = hdr->mpls.label;local_metadata->mpls_ttl = hdr->mpls.ttl;        goto parse_ipv4;
    }
    parse_ipv4: {
/* extract(hdr->ipv4) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160 + 0)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->ipv4.version = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->ipv4.ihl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->ipv4.dscp = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 2) & EBPF_MASK(u8, 6));
        ebpf_packetOffsetInBits += 6;

        hdr->ipv4.ecn = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 2));
        ebpf_packetOffsetInBits += 2;

        hdr->ipv4.total_len = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.identification = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.flags = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->ipv4.frag_offset = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 13));
        ebpf_packetOffsetInBits += 13;

        hdr->ipv4.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->ipv4.protocol = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->ipv4.hdr_checksum = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.src_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 32;

        hdr->ipv4.dst_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 32;

        hdr->ipv4.ebpf_valid = 1;

local_metadata->ip_proto = hdr->ipv4.protocol;local_metadata->ip_eth_type = 2048;local_metadata->ipv4_src_addr = hdr->ipv4.src_addr;local_metadata->ipv4_dst_addr = hdr->ipv4.dst_addr;        goto accept;
    }

    reject: {
        if (ebpf_errorCode == 0) {
            return XDP_ABORTED;
        }
        goto accept;
    }


    accept: {
        struct psa_ingress_input_metadata_t standard_metadata = {
            .ingress_port = skb->ingress_ifindex,
            .packet_path = 0,
            .parser_error = ebpf_errorCode,
    };
        standard_metadata.ingress_timestamp = tstamp;
        u8 hit_3;
        u8 tmp;
        struct psa_ingress_output_metadata_t meta_3;
        __builtin_memset((void *) &meta_3, 0, sizeof(struct psa_ingress_output_metadata_t ));
        u32 egress_port_1;
        struct psa_ingress_output_metadata_t meta_4;
        __builtin_memset((void *) &meta_4, 0, sizeof(struct psa_ingress_output_metadata_t ));
        struct psa_ingress_output_metadata_t meta_5;
        __builtin_memset((void *) &meta_5, 0, sizeof(struct psa_ingress_output_metadata_t ));
        u8 hasReturned;
        {
hasReturned = false;
                        {
                /* construct key */
                struct ingress_ingress_port_vlan_key key = {};
                key.field0 = standard_metadata.ingress_port;
                key.field1 = hdr->vlan_tag.vlan_id;
                key.field2 = hdr->inner_vlan_tag.vlan_id;
                key.field3 =                 hdr->vlan_tag.ebpf_valid;
                /* value */
                struct ingress_ingress_port_vlan_value *value = NULL;
                /* perform lookup */
                struct ingress_ingress_port_vlan_value_cache* cached_value = NULL;
                cached_value = BPF_MAP_LOOKUP_ELEM(ingress_ingress_port_vlan_cache, &key);
                if (cached_value != NULL) {
                    value = &(cached_value->value);
                    hit_3 = cached_value->hit;
                } else {
                    struct ingress_ingress_port_vlan_key_mask head = {0};
                    struct ingress_ingress_port_vlan_value_mask *val = BPF_MAP_LOOKUP_ELEM(ingress_ingress_port_vlan_prefixes, &head);
                    if (val && val->has_next != 0) {
                        struct ingress_ingress_port_vlan_key_mask next = val->next_tuple_mask;
                        #pragma clang loop unroll(disable)
                        for (int i = 0; i < MAX_INGRESS_INGRESS_PORT_VLAN_KEY_MASKS; i++) {
                            struct ingress_ingress_port_vlan_value_mask *v = BPF_MAP_LOOKUP_ELEM(ingress_ingress_port_vlan_prefixes, &next);
                            if (!v) {
                                break;
                            }
                            struct ingress_ingress_port_vlan_key k = {};
                            __u32 *chunk = ((__u32 *) &k);
                            __u32 *mask = ((__u32 *) &next);
                            #pragma clang loop unroll(disable)
                            for (int i = 0; i < sizeof(struct ingress_ingress_port_vlan_key_mask) / 4; i++) {
                                chunk[i] = ((__u32 *) &key)[i] & mask[i];
                            }
                            __u32 tuple_id = v->tuple_id;
                            next = v->next_tuple_mask;
                            struct bpf_elf_map *tuple = BPF_MAP_LOOKUP_ELEM(ingress_ingress_port_vlan_tuples_map, &tuple_id);
                            if (!tuple) {
                                break;
                            }
                            struct ingress_ingress_port_vlan_value *tuple_entry = bpf_map_lookup_elem(tuple, &k);
                            if (!tuple_entry) {
                                if (v->has_next == 0) {
                                    break;
                                }
                                continue;
                            }
                            if (value == NULL || tuple_entry->priority > value->priority) {
                                value = tuple_entry;
                            }
                            if (v->has_next == 0) {
                                break;
                            }
                        }
                    }
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_3 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(ingress_ingress_port_vlan_defaultAction, &ebpf_zero);
                    } else {
                        hit_3 = 1;
                    }
                    if (value != NULL) {
                        struct ingress_ingress_port_vlan_value_cache cache_update = {0};
                        cache_update.hit = hit_3;
                        __builtin_memcpy((void *) &(cache_update.value), (void *) value, sizeof(struct ingress_ingress_port_vlan_value));
                        BPF_MAP_UPDATE_ELEM(ingress_ingress_port_vlan_cache, &key, &cache_update, BPF_ANY);
                    }
                }
                if (value != NULL) {
                    /* run action */
                    switch (value->action) {
                        case INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_DENY: 
                            {
local_metadata->skip_forwarding = true;
                                local_metadata->port_type = 0;
                            }
                            break;
                        case INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_PERMIT: 
                            {
local_metadata->port_type = value->u.ingress_permit.port_type;
                            }
                            break;
                        case INGRESS_INGRESS_PORT_VLAN_ACT_INGRESS_PERMIT_WITH_INTERNAL_VLAN: 
                            {
local_metadata->vlan_id = value->u.ingress_permit_with_internal_vlan.vlan_id;
                                local_metadata->port_type = value->u.ingress_permit_with_internal_vlan.port_type;
                            }
                            break;
                        default:
                            return XDP_ABORTED;
                    }
                } else {
                    return XDP_ABORTED;
                }
            }
;
                        {
                /* construct key */
                struct ingress_fwd_classifier_key key = {};
                key.field0 = hdr->ethernet.dst_addr;
                key.field1 = standard_metadata.ingress_port;
                key.field2 = hdr->eth_type.value;
                key.field3 = local_metadata->ip_eth_type;
                /* value */
                struct ingress_fwd_classifier_value *value = NULL;
                /* perform lookup */
                struct ingress_fwd_classifier_value_cache* cached_value = NULL;
                cached_value = BPF_MAP_LOOKUP_ELEM(ingress_fwd_classifier_cache, &key);
                if (cached_value != NULL) {
                    value = &(cached_value->value);
                    hit_3 = cached_value->hit;
                } else {
                    struct ingress_fwd_classifier_key_mask head = {0};
                    struct ingress_fwd_classifier_value_mask *val = BPF_MAP_LOOKUP_ELEM(ingress_fwd_classifier_prefixes, &head);
                    if (val && val->has_next != 0) {
                        struct ingress_fwd_classifier_key_mask next = val->next_tuple_mask;
                        #pragma clang loop unroll(disable)
                        for (int i = 0; i < MAX_INGRESS_FWD_CLASSIFIER_KEY_MASKS; i++) {
                            struct ingress_fwd_classifier_value_mask *v = BPF_MAP_LOOKUP_ELEM(ingress_fwd_classifier_prefixes, &next);
                            if (!v) {
                                break;
                            }
                            struct ingress_fwd_classifier_key k = {};
                            __u32 *chunk = ((__u32 *) &k);
                            __u32 *mask = ((__u32 *) &next);
                            #pragma clang loop unroll(disable)
                            for (int i = 0; i < sizeof(struct ingress_fwd_classifier_key_mask) / 4; i++) {
                                chunk[i] = ((__u32 *) &key)[i] & mask[i];
                            }
                            __u32 tuple_id = v->tuple_id;
                            next = v->next_tuple_mask;
                            struct bpf_elf_map *tuple = BPF_MAP_LOOKUP_ELEM(ingress_fwd_classifier_tuples_map, &tuple_id);
                            if (!tuple) {
                                break;
                            }
                            struct ingress_fwd_classifier_value *tuple_entry = bpf_map_lookup_elem(tuple, &k);
                            if (!tuple_entry) {
                                if (v->has_next == 0) {
                                    break;
                                }
                                continue;
                            }
                            if (value == NULL || tuple_entry->priority > value->priority) {
                                value = tuple_entry;
                            }
                            if (v->has_next == 0) {
                                break;
                            }
                        }
                    }
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_3 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(ingress_fwd_classifier_defaultAction, &ebpf_zero);
                    } else {
                        hit_3 = 1;
                    }
                    if (value != NULL) {
                        struct ingress_fwd_classifier_value_cache cache_update = {0};
                        cache_update.hit = hit_3;
                        __builtin_memcpy((void *) &(cache_update.value), (void *) value, sizeof(struct ingress_fwd_classifier_value));
                        BPF_MAP_UPDATE_ELEM(ingress_fwd_classifier_cache, &key, &cache_update, BPF_ANY);
                    }
                }
                if (value != NULL) {
                    /* run action */
                    switch (value->action) {
                        case INGRESS_FWD_CLASSIFIER_ACT_INGRESS_SET_FORWARDING_TYPE: 
                            {
local_metadata->fwd_type = value->u.ingress_set_forwarding_type.fwd_type;
                            }
                            break;
                        default:
                            return XDP_ABORTED;
                    }
                } else {
                    return XDP_ABORTED;
                }
            }
;
            if (local_metadata->skip_forwarding) {
;            }

            else {
if (local_metadata->fwd_type == 2) {
                    {
                        /* construct key */
                        struct ingress_routing_v4_key key = {};
                        key.prefixlen = sizeof(key)*8 - 32;
                        u32 tmp_field0 = local_metadata->ipv4_dst_addr;
                        key.field0 = bpf_htonl(tmp_field0);
                        /* value */
                        struct ingress_routing_v4_value *value = NULL;
                        /* perform lookup */
                        struct ingress_routing_v4_value_cache* cached_value = NULL;
                        cached_value = BPF_MAP_LOOKUP_ELEM(ingress_routing_v4_cache, &key);
                        if (cached_value != NULL) {
                            value = &(cached_value->value);
                            hit_3 = cached_value->hit;
                        } else {
                            value = BPF_MAP_LOOKUP_ELEM(ingress_routing_v4, &key);
                            if (value == NULL) {
                                /* miss; find default action */
                                hit_3 = 0;
                                value = BPF_MAP_LOOKUP_ELEM(ingress_routing_v4_defaultAction, &ebpf_zero);
                            } else {
                                hit_3 = 1;
                            }
                            if (value != NULL) {
                                struct ingress_routing_v4_value_cache cache_update = {0};
                                cache_update.hit = hit_3;
                                __builtin_memcpy((void *) &(cache_update.value), (void *) value, sizeof(struct ingress_routing_v4_value));
                                BPF_MAP_UPDATE_ELEM(ingress_routing_v4_cache, &key, &cache_update, BPF_ANY);
                            }
                        }
                        if (value != NULL) {
                            /* run action */
                            switch (value->action) {
                                case INGRESS_ROUTING_V4_ACT_INGRESS_ROUTE: 
                                    {
hdr->ethernet.src_addr = value->u.ingress_route.smac;
                                        hdr->ethernet.dst_addr = value->u.ingress_route.dmac;
                                        {
meta_3 = ostd;
                                            egress_port_1 = value->u.ingress_route.port_num;
                                            meta_3.drop = false;
                                            meta_3.multicast_group = 0;
                                            meta_3.egress_port = egress_port_1;
                                            ostd = meta_3;
                                        }
                                    }
                                    break;
                                case 0: 
                                    {
                                    }
                                    break;
                                default:
                                    return XDP_ABORTED;
                            }
                        } else {
                            return XDP_ABORTED;
                        }
                    }
;                }

                                {
                    /* construct key */
                    struct ingress_next_vlan_key key = {};
                    key.field0 = ostd.egress_port;
                    /* value */
                    struct ingress_next_vlan_value *value = NULL;
                    /* perform lookup */
                    value = BPF_MAP_LOOKUP_ELEM(ingress_next_vlan, &key);
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_3 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(ingress_next_vlan_defaultAction, &ebpf_zero);
                    } else {
                        hit_3 = 1;
                    }
                    if (value != NULL) {
                        /* run action */
                        switch (value->action) {
                            case INGRESS_NEXT_VLAN_ACT_INGRESS_SET_VLAN: 
                                {
local_metadata->vlan_id = value->u.ingress_set_vlan.vlan_id;
                                }
                                break;
                            case INGRESS_NEXT_VLAN_ACT_INGRESS_SET_DOUBLE_VLAN: 
                                {
local_metadata->vlan_id = value->u.ingress_set_double_vlan.outer_vlan_id;
                                    local_metadata->push_double_vlan = true;
                                    local_metadata->inner_vlan_id = value->u.ingress_set_double_vlan.inner_vlan_id;
                                    local_metadata->bng.s_tag = value->u.ingress_set_double_vlan.outer_vlan_id;
                                    local_metadata->bng.c_tag = value->u.ingress_set_double_vlan.inner_vlan_id;
                                }
                                break;
                            case 0: 
                                {
                                }
                                break;
                            default:
                                return XDP_ABORTED;
                        }
                    } else {
                        return XDP_ABORTED;
                    }
                }
;
            }
                        {
                /* construct key */
                struct ingress_t_line_map_key key = {};
                key.field0 = local_metadata->bng.s_tag;
                key.field1 = local_metadata->bng.c_tag;
                /* value */
                struct ingress_t_line_map_value *value = NULL;
                /* perform lookup */
                value = BPF_MAP_LOOKUP_ELEM(ingress_t_line_map, &key);
                if (value == NULL) {
                    /* miss; find default action */
                    hit_3 = 0;
                    value = BPF_MAP_LOOKUP_ELEM(ingress_t_line_map_defaultAction, &ebpf_zero);
                } else {
                    hit_3 = 1;
                }
                if (value != NULL) {
                    /* run action */
                    switch (value->action) {
                        case INGRESS_T_LINE_MAP_ACT_INGRESS_SET_LINE: 
                            {
local_metadata->bng.line_id = value->u.ingress_set_line.line_id;
                            }
                            break;
                        default:
                            return XDP_ABORTED;
                    }
                } else {
                    return XDP_ABORTED;
                }
            }
;
            if (            hdr->pppoe.ebpf_valid) {
local_metadata->bng.type = 1;
                                {
                    /* construct key */
                    struct ingress_t_pppoe_cp_key key = {};
                    key.field0 = hdr->pppoe.code;
                    /* value */
                    struct ingress_t_pppoe_cp_value *value = NULL;
                    /* perform lookup */
                    value = BPF_MAP_LOOKUP_ELEM(ingress_t_pppoe_cp, &key);
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_3 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(ingress_t_pppoe_cp_defaultAction, &ebpf_zero);
                    } else {
                        hit_3 = 1;
                    }
                    if (value != NULL) {
                        /* run action */
                        switch (value->action) {
                            case INGRESS_T_PPPOE_CP_ACT_INGRESS_PUNT_TO_CPU: 
                                {
ostd.multicast_group = 0;
                                    {
                                        ingress_c_control_value *value_6;
                                        ingress_c_control_key key_0 = local_metadata->bng.line_id;
                                        value_6 = BPF_MAP_LOOKUP_ELEM(ingress_c_control, &key_0);
                                        if (value_6 != NULL) {
                                            __sync_fetch_and_add(&(value_6->packets), 1);
                                        } else {
                                        }
                                    };
                                }
                                break;
                            case 0: 
                                {
                                }
                                break;
                            default:
                                return XDP_ABORTED;
                        }
                    } else {
                        return XDP_ABORTED;
                    }
                }
                if (hit_3) {
hasReturned = true;                }

                if (hasReturned) {
;                }

                else {
if (                    hdr->ipv4.ebpf_valid) {
                        unsigned int action_run = 0;
                        {
                            /* construct key */
                            struct ingress_t_pppoe_term_v4_key key = {};
                            key.field0 = local_metadata->bng.line_id;
                            key.field1 = hdr->ipv4.src_addr;
                            key.field2 = hdr->pppoe.session_id;
                            /* value */
                            struct ingress_t_pppoe_term_v4_value *value = NULL;
                            /* perform lookup */
                            value = BPF_MAP_LOOKUP_ELEM(ingress_t_pppoe_term_v4, &key);
                            if (value == NULL) {
                                /* miss; find default action */
                                hit_3 = 0;
                                value = BPF_MAP_LOOKUP_ELEM(ingress_t_pppoe_term_v4_defaultAction, &ebpf_zero);
                            } else {
                                hit_3 = 1;
                            }
                            if (value != NULL) {
                                /* run action */
                                switch (value->action) {
                                    case INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_ENABLED_V4: 
                                        {
{
hdr->eth_type.value = 2048;
                                                                                                hdr->pppoe.ebpf_valid = false;
                                                {
                                                    ingress_c_terminated_value *value_7;
                                                    ingress_c_terminated_key key_1 = local_metadata->bng.line_id;
                                                    value_7 = BPF_MAP_LOOKUP_ELEM(ingress_c_terminated, &key_1);
                                                    if (value_7 != NULL) {
                                                        __sync_fetch_and_add(&(value_7->bytes), pkt_len);
                                                    } else {
                                                    }
                                                };
                                            }
                                        }
                                        break;
                                    case INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_DISABLED: 
                                        {
local_metadata->bng.type = 0;
                                            {
meta_4 = ostd;
                                                meta_4.drop = true;
                                                ostd = meta_4;
                                            }
                                        }
                                        break;
                                    default:
                                        return XDP_ABORTED;
                                }
                                action_run = value->action;
                            } else {
                                return XDP_ABORTED;
                            }
                        }
                        switch (action_run) {
                            case INGRESS_T_PPPOE_TERM_V4_ACT_INGRESS_TERM_DISABLED:
                            {
{
                                    ingress_c_dropped_value *value_8;
                                    ingress_c_dropped_key key_2 = local_metadata->bng.line_id;
                                    value_8 = BPF_MAP_LOOKUP_ELEM(ingress_c_dropped, &key_2);
                                    if (value_8 != NULL) {
                                        __sync_fetch_and_add(&(value_8->bytes), pkt_len);
                                    } else {
                                    }
                                };
                            }
                            break;
                            default:
                            {
                            }
                            break;
                        }                    }
                }

            }
            else {
                {
                    /* construct key */
                    struct ingress_t_line_session_map_key key = {};
                    key.field0 = local_metadata->bng.line_id;
                    /* value */
                    struct ingress_t_line_session_map_value *value = NULL;
                    /* perform lookup */
                    value = BPF_MAP_LOOKUP_ELEM(ingress_t_line_session_map, &key);
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_3 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(ingress_t_line_session_map_defaultAction, &ebpf_zero);
                    } else {
                        hit_3 = 1;
                    }
                    if (value != NULL) {
                        /* run action */
                        switch (value->action) {
                            case 0: 
                                {
                                }
                                break;
                            case INGRESS_T_LINE_SESSION_MAP_ACT_INGRESS_SET_SESSION: 
                                {
local_metadata->bng.type = 2;
                                    local_metadata->bng.pppoe_session_id = value->u.ingress_set_session.pppoe_session_id;
                                    {
                                        ingress_c_line_rx_value *value_9;
                                        ingress_c_line_rx_key key_3 = local_metadata->bng.line_id;
                                        value_9 = BPF_MAP_LOOKUP_ELEM(ingress_c_line_rx, &key_3);
                                        if (value_9 != NULL) {
                                            __sync_fetch_and_add(&(value_9->bytes), pkt_len);
                                        } else {
                                        }
                                    };
                                }
                                break;
                            case INGRESS_T_LINE_SESSION_MAP_ACT_INGRESS_DROP: 
                                {
local_metadata->bng.type = 2;
                                    {
                                        ingress_c_line_rx_value *value_10;
                                        ingress_c_line_rx_key key_4 = local_metadata->bng.line_id;
                                        value_10 = BPF_MAP_LOOKUP_ELEM(ingress_c_line_rx, &key_4);
                                        if (value_10 != NULL) {
                                            __sync_fetch_and_add(&(value_10->bytes), pkt_len);
                                        } else {
                                        }
                                    };
                                    {
meta_5 = ostd;
                                        meta_5.drop = true;
                                        ostd = meta_5;
                                    }
                                }
                                break;
                            default:
                                return XDP_ABORTED;
                        }
                    } else {
                        return XDP_ABORTED;
                    }
                }
                if (hit_3) {
if (                    hdr->ipv4.ebpf_valid) {
                        unsigned int action_run_0 = 0;
                        {
                            /* construct key */
                            struct ingress_t_qos_v4_key key = {};
                            key.field0 = local_metadata->bng.line_id;
                            key.field1 = hdr->ipv4.src_addr;
                            key.field2 = hdr->ipv4.dscp;
                            key.field3 = hdr->ipv4.ecn;
                            /* value */
                            struct ingress_t_qos_v4_value *value = NULL;
                            /* perform lookup */
                            struct ingress_t_qos_v4_value_cache* cached_value = NULL;
                            cached_value = BPF_MAP_LOOKUP_ELEM(ingress_t_qos_v4_cache, &key);
                            if (cached_value != NULL) {
                                value = &(cached_value->value);
                                hit_3 = cached_value->hit;
                            } else {
                                struct ingress_t_qos_v4_key_mask head = {0};
                                struct ingress_t_qos_v4_value_mask *val = BPF_MAP_LOOKUP_ELEM(ingress_t_qos_v4_prefixes, &head);
                                if (val && val->has_next != 0) {
                                    struct ingress_t_qos_v4_key_mask next = val->next_tuple_mask;
                                    #pragma clang loop unroll(disable)
                                    for (int i = 0; i < MAX_INGRESS_T_QOS_V4_KEY_MASKS; i++) {
                                        struct ingress_t_qos_v4_value_mask *v = BPF_MAP_LOOKUP_ELEM(ingress_t_qos_v4_prefixes, &next);
                                        if (!v) {
                                            break;
                                        }
                                        struct ingress_t_qos_v4_key k = {};
                                        __u32 *chunk = ((__u32 *) &k);
                                        __u32 *mask = ((__u32 *) &next);
                                        #pragma clang loop unroll(disable)
                                        for (int i = 0; i < sizeof(struct ingress_t_qos_v4_key_mask) / 4; i++) {
                                            chunk[i] = ((__u32 *) &key)[i] & mask[i];
                                        }
                                        __u32 tuple_id = v->tuple_id;
                                        next = v->next_tuple_mask;
                                        struct bpf_elf_map *tuple = BPF_MAP_LOOKUP_ELEM(ingress_t_qos_v4_tuples_map, &tuple_id);
                                        if (!tuple) {
                                            break;
                                        }
                                        struct ingress_t_qos_v4_value *tuple_entry = bpf_map_lookup_elem(tuple, &k);
                                        if (!tuple_entry) {
                                            if (v->has_next == 0) {
                                                break;
                                            }
                                            continue;
                                        }
                                        if (value == NULL || tuple_entry->priority > value->priority) {
                                            value = tuple_entry;
                                        }
                                        if (v->has_next == 0) {
                                            break;
                                        }
                                    }
                                }
                                if (value == NULL) {
                                    /* miss; find default action */
                                    hit_3 = 0;
                                    value = BPF_MAP_LOOKUP_ELEM(ingress_t_qos_v4_defaultAction, &ebpf_zero);
                                } else {
                                    hit_3 = 1;
                                }
                                if (value != NULL) {
                                    struct ingress_t_qos_v4_value_cache cache_update = {0};
                                    cache_update.hit = hit_3;
                                    __builtin_memcpy((void *) &(cache_update.value), (void *) value, sizeof(struct ingress_t_qos_v4_value));
                                    BPF_MAP_UPDATE_ELEM(ingress_t_qos_v4_cache, &key, &cache_update, BPF_ANY);
                                }
                            }
                            if (value != NULL) {
                                /* run action */
                                switch (value->action) {
                                    case INGRESS_T_QOS_V4_ACT_INGRESS_QOS_PRIO: 
                                        {
ostd.class_of_service = 1;
                                        }
                                        break;
                                    case INGRESS_T_QOS_V4_ACT_INGRESS_QOS_BESTEFF: 
                                        {
ostd.class_of_service = 0;
                                        }
                                        break;
                                    default:
                                        return XDP_ABORTED;
                                }
                                action_run_0 = value->action;
                            } else {
                                return XDP_ABORTED;
                            }
                        }
                        switch (action_run_0) {
                            case INGRESS_T_QOS_V4_ACT_INGRESS_QOS_PRIO:
                            {
local_metadata->bng.ds_meter_result = meter_execute_bytes(&ingress_m_prio, &pkt_len, &local_metadata->bng.line_id, &tstamp);
                            }
                            break;
                            case INGRESS_T_QOS_V4_ACT_INGRESS_QOS_BESTEFF:
                            {
local_metadata->bng.ds_meter_result = meter_execute_bytes(&ingress_m_besteff, &pkt_len, &local_metadata->bng.line_id, &tstamp);
                            }
                            break;
                        }                    }
                }
            }

            if (hasReturned) {
;            }

            else {
if (ostd.drop) {
;                }

                else {
                    hdr->bmd.ebpf_valid = true;
                    hdr->bmd.vlan_id = local_metadata->vlan_id;
                    hdr->bmd.bng_type = (u8)local_metadata->bng.type;
                    hdr->bmd.fwd_type = (u8)local_metadata->fwd_type;
                    if (local_metadata->push_double_vlan) {
tmp = 1;                    }

                    else {
tmp = 0;                    }

                    hdr->bmd.push_double_vlan = tmp;
                    hdr->bmd.pppoe_session_id = local_metadata->bng.pppoe_session_id;
                    hdr->bmd.inner_vlan_id = local_metadata->inner_vlan_id;
                }            }

        }
    }
    {
        {
;
            ;
            ;
            ;
            ;
            ;
            ;
            ;
        }

        if (ostd.clone || ostd.multicast_group != 0) {
            struct xdp2tc_metadata xdp2tc_md = {};
            xdp2tc_md.headers = *hdr;
            xdp2tc_md.ostd = ostd;
            xdp2tc_md.packetOffsetInBits = ebpf_packetOffsetInBits;
                void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;
    struct ethhdr *eth = data;
    if ((void *)((struct ethhdr *) eth + 1) > data_end) {
        return XDP_ABORTED;
    }
    xdp2tc_md.pkt_ether_type = eth->h_proto;
    eth->h_proto = bpf_htons(0x0800);
            int ret = bpf_xdp_adjust_head(skb, -(int)sizeof(struct xdp2tc_metadata));
            if (ret) {
                return XDP_ABORTED;
            }
                data = (void *)(long)skb->data;
    data_end = (void *)(long)skb->data_end;
    if (((char *) data + 14 + sizeof(struct xdp2tc_metadata)) > (char *) data_end) {
        return XDP_ABORTED;
    }
__builtin_memmove(data, data + sizeof(struct xdp2tc_metadata), 14);
__builtin_memcpy(data + 14, &xdp2tc_md, sizeof(struct xdp2tc_metadata));
            return XDP_PASS;
        }
        if (ostd.drop || ostd.resubmit) {
            return XDP_ABORTED;
        }
        int outHeaderLength = 0;
        if (hdr->ethernet.ebpf_valid) {
            outHeaderLength += 96;
        }
        if (hdr->vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->eth_type.ebpf_valid) {
            outHeaderLength += 16;
        }
        if (hdr->pppoe.ebpf_valid) {
            outHeaderLength += 64;
        }
        if (hdr->mpls.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->ipv4.ebpf_valid) {
            outHeaderLength += 160;
        }

        int outHeaderOffset = BYTES(outHeaderLength) - BYTES(ebpf_packetOffsetInBits);
        if (outHeaderOffset != 0) {
            int returnCode = 0;
            returnCode = bpf_xdp_adjust_head(skb, -outHeaderOffset);
            if (returnCode) {
                return XDP_ABORTED;
            }
        }
        pkt = ((void*)(long)skb->data);
        ebpf_packetEnd = ((void*)(long)skb->data_end);
        ebpf_packetOffsetInBits = 0;
        if (hdr->ethernet.ebpf_valid) {
            ebpf_packetOffsetInBits += 96;
        }
        if (hdr->vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            u16 eth_type_val = hdr->vlan_tag.eth_type;
            eth_type_val = bpf_htons(eth_type_val);
            ebpf_byte = ((char*)(&eth_type_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&eth_type_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u8 pri_val = hdr->vlan_tag.pri;
            ebpf_byte = ((char*)(&pri_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            u8 cfi_val = hdr->vlan_tag.cfi;
            ebpf_byte = ((char*)(&cfi_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            u16 vlan_id_val = hdr->vlan_tag.vlan_id;
            vlan_id_val = bpf_htons(vlan_id_val << 4);
            ebpf_byte = ((char*)(&vlan_id_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&vlan_id_val))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            u16 eth_type_val = hdr->inner_vlan_tag.eth_type;
            eth_type_val = bpf_htons(eth_type_val);
            ebpf_byte = ((char*)(&eth_type_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&eth_type_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u8 pri_val = hdr->inner_vlan_tag.pri;
            ebpf_byte = ((char*)(&pri_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            u8 cfi_val = hdr->inner_vlan_tag.cfi;
            ebpf_byte = ((char*)(&cfi_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            u16 vlan_id_val = hdr->inner_vlan_tag.vlan_id;
            vlan_id_val = bpf_htons(vlan_id_val << 4);
            ebpf_byte = ((char*)(&vlan_id_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&vlan_id_val))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->eth_type.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16)) {
                return XDP_ABORTED;
            }
            
            u16 value_val = hdr->eth_type.value;
            value_val = bpf_htons(value_val);
            ebpf_byte = ((char*)(&value_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&value_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->pppoe.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 64)) {
                return XDP_ABORTED;
            }
            
            u8 version_val = hdr->pppoe.version;
            ebpf_byte = ((char*)(&version_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            u8 type_id_val = hdr->pppoe.type_id;
            ebpf_byte = ((char*)(&type_id_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            u8 code_val = hdr->pppoe.code;
            ebpf_byte = ((char*)(&code_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            u16 session_id_val = hdr->pppoe.session_id;
            session_id_val = bpf_htons(session_id_val);
            ebpf_byte = ((char*)(&session_id_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&session_id_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u16 length_val = hdr->pppoe.length;
            length_val = bpf_htons(length_val);
            ebpf_byte = ((char*)(&length_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&length_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u16 protocol_val = hdr->pppoe.protocol;
            protocol_val = bpf_htons(protocol_val);
            ebpf_byte = ((char*)(&protocol_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&protocol_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->mpls.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            u32 label_val = hdr->mpls.label;
            label_val = htonl(label_val << 12);
            ebpf_byte = ((char*)(&label_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&label_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&label_val))[2];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 2, 4, 4, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 20;

            u8 tc_val = hdr->mpls.tc;
            ebpf_byte = ((char*)(&tc_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 1, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            u8 bos_val = hdr->mpls.bos;
            ebpf_byte = ((char*)(&bos_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            u8 ttl_val = hdr->mpls.ttl;
            ebpf_byte = ((char*)(&ttl_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

        }
        if (hdr->ipv4.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160)) {
                return XDP_ABORTED;
            }
            
            u8 version_val = hdr->ipv4.version;
            ebpf_byte = ((char*)(&version_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            u8 ihl_val = hdr->ipv4.ihl;
            ebpf_byte = ((char*)(&ihl_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            u8 dscp_val = hdr->ipv4.dscp;
            ebpf_byte = ((char*)(&dscp_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 6, 2, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 6;

            u8 ecn_val = hdr->ipv4.ecn;
            ebpf_byte = ((char*)(&ecn_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 2, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 2;

            u16 total_len_val = hdr->ipv4.total_len;
            total_len_val = bpf_htons(total_len_val);
            ebpf_byte = ((char*)(&total_len_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&total_len_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u16 identification_val = hdr->ipv4.identification;
            identification_val = bpf_htons(identification_val);
            ebpf_byte = ((char*)(&identification_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&identification_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u8 flags_val = hdr->ipv4.flags;
            ebpf_byte = ((char*)(&flags_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            u16 frag_offset_val = hdr->ipv4.frag_offset;
            frag_offset_val = bpf_htons(frag_offset_val << 3);
            ebpf_byte = ((char*)(&frag_offset_val))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 5, 0, (ebpf_byte >> 3));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 3, 5, (ebpf_byte));
            ebpf_byte = ((char*)(&frag_offset_val))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 5, 0, (ebpf_byte >> 3));
            ebpf_packetOffsetInBits += 13;

            u8 ttl_val = hdr->ipv4.ttl;
            ebpf_byte = ((char*)(&ttl_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            u8 protocol_val = hdr->ipv4.protocol;
            ebpf_byte = ((char*)(&protocol_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            u16 hdr_checksum_val = hdr->ipv4.hdr_checksum;
            hdr_checksum_val = bpf_htons(hdr_checksum_val);
            ebpf_byte = ((char*)(&hdr_checksum_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr_checksum_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            u32 src_addr_val = hdr->ipv4.src_addr;
            src_addr_val = htonl(src_addr_val);
            ebpf_byte = ((char*)(&src_addr_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&src_addr_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&src_addr_val))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&src_addr_val))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

            u32 dst_addr_val = hdr->ipv4.dst_addr;
            dst_addr_val = htonl(dst_addr_val);
            ebpf_byte = ((char*)(&dst_addr_val))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&dst_addr_val))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&dst_addr_val))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&dst_addr_val))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

        }

        hdr->__helper_variable = ostd.egress_port;
        BPF_MAP_UPDATE_ELEM(bridged_headers, &ebpf_zero, &(*hdr), BPF_ANY);
        bpf_tail_call(skb, &egress_progs_table, 0);
    }
}

SEC("xdp/xdp-egress")
int xdp_egress_func(struct xdp_md *skb) {
    struct local_metadata_t *local_metadata;

    unsigned ebpf_packetOffsetInBits = 0;
    unsigned ebpf_packetOffsetInBits_save = 0;
    ParserError_t ebpf_errorCode = NoError;
    void* pkt = ((void*)(long)skb->data);
    void* ebpf_packetEnd = ((void*)(long)skb->data_end);
    u32 ebpf_zero = 0;
    u32 ebpf_one = 1;
    unsigned char ebpf_byte;
    u32 pkt_len = skb->data_end - skb->data;

    pkt_len += 12;
    struct headers_t *hdr;
    hdr = BPF_MAP_LOOKUP_ELEM(bridged_headers, &ebpf_zero);
    if (!hdr) {
        return XDP_DROP;
    }
    __u32 egress_ifindex = hdr->__helper_variable;
    struct hdr_md *hdrMd;

    hdrMd = BPF_MAP_LOOKUP_ELEM(hdr_md_cpumap, &ebpf_one);
    if (!hdrMd)
        return XDP_DROP;
    __builtin_memset(hdrMd, 0, sizeof(struct hdr_md));

    local_metadata = &(hdrMd->cpumap_usermeta);
    struct psa_egress_output_metadata_t ostd = {
       .clone = false,
            .drop = false,
        };

    struct psa_egress_input_metadata_t istd = {
            .class_of_service = 0,
            .egress_port = egress_ifindex,
            .packet_path = 0,
            .instance = 0,
            .parser_error = ebpf_errorCode,
        };
    if (istd.egress_port == PSA_PORT_RECIRCULATE) {
        istd.egress_port = P4C_PSA_PORT_RECIRCULATE;
    }
    u8 hdr_bmd_ingress_ebpf_valid = hdr->bmd.ebpf_valid;
    u8 hdr_ethernet_ingress_ebpf_valid = hdr->ethernet.ebpf_valid;
    u8 hdr_vlan_tag_ingress_ebpf_valid = hdr->vlan_tag.ebpf_valid;
    u8 hdr_inner_vlan_tag_ingress_ebpf_valid = hdr->inner_vlan_tag.ebpf_valid;
    u8 hdr_eth_type_ingress_ebpf_valid = hdr->eth_type.ebpf_valid;
    u8 hdr_pppoe_ingress_ebpf_valid = hdr->pppoe.ebpf_valid;
    u8 hdr_mpls_ingress_ebpf_valid = hdr->mpls.ebpf_valid;
    u8 hdr_ipv4_ingress_ebpf_valid = hdr->ipv4.ebpf_valid;
    hdr->bmd.ebpf_valid = 0;
    hdr->ethernet.ebpf_valid = 0;
    hdr->vlan_tag.ebpf_valid = 0;
    hdr->inner_vlan_tag.ebpf_valid = 0;
    hdr->eth_type.ebpf_valid = 0;
    hdr->pppoe.ebpf_valid = 0;
    hdr->mpls.ebpf_valid = 0;
    hdr->ipv4.ebpf_valid = 0;
    struct eth_type_t eth_type_2;
    struct eth_type_t eth_type_3;
    u16 ck_0_state = 0;
    start: {
/* extract(hdr->bmd) */
        if (!hdr_bmd_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96 + 4)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->bmd.line_id = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 32;

            hdr->bmd.pppoe_session_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->bmd.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u16, 12));
            ebpf_packetOffsetInBits += 12;

            hdr->bmd.bng_type = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
            ebpf_packetOffsetInBits += 8;

            hdr->bmd.fwd_type = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
            ebpf_packetOffsetInBits += 8;

            hdr->bmd.push_double_vlan = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
            ebpf_packetOffsetInBits += 8;

            hdr->bmd.inner_vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
            ebpf_packetOffsetInBits += 12;

            hdr->bmd.ebpf_valid = 1;

        } else {
            hdr->bmd.ebpf_valid = 1;
        }
/* extract(hdr->ethernet) */
        if (!hdr_ethernet_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96 + 16)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->ethernet.dst_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
            ebpf_packetOffsetInBits += 48;

            hdr->ethernet.src_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
            ebpf_packetOffsetInBits += 48;

            hdr->ethernet.ebpf_valid = 1;

        } else {
            hdr->ethernet.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 96;
        }
local_metadata->vlan_id = 4094;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_2.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_2.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_2.value) {
            case 34984: goto parse_vlan_tag;
            case 37120: goto parse_vlan_tag;
            case 33024: goto parse_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_vlan_tag: {
/* extract(hdr->vlan_tag) */
        if (!hdr_vlan_tag_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
            ebpf_packetOffsetInBits += 3;

            hdr->vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
            ebpf_packetOffsetInBits += 1;

            hdr->vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
            ebpf_packetOffsetInBits += 12;

            hdr->vlan_tag.ebpf_valid = 1;

        } else {
            hdr->vlan_tag.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 32;
        }
local_metadata->bng.s_tag = hdr->vlan_tag.vlan_id;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_3.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_3.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_3.value) {
            case 33024: goto parse_inner_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_inner_vlan_tag: {
/* extract(hdr->inner_vlan_tag) */
        if (!hdr_inner_vlan_tag_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->inner_vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->inner_vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
            ebpf_packetOffsetInBits += 3;

            hdr->inner_vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
            ebpf_packetOffsetInBits += 1;

            hdr->inner_vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
            ebpf_packetOffsetInBits += 12;

            hdr->inner_vlan_tag.ebpf_valid = 1;

        } else {
            hdr->inner_vlan_tag.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 32;
        }
local_metadata->bng.c_tag = hdr->inner_vlan_tag.vlan_id;        goto parse_eth_type;
    }
    parse_eth_type: {
/* extract(hdr->eth_type) */
        if (!hdr_eth_type_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->eth_type.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->eth_type.ebpf_valid = 1;

        } else {
            hdr->eth_type.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 16;
        }
        switch (hdr->eth_type.value) {
            case 34887: goto parse_mpls;
            case 2048: goto parse_ipv4;
            default: goto accept;
        }
    }
    parse_mpls: {
/* extract(hdr->mpls) */
        if (!hdr_mpls_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 12)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->mpls.label = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits)) >> 12) & EBPF_MASK(u32, 20));
            ebpf_packetOffsetInBits += 20;

            hdr->mpls.tc = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 1) & EBPF_MASK(u8, 3));
            ebpf_packetOffsetInBits += 3;

            hdr->mpls.bos = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 1));
            ebpf_packetOffsetInBits += 1;

            hdr->mpls.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 8;

            hdr->mpls.ebpf_valid = 1;

        } else {
            hdr->mpls.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 32;
        }
local_metadata->mpls_label = hdr->mpls.label;local_metadata->mpls_ttl = hdr->mpls.ttl;        goto parse_ipv4;
    }
    parse_ipv4: {
/* extract(hdr->ipv4) */
        if (!hdr_ipv4_ingress_ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            hdr->ipv4.version = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 4));
            ebpf_packetOffsetInBits += 4;

            hdr->ipv4.ihl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 4));
            ebpf_packetOffsetInBits += 4;

            hdr->ipv4.dscp = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 2) & EBPF_MASK(u8, 6));
            ebpf_packetOffsetInBits += 6;

            hdr->ipv4.ecn = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 2));
            ebpf_packetOffsetInBits += 2;

            hdr->ipv4.total_len = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.identification = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.flags = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
            ebpf_packetOffsetInBits += 3;

            hdr->ipv4.frag_offset = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 13));
            ebpf_packetOffsetInBits += 13;

            hdr->ipv4.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 8;

            hdr->ipv4.protocol = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 8;

            hdr->ipv4.hdr_checksum = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.src_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 32;

            hdr->ipv4.dst_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 32;

            hdr->ipv4.ebpf_valid = 1;

        } else {
            hdr->ipv4.ebpf_valid = 1;
            ebpf_packetOffsetInBits += 160;
        }
/* ck_0.subtract(hdr->ipv4.hdr_checksum) */
        {
            u16 ck_0_tmp = 0;
            ck_0_tmp = hdr->ipv4.hdr_checksum;
            ck_0_state = csum16_sub(ck_0_state, ck_0_tmp);
        }
/* ck_0.subtract(hdr->ipv4.ttl, hdr->ipv4.protocol) */
        {
            u16 ck_0_tmp_0 = 0;
            ck_0_tmp_0 = (hdr->ipv4.ttl << 8) | hdr->ipv4.protocol;
            ck_0_state = csum16_sub(ck_0_state, ck_0_tmp_0);
        }
hdr->ipv4.hdr_checksum = /* ck_0.get() */
((u16) (~ck_0_state));local_metadata->ip_proto = hdr->ipv4.protocol;local_metadata->ip_eth_type = 2048;local_metadata->ipv4_src_addr = hdr->ipv4.src_addr;local_metadata->ipv4_dst_addr = hdr->ipv4.dst_addr;        goto accept;
    }

    reject: {
        if (ebpf_errorCode == 0) {
            return XDP_ABORTED;
        }
        goto accept;
    }

    accept:
    istd.parser_error = ebpf_errorCode;
    return XDP_DROP;
    {

        u8 hit_4;
        struct psa_egress_output_metadata_t meta_6;
        __builtin_memset((void *) &meta_6, 0, sizeof(struct psa_egress_output_metadata_t ));
        struct psa_egress_output_metadata_t meta_0;
        __builtin_memset((void *) &meta_0, 0, sizeof(struct psa_egress_output_metadata_t ));
        struct psa_egress_output_metadata_t meta_1;
        __builtin_memset((void *) &meta_1, 0, sizeof(struct psa_egress_output_metadata_t ));
        {
if (hdr->bmd.push_double_vlan == 1) {
{
                    hdr->vlan_tag.ebpf_valid = true;
                    hdr->vlan_tag.eth_type = 33024;
                    hdr->vlan_tag.vlan_id = hdr->bmd.vlan_id;
                };
                {
                    hdr->inner_vlan_tag.ebpf_valid = true;
                    hdr->inner_vlan_tag.vlan_id = hdr->bmd.inner_vlan_id;
                    hdr->inner_vlan_tag.eth_type = 33024;
                    hdr->vlan_tag.eth_type = 33024;
                };
            }
            else {
                hdr->inner_vlan_tag.ebpf_valid = false;
                                {
                    /* construct key */
                    struct egress_egress_vlan_key key = {};
                    key.field0 = hdr->bmd.vlan_id;
                    key.field1 = istd.egress_port;
                    /* value */
                    struct egress_egress_vlan_value *value = NULL;
                    /* perform lookup */
                    value = BPF_MAP_LOOKUP_ELEM(egress_egress_vlan, &key);
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_4 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(egress_egress_vlan_defaultAction, &ebpf_zero);
                    } else {
                        hit_4 = 1;
                    }
                    if (value != NULL) {
                        /* run action */
                        switch (value->action) {
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_PUSH_VLAN: 
                                {
                                    hdr->vlan_tag.ebpf_valid = true;
                                    hdr->vlan_tag.eth_type = 33024;
                                    hdr->vlan_tag.vlan_id = hdr->bmd.vlan_id;
                                }
                                break;
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_POP_VLAN: 
                                {
                                    hdr->vlan_tag.ebpf_valid = false;
                                }
                                break;
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_DROP: 
                                {
{
meta_6 = ostd;
                                        meta_6.drop = true;
                                        ostd = meta_6;
                                    }
                                }
                                break;
                            default:
                                return XDP_ABORTED;
                        }
                    } else {
                        return XDP_ABORTED;
                    }
                }
;
            }
            if (            hdr->mpls.ebpf_valid) {
hdr->mpls.ttl = hdr->mpls.ttl + 255;
                if (hdr->mpls.ttl == 0) {
{
meta_0 = ostd;
                        meta_0.drop = true;
                        ostd = meta_0;
                    };                }

            }
            else {
if (                hdr->ipv4.ebpf_valid && hdr->bmd.fwd_type != 0) {
hdr->ipv4.ttl = hdr->ipv4.ttl + 255;
                    if (hdr->ipv4.ttl == 0) {
{
meta_1 = ostd;
                            meta_1.drop = true;
                            ostd = meta_1;
                        };                    }

                }            }

            if (hdr->bmd.bng_type == 2) {
if (                hdr->ipv4.ebpf_valid) {
{
{
hdr->eth_type.value = 34916;
                                                        hdr->pppoe.ebpf_valid = true;
                            hdr->pppoe.version = 1;
                            hdr->pppoe.type_id = 1;
                            hdr->pppoe.code = 0;
                            hdr->pppoe.session_id = hdr->bmd.pppoe_session_id;
                            {
                                egress_c_line_tx_value *value_11;
                                egress_c_line_tx_key key_5 = local_metadata->bng.line_id;
                                value_11 = BPF_MAP_LOOKUP_ELEM(egress_c_line_tx, &key_5);
                                if (value_11 != NULL) {
                                    __sync_fetch_and_add(&(value_11->bytes), pkt_len);
                                } else {
                                }
                            };
                        }
                        hdr->pppoe.length = hdr->ipv4.total_len + 1;
                        hdr->pppoe.protocol = 33;
                    };                }
            }

        }
    }
    {
        u16 ck_1_state = 0;
{
            {
                u16 ck_1_tmp = 0;
                ck_1_tmp = hdr->ipv4.hdr_checksum;
                ck_1_state = csum16_sub(ck_1_state, ck_1_tmp);
            }
;
                        {
                u16 ck_1_tmp_0 = 0;
                ck_1_tmp_0 = (hdr->ipv4.ttl << 8) | hdr->ipv4.protocol;
                ck_1_state = csum16_add(ck_1_state, ck_1_tmp_0);
            }
;
            hdr->ipv4.hdr_checksum = ((u16) (~ck_1_state));
            ;
            ;
            ;
            ;
            ;
            ;
            ;
        }

        if (ostd.drop) {
            return XDP_ABORTED;
        }
        int outHeaderLength = 0;
        if (hdr->ethernet.ebpf_valid) {
            outHeaderLength += 96;
        }
        if (hdr->vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->eth_type.ebpf_valid) {
            outHeaderLength += 16;
        }
        if (hdr->pppoe.ebpf_valid) {
            outHeaderLength += 64;
        }
        if (hdr->mpls.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->ipv4.ebpf_valid) {
            outHeaderLength += 160;
        }

        int outHeaderOffset = BYTES(outHeaderLength) - BYTES(ebpf_packetOffsetInBits);
        if (outHeaderOffset != 0) {
            int returnCode = 0;
            returnCode = bpf_xdp_adjust_head(skb, -outHeaderOffset);
            if (returnCode) {
                return XDP_ABORTED;
            }
        }
        pkt = ((void*)(long)skb->data);
        ebpf_packetEnd = ((void*)(long)skb->data_end);
        ebpf_packetOffsetInBits = 0;
        if (hdr->ethernet.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96)) {
                return XDP_ABORTED;
            }
            
            hdr->ethernet.dst_addr = htonll(hdr->ethernet.dst_addr << 16);
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[4];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[5];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
            ebpf_packetOffsetInBits += 48;

            hdr->ethernet.src_addr = htonll(hdr->ethernet.src_addr << 16);
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[4];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[5];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
            ebpf_packetOffsetInBits += 48;

        }
        if (hdr->vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->vlan_tag.eth_type = bpf_htons(hdr->vlan_tag.eth_type);
            ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->vlan_tag.pri))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->vlan_tag.cfi))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            hdr->vlan_tag.vlan_id = bpf_htons(hdr->vlan_tag.vlan_id << 4);
            ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->inner_vlan_tag.eth_type = bpf_htons(hdr->inner_vlan_tag.eth_type);
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.pri))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.cfi))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            hdr->inner_vlan_tag.vlan_id = bpf_htons(hdr->inner_vlan_tag.vlan_id << 4);
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->eth_type.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16)) {
                return XDP_ABORTED;
            }
            
            hdr->eth_type.value = bpf_htons(hdr->eth_type.value);
            ebpf_byte = ((char*)(&hdr->eth_type.value))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->eth_type.value))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->pppoe.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 64)) {
                return XDP_ABORTED;
            }
            
            ebpf_byte = ((char*)(&hdr->pppoe.version))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->pppoe.type_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->pppoe.code))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            hdr->pppoe.session_id = bpf_htons(hdr->pppoe.session_id);
            ebpf_byte = ((char*)(&hdr->pppoe.session_id))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.session_id))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->pppoe.length = bpf_htons(hdr->pppoe.length);
            ebpf_byte = ((char*)(&hdr->pppoe.length))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.length))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->pppoe.protocol = bpf_htons(hdr->pppoe.protocol);
            ebpf_byte = ((char*)(&hdr->pppoe.protocol))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.protocol))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->mpls.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->mpls.label = htonl(hdr->mpls.label << 12);
            ebpf_byte = ((char*)(&hdr->mpls.label))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->mpls.label))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->mpls.label))[2];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 2, 4, 4, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 20;

            ebpf_byte = ((char*)(&hdr->mpls.tc))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 1, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->mpls.bos))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            ebpf_byte = ((char*)(&hdr->mpls.ttl))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

        }
        if (hdr->ipv4.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160)) {
                return XDP_ABORTED;
            }
            
            ebpf_byte = ((char*)(&hdr->ipv4.version))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->ipv4.ihl))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->ipv4.dscp))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 6, 2, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 6;

            ebpf_byte = ((char*)(&hdr->ipv4.ecn))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 2, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 2;

            hdr->ipv4.total_len = bpf_htons(hdr->ipv4.total_len);
            ebpf_byte = ((char*)(&hdr->ipv4.total_len))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.total_len))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.identification = bpf_htons(hdr->ipv4.identification);
            ebpf_byte = ((char*)(&hdr->ipv4.identification))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.identification))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->ipv4.flags))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            hdr->ipv4.frag_offset = bpf_htons(hdr->ipv4.frag_offset << 3);
            ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 5, 0, (ebpf_byte >> 3));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 3, 5, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 5, 0, (ebpf_byte >> 3));
            ebpf_packetOffsetInBits += 13;

            ebpf_byte = ((char*)(&hdr->ipv4.ttl))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            ebpf_byte = ((char*)(&hdr->ipv4.protocol))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            hdr->ipv4.hdr_checksum = bpf_htons(hdr->ipv4.hdr_checksum);
            ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.src_addr = htonl(hdr->ipv4.src_addr);
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

            hdr->ipv4.dst_addr = htonl(hdr->ipv4.dst_addr);
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

        }

    }

    if (ostd.clone || ostd.drop) {
        return XDP_DROP;
    }

    return bpf_redirect_map(&tx_port, istd.egress_port%DEVMAP_SIZE, 0);
}

SEC("xdp_redirect_dummy_sec")
int xdp_redirect_dummy(struct xdp_md *skb) {
    return XDP_PASS;
}

SEC("classifier/tc-ingress")
int tc_ingress_func(SK_BUFF *skb) {
        unsigned ebpf_packetOffsetInBits = 0;
    unsigned ebpf_packetOffsetInBits_save = 0;
    ParserError_t ebpf_errorCode = NoError;
    void* pkt = ((void*)(long)skb->data);
    void* ebpf_packetEnd = ((void*)(long)skb->data_end);
    u32 ebpf_zero = 0;
    u32 ebpf_one = 1;
    unsigned char ebpf_byte;
    u32 pkt_len = skb->len;
    u64 tstamp = bpf_ktime_get_ns();
        void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;
    if (((char *) data + 14 + sizeof(struct xdp2tc_metadata)) > (char *) data_end) {
        return TC_ACT_SHOT;
    }
    struct xdp2tc_metadata xdp2tc_md = {};
    bpf_skb_load_bytes(skb, 14, &xdp2tc_md, sizeof(struct xdp2tc_metadata));
        __u16 *ether_type = (__u16 *) ((void *) (long)skb->data + 12);
    if ((void *) ((__u16 *) ether_type + 1) >     (void *) (long) skb->data_end) {
        return TC_ACT_SHOT;
    }
    *ether_type = xdp2tc_md.pkt_ether_type;
    struct psa_ingress_output_metadata_t ostd = xdp2tc_md.ostd;
        struct headers_t *hdr;
    hdr = &(xdp2tc_md.headers);
    ebpf_packetOffsetInBits = xdp2tc_md.packetOffsetInBits;
    int ret = bpf_skb_adjust_room(skb, -(int)sizeof(struct xdp2tc_metadata), 1, 0);
    if (ret) {
        return XDP_ABORTED;
    }
    
if (ostd.clone) {
        do_packet_clones(skb, &clone_session_tbl, ostd.clone_session_id, CLONE_I2E, 1);
    }
    int outHeaderLength = 0;
    if (hdr->bmd.ebpf_valid) {
        outHeaderLength += 96;
    }
    if (hdr->ethernet.ebpf_valid) {
        outHeaderLength += 96;
    }
    if (hdr->vlan_tag.ebpf_valid) {
        outHeaderLength += 32;
    }
    if (hdr->inner_vlan_tag.ebpf_valid) {
        outHeaderLength += 32;
    }
    if (hdr->eth_type.ebpf_valid) {
        outHeaderLength += 16;
    }
    if (hdr->pppoe.ebpf_valid) {
        outHeaderLength += 64;
    }
    if (hdr->mpls.ebpf_valid) {
        outHeaderLength += 32;
    }
    if (hdr->ipv4.ebpf_valid) {
        outHeaderLength += 160;
    }

    int outHeaderOffset = BYTES(outHeaderLength) - BYTES(ebpf_packetOffsetInBits);
    if (outHeaderOffset != 0) {
        int returnCode = 0;
        returnCode = bpf_skb_adjust_room(skb, outHeaderOffset, 1, 0);
        if (returnCode) {
            return XDP_ABORTED;
        }
    }
    pkt = ((void*)(long)skb->data);
    ebpf_packetEnd = ((void*)(long)skb->data_end);
    ebpf_packetOffsetInBits = 0;
    if (hdr->bmd.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96)) {
            return XDP_ABORTED;
        }
        
        hdr->bmd.line_id = htonl(hdr->bmd.line_id);
        ebpf_byte = ((char*)(&hdr->bmd.line_id))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.line_id))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.line_id))[2];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.line_id))[3];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
        ebpf_packetOffsetInBits += 32;

        hdr->bmd.pppoe_session_id = bpf_htons(hdr->bmd.pppoe_session_id);
        ebpf_byte = ((char*)(&hdr->bmd.pppoe_session_id))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.pppoe_session_id))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        hdr->bmd.vlan_id = bpf_htons(hdr->bmd.vlan_id << 4);
        ebpf_byte = ((char*)(&hdr->bmd.vlan_id))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.vlan_id))[1];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 4, (ebpf_byte >> 4));
        ebpf_packetOffsetInBits += 12;

        ebpf_byte = ((char*)(&hdr->bmd.bng_type))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        ebpf_byte = ((char*)(&hdr->bmd.fwd_type))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        ebpf_byte = ((char*)(&hdr->bmd.push_double_vlan))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        hdr->bmd.inner_vlan_id = bpf_htons(hdr->bmd.inner_vlan_id << 4);
        ebpf_byte = ((char*)(&hdr->bmd.inner_vlan_id))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->bmd.inner_vlan_id))[1];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
        ebpf_packetOffsetInBits += 12;

    }
    if (hdr->ethernet.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96)) {
            return XDP_ABORTED;
        }
        
        hdr->ethernet.dst_addr = htonll(hdr->ethernet.dst_addr << 16);
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[2];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[3];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[4];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[5];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
        ebpf_packetOffsetInBits += 48;

        hdr->ethernet.src_addr = htonll(hdr->ethernet.src_addr << 16);
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[2];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[3];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[4];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[5];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
        ebpf_packetOffsetInBits += 48;

    }
    if (hdr->vlan_tag.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
            return XDP_ABORTED;
        }
        
        hdr->vlan_tag.eth_type = bpf_htons(hdr->vlan_tag.eth_type);
        ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        ebpf_byte = ((char*)(&hdr->vlan_tag.pri))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 3;

        ebpf_byte = ((char*)(&hdr->vlan_tag.cfi))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 1;

        hdr->vlan_tag.vlan_id = bpf_htons(hdr->vlan_tag.vlan_id << 4);
        ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[1];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
        ebpf_packetOffsetInBits += 12;

    }
    if (hdr->inner_vlan_tag.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
            return XDP_ABORTED;
        }
        
        hdr->inner_vlan_tag.eth_type = bpf_htons(hdr->inner_vlan_tag.eth_type);
        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.pri))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 3;

        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.cfi))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 1;

        hdr->inner_vlan_tag.vlan_id = bpf_htons(hdr->inner_vlan_tag.vlan_id << 4);
        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[1];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
        ebpf_packetOffsetInBits += 12;

    }
    if (hdr->eth_type.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16)) {
            return XDP_ABORTED;
        }
        
        hdr->eth_type.value = bpf_htons(hdr->eth_type.value);
        ebpf_byte = ((char*)(&hdr->eth_type.value))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->eth_type.value))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

    }
    if (hdr->pppoe.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 64)) {
            return XDP_ABORTED;
        }
        
        ebpf_byte = ((char*)(&hdr->pppoe.version))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 4;

        ebpf_byte = ((char*)(&hdr->pppoe.type_id))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 4;

        ebpf_byte = ((char*)(&hdr->pppoe.code))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        hdr->pppoe.session_id = bpf_htons(hdr->pppoe.session_id);
        ebpf_byte = ((char*)(&hdr->pppoe.session_id))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->pppoe.session_id))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        hdr->pppoe.length = bpf_htons(hdr->pppoe.length);
        ebpf_byte = ((char*)(&hdr->pppoe.length))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->pppoe.length))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        hdr->pppoe.protocol = bpf_htons(hdr->pppoe.protocol);
        ebpf_byte = ((char*)(&hdr->pppoe.protocol))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->pppoe.protocol))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

    }
    if (hdr->mpls.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
            return XDP_ABORTED;
        }
        
        hdr->mpls.label = htonl(hdr->mpls.label << 12);
        ebpf_byte = ((char*)(&hdr->mpls.label))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->mpls.label))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->mpls.label))[2];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 2, 4, 4, (ebpf_byte >> 4));
        ebpf_packetOffsetInBits += 20;

        ebpf_byte = ((char*)(&hdr->mpls.tc))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 1, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 3;

        ebpf_byte = ((char*)(&hdr->mpls.bos))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 0, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 1;

        ebpf_byte = ((char*)(&hdr->mpls.ttl))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

    }
    if (hdr->ipv4.ebpf_valid) {
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160)) {
            return XDP_ABORTED;
        }
        
        ebpf_byte = ((char*)(&hdr->ipv4.version))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 4;

        ebpf_byte = ((char*)(&hdr->ipv4.ihl))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 4;

        ebpf_byte = ((char*)(&hdr->ipv4.dscp))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 6, 2, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 6;

        ebpf_byte = ((char*)(&hdr->ipv4.ecn))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 2, 0, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 2;

        hdr->ipv4.total_len = bpf_htons(hdr->ipv4.total_len);
        ebpf_byte = ((char*)(&hdr->ipv4.total_len))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.total_len))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.identification = bpf_htons(hdr->ipv4.identification);
        ebpf_byte = ((char*)(&hdr->ipv4.identification))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.identification))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        ebpf_byte = ((char*)(&hdr->ipv4.flags))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
        ebpf_packetOffsetInBits += 3;

        hdr->ipv4.frag_offset = bpf_htons(hdr->ipv4.frag_offset << 3);
        ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[0];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 5, 0, (ebpf_byte >> 3));
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 3, 5, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[1];
        write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 5, 0, (ebpf_byte >> 3));
        ebpf_packetOffsetInBits += 13;

        ebpf_byte = ((char*)(&hdr->ipv4.ttl))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        ebpf_byte = ((char*)(&hdr->ipv4.protocol))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_packetOffsetInBits += 8;

        hdr->ipv4.hdr_checksum = bpf_htons(hdr->ipv4.hdr_checksum);
        ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.src_addr = htonl(hdr->ipv4.src_addr);
        ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[2];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[3];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
        ebpf_packetOffsetInBits += 32;

        hdr->ipv4.dst_addr = htonl(hdr->ipv4.dst_addr);
        ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[0];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[1];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[2];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
        ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[3];
        write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
        ebpf_packetOffsetInBits += 32;

    }

    if (ostd.multicast_group != 0) {
        do_packet_clones(skb, &multicast_grp_tbl, ostd.multicast_group, NORMAL_MULTICAST, 2);
        return TC_ACT_SHOT;
    }
    skb->priority = ostd.class_of_service;
    return bpf_redirect(ostd.egress_port, 0);    }

SEC("classifier/tc-egress")
int tc_egress_func(SK_BUFF *skb) {
    struct psa_global_metadata *compiler_meta__ = (struct psa_global_metadata *) skb->cb;
    unsigned ebpf_packetOffsetInBits = 0;
    unsigned ebpf_packetOffsetInBits_save = 0;
    ParserError_t ebpf_errorCode = NoError;
    void* pkt = ((void*)(long)skb->data);
    void* ebpf_packetEnd = ((void*)(long)skb->data_end);
    u32 ebpf_zero = 0;
    u32 ebpf_one = 1;
    unsigned char ebpf_byte;
    u32 pkt_len = skb->len;
    struct local_metadata_t *local_metadata;
    struct hdr_md *hdrMd;
    struct headers_t *hdr;    hdrMd = BPF_MAP_LOOKUP_ELEM(hdr_md_cpumap, &ebpf_one);
    if (!hdrMd)
        return TC_ACT_SHOT;
    __builtin_memset(hdrMd, 0, sizeof(struct hdr_md));

    hdr = &(hdrMd->cpumap_hdr);
    local_metadata = &(hdrMd->cpumap_usermeta);
    struct psa_egress_output_metadata_t ostd = {
       .clone = false,
            .drop = false,
        };

    struct psa_egress_input_metadata_t istd = {
            .class_of_service = skb->priority,
            .egress_port = skb->ifindex,
            .packet_path = compiler_meta__->packet_path,
            .instance = compiler_meta__->instance,
            .parser_error = ebpf_errorCode,
        };
    if (istd.egress_port == PSA_PORT_RECIRCULATE) {
        istd.egress_port = P4C_PSA_PORT_RECIRCULATE;
    }
    struct eth_type_t eth_type_2;
    struct eth_type_t eth_type_3;
    u16 ck_0_state_0 = 0;
    start: {
/* extract(hdr->bmd) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96 + 4)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->bmd.line_id = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 32;

        hdr->bmd.pppoe_session_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->bmd.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->bmd.bng_type = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
        ebpf_packetOffsetInBits += 8;

        hdr->bmd.fwd_type = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
        ebpf_packetOffsetInBits += 8;

        hdr->bmd.push_double_vlan = (u8)((load_half(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 8));
        ebpf_packetOffsetInBits += 8;

        hdr->bmd.inner_vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->bmd.ebpf_valid = 1;

/* extract(hdr->ethernet) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96 + 16)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->ethernet.dst_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
        ebpf_packetOffsetInBits += 48;

        hdr->ethernet.src_addr = (u64)((load_dword(pkt, BYTES(ebpf_packetOffsetInBits)) >> 16) & EBPF_MASK(u64, 48));
        ebpf_packetOffsetInBits += 48;

        hdr->ethernet.ebpf_valid = 1;

local_metadata->vlan_id = 4094;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_2.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_2.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_2.value) {
            case 34984: goto parse_vlan_tag;
            case 37120: goto parse_vlan_tag;
            case 33024: goto parse_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_vlan_tag: {
/* extract(hdr->vlan_tag) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->vlan_tag.ebpf_valid = 1;

local_metadata->bng.s_tag = hdr->vlan_tag.vlan_id;        {
            ebpf_packetOffsetInBits_save = ebpf_packetOffsetInBits;
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
                ebpf_errorCode = PacketTooShort;
                goto reject;
            }

            eth_type_3.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
            ebpf_packetOffsetInBits += 16;

            eth_type_3.ebpf_valid = 1;

            ebpf_packetOffsetInBits = ebpf_packetOffsetInBits_save;
        }
        switch (eth_type_3.value) {
            case 33024: goto parse_inner_vlan_tag;
            default: goto parse_eth_type;
        }
    }
    parse_inner_vlan_tag: {
/* extract(hdr->inner_vlan_tag) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 4)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->inner_vlan_tag.eth_type = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->inner_vlan_tag.pri = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->inner_vlan_tag.cfi = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->inner_vlan_tag.vlan_id = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 12));
        ebpf_packetOffsetInBits += 12;

        hdr->inner_vlan_tag.ebpf_valid = 1;

local_metadata->bng.c_tag = hdr->inner_vlan_tag.vlan_id;        goto parse_eth_type;
    }
    parse_eth_type: {
/* extract(hdr->eth_type) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16 + 0)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->eth_type.value = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->eth_type.ebpf_valid = 1;

        switch (hdr->eth_type.value) {
            case 34887: goto parse_mpls;
            case 2048: goto parse_ipv4;
            default: goto accept;
        }
    }
    parse_mpls: {
/* extract(hdr->mpls) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32 + 12)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->mpls.label = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits)) >> 12) & EBPF_MASK(u32, 20));
        ebpf_packetOffsetInBits += 20;

        hdr->mpls.tc = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 1) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->mpls.bos = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 1));
        ebpf_packetOffsetInBits += 1;

        hdr->mpls.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->mpls.ebpf_valid = 1;

local_metadata->mpls_label = hdr->mpls.label;local_metadata->mpls_ttl = hdr->mpls.ttl;        goto parse_ipv4;
    }
    parse_ipv4: {
/* extract(hdr->ipv4) */
        if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160 + 0)) {
            ebpf_errorCode = PacketTooShort;
            goto reject;
        }

        hdr->ipv4.version = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 4) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->ipv4.ihl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 4));
        ebpf_packetOffsetInBits += 4;

        hdr->ipv4.dscp = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 2) & EBPF_MASK(u8, 6));
        ebpf_packetOffsetInBits += 6;

        hdr->ipv4.ecn = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u8, 2));
        ebpf_packetOffsetInBits += 2;

        hdr->ipv4.total_len = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.identification = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.flags = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits)) >> 5) & EBPF_MASK(u8, 3));
        ebpf_packetOffsetInBits += 3;

        hdr->ipv4.frag_offset = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))) & EBPF_MASK(u16, 13));
        ebpf_packetOffsetInBits += 13;

        hdr->ipv4.ttl = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->ipv4.protocol = (u8)((load_byte(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 8;

        hdr->ipv4.hdr_checksum = (u16)((load_half(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 16;

        hdr->ipv4.src_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 32;

        hdr->ipv4.dst_addr = (u32)((load_word(pkt, BYTES(ebpf_packetOffsetInBits))));
        ebpf_packetOffsetInBits += 32;

        hdr->ipv4.ebpf_valid = 1;

/* ck_0.subtract(hdr->ipv4.hdr_checksum) */
        {
            u16 ck_0_tmp_1 = 0;
            ck_0_tmp_1 = hdr->ipv4.hdr_checksum;
            ck_0_state_0 = csum16_sub(ck_0_state_0, ck_0_tmp_1);
        }
/* ck_0.subtract(hdr->ipv4.ttl, hdr->ipv4.protocol) */
        {
            u16 ck_0_tmp_2 = 0;
            ck_0_tmp_2 = (hdr->ipv4.ttl << 8) | hdr->ipv4.protocol;
            ck_0_state_0 = csum16_sub(ck_0_state_0, ck_0_tmp_2);
        }
hdr->ipv4.hdr_checksum = /* ck_0.get() */
((u16) (~ck_0_state_0));local_metadata->ip_proto = hdr->ipv4.protocol;local_metadata->ip_eth_type = 2048;local_metadata->ipv4_src_addr = hdr->ipv4.src_addr;local_metadata->ipv4_dst_addr = hdr->ipv4.dst_addr;        goto accept;
    }

    reject: {
        if (ebpf_errorCode == 0) {
            return XDP_ABORTED;
        }
        goto accept;
    }

    accept:
    istd.parser_error = ebpf_errorCode;
    {
        u8 hit_5;
        struct psa_egress_output_metadata_t meta_6;
        __builtin_memset((void *) &meta_6, 0, sizeof(struct psa_egress_output_metadata_t ));
        struct psa_egress_output_metadata_t meta_0;
        __builtin_memset((void *) &meta_0, 0, sizeof(struct psa_egress_output_metadata_t ));
        struct psa_egress_output_metadata_t meta_1;
        __builtin_memset((void *) &meta_1, 0, sizeof(struct psa_egress_output_metadata_t ));
        {
if (hdr->bmd.push_double_vlan == 1) {
{
                    hdr->vlan_tag.ebpf_valid = true;
                    hdr->vlan_tag.eth_type = 33024;
                    hdr->vlan_tag.vlan_id = hdr->bmd.vlan_id;
                };
                {
                    hdr->inner_vlan_tag.ebpf_valid = true;
                    hdr->inner_vlan_tag.vlan_id = hdr->bmd.inner_vlan_id;
                    hdr->inner_vlan_tag.eth_type = 33024;
                    hdr->vlan_tag.eth_type = 33024;
                };
            }
            else {
                hdr->inner_vlan_tag.ebpf_valid = false;
                                {
                    /* construct key */
                    struct egress_egress_vlan_key key = {};
                    key.field0 = hdr->bmd.vlan_id;
                    key.field1 = istd.egress_port;
                    /* value */
                    struct egress_egress_vlan_value *value = NULL;
                    /* perform lookup */
                    value = BPF_MAP_LOOKUP_ELEM(egress_egress_vlan, &key);
                    if (value == NULL) {
                        /* miss; find default action */
                        hit_5 = 0;
                        value = BPF_MAP_LOOKUP_ELEM(egress_egress_vlan_defaultAction, &ebpf_zero);
                    } else {
                        hit_5 = 1;
                    }
                    if (value != NULL) {
                        /* run action */
                        switch (value->action) {
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_PUSH_VLAN: 
                                {
                                    hdr->vlan_tag.ebpf_valid = true;
                                    hdr->vlan_tag.eth_type = 33024;
                                    hdr->vlan_tag.vlan_id = hdr->bmd.vlan_id;
                                }
                                break;
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_POP_VLAN: 
                                {
                                    hdr->vlan_tag.ebpf_valid = false;
                                }
                                break;
                            case EGRESS_EGRESS_VLAN_ACT_EGRESS_DROP: 
                                {
{
meta_6 = ostd;
                                        meta_6.drop = true;
                                        ostd = meta_6;
                                    }
                                }
                                break;
                            default:
                                return XDP_ABORTED;
                        }
                    } else {
                        return XDP_ABORTED;
                    }
                }
;
            }
            if (            hdr->mpls.ebpf_valid) {
hdr->mpls.ttl = hdr->mpls.ttl + 255;
                if (hdr->mpls.ttl == 0) {
{
meta_0 = ostd;
                        meta_0.drop = true;
                        ostd = meta_0;
                    };                }

            }
            else {
if (                hdr->ipv4.ebpf_valid && hdr->bmd.fwd_type != 0) {
hdr->ipv4.ttl = hdr->ipv4.ttl + 255;
                    if (hdr->ipv4.ttl == 0) {
{
meta_1 = ostd;
                            meta_1.drop = true;
                            ostd = meta_1;
                        };                    }

                }            }

            if (hdr->bmd.bng_type == 2) {
if (                hdr->ipv4.ebpf_valid) {
{
{
hdr->eth_type.value = 34916;
                                                        hdr->pppoe.ebpf_valid = true;
                            hdr->pppoe.version = 1;
                            hdr->pppoe.type_id = 1;
                            hdr->pppoe.code = 0;
                            hdr->pppoe.session_id = hdr->bmd.pppoe_session_id;
                            {
                                egress_c_line_tx_value *value_12;
                                egress_c_line_tx_key key_6 = local_metadata->bng.line_id;
                                value_12 = BPF_MAP_LOOKUP_ELEM(egress_c_line_tx, &key_6);
                                if (value_12 != NULL) {
                                    __sync_fetch_and_add(&(value_12->bytes), pkt_len);
                                } else {
                                }
                            };
                        }
                        hdr->pppoe.length = hdr->ipv4.total_len + 1;
                        hdr->pppoe.protocol = 33;
                    };                }
            }

        }
    }
    {
        u16 ck_1_state_0 = 0;
{
            {
                u16 ck_1_tmp_1 = 0;
                ck_1_tmp_1 = hdr->ipv4.hdr_checksum;
                ck_1_state_0 = csum16_sub(ck_1_state_0, ck_1_tmp_1);
            }
;
                        {
                u16 ck_1_tmp_2 = 0;
                ck_1_tmp_2 = (hdr->ipv4.ttl << 8) | hdr->ipv4.protocol;
                ck_1_state_0 = csum16_add(ck_1_state_0, ck_1_tmp_2);
            }
;
            hdr->ipv4.hdr_checksum = ((u16) (~ck_1_state_0));
            ;
            ;
            ;
            ;
            ;
            ;
            ;
        }

        int outHeaderLength = 0;
        if (hdr->ethernet.ebpf_valid) {
            outHeaderLength += 96;
        }
        if (hdr->vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->eth_type.ebpf_valid) {
            outHeaderLength += 16;
        }
        if (hdr->pppoe.ebpf_valid) {
            outHeaderLength += 64;
        }
        if (hdr->mpls.ebpf_valid) {
            outHeaderLength += 32;
        }
        if (hdr->ipv4.ebpf_valid) {
            outHeaderLength += 160;
        }

        int outHeaderOffset = BYTES(outHeaderLength) - BYTES(ebpf_packetOffsetInBits);
        if (outHeaderOffset != 0) {
            int returnCode = 0;
            returnCode = bpf_skb_adjust_room(skb, outHeaderOffset, 1, 0);
            if (returnCode) {
                return XDP_ABORTED;
            }
        }
        pkt = ((void*)(long)skb->data);
        ebpf_packetEnd = ((void*)(long)skb->data_end);
        ebpf_packetOffsetInBits = 0;
        if (hdr->ethernet.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 96)) {
                return XDP_ABORTED;
            }
            
            hdr->ethernet.dst_addr = htonll(hdr->ethernet.dst_addr << 16);
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[4];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.dst_addr))[5];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
            ebpf_packetOffsetInBits += 48;

            hdr->ethernet.src_addr = htonll(hdr->ethernet.src_addr << 16);
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[4];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ethernet.src_addr))[5];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 5, (ebpf_byte));
            ebpf_packetOffsetInBits += 48;

        }
        if (hdr->vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->vlan_tag.eth_type = bpf_htons(hdr->vlan_tag.eth_type);
            ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->vlan_tag.eth_type))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->vlan_tag.pri))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->vlan_tag.cfi))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            hdr->vlan_tag.vlan_id = bpf_htons(hdr->vlan_tag.vlan_id << 4);
            ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->vlan_tag.vlan_id))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->inner_vlan_tag.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->inner_vlan_tag.eth_type = bpf_htons(hdr->inner_vlan_tag.eth_type);
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.eth_type))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.pri))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.cfi))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            hdr->inner_vlan_tag.vlan_id = bpf_htons(hdr->inner_vlan_tag.vlan_id << 4);
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 4));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 4, 4, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->inner_vlan_tag.vlan_id))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 4, 0, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 12;

        }
        if (hdr->eth_type.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 16)) {
                return XDP_ABORTED;
            }
            
            hdr->eth_type.value = bpf_htons(hdr->eth_type.value);
            ebpf_byte = ((char*)(&hdr->eth_type.value))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->eth_type.value))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->pppoe.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 64)) {
                return XDP_ABORTED;
            }
            
            ebpf_byte = ((char*)(&hdr->pppoe.version))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->pppoe.type_id))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->pppoe.code))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            hdr->pppoe.session_id = bpf_htons(hdr->pppoe.session_id);
            ebpf_byte = ((char*)(&hdr->pppoe.session_id))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.session_id))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->pppoe.length = bpf_htons(hdr->pppoe.length);
            ebpf_byte = ((char*)(&hdr->pppoe.length))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.length))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->pppoe.protocol = bpf_htons(hdr->pppoe.protocol);
            ebpf_byte = ((char*)(&hdr->pppoe.protocol))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->pppoe.protocol))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

        }
        if (hdr->mpls.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 32)) {
                return XDP_ABORTED;
            }
            
            hdr->mpls.label = htonl(hdr->mpls.label << 12);
            ebpf_byte = ((char*)(&hdr->mpls.label))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->mpls.label))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->mpls.label))[2];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 2, 4, 4, (ebpf_byte >> 4));
            ebpf_packetOffsetInBits += 20;

            ebpf_byte = ((char*)(&hdr->mpls.tc))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 1, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            ebpf_byte = ((char*)(&hdr->mpls.bos))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 1, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 1;

            ebpf_byte = ((char*)(&hdr->mpls.ttl))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

        }
        if (hdr->ipv4.ebpf_valid) {
            if (ebpf_packetEnd < pkt + BYTES(ebpf_packetOffsetInBits + 160)) {
                return XDP_ABORTED;
            }
            
            ebpf_byte = ((char*)(&hdr->ipv4.version))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 4, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->ipv4.ihl))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 4, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 4;

            ebpf_byte = ((char*)(&hdr->ipv4.dscp))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 6, 2, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 6;

            ebpf_byte = ((char*)(&hdr->ipv4.ecn))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 2, 0, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 2;

            hdr->ipv4.total_len = bpf_htons(hdr->ipv4.total_len);
            ebpf_byte = ((char*)(&hdr->ipv4.total_len))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.total_len))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.identification = bpf_htons(hdr->ipv4.identification);
            ebpf_byte = ((char*)(&hdr->ipv4.identification))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.identification))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            ebpf_byte = ((char*)(&hdr->ipv4.flags))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 3, 5, (ebpf_byte >> 0));
            ebpf_packetOffsetInBits += 3;

            hdr->ipv4.frag_offset = bpf_htons(hdr->ipv4.frag_offset << 3);
            ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[0];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0, 5, 0, (ebpf_byte >> 3));
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 0 + 1, 3, 5, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.frag_offset))[1];
            write_partial(pkt + BYTES(ebpf_packetOffsetInBits) + 1, 5, 0, (ebpf_byte >> 3));
            ebpf_packetOffsetInBits += 13;

            ebpf_byte = ((char*)(&hdr->ipv4.ttl))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            ebpf_byte = ((char*)(&hdr->ipv4.protocol))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_packetOffsetInBits += 8;

            hdr->ipv4.hdr_checksum = bpf_htons(hdr->ipv4.hdr_checksum);
            ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.hdr_checksum))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_packetOffsetInBits += 16;

            hdr->ipv4.src_addr = htonl(hdr->ipv4.src_addr);
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.src_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

            hdr->ipv4.dst_addr = htonl(hdr->ipv4.dst_addr);
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[0];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 0, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[1];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 1, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[2];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 2, (ebpf_byte));
            ebpf_byte = ((char*)(&hdr->ipv4.dst_addr))[3];
            write_byte(pkt, BYTES(ebpf_packetOffsetInBits) + 3, (ebpf_byte));
            ebpf_packetOffsetInBits += 32;

        }

    }
    if (ostd.clone) {
        do_packet_clones(skb, &clone_session_tbl, ostd.clone_session_id, CLONE_E2E, 3);
    }

    if (ostd.drop) {
        return TC_ACT_SHOT;;
    }

    if (istd.egress_port == P4C_PSA_PORT_RECIRCULATE) {
        compiler_meta__->packet_path = RECIRCULATE;
        return bpf_redirect(PSA_PORT_RECIRCULATE, BPF_F_INGRESS);
    }

    
    return TC_ACT_OK;
}
char _license[] SEC("license") = "GPL";
