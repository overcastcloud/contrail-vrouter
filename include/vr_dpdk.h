/*
 * Copyright (C) 2014 Semihalf.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * vr_dpdk.h -- vRouter/DPDK definitions
 *
 */

#ifndef _VR_DPDK_H_
#define _VR_DPDK_H_

#include <net/if.h>
#include <sys/queue.h>

#include "vr_os.h"
#include "vr_packet.h"
#include "vr_interface.h"

#include <rte_config.h>
#include <rte_pci.h>
#include <rte_version.h>
#include <rte_kni.h>
#include <rte_ethdev.h>
#include <rte_port.h>

#define RTE_LOGTYPE_VROUTER         RTE_LOGTYPE_USER1
#define RTE_LOGTYPE_USOCK           RTE_LOGTYPE_USER2
#define RTE_LOGTYPE_UVHOST          RTE_LOGTYPE_USER3
#undef RTE_LOG_LEVEL
#define RTE_LOG_LEVEL               RTE_LOG_INFO

/*
 * Debug options:
 *
#define RTE_LOG_LEVEL               RTE_LOG_DEBUG
#define VR_DPDK_NETLINK_DEBUG
#define VR_DPDK_NETLINK_PKT_DUMP
#define VR_DPDK_USOCK_DUMP
#define VR_DPDK_RX_PKT_DUMP
#define VR_DPDK_TX_PKT_DUMP
#define VR_DPDK_PKT_DUMP_VIF_FILTER(vif) (vif->vif_type == VIF_TYPE_AGENT \
                                        || vif->vif_type == VIF_TYPE_VIRTUAL)
 */

/* Default lcore mask. Used only when sched_getaffinity() is failed */
#define VR_DPDK_DEF_LCORE_MASK      0xf
/* Number of service lcores */
#define VR_DPDK_NB_SERVICE_LCORES   2
/* Minimum number of lcores */
#define VR_DPDK_MIN_LCORES          2
/* Memory to allocate at startup in MB */
#define VR_DPDK_MAX_MEM             "512"
/* Number of memory channels to use */
#define VR_DPDK_MAX_MEMCHANNELS     "4"
/* Use UDP source port hashing */
#define VR_DPDK_USE_MPLS_UDP_ECMP   true
/* Use hardware filtering (Flow Director) */
#define VR_DPDK_USE_HW_FILTERING    false
/* KNI generates random MACs for e1000e NICs, so we need this
 * option enabled for the development on servers with those NICs */
#define VR_DPDK_ENABLE_PROMISC      false
/* Maximum number of hardware RX queues to use for RSS and filtering
 * (limited by NIC and number of per queue TX/RX descriptors) */
#define VR_DPDK_MAX_NB_RX_QUEUES    11
/* Maximum number of hardware TX queues to use (limited by the number of lcores) */
#define VR_DPDK_MAX_NB_TX_QUEUES    5
/* Maximum number of hardware RX queues to use for RSS (limited by the number of lcores) */
#define VR_DPDK_MAX_NB_RSS_QUEUES   4
/* Number of hardware RX ring descriptors */
#define VR_DPDK_NB_RXD              256
/* Number of hardware TX ring descriptors */
#define VR_DPDK_NB_TXD              512
/* Offset to MPLS label for hardware filtering (in 16-bit word units) */
#define VR_DPDK_MPLS_OFFSET         ((VR_ETHER_HLEN             \
                                    + sizeof(struct vr_ip)      \
                                    + sizeof(struct vr_udp))/2)
/* Maximum number of rings per lcore (maximum is VR_MAX_INTERFACES*RTE_MAX_LCORE) */
#define VR_DPDK_MAX_RINGS           (VR_MAX_INTERFACES*2)
/* Max size of a single packet */
#define VR_DPDK_MAX_PACKET_SZ       2048
/* Number of bytes needed for each mbuf */
#define VR_DPDK_MBUF_SZ             (VR_DPDK_MAX_PACKET_SZ      \
                                    + sizeof(struct rte_mbuf)   \
                                    + RTE_PKTMBUF_HEADROOM      \
                                    + sizeof(struct vr_packet))
/* How many packets to read/write from/to queue in one go */
#define VR_DPDK_MAX_BURST_SZ        RTE_PORT_IN_BURST_SIZE_MAX
#define VR_DPDK_ETH_RX_BURST_SZ     32
#define VR_DPDK_ETH_TX_BURST_SZ     32
#define VR_DPDK_KNI_RX_BURST_SZ     32
#define VR_DPDK_KNI_TX_BURST_SZ     32
#define VR_DPDK_RING_RX_BURST_SZ    32
#define VR_DPDK_RING_TX_BURST_SZ    32
/* Number of mbufs in TX ring */
#define VR_DPDK_TX_RING_SZ          (VR_DPDK_MAX_BURST_SZ*2)
/* Number of mbufs in virtio mempool */
#define VR_DPDK_VIRTIO_MEMPOOL_SZ   8192
/* How many objects (mbufs) to keep in per-lcore virtio mempool cache */
#define VR_DPDK_VIRTIO_MEMPOOL_CACHE_SZ (VR_DPDK_VIRTIO_RX_BURST_SZ*8)
/* Number of mbufs in RSS mempool */
#define VR_DPDK_RSS_MEMPOOL_SZ      8192
/* How many objects (mbufs) to keep in per-lcore RSS mempool cache */
#define VR_DPDK_RSS_MEMPOOL_CACHE_SZ    (VR_DPDK_MAX_BURST_SZ*8)
/* Number of VM mempools */
#define VR_DPDK_MAX_VM_MEMPOOLS     (VR_DPDK_MAX_NB_RX_QUEUES*2 - VR_DPDK_MIN_LCORES)
/* Number of mbufs in VM mempool */
#define VR_DPDK_VM_MEMPOOL_SZ       1024
/* How many objects (mbufs) to keep in per-lcore VM mempool cache */
#define VR_DPDK_VM_MEMPOOL_CACHE_SZ (VR_DPDK_MAX_BURST_SZ*8)
/* Use timer to measure flushes (slower, but should improve latency) */
#define VR_DPDK_USE_TIMER           false
/* TX flush timeout (in loops or US if USE_TIMER defined) */
#define VR_DPDK_TX_FLUSH_LOOPS      5
#define VR_DPDK_TX_FLUSH_US         100
/* Sleep time in US if there are no queues to poll */
#define VR_DPDK_SLEEP_NO_QUEUES_US  10000
/* Sleep (in US) or yield if no packets received (use 0 to disable) */
#define VR_DPDK_SLEEP_NO_PACKETS_US 0
#define VR_DPDK_YIELD_NO_PACKETS    1
/* Timers handling periodicity in US */
#define VR_DPDK_SLEEP_TIMER_US      100
/* KNI handling periodicity in US */
#define VR_DPDK_SLEEP_KNI_US        500
/* Sleep time in US for service lcore */
#define VR_DPDK_SLEEP_SERVICE_US    100
/* Invalid port ID */
#define VR_DPDK_INVALID_PORT_ID     0xFF
/* Invalid queue ID */
#define VR_DPDK_INVALID_QUEUE_ID    0xFFFF
/* Socket connection retry timeout in seconds (use power of 2) */
#define VR_DPDK_RETRY_CONNECT_SECS  64

/*
 * VRouter/DPDK Data Structures
 * ============================
 *
 * Changes since the initial commit:
 *   lcore_ctx -> vr_dpdk_lcore
 *   vif_port -> vr_dpdk_queue
 *
 * TODO: update the description
 */

/* Init queue operation */
typedef struct vr_dpdk_queue *
    (*vr_dpdk_queue_init_op)(unsigned lcore_id, struct vr_interface *vif,
        unsigned queue_or_lcore_id);
/* Release queue operation */
typedef void
    (*vr_dpdk_queue_release_op)(unsigned lcore_id, struct vr_interface *vif);

struct vr_dpdk_queue {
    SLIST_ENTRY(vr_dpdk_queue) q_next;
    union {
        /* DPDK TX queue operators */
        struct rte_port_out_ops txq_ops;
        /* DPDK RX queue operators */
        struct rte_port_in_ops rxq_ops;
    };
    /* Queue handler */
    void *q_queue_h;
    /* Pointer to vRouter interface */
    struct vr_interface *q_vif;
    /* RX burst size */
    uint16_t rxq_burst_size;
};

/* We store the queue params in the separate structure to increase CPU
 * cache hit rate
 */
struct vr_dpdk_queue_params {
    /* Pointer to release function */
    vr_dpdk_queue_release_op qp_release_op;
    /* Extra queue params */
    union {
        struct {
            struct rte_ring *ring_p;
            unsigned host_lcore_id;
        } qp_ring;
    };
};

struct vr_dpdk_ring_to_push {
    /* Ring pointer */
    struct rte_ring *rtp_tx_ring;
    /* TX queue pointer */
    struct vr_dpdk_queue *rtp_tx_queue;
};

SLIST_HEAD(vr_dpdk_q_slist, vr_dpdk_queue);

/* Lcore commands */
enum vr_dpdk_lcore_cmd {
    /* No command */
    VR_DPDK_LCORE_NO_CMD = 0,
    /* Stop and exit the lcore loop */
    VR_DPDK_LCORE_STOP_CMD,
    /* Remove RX queue */
    VR_DPDK_LCORE_RX_RM_CMD,
    /* Remove TX queue */
    VR_DPDK_LCORE_TX_RM_CMD,
};

struct vr_dpdk_lcore {
    /* RX queues head */
    struct vr_dpdk_q_slist lcore_rx_head;
    /* Table of RX queues */
    struct vr_dpdk_queue lcore_rx_queues[VR_MAX_INTERFACES];
    /* TX queues head */
    struct vr_dpdk_q_slist lcore_tx_head;
    /* Table of TX queues */
    struct vr_dpdk_queue lcore_tx_queues[VR_MAX_INTERFACES];
    /* Number of rings to push for the lcore */
    volatile uint16_t lcore_nb_rings_to_push;
    /* List of rings to push */
    struct vr_dpdk_ring_to_push lcore_rings_to_push[VR_DPDK_MAX_RINGS];
    /* Table of RX queue params */
    struct vr_dpdk_queue_params lcore_rx_queue_params[VR_MAX_INTERFACES];
    /* Table of TX queue params */
    struct vr_dpdk_queue_params lcore_tx_queue_params[VR_MAX_INTERFACES];
    /* Event socket */
    void *lcore_event_sock;
    /* Lcore command param */
    rte_atomic32_t lcore_cmd_param;
    /* Lcore command */
    rte_atomic16_t lcore_cmd;
    /* Number of RX queues assigned to the lcore (for the scheduler) */
    uint16_t lcore_nb_rx_queues;
};

/* Hardware RX queue state */
enum vr_dpdk_queue_state {
    /* No queue available */
    VR_DPDK_QUEUE_NONE,
    /* The queue is ready to use for RSS or filtering */
    VR_DPDK_QUEUE_READY_STATE,
    /* The queue is being used for RSS */
    VR_DPDK_QUEUE_RSS_STATE,
    /* The queue is being used for filtering */
    VR_DPDK_QUEUE_FILTERING_STATE
};

/* Ethdev configuration */
struct vr_dpdk_ethdev {
    /* Pointer to ethdev or NULL if the device is not used */
    struct rte_eth_dev *ethdev_ptr;
    /* Number of HW RX queues (limited by NIC hardware) */
    uint16_t ethdev_nb_rx_queues;
    /* Number of HW TX queues (limited by the nb of lcores) */
    uint16_t ethdev_nb_tx_queues;
    /* Number of HW RX queues used for RSS (limited by the nb of lcores) */
    uint16_t ethdev_nb_rss_queues;
    uint8_t ethdev_port_id;
    /* Hardware RX queue states */
    uint8_t ethdev_queue_states[VR_DPDK_MAX_NB_RX_QUEUES];
    /* Pointers to memory pools */
    struct rte_mempool *ethdev_mempools[VR_DPDK_MAX_NB_RX_QUEUES];
};

struct vr_dpdk_global {
    /* Pointer to virtio memory pool */
    struct rte_mempool *virtio_mempool;
    /* Pointer to RSS memory pool */
    struct rte_mempool *rss_mempool;
    /* Number of free memory pools */
    uint16_t nb_free_mempools;
    /* List of free memory pools */
    struct rte_mempool *free_mempools[VR_DPDK_MAX_VM_MEMPOOLS];
    /* Number of forwarding lcores */
    unsigned nb_fwd_lcores;
    /* Table of pointers to forwarding lcore */
    struct vr_dpdk_lcore *lcores[RTE_MAX_LCORE];
    /* Global stop flag */
    rte_atomic16_t stop_flag;
    /* NetLink socket handler */
    void *netlink_sock;
    void *flow_table;
    /* Packet socket */
    struct rte_ring *packet_ring;
    void *packet_transport;
    unsigned packet_lcore_id;
    /* KNI thread ID */
    pthread_t kni_thread;
    /* Timer thread ID */
    pthread_t timer_thread;
    /* User space vhost thread */
    pthread_t uvh_thread;
    /* Table of KNIs */
    struct rte_kni *knis[VR_MAX_INTERFACES];
    /* Table of vHosts */
    struct vr_interface *vhosts[VR_MAX_INTERFACES];
    /* Table of ethdevs */
    struct vr_dpdk_ethdev ethdevs[RTE_MAX_ETHPORTS];
    /* Table of monitoring redirections (for tcpdump) */
    uint16_t monitorings[VR_MAX_INTERFACES];
    /* Interface configuration mutex
     * ATM we use it just to synchronize access between the NetLink interface
     * and kernel KNI events. The datapath is not affected. */
    pthread_mutex_t if_lock;
};

extern struct vr_dpdk_global vr_dpdk;

/*
 * rte_mbuf <=> vr_packet conversion
 *
 * We use the tailroom to store vr_packet structure:
 *     struct rte_mbuf + headroom + data + tailroom + struct vr_packet
 *
 * rte_mbuf: *buf_addr(buf_len) + headroom + *pkt.data(data_len) + tailroom
 *
 * rte_mbuf->buf_addr = rte_mbuf + sizeof(rte_mbuf)
 * rte_mbuf->buf_len = elt_size - sizeof(rte_mbuf) - sizeof(vr_packet)
 * rte_mbuf->pkt.data = rte_mbuf->buf_addr + RTE_PKTMBUF_HEADROOM
 *
 *
 * vr_packet: *vp_head + headroom + vp_data(vp_len) + vp_tail + tailroom
 *                + vp_end
 *
 * vr_packet->vp_head = rte_mbuf->buf_addr (set in mbuf constructor)
 * vr_packet->vp_data = rte_mbuf->pkt.data - rte_mbuf->buf_addr
 * vr_packet->vp_len  = rte_mbuf->pkt.data_len
 * vr_packet->vp_tail = vr_packet->vp_data + vr_packet->vp_len
 * vr_packet->vp_end  = rte_mbuf->buf_len (set in mbuf constructor)
 */
static inline struct rte_mbuf *
vr_dpdk_pkt_to_mbuf(struct vr_packet *pkt)
{
    return (struct rte_mbuf *)((uintptr_t)pkt->vp_head - sizeof(struct rte_mbuf));
}
static inline struct vr_packet *
vr_dpdk_mbuf_to_pkt(struct rte_mbuf *mbuf)
{
    return (struct vr_packet *)((uintptr_t)mbuf->buf_addr + mbuf->buf_len);
}

/*
 * vr_dpdk_mbuf_reset - if the mbuf changes, possibley due to
 * pskb_may_pull, reset fields of the pkt structure that point at
 * the mbuf fields.
 * Note: we do not reset pkt->data here
 */
