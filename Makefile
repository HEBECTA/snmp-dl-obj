include $(TOPDIR)/rules.mk

PKG_NAME:=snmp-dl-obj
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/snmp-dl-obj
	DEPENDS:=+libnetsnmp +libuci
	CATEGORY:=Base system
	TITLE:=snmp dynamically loaded object
endef

define Package/snmp-dl-obj/description
	snmp dynamically loaded object
endef

define Package/snmp-dl-obj/install
	$(INSTALL_DIR) $(1)/usr/lib/snmpd-mod
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/network.so $(1)/usr/lib/snmpd-mod
	$(INSTALL_BIN) ./files/snmpd_tmp.init $(1)/etc/init.d/snmpd_tmp
	$(INSTALL_CONF) ./files/snmpd_tmp.config $(1)/etc/config/snmpd_tmp
endef

$(eval $(call BuildPackage,snmp-dl-obj))