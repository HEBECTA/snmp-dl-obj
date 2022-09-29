#ifndef NETWORK_H
#define NETWORK_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#define CONFIG_PATH "/etc/config"
#define CONFIG_FILE "network"
#define CONFIG_SECTION "lan"
#define OPTION "ipaddr"
#define SECTION_NAME "network.lan"
#define IP_BUFF_SIZE 16

#define OID "1.3.6.1.4.1.48690.7.1.0"
static oid network_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 7, 1, 0};

void init_nstAgentPluginObject (void);

void deinit_nstAgentPluginObject (void);

static int operation_handler(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);

static int get_uci_ipaddr(char *ip_buff);

static int set_uci_ipaddr(char *ip_addr);


#endif