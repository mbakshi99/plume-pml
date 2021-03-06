/*
Copyright (c) 2015, Plume Design Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the name of the Plume Design Inc. nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Plume Design Inc. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Band Steering Manager - Clients
 */
#ifndef __BM_CLIENT_H__
#define __BM_CLIENT_H__

#ifndef OVSDB_UUID_LEN
#define OVSDB_UUID_LEN              37
#endif /* OVSDB_UUID_LEN */

#ifndef MAC_STR_LEN
#define MAC_STR_LEN                 18
#endif /* MAC_STR_LEN */

#define BM_CLIENT_MIN_HWM           1
#define BM_CLIENT_MAX_HWM           128

#define BM_CLIENT_MIN_LWM           1
#define BM_CLIENT_MAX_LWM           128

#define BM_CLIENT_ROGUE_SNR_LEVEL   5
#define BM_CLIENT_RSSI_HYSTERESIS   2

/*****************************************************************************/
typedef enum {
    BM_CLIENT_KICK_NONE             = 0,
    BM_CLIENT_KICK_DISASSOC,
    BM_CLIENT_KICK_DEAUTH
} bm_client_kick_t;

typedef enum {
    BM_CLIENT_REJECT_NONE           = 0,
    BM_CLIENT_REJECT_PROBE_ALL,
    BM_CLIENT_REJECT_PROBE_NULL,
    BM_CLIENT_REJECT_PROBE_DIRECT,
    BM_CLIENT_REJECT_AUTH_BLOCKED
} bm_client_reject_t;

typedef enum {
    BM_CLIENT_STATE_DISCONNECTED    = 0,
    BM_CLIENT_STATE_CONNECTED,
    BM_CLIENT_STATE_STEERING_5G,
    BM_CLIENT_STATE_STEERING_2G,
    BM_CLIENT_STATE_BACKOFF
} bm_client_state_t;

typedef enum {
    BM_CLIENT_CS_MODE_OFF           = 0,
    BM_CLIENT_CS_MODE_AWAY,
    BM_CLIENT_CS_MODE_HOME
} bm_client_cs_mode_t;

typedef enum {
    BM_CLIENT_CS_STATE_NONE         = 0,
    BM_CLIENT_CS_STATE_STEERING,
    BM_CLIENT_CS_STATE_EXPIRED,
    BM_CLIENT_CS_STATE_FAILED,
    BM_CLIENT_CS_STATE_XING_LOW,
    BM_CLIENT_CS_STATE_XING_HIGH,
    BM_CLIENT_CS_STATE_XING_DISABLED
} bm_client_cs_state_t;

typedef enum {
    BM_CLIENT_STEERING_NONE         = 0,
    BM_CLIENT_BAND_STEERING,
    BM_CLIENT_CLIENT_STEERING
} bm_client_steering_state_t;

typedef struct {
    uint32_t                    rejects;
    uint32_t                    connects;
    uint32_t                    disconnects;
    uint32_t                    activity_changes;

    uint32_t                    steering_success_cnt;
    uint32_t                    steering_fail_cnt;
    uint32_t                    steering_kick_cnt;
    uint32_t                    sticky_kick_cnt;

    struct {
        uint32_t                null_cnt;
        uint32_t                null_blocked;
        uint32_t                direct_cnt;
        uint32_t                direct_blocked;
    } probe;

    struct {
        uint32_t                higher;
        uint32_t                lower;
        uint32_t                inact_higher;
        uint32_t                inact_lower;
    } rssi;

    struct {
        bsal_disc_source_t      source;
        bsal_disc_type_t        type;
        uint8_t                 reason;
    } last_disconnect;
} bm_client_stats_t;

typedef struct {
    time_t                      last_kick;
    time_t                      last_connect;
    time_t                      last_disconnect;
    time_t                      last_state_change;
    time_t                      last_activity_change;

    struct {
        time_t                  first;
        time_t                  last;
    } reject;

    struct {
        time_t                  last;
        time_t                  last_null;
        time_t                  last_direct;
        time_t                  last_blocked;
    } probe[BSAL_BAND_COUNT];
} bm_client_times_t;

typedef struct {
    char                        mac_addr[MAC_STR_LEN];

    bm_client_reject_t          reject_detection;
    bm_client_kick_t            kick_type;
    bm_client_kick_t            sc_kick_type;
    uint8_t                     kick_reason;
    uint8_t                     sc_kick_reason;
    uint8_t                     hwm;
    uint8_t                     lwm;
    int                         max_rejects;
    int                         max_rejects_period;
    int                         backoff_period;
    uint16_t                    kick_debounce_period;
    uint16_t                    sc_kick_debounce_period;

    // Client steering specific variables
    bm_client_reject_t          cs_reject_detection;
    uint8_t                     cs_hwm;
    uint8_t                     cs_lwm;
    int                         cs_max_rejects;
    int                         cs_max_rejects_period;
    int                         cs_enforce_period;
    bsal_band_t                 cs_band;
    bool                        cs_probe_block;
    bool                        cs_auth_block;
    int                         cs_auth_reject_reason;
    bm_client_cs_mode_t         cs_mode;
    bm_client_cs_state_t        cs_state;
    bool                        cs_auto_disable;

    int                         num_rejects;
    int                         num_rejects_copy;
    bool                        active;
    bool                        connected;
    bm_pair_t                   *pair;
    bsal_band_t                 band;
    bm_client_state_t           state;
    bm_client_stats_t           stats[BSAL_BAND_COUNT];
    bm_client_times_t           times;
    bm_client_steering_state_t  steering_state;

    evsched_task_t              backoff_task;
    evsched_task_t              cs_task;
    evsched_task_t              rssi_xing_task;
    evsched_task_t              state_task;

    char                        uuid[OVSDB_UUID_LEN];

    ds_tree_node_t              dst_node;
} bm_client_t;


/*****************************************************************************/
extern bool                 bm_client_init(void);
extern bool                 bm_client_cleanup(void);
extern bool                 bm_client_add_all_to_pair(bm_pair_t *pair);
extern bool                 bm_client_remove_all_from_pair(bm_pair_t *pair);
extern bool                 bm_client_ovsdb_update(bm_client_t *client);
extern void                 bm_client_connected(bm_client_t *client, bsal_t bsal,
                                                   bsal_band_t band, bsal_event_t *event);
extern void                 bm_client_disconnected(bm_client_t *client);
extern void                 bm_client_rejected(bm_client_t *client, bsal_event_t *event);
extern void                 bm_client_success(bm_client_t *client, bsal_band_t band);
extern void                 bm_client_cs_connect(bm_client_t *client, bsal_band_t band);
extern void                 bm_client_set_state(bm_client_t *client, bm_client_state_t state);
extern bool                 bm_client_update_cs_state( bm_client_t *client );

extern bm_client_reject_t   bm_client_get_reject_detection( bm_client_t *client );
extern void                 bm_client_cs_check_rssi_xing( bm_client_t *client, bsal_event_t *event );
extern void                 bm_client_disable_client_steering( bm_client_t *client );

extern ds_tree_t *          bm_client_get_tree(void);
extern bm_client_t *        bm_client_find_by_uuid(const char *uuid);
extern bm_client_t *        bm_client_find_by_macstr(char *mac_str);
extern bm_client_t *        bm_client_find_by_macaddr(os_macaddr_t mac_addr);
extern bm_client_t *        bm_client_find_or_add_by_macaddr(os_macaddr_t *mac_addr);

#endif /* __BM_CLIENT_H__ */