static inline void
vr_dpdk_mbuf_reset(struct vr_packet *pkt)
{
    struct rte_mbuf *mbuf = vr_dpdk_pkt_to_mbuf(pkt);

    pkt->vp_head = mbuf->buf_addr;
    pkt->vp_tail = rte_pktmbuf_headroom(mbuf) + mbuf->pkt.data_len;
    pkt->vp_end = mbuf->buf_len;
    pkt->vp_len = pkt->vp_tail - pkt->vp_data;

    return;
}

/*
 * dpdk_vrouter.c
 */
/* pktmbuf constructor with vr_packet support */
void vr_dpdk_pktmbuf_init(struct rte_mempool *mp, void *opaque_arg, void *_m, unsigned i);
/* Check if the stop flag is set */
int vr_dpdk_is_stop_flag_set(void);

/*
 * vr_dpdk_ethdev.c
 */
/* Init eth RX queue */
struct vr_dpdk_queue *
vr_dpdk_ethdev_rx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned rx_queue_id);
/* Init eth TX queue */
struct vr_dpdk_queue *
vr_dpdk_ethdev_tx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned tx_queue_id);
/* Init ethernet device */
int vr_dpdk_ethdev_init(struct vr_dpdk_ethdev *);
/* Release ethernet device */
int vr_dpdk_ethdev_release(struct vr_dpdk_ethdev *);
/* Get free queue ID */
uint16_t vr_dpdk_ethdev_ready_queue_id_get(struct vr_interface *vif);
/* Add hardware filter */
int vr_dpdk_ethdev_filter_add(struct vr_interface *vif, uint16_t queue_id,
    unsigned dst_ip, unsigned mpls_label);
/* Init hardware filtering */
int vr_dpdk_ethdev_filtering_init(struct vr_interface *vif, struct vr_dpdk_ethdev *ethdev);
/* Init RSS */
int vr_dpdk_ethdev_rss_init(struct vr_dpdk_ethdev *ethdev);

/*
 * vr_dpdk_flow_mem.c
 */
int vr_dpdk_flow_mem_init(void);
int vr_dpdk_flow_init(void);

/*
 * vr_dpdk_host.c
 */
int vr_dpdk_host_init(void);
void vr_dpdk_host_exit(void);
/* Convert internal packet fields */
struct vr_packet * vr_dpdk_packet_get(struct rte_mbuf *m, struct vr_interface *vif);
void vr_dpdk_pfree(struct rte_mbuf *mbuf, unsigned short reason);
/* Retry socket connection */
int vr_dpdk_retry_connect(int sockfd, const struct sockaddr *addr,
                            socklen_t alen);
/* Generates unique log message */
int vr_dpdk_ulog(uint32_t level, uint32_t logtype, uint32_t *last_hash,
                    const char *format, ...);
