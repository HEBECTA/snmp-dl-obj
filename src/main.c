#include "network.h"

#include <syslog.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <uci.h>

void init_network (void)
{
        int rc = 0;

        netsnmp_handler_registration *reginfo = NULL;

        reginfo = netsnmp_create_handler_registration("ipaddr", operation_handler, network_oid,
                                OID_LENGTH(network_oid), HANDLER_CAN_RWRITE);

        if (reginfo == NULL)
                goto EXIT_INIT_NET_FUN;

        rc = netsnmp_register_handler(reginfo);
        if (rc != SNMPERR_SUCCESS)
                goto EXIT_INIT_NET_FUN;

        return;

EXIT_INIT_NET_FUN:

        syslog(LOG_ERR, "SNMP agent network module: Failed to initialize OID: %s", OID);
}

void deinit_network (void)
{
        unregister_mib(network_oid,OID_LENGTH(network_oid));
}

static int operation_handler(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
        int rc = 0;
        char ip_buff[IP_BUFF_SIZE];
        struct sockaddr_in sa;

        switch (reqinfo->mode){

        case MODE_GET:

                syslog(LOG_NOTICE, "SNMP agent network module: OID: %s received operation get", OID);

                rc = get_uci_ipaddr(ip_buff);

                if (rc){

                        syslog(LOG_ERR, "SNMP agent network module: OID: %s failed to get ip adress from uci", OID);

                        return SNMP_ERR_RESOURCEUNAVAILABLE;
                }

                snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, ip_buff, strlen(ip_buff));

                break;

        case MODE_SET_RESERVE1:

                if (requests->requestvb->type != ASN_OCTET_STR)
                        return SNMP_ERR_WRONGTYPE;

                rc = inet_pton(AF_INET, (char *)requests->requestvb->val.string, &(sa.sin_addr));

                if (!rc){

                        syslog(LOG_ERR, "SNMP agent network module: OID: %s received value %s which is not ip address", OID, (char *)requests->requestvb->val.string);

                        return SNMP_ERR_WRONGVALUE;
                }

                break;

        case MODE_SET_RESERVE2:

                break;

        case MODE_SET_FREE:

                break;

        case MODE_SET_ACTION:

                break;

        case MODE_SET_COMMIT:

                syslog(LOG_NOTICE, "SNMP agent network module: OID: %s received operation set, value %s", OID, (char *)requests->requestvb->val.string);

                rc = get_uci_ipaddr(ip_buff);

                if (!rc && strcmp(ip_buff, (char *)requests->requestvb->val.string) == 0)
                        break;

                rc = set_uci_ipaddr((char *)requests->requestvb->val.string);
                if (rc)
                        syslog(LOG_NOTICE, "SNMP agent network module: OID: %s failed to set router's ip", OID);

                break;

        case MODE_SET_UNDO:

                break;

        default:

                return SNMP_ERR_GENERR;
        }

        return SNMP_ERR_NOERROR;
}

static int get_uci_ipaddr(char *ip_buff)
{
        int rc = 0;

        struct uci_context *uci_ctx = NULL;
        struct uci_package *package = NULL;
        struct uci_section *sct = NULL;

        uci_ctx = uci_alloc_context();
        if (uci_ctx == NULL) 
                return ENODATA;

        rc = uci_set_confdir(uci_ctx, CONFIG_PATH);
        if (rc)
                goto EXIT_GET_IP_FUN;

        rc = uci_load(uci_ctx, CONFIG_FILE, &package);
        if (rc)
               goto EXIT_GET_IP_FUN;

        sct = uci_lookup_section(uci_ctx, package, CONFIG_SECTION);

        if (sct == NULL){

                rc = ENODATA;
                goto EXIT_GET_IP_FUN;
        }

        struct uci_option *o = uci_lookup_option(uci_ctx, sct, OPTION);

        if (o == NULL){

                rc = ENODATA;
                goto EXIT_GET_IP_FUN;
        }

        if (o->type != UCI_TYPE_STRING){

                rc = ENODATA;
                goto EXIT_GET_IP_FUN;
        }       

        int uci_option_size = strlen(o->v.string);

        if (!(uci_option_size < IP_BUFF_SIZE)){

                rc = ENOBUFS;
                goto EXIT_GET_IP_FUN;
        }

        strncpy(ip_buff, o->v.string, uci_option_size);
        ip_buff[uci_option_size] = '\0';

EXIT_GET_IP_FUN:

        uci_free_context(uci_ctx);

        return rc;
}

static int set_uci_ipaddr(char *ip_addr)
{

        int rc = 0;

        struct uci_ptr config;
        struct uci_context *uci_ctx = NULL;

        uci_ctx = uci_alloc_context();

        if (!uci_ctx) 
                return ENODATA;

        char section_name[] = SECTION_NAME;

        if (uci_lookup_ptr(uci_ctx, &config, section_name, true) != UCI_OK || !config.s) {

                rc = ENODATA;
                goto EXIT_SET_IP_FUN;
        }

        config.option = OPTION;
        config.value = ip_addr;

        if (uci_set(uci_ctx, &config) != UCI_OK) {

                rc = EPERM;
                goto EXIT_SET_IP_FUN;
        }

        if (uci_commit(uci_ctx, &config.p, false) != UCI_OK) {

                rc = EPERM;
                goto EXIT_SET_IP_FUN;
        }

        rc = system("/etc/init.d/network reload");

EXIT_SET_IP_FUN:

        uci_free_context(uci_ctx);

        return rc;
}