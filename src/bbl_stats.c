/*
 * BNG Blaster (BBL) - Stats
 *
 * Hannes Gredler, July 2020
 * Christian Giese, October 2020
 *
 * Copyright (C) 2020-2021, RtBrick, Inc.
 */

#include "bbl.h"
#include "bbl_stats.h"
#include <jansson.h>

extern const char banner[];

void
bbl_stats_update_cps (bbl_ctx_s *ctx) {
    struct timespec time_diff = {0};
    int ms;
    double x, y;

    /* Session setup time and rate */
    if(ctx->sessions_established_max > ctx->stats.sessions_established_max) {
        ctx->stats.sessions_established_max = ctx->sessions_established_max;

        timespec_sub(&time_diff,
		     &ctx->stats.last_session_established,
		     &ctx->stats.first_session_tx);

        ms = round(time_diff.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        ctx->stats.setup_time = (time_diff.tv_sec * 1000) + ms; // Setup time in milliseconds

        x = ctx->sessions_established_max;
        y = ctx->stats.setup_time;
        ctx->stats.cps = (x / y) * 1000.0;

        ctx->stats.cps_sum += ctx->stats.cps;
        ctx->stats.cps_count++;
        ctx->stats.cps_avg = ctx->stats.cps_sum / ctx->stats.cps_count;

        if(ctx->stats.cps_min) {
            if(ctx->stats.cps < ctx->stats.cps_min) ctx->stats.cps_min = ctx->stats.cps;
        } else {
            ctx->stats.cps_min = ctx->stats.cps;
        }
        if(ctx->stats.cps > ctx->stats.cps_max) ctx->stats.cps_max = ctx->stats.cps;
    }
}

void
bbl_stats_generate (bbl_ctx_s *ctx, bbl_stats_t * stats) {

    struct dict_itor *itor;
    bbl_session_s *session;

    int join_delays = 0;
    int leave_delays = 0;

    bbl_stats_update_cps(ctx);

    /* Iterate over all sessions */
    itor = dict_itor_new(ctx->session_dict);
    dict_itor_first(itor);
    for (; dict_itor_valid(itor); dict_itor_next(itor)) {
        session = (bbl_session_s*)*dict_itor_datum(itor);

        /* Multicast */
        stats->mc_old_rx_after_first_new += session->stats.mc_old_rx_after_first_new;
        stats->mc_not_received += session->stats.mc_not_received;

        if(session->stats.avg_join_delay) {
            join_delays++;
            stats->avg_join_delay += session->stats.avg_join_delay;
            if(session->stats.max_join_delay > stats->max_join_delay) stats->max_join_delay = session->stats.max_join_delay;
            if(stats->min_join_delay) {
                if(session->stats.min_join_delay < stats->min_join_delay) stats->min_join_delay = session->stats.min_join_delay;
            } else {
                stats->min_join_delay = session->stats.min_join_delay;
            }

        }
        if(session->stats.avg_leave_delay) {
            leave_delays++;
            stats->avg_leave_delay += session->stats.avg_leave_delay;
            if(session->stats.max_leave_delay > stats->max_leave_delay) stats->max_leave_delay = session->stats.max_leave_delay;
            if(stats->min_leave_delay) {
                if(session->stats.min_leave_delay < stats->min_leave_delay) stats->min_leave_delay = session->stats.min_leave_delay;
            } else {
                stats->min_leave_delay = session->stats.min_leave_delay;
            }
        }

        /* Session Traffic */
        if(session->access_ipv4_rx_first_seq) stats->sessions_access_ipv4_rx++;
        if(session->network_ipv4_rx_first_seq) stats->sessions_network_ipv4_rx++;
        if(session->access_ipv6_rx_first_seq) stats->sessions_access_ipv6_rx++;
        if(session->network_ipv6_rx_first_seq) stats->sessions_network_ipv6_rx++;
        if(session->access_ipv6pd_rx_first_seq) stats->sessions_access_ipv6pd_rx++;
        if(session->network_ipv6pd_rx_first_seq) stats->sessions_network_ipv6pd_rx++;

        if(stats->min_access_ipv4_rx_first_seq) {
            if(session->access_ipv4_rx_first_seq < stats->min_access_ipv4_rx_first_seq) stats->min_access_ipv4_rx_first_seq = session->access_ipv4_rx_first_seq;
        } else {
            stats->min_access_ipv4_rx_first_seq = session->access_ipv4_rx_first_seq;
        }
        if(session->access_ipv4_rx_first_seq > stats->max_access_ipv4_rx_first_seq) stats->max_access_ipv4_rx_first_seq = session->access_ipv4_rx_first_seq;

        if(stats->min_network_ipv4_rx_first_seq) {
            if(session->network_ipv4_rx_first_seq < stats->min_network_ipv4_rx_first_seq) stats->min_network_ipv4_rx_first_seq = session->network_ipv4_rx_first_seq;
        } else {
            stats->min_network_ipv4_rx_first_seq = session->network_ipv4_rx_first_seq;
        }
        if(session->network_ipv4_rx_first_seq > stats->max_network_ipv4_rx_first_seq) stats->max_network_ipv4_rx_first_seq = session->network_ipv4_rx_first_seq;

        if(stats->min_access_ipv6_rx_first_seq) {
            if(session->access_ipv6_rx_first_seq < stats->min_access_ipv6_rx_first_seq) stats->min_access_ipv6_rx_first_seq = session->access_ipv6_rx_first_seq;
        } else {
            stats->min_access_ipv6_rx_first_seq = session->access_ipv6_rx_first_seq;
        }
        if(session->access_ipv6_rx_first_seq > stats->max_access_ipv6_rx_first_seq) stats->max_access_ipv6_rx_first_seq = session->access_ipv6_rx_first_seq;

        if(stats->min_network_ipv6_rx_first_seq) {
            if(session->network_ipv6_rx_first_seq < stats->min_network_ipv6_rx_first_seq) stats->min_network_ipv6_rx_first_seq = session->network_ipv6_rx_first_seq;
        } else {
            stats->min_network_ipv6_rx_first_seq = session->network_ipv6_rx_first_seq;
        }
        if(session->network_ipv6_rx_first_seq > stats->max_network_ipv6_rx_first_seq) stats->max_network_ipv6_rx_first_seq = session->network_ipv6_rx_first_seq;

        if(stats->min_access_ipv6pd_rx_first_seq) {
            if(session->access_ipv6pd_rx_first_seq < stats->min_access_ipv6pd_rx_first_seq) stats->min_access_ipv6pd_rx_first_seq = session->access_ipv6pd_rx_first_seq;
        } else {
            stats->min_access_ipv6pd_rx_first_seq = session->access_ipv6pd_rx_first_seq;
        }
        if(session->access_ipv6pd_rx_first_seq > stats->max_access_ipv6pd_rx_first_seq) stats->max_access_ipv6pd_rx_first_seq = session->access_ipv6pd_rx_first_seq;

        if(stats->min_network_ipv6pd_rx_first_seq) {
            if(session->network_ipv6pd_rx_first_seq < stats->min_network_ipv6pd_rx_first_seq) stats->min_network_ipv6pd_rx_first_seq = session->network_ipv6pd_rx_first_seq;
        } else {
            stats->min_network_ipv6pd_rx_first_seq = session->network_ipv6pd_rx_first_seq;
        }
        if(session->network_ipv6pd_rx_first_seq > stats->max_network_ipv6pd_rx_first_seq) stats->max_network_ipv6pd_rx_first_seq = session->network_ipv6pd_rx_first_seq;

    }
    if(join_delays) {
        stats->avg_join_delay = round(stats->avg_join_delay / join_delays);
    }
    if(leave_delays) {
        stats->avg_leave_delay = round(stats->avg_leave_delay / leave_delays);
    }
}

void
bbl_stats_stdout (bbl_ctx_s *ctx, bbl_stats_t * stats) {
    struct bbl_interface_ *access_if;    
    int i;

    printf("%s", banner);
    printf("Report:\n\n");
    printf("Sessions PPPoE: %u IPoE: %u\n", ctx->sessions_pppoe, ctx->sessions_ipoe);
    printf("Sessions established: %u/%u\n", ctx->sessions_established_max, ctx->config.sessions);
    printf("DHCPv6 sessions established: %u\n", ctx->dhcpv6_established_max);
    printf("Setup Time: %u ms\n", ctx->stats.setup_time);
    printf("Setup Rate: %0.02lf CPS (MIN: %0.02lf AVG: %0.02lf MAX: %0.02lf)\n",
           ctx->stats.cps, ctx->stats.cps_min, ctx->stats.cps_avg, ctx->stats.cps_max);
    printf("Flapped: %u\n", ctx->sessions_flapped);

    if(ctx->op.network_if) {
        if(dict_count(ctx->li_flow_dict)) {
            printf("\nLI Statistics:\n");
            printf("  Flows:        %10lu\n", dict_count(ctx->li_flow_dict));
            printf("  RX Packets:   %10lu\n", ctx->op.network_if->stats.li_rx);
        }
        if(ctx->config.l2tp_server) {
            printf("\nL2TP LNS Statistics:\n");
            printf("  Tunnels:      %10u\n", ctx->l2tp_tunnels_max);
            printf("  Established:  %10u\n", ctx->l2tp_tunnels_established_max);
            printf("  Sessions:     %10u\n", ctx->l2tp_sessions_max);
            printf("  Packets:\n");
            printf("    TX Control:      %10u packets (%u retries)\n", 
                ctx->op.network_if->stats.l2tp_control_tx, ctx->op.network_if->stats.l2tp_control_retry);
            printf("    RX Control:      %10u packets (%u duplicate %u out-of-order)\n", 
                ctx->op.network_if->stats.l2tp_control_rx, 
                ctx->op.network_if->stats.l2tp_control_rx_dup, 
                ctx->op.network_if->stats.l2tp_control_rx_ooo);
            printf("    TX Data:         %10lu packets\n", ctx->op.network_if->stats.l2tp_data_tx);
            printf("    RX Data:         %10lu packets\n", ctx->op.network_if->stats.l2tp_data_rx);
        }
        printf("\nNetwork Interface ( %s ):\n", ctx->op.network_if->name);
        printf("  TX:                %10lu packets\n", ctx->op.network_if->stats.packets_tx);
        printf("  RX:                %10lu packets\n", ctx->op.network_if->stats.packets_rx);
        printf("  TX Session:        %10lu packets\n", ctx->op.network_if->stats.session_ipv4_tx);
        printf("  RX Session:        %10lu packets (%lu loss)\n", ctx->op.network_if->stats.session_ipv4_rx,
               ctx->op.network_if->stats.session_ipv4_loss);
        printf("  TX Session IPv6:   %10lu packets\n", ctx->op.network_if->stats.session_ipv6_tx);
        printf("  RX Session IPv6:   %10lu packets (%lu loss)\n", ctx->op.network_if->stats.session_ipv6_rx,
               ctx->op.network_if->stats.session_ipv6_loss);
        printf("  TX Session IPv6PD: %10lu packets\n", ctx->op.network_if->stats.session_ipv6pd_tx);
        printf("  RX Session IPv6PD: %10lu packets (%lu loss)\n", ctx->op.network_if->stats.session_ipv6pd_rx,
               ctx->op.network_if->stats.session_ipv6pd_loss);
        printf("  TX Multicast:      %10lu packets\n", ctx->op.network_if->stats.mc_tx);
        printf("  RX Drop Unknown:   %10lu packets\n", ctx->op.network_if->stats.packets_rx_drop_unknown);
        printf("  TX Encode Error:   %10lu\n", ctx->op.network_if->stats.encode_errors);
        printf("  RX Decode Error:   %10lu packets\n", ctx->op.network_if->stats.packets_rx_drop_decode_error);
        printf("  TX Send Failed:    %10lu\n", ctx->op.network_if->stats.sendto_failed);
        printf("  TX No Buffer:      %10lu\n", ctx->op.network_if->stats.no_tx_buffer);
        printf("  TX Poll Kernel:    %10lu\n", ctx->op.network_if->stats.poll_tx);
        printf("  RX Poll Kernel:    %10lu\n", ctx->op.network_if->stats.poll_rx);
    }

    for(i=0; i < ctx->op.access_if_count; i++) {
        access_if = ctx->op.access_if[i];
        if(access_if) {
            printf("\nAccess Interface ( %s ):\n", access_if->name);
            printf("  TX:                %10lu packets\n", access_if->stats.packets_tx);
            printf("  RX:                %10lu packets\n", access_if->stats.packets_rx);
            printf("  TX Session:        %10lu packets\n", access_if->stats.session_ipv4_tx);
            printf("  RX Session:        %10lu packets (%lu loss, %lu wrong session)\n", access_if->stats.session_ipv4_rx,
                access_if->stats.session_ipv4_loss, access_if->stats.session_ipv4_wrong_session);
            printf("  TX Session IPv6:   %10lu packets\n", access_if->stats.session_ipv6_tx);
            printf("  RX Session IPv6:   %10lu packets (%lu loss, %lu wrong session)\n", access_if->stats.session_ipv6_rx,
                access_if->stats.session_ipv6_loss, access_if->stats.session_ipv6_wrong_session);
            printf("  TX Session IPv6PD: %10lu packets\n", access_if->stats.session_ipv6pd_tx);
            printf("  RX Session IPv6PD: %10lu packets (%lu loss, %lu wrong session)\n", access_if->stats.session_ipv6pd_rx,
                access_if->stats.session_ipv6pd_loss, access_if->stats.session_ipv6pd_wrong_session);
            printf("  RX Multicast:      %10lu packets (%lu loss)\n", access_if->stats.mc_rx,
                access_if->stats.mc_loss);
            printf("  RX Drop Unknown:   %10lu packets\n", access_if->stats.packets_rx_drop_unknown);
            printf("  TX Encode Error:   %10lu packets\n", access_if->stats.encode_errors);
            printf("  RX Decode Error:   %10lu packets\n", access_if->stats.packets_rx_drop_decode_error);
            printf("  TX Send Failed:    %10lu\n", access_if->stats.sendto_failed);
            printf("  TX No Buffer:      %10lu\n", access_if->stats.no_tx_buffer);
            printf("  TX Poll Kernel:    %10lu\n", access_if->stats.poll_tx);
            printf("  RX Poll Kernel:    %10lu\n", access_if->stats.poll_rx);
            printf("\n  Access Interface Protocol Packet Stats:\n");
            printf("    ARP    TX: %10u RX: %10u\n", access_if->stats.arp_tx, access_if->stats.arp_rx);
            printf("    PADI   TX: %10u RX: %10u\n", access_if->stats.padi_tx, 0);
            printf("    PADO   TX: %10u RX: %10u\n", 0, access_if->stats.pado_rx);
            printf("    PADR   TX: %10u RX: %10u\n", access_if->stats.padr_tx, 0);
            printf("    PADS   TX: %10u RX: %10u\n", 0, access_if->stats.pads_rx);
            printf("    PADT   TX: %10u RX: %10u\n", access_if->stats.padt_tx, access_if->stats.padt_rx);
            printf("    LCP    TX: %10u RX: %10u\n", access_if->stats.lcp_tx, access_if->stats.lcp_rx);
            printf("    PAP    TX: %10u RX: %10u\n", access_if->stats.pap_tx, access_if->stats.pap_rx);
            printf("    CHAP   TX: %10u RX: %10u\n", access_if->stats.chap_tx, access_if->stats.chap_rx);
            printf("    IPCP   TX: %10u RX: %10u\n", access_if->stats.ipcp_tx, access_if->stats.ipcp_rx);
            printf("    IP6CP  TX: %10u RX: %10u\n", access_if->stats.ip6cp_tx, access_if->stats.ip6cp_rx);
            printf("    IGMP   TX: %10u RX: %10u\n", access_if->stats.igmp_tx, access_if->stats.igmp_rx);
            printf("    ICMP   TX: %10u RX: %10u\n", access_if->stats.icmp_tx, access_if->stats.icmp_rx);
            printf("    ICMPv6 TX: %10u RX: %10u\n", access_if->stats.icmpv6_tx, access_if->stats.icmpv6_rx);
            printf("    DHCPv6 TX: %10u RX: %10u\n", access_if->stats.dhcpv6_tx, access_if->stats.dhcpv6_rx);
            printf("\n  Access Interface Protocol Timeout Stats:\n");
            printf("    LCP Echo Request: %10u\n", access_if->stats.lcp_echo_timeout);
            printf("    LCP Request:      %10u\n", access_if->stats.lcp_timeout);
            printf("    IPCP Request:     %10u\n", access_if->stats.ipcp_timeout);
            printf("    IP6CP Request:    %10u\n", access_if->stats.ip6cp_timeout);
            printf("    PAP:              %10u\n", access_if->stats.pap_timeout);
            printf("    CHAP:             %10u\n", access_if->stats.chap_timeout);
            printf("    ICMPv6 RS:        %10u\n", access_if->stats.dhcpv6_timeout);
            printf("    DHCPv6 Request:   %10u\n", access_if->stats.dhcpv6_timeout);
        }
    }

    if(ctx->stats.session_traffic_flows) {
        printf("\nSession Traffic:\n");
        printf("  Config:\n");
        printf("    IPv4    PPS:    %8u\n", ctx->config.session_traffic_ipv4_pps);
        printf("    IPv6    PPS:    %8u\n", ctx->config.session_traffic_ipv6_pps);
        printf("    IPv6PD  PPS:    %8u\n", ctx->config.session_traffic_ipv6pd_pps);
        printf("  Verified Traffic Flows: %u/%u\n", ctx->stats.session_traffic_flows_verified, ctx->stats.session_traffic_flows);
        printf("    Access  IPv4:   %8u\n", stats->sessions_access_ipv4_rx);
        printf("    Access  IPv6:   %8u\n", stats->sessions_access_ipv6_rx);
        printf("    Access  IPv6PD: %8u\n", stats->sessions_access_ipv6pd_rx);
        printf("    Network IPv4:   %8u\n", stats->sessions_network_ipv4_rx);
        printf("    Network IPv6:   %8u\n", stats->sessions_network_ipv6_rx);
        printf("    Network IPv6PD: %8u\n", stats->sessions_network_ipv6pd_rx);
        printf("  First Sequence Number Received:\n");
        printf("    Access  IPv4    MIN: %8lu MAX: %8lu\n", stats->min_access_ipv4_rx_first_seq, stats->max_access_ipv4_rx_first_seq);
        printf("    Access  IPv6    MIN: %8lu MAX: %8lu\n", stats->min_access_ipv6_rx_first_seq, stats->max_access_ipv6_rx_first_seq);
        printf("    Access  IPv6PD  MIN: %8lu MAX: %8lu\n", stats->min_access_ipv6pd_rx_first_seq, stats->max_access_ipv6pd_rx_first_seq);
        printf("    Network IPv4    MIN: %8lu MAX: %8lu\n", stats->min_network_ipv4_rx_first_seq, stats->max_network_ipv4_rx_first_seq);
        printf("    Network IPv6    MIN: %8lu MAX: %8lu\n", stats->min_network_ipv6_rx_first_seq, stats->max_network_ipv6_rx_first_seq);
        printf("    Network IPv6PD  MIN: %8lu MAX: %8lu\n", stats->min_network_ipv6pd_rx_first_seq, stats->max_network_ipv6pd_rx_first_seq);
    }

    if(ctx->config.igmp_group_count > 1) {
        printf("\nIGMP Config:\n");
        printf("  Version: %d\n", ctx->config.igmp_version);
        printf("  Start Delay: %us\n", ctx->config.igmp_start_delay);
        printf("  Group Count: %u \n", ctx->config.igmp_group_count);
        printf("  Zapping Interval: %us\n", ctx->config.igmp_zap_interval);
        printf("  Zapping Count: %u \n", ctx->config.igmp_zap_count);
        printf("  Zapping Channel View Duration: %us\n", ctx->config.igmp_zap_view_duration);
        if(ctx->config.igmp_zap_interval > 0) {
            printf("\nIGMP Zapping Stats:\n");
            printf("  Join Delay:\n");
            printf("    MIN: %u ms\n", stats->min_join_delay);
            printf("    AVG: %u ms\n", stats->avg_join_delay);
            printf("    MAX: %u ms\n", stats->max_join_delay);
            printf("  Leave Delay:\n");
            printf("    MIN: %u ms\n", stats->min_leave_delay);
            printf("    AVG: %u ms\n", stats->avg_leave_delay);
            printf("    MAX: %u ms\n", stats->max_leave_delay);
            printf("  Multicast:\n");
            printf("    Overlap: %u packets\n", stats->mc_old_rx_after_first_new);
            printf("    Not Received: %u\n", stats->mc_not_received);
        }
    }
}

void
bbl_stats_json (bbl_ctx_s *ctx, bbl_stats_t * stats) {
    struct bbl_interface_ *access_if;    
    int i;

    json_t *root               = NULL;
    json_t *jobj               = NULL;
    json_t *jobj_array         = NULL;
    json_t *jobj_access_if     = NULL;
    json_t *jobj_network_if    = NULL;
    json_t *jobj_l2tp          = NULL;
    json_t *jobj_li            = NULL;
    json_t *jobj_straffic      = NULL;
    json_t *jobj_multicast     = NULL;
    json_t *jobj_protocols     = NULL;

    if(!ctx->config.json_report_filename) return;

    root = json_object();
    jobj = json_object();

    json_object_set(jobj, "sessions", json_integer(ctx->config.sessions));
    json_object_set(jobj, "sessions-pppoe", json_integer(ctx->sessions_pppoe));
    json_object_set(jobj, "sessions-ipoe", json_integer(ctx->sessions_ipoe));
    json_object_set(jobj, "sessions-established", json_integer(ctx->sessions_established_max));
    json_object_set(jobj, "sessions-flapped", json_integer(ctx->sessions_flapped));
    json_object_set(jobj, "setup-time-ms", json_integer(ctx->stats.setup_time));
    json_object_set(jobj, "setup-rate-cps", json_real(ctx->stats.cps));
    json_object_set(jobj, "setup-rate-cps-min", json_real(ctx->stats.cps_min));
    json_object_set(jobj, "setup-rate-cps-avg", json_real(ctx->stats.cps_avg));
    json_object_set(jobj, "setup-rate-cps-max", json_real(ctx->stats.cps_max));
    json_object_set(jobj, "dhcpv6-sessions-established", json_integer(ctx->dhcpv6_established_max));

    jobj_array = json_array();
    if (ctx->op.network_if) {
        if(dict_count(ctx->li_flow_dict)) {
            jobj_li = json_object();
            json_object_set(jobj_li, "flows", json_integer(dict_count(ctx->li_flow_dict)));
            json_object_set(jobj_li, "rx-packets", json_integer(ctx->op.network_if->stats.li_rx));
            json_object_set(jobj, "li-statistics", jobj_li);
        }
        if(ctx->config.l2tp_server) {
            jobj_l2tp = json_object();
            json_object_set(jobj_l2tp, "tunnels", json_integer(ctx->l2tp_tunnels_max));
            json_object_set(jobj_l2tp, "tunnels-established", json_integer(ctx->l2tp_tunnels_established_max));
            json_object_set(jobj_l2tp, "sessions", json_integer(ctx->l2tp_sessions_max));
            json_object_set(jobj_l2tp, "tx-control-packets", json_integer(ctx->op.network_if->stats.l2tp_control_tx));
            json_object_set(jobj_l2tp, "tx-control-packets-retry", json_integer(ctx->op.network_if->stats.l2tp_control_retry));
            json_object_set(jobj_l2tp, "rx-control-packets", json_integer(ctx->op.network_if->stats.l2tp_control_rx));
            json_object_set(jobj_l2tp, "rx-control-packets-duplicate", json_integer(ctx->op.network_if->stats.l2tp_control_rx_dup));
            json_object_set(jobj_l2tp, "rx-control-packets-out-of-order", json_integer(ctx->op.network_if->stats.l2tp_control_rx_ooo));
            json_object_set(jobj_l2tp, "tx-data-packets", json_integer(ctx->op.network_if->stats.l2tp_data_tx));
            json_object_set(jobj_l2tp, "rx-data-packets", json_integer(ctx->op.network_if->stats.l2tp_data_rx));
            json_object_set(jobj, "l2tp", jobj_l2tp);
        }
        jobj_network_if = json_object();
        json_object_set(jobj_network_if, "name", json_string(ctx->op.network_if->name));
        json_object_set(jobj_network_if, "tx-packets", json_integer(ctx->op.network_if->stats.packets_tx));
        json_object_set(jobj_network_if, "rx-packets", json_integer(ctx->op.network_if->stats.packets_rx));
        json_object_set(jobj_network_if, "tx-session-packets", json_integer(ctx->op.network_if->stats.session_ipv4_tx));
        json_object_set(jobj_network_if, "rx-session-packets", json_integer(ctx->op.network_if->stats.session_ipv4_rx));
        json_object_set(jobj_network_if, "rx-session-packets-loss", json_integer(ctx->op.network_if->stats.session_ipv4_loss));
        json_object_set(jobj_network_if, "tx-session-packets-avg-pps-max", json_integer(ctx->op.network_if->stats.rate_session_ipv4_tx.avg_max));
        json_object_set(jobj_network_if, "rx-session-packets-avg-pps-max", json_integer(ctx->op.network_if->stats.rate_session_ipv4_rx.avg_max));
        json_object_set(jobj_network_if, "tx-session-packets-ipv6", json_integer(ctx->op.network_if->stats.session_ipv6_tx));
        json_object_set(jobj_network_if, "rx-session-packets-ipv6", json_integer(ctx->op.network_if->stats.session_ipv6_rx));
        json_object_set(jobj_network_if, "rx-session-packets-ipv6-loss", json_integer(ctx->op.network_if->stats.session_ipv6_loss));
        json_object_set(jobj_network_if, "tx-session-packets-avg-pps-max-ipv6", json_integer(ctx->op.network_if->stats.rate_session_ipv6_tx.avg_max));
        json_object_set(jobj_network_if, "rx-session-packets-avg-pps-max-ipv6", json_integer(ctx->op.network_if->stats.rate_session_ipv6_rx.avg_max));
        json_object_set(jobj_network_if, "tx-session-packets-ipv6pd", json_integer(ctx->op.network_if->stats.session_ipv6pd_tx));
        json_object_set(jobj_network_if, "rx-session-packets-ipv6pd", json_integer(ctx->op.network_if->stats.session_ipv6pd_rx));
        json_object_set(jobj_network_if, "rx-session-packets-ipv6pd-loss", json_integer(ctx->op.network_if->stats.session_ipv6pd_loss));
        json_object_set(jobj_network_if, "tx-session-packets-avg-pps-max-ipv6pd", json_integer(ctx->op.network_if->stats.rate_session_ipv6pd_tx.avg_max));
        json_object_set(jobj_network_if, "rx-session-packets-avg-pps-max-ipv6pd", json_integer(ctx->op.network_if->stats.rate_session_ipv6pd_rx.avg_max));
        json_object_set(jobj_network_if, "tx-multicast-packets", json_integer(ctx->op.network_if->stats.mc_tx));
        json_array_append(jobj_array, jobj_network_if);
    }
    json_object_set(jobj, "network-interfaces", jobj_array);

    jobj_array = json_array();
    for(i=0; i < ctx->op.access_if_count; i++) {
        access_if = ctx->op.access_if[i];
        if (access_if) {
            jobj_access_if = json_object();
            json_object_set(jobj_access_if, "name", json_string(access_if->name));
            json_object_set(jobj_access_if, "tx-packets", json_integer(access_if->stats.packets_tx));
            json_object_set(jobj_access_if, "rx-packets", json_integer(access_if->stats.packets_rx));
            json_object_set(jobj_access_if, "tx-session-packets", json_integer(access_if->stats.session_ipv4_tx));
            json_object_set(jobj_access_if, "rx-session-packets", json_integer(access_if->stats.session_ipv4_rx));
            json_object_set(jobj_access_if, "rx-session-packets-loss", json_integer(access_if->stats.session_ipv4_loss));
            json_object_set(jobj_access_if, "rx-session-packets-wrong-session", json_integer(access_if->stats.session_ipv4_wrong_session));
            json_object_set(jobj_access_if, "tx-session-packets-avg-pps-max", json_integer(access_if->stats.rate_session_ipv4_tx.avg_max));
            json_object_set(jobj_access_if, "rx-session-packets-avg-pps-max", json_integer(access_if->stats.rate_session_ipv4_rx.avg_max));
            json_object_set(jobj_access_if, "tx-session-packets-ipv6", json_integer(access_if->stats.session_ipv6_tx));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6", json_integer(access_if->stats.session_ipv6_rx));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6-loss", json_integer(access_if->stats.session_ipv6_loss));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6-wrong-session", json_integer(access_if->stats.session_ipv6_wrong_session));
            json_object_set(jobj_access_if, "tx-session-packets-avg-pps-max-ipv6", json_integer(access_if->stats.rate_session_ipv6_tx.avg_max));
            json_object_set(jobj_access_if, "rx-session-packets-avg-pps-max-ipv6", json_integer(access_if->stats.rate_session_ipv6_rx.avg_max));
            json_object_set(jobj_access_if, "tx-session-packets-ipv6pd", json_integer(access_if->stats.session_ipv6pd_tx));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6pd", json_integer(access_if->stats.session_ipv6pd_rx));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6pd-loss", json_integer(access_if->stats.session_ipv6pd_loss));
            json_object_set(jobj_access_if, "rx-session-packets-ipv6pd-wrong-session", json_integer(access_if->stats.session_ipv6pd_wrong_session));
            json_object_set(jobj_access_if, "tx-session-packets-avg-pps-max-ipv6pd", json_integer(access_if->stats.rate_session_ipv6pd_tx.avg_max));
            json_object_set(jobj_access_if, "rx-session-packets-avg-pps-max-ipv6pd", json_integer(access_if->stats.rate_session_ipv6pd_rx.avg_max));
            json_object_set(jobj_access_if, "rx-multicast-packets", json_integer(access_if->stats.mc_rx));
            json_object_set(jobj_access_if, "rx-multicast-packets-loss", json_integer(access_if->stats.mc_loss));
            jobj_protocols = json_object();
            json_object_set(jobj_protocols, "arp-tx", json_integer(access_if->stats.arp_tx));
            json_object_set(jobj_protocols, "arp-rx", json_integer(access_if->stats.arp_rx));
            json_object_set(jobj_protocols, "padi-tx", json_integer(access_if->stats.padi_tx));
            json_object_set(jobj_protocols, "pado-rx", json_integer(access_if->stats.pado_rx));
            json_object_set(jobj_protocols, "padr-tx", json_integer(access_if->stats.padr_tx));
            json_object_set(jobj_protocols, "pads-rx", json_integer(access_if->stats.pads_rx));
            json_object_set(jobj_protocols, "padt-tx", json_integer(access_if->stats.padt_tx));
            json_object_set(jobj_protocols, "padt-rx", json_integer(access_if->stats.padt_rx));
            json_object_set(jobj_protocols, "lcp-tx", json_integer(access_if->stats.lcp_tx));
            json_object_set(jobj_protocols, "lcp-rx", json_integer(access_if->stats.lcp_rx));
            json_object_set(jobj_protocols, "pap-tx", json_integer(access_if->stats.pap_tx));
            json_object_set(jobj_protocols, "pap-rx", json_integer(access_if->stats.pap_rx));
            json_object_set(jobj_protocols, "chap-tx", json_integer(access_if->stats.chap_tx));
            json_object_set(jobj_protocols, "chap-rx", json_integer(access_if->stats.chap_rx));
            json_object_set(jobj_protocols, "ipcp-tx", json_integer(access_if->stats.ipcp_tx));
            json_object_set(jobj_protocols, "ipcp-rx", json_integer(access_if->stats.ipcp_rx));
            json_object_set(jobj_protocols, "ip6cp-tx", json_integer(access_if->stats.ip6cp_tx));
            json_object_set(jobj_protocols, "ip6cp-rx", json_integer(access_if->stats.ip6cp_rx));
            json_object_set(jobj_protocols, "igmp-tx", json_integer(access_if->stats.igmp_tx));
            json_object_set(jobj_protocols, "igmp-rx", json_integer(access_if->stats.igmp_rx));
            json_object_set(jobj_protocols, "icmp-tx", json_integer(access_if->stats.icmp_tx));
            json_object_set(jobj_protocols, "icmp-rx", json_integer(access_if->stats.icmp_rx));
            json_object_set(jobj_protocols, "icmpv6-tx", json_integer(access_if->stats.icmpv6_tx));
            json_object_set(jobj_protocols, "icmpv6-rx", json_integer(access_if->stats.icmpv6_rx));
            json_object_set(jobj_protocols, "dhcpv6-tx", json_integer(access_if->stats.dhcpv6_tx));
            json_object_set(jobj_protocols, "dhcpv6-rx", json_integer(access_if->stats.dhcpv6_rx));
            json_object_set(jobj_protocols, "lcp-echo-timeout", json_integer(access_if->stats.lcp_echo_timeout));
            json_object_set(jobj_protocols, "lcp-request-timeout", json_integer(access_if->stats.lcp_timeout));
            json_object_set(jobj_protocols, "ipcp-request-timeout", json_integer(access_if->stats.ipcp_timeout));
            json_object_set(jobj_protocols, "ip6cp-request-timeout", json_integer(access_if->stats.ip6cp_timeout));
            json_object_set(jobj_protocols, "pap-timeout", json_integer(access_if->stats.pap_timeout));
            json_object_set(jobj_protocols, "chap-timeout", json_integer(access_if->stats.chap_timeout));
            json_object_set(jobj_protocols, "icmpv6-rs-timeout", json_integer(access_if->stats.dhcpv6_timeout));
            json_object_set(jobj_protocols, "dhcpv6-timeout", json_integer(access_if->stats.dhcpv6_timeout));
            json_object_set(jobj_access_if, "protocol-stats", jobj_protocols);
            json_array_append(jobj_array, jobj_access_if);
        }
    }
    json_object_set(jobj, "access-interfaces", jobj_array);

    if(ctx->stats.session_traffic_flows) {
        jobj_straffic = json_object();
        json_object_set(jobj_straffic, "config-ipv4-pps", json_integer(ctx->config.session_traffic_ipv4_pps));
        json_object_set(jobj_straffic, "config-ipv6-pps", json_integer(ctx->config.session_traffic_ipv6_pps));
        json_object_set(jobj_straffic, "config-ipv6pd-pps", json_integer(ctx->config.session_traffic_ipv6pd_pps));
        json_object_set(jobj_straffic, "total-flows", json_integer(ctx->stats.session_traffic_flows));
        json_object_set(jobj_straffic, "verified-flows", json_integer(ctx->stats.session_traffic_flows_verified));
        json_object_set(jobj_straffic, "verified-flows-access-ipv4", json_integer(stats->sessions_access_ipv4_rx));
        json_object_set(jobj_straffic, "verified-flows-access-ipv6", json_integer(stats->sessions_access_ipv6_rx));
        json_object_set(jobj_straffic, "verified-flows-access-ipv6pd", json_integer(stats->sessions_access_ipv6pd_rx));
        json_object_set(jobj_straffic, "verified-flows-network-ipv4", json_integer(stats->sessions_network_ipv4_rx));
        json_object_set(jobj_straffic, "verified-flows-network-ipv6", json_integer(stats->sessions_network_ipv6_rx));
        json_object_set(jobj_straffic, "verified-flows-network-ipv6pd", json_integer(stats->sessions_network_ipv6pd_rx));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv4-min", json_integer(stats->min_access_ipv4_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv4-max", json_integer(stats->max_access_ipv4_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv6-min", json_integer(stats->min_access_ipv6_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv6-max", json_integer(stats->max_access_ipv6_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv6pd-min", json_integer(stats->min_access_ipv6pd_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-access-ipv6pd-max", json_integer(stats->max_access_ipv6pd_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv4-min", json_integer(stats->min_network_ipv4_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv4-max", json_integer(stats->max_network_ipv4_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv6-min", json_integer(stats->min_network_ipv6_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv6-max", json_integer(stats->max_network_ipv6_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv6pd-min", json_integer(stats->min_network_ipv6pd_rx_first_seq));
        json_object_set(jobj_straffic, "first-seq-rx-network-ipv6pd-max", json_integer(stats->max_network_ipv6pd_rx_first_seq));
        json_object_set(jobj, "session-traffic", jobj_straffic);
    }
    if(ctx->config.igmp_group_count > 1) {
        jobj_multicast = json_object();
        json_object_set(jobj_multicast, "config-version", json_integer(ctx->config.igmp_version));
        json_object_set(jobj_multicast, "config-start-delay", json_integer(ctx->config.igmp_start_delay));
        json_object_set(jobj_multicast, "config-group-count", json_integer(ctx->config.igmp_group_count));
        json_object_set(jobj_multicast, "config-zapping-interval", json_integer(ctx->config.igmp_zap_interval));
        json_object_set(jobj_multicast, "config-zapping-count", json_integer(ctx->config.igmp_zap_count));
        json_object_set(jobj_multicast, "config-zapping-view-duration", json_integer(ctx->config.igmp_zap_view_duration));
        if(ctx->config.igmp_zap_interval > 0) {
            json_object_set(jobj_multicast, "zapping-join-delay-ms-min", json_integer(stats->min_join_delay));
            json_object_set(jobj_multicast, "zapping-join-delay-ms-avg", json_integer(stats->avg_join_delay));
            json_object_set(jobj_multicast, "zapping-join-delay-ms-max", json_integer(stats->max_join_delay));
            json_object_set(jobj_multicast, "zapping-leave-delay-ms-min", json_integer(stats->min_leave_delay));
            json_object_set(jobj_multicast, "zapping-leave-delay-ms-avg", json_integer(stats->avg_leave_delay));
            json_object_set(jobj_multicast, "zapping-leave-delay-ms-max", json_integer(stats->max_leave_delay));
            json_object_set(jobj_multicast, "zapping-multicast-packets-overlap", json_integer(stats->mc_old_rx_after_first_new));
            json_object_set(jobj_multicast, "zapping-multicast-not-received", json_integer(stats->mc_not_received));
            json_object_set(jobj, "multicast", jobj_multicast);
        }
    }
    json_object_set(root, "report", jobj);
    if(json_dump_file(root, ctx->config.json_report_filename, JSON_REAL_PRECISION(4)) != 0) {
        LOG(ERROR, "Failed to create JSON report file %s\n", ctx->config.json_report_filename);
    }
    json_decref(root);
}

/*
 * Compute a pps rate using a moving average of <BBL_AVG_SAMPLE> samples.
 */
void
bbl_compute_avg_rate (bbl_rate_s *rate, uint64_t current_value)
{
    uint idx, div;
    uint64_t sum;

    if(current_value == 0) return;

    rate->diff_value[rate->cursor] = current_value - rate->last_value;

    sum = 0;
    div = 0;
    for (idx = 0; idx < BBL_AVG_SAMPLES; idx++) {
        if (rate->diff_value[idx]) {
            sum += rate->diff_value[idx];
            div++;
        }
    }
    if (div) {
	    rate->avg = sum / div;
    } else {
	    rate->avg = 0;
    }
    if(rate->avg > rate->avg_max) {
        rate->avg_max = rate->avg;
    }
    rate->cursor = (rate->cursor + 1) % BBL_AVG_SAMPLES;
    rate->last_value = current_value;
}

void
bbl_compute_interface_rate_job (timer_s *timer)
{
    bbl_interface_s *interface;

    interface = timer->data;

    bbl_compute_avg_rate(&interface->stats.rate_packets_tx, interface->stats.packets_tx);
    bbl_compute_avg_rate(&interface->stats.rate_packets_rx, interface->stats.packets_rx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv4_tx, interface->stats.session_ipv4_tx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv4_rx, interface->stats.session_ipv4_rx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv6_tx, interface->stats.session_ipv6_tx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv6_rx, interface->stats.session_ipv6_rx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv6pd_tx, interface->stats.session_ipv6pd_tx);
    bbl_compute_avg_rate(&interface->stats.rate_session_ipv6pd_rx, interface->stats.session_ipv6pd_rx);
    bbl_compute_avg_rate(&interface->stats.rate_mc_rx, interface->stats.mc_rx);
    if(!interface->access) {
        bbl_compute_avg_rate(&interface->stats.rate_mc_tx, interface->stats.mc_tx);
        bbl_compute_avg_rate(&interface->stats.rate_l2tp_data_rx, interface->stats.l2tp_data_rx);
        bbl_compute_avg_rate(&interface->stats.rate_l2tp_data_tx, interface->stats.l2tp_data_tx);
        bbl_compute_avg_rate(&interface->stats.rate_li_rx, interface->stats.li_rx);
    }
}