#define DPDK_ULOG(l, t, h, ...)                         \
    (void)(((RTE_LOG_ ## l <= RTE_LOG_LEVEL) &&         \
    (RTE_LOG_ ## l <= rte_logs.level) &&                \
    (RTE_LOGTYPE_ ## t & rte_logs.type)) ?              \
    vr_dpdk_ulog(RTE_LOG_ ## l,                         \
        RTE_LOGTYPE_ ## t, h, # t ": " __VA_ARGS__) : 0)

#if RTE_LOG_LEVEL == RTE_LOG_DEBUG
#define DPDK_DEBUG_VAR(v) v
#define DPDK_UDEBUG(t, h, ...)                          \
    (void)(((RTE_LOGTYPE_ ## t & rte_logs.type)) ?      \
    vr_dpdk_ulog(RTE_LOG_DEBUG,                         \
        RTE_LOGTYPE_ ## t, h, # t ": " __VA_ARGS__) : 0)
#else
#define DPDK_DEBUG_VAR(v)
#define DPDK_UDEBUG(t, h, ...)
#endif

/*
 * vr_dpdk_interface.c
 */
/* Lock interface operations */
static inline int vr_dpdk_if_lock()
{ return pthread_mutex_lock(&vr_dpdk.if_lock); }
/* Unlock interface operations */
static inline int vr_dpdk_if_unlock()
{ return pthread_mutex_unlock(&vr_dpdk.if_lock); }

/*
 * vr_dpdk_knidev.c
 */
/* Init KNI */
int vr_dpdk_knidev_init(struct vr_interface *vif);
/* Release KNI */
int vr_dpdk_knidev_release(struct vr_interface *vif);
/* Init KNI RX queue */
struct vr_dpdk_queue *
vr_dpdk_kni_rx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned host_lcore_id);
/* Init KNI TX queue */
struct vr_dpdk_queue *
vr_dpdk_kni_tx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned host_lcore_id);
/* Handle all KNIs attached */
void vr_dpdk_knidev_all_handle(void);

/*
 * vr_dpdk_packet.c
 */
void vr_dpdk_packet_wakeup(struct vr_dpdk_lcore *lcorep);
int dpdk_packet_socket_init(void);
void dpdk_packet_socket_close(void);
int dpdk_packet_io(void);

/*
 * vr_dpdk_lcore.c
 */
/* Launch lcore main loop */
int vr_dpdk_lcore_launch(void *dummy);
/* Schedule an interface */
int vr_dpdk_lcore_if_schedule(struct vr_interface *vif, unsigned least_used_id,
    uint16_t nb_rx_queues, vr_dpdk_queue_init_op rx_queue_init_op,
    uint16_t nb_tx_queues, vr_dpdk_queue_init_op tx_queue_init_op);
/* Unschedule an interface */
void vr_dpdk_lcore_if_unschedule(struct vr_interface *vif);
/* Schedule an MPLS label queue */
int vr_dpdk_lcore_mpls_schedule(struct vr_interface *vif, unsigned dst_ip,
    unsigned mpls_label);
/* Returns the least used lcore or RTE_MAX_LCORE */
unsigned vr_dpdk_lcore_least_used_get(void);
/* Returns the least used lcore among the ones that handle physical intf TX */
unsigned int vr_dpdk_phys_lcore_least_used_get(void);
/* Flush TX queues */
static inline void
vr_dpdk_lcore_flush(struct vr_dpdk_lcore *lcore)
{
    struct vr_dpdk_queue *tx_queue;
    SLIST_FOREACH(tx_queue, &lcore->lcore_tx_head, q_next) {
        tx_queue->txq_ops.f_flush(tx_queue->q_queue_h);
    }
}
/* Send a burst of vr_packets to vRouter */
void
vr_dpdk_packets_vroute(struct vr_interface *vif,
    struct vr_packet *pkts[VR_DPDK_MAX_BURST_SZ], uint32_t nb_pkts);
/* Handle an IPC command */
int
vr_dpdk_lcore_cmd_handle(struct vr_dpdk_lcore *lcore);
/* Post an lcore command */
void
vr_dpdk_lcore_cmd_post(struct vr_dpdk_lcore *lcore, uint16_t cmd,
    uint32_t cmd_param);


/*
 * vr_dpdk_netlink.c
 */
void dpdk_netlink_exit(void);
int dpdk_netlink_init(void);
int dpdk_netlink_receive(void *usockp, char *nl_buf, unsigned int nl_len);
int dpdk_netlink_io(void);

/*
 * vr_dpdk_ringdev.c
 */
/* Allocates a new ring */
struct rte_ring *
vr_dpdk_ring_allocate(unsigned host_lcore_id, char *ring_name,
    unsigned vr_dpdk_tx_ring_sz);
/* Init ring RX queue */
struct vr_dpdk_queue *
vr_dpdk_ring_rx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned host_lcore_id);
/* Init ring TX queue */
struct vr_dpdk_queue *
vr_dpdk_ring_tx_queue_init(unsigned lcore_id, struct vr_interface *vif,
    unsigned host_lcore_id);
void dpdk_ring_to_push_add(unsigned lcore_id, struct rte_ring *tx_ring,
    struct vr_dpdk_queue *tx_queue);

void
dpdk_ring_to_push_remove(unsigned lcore_id, struct rte_ring *tx_ring);

#endif /*_VR_DPDK_H_ */
