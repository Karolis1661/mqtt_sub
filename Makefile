include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt_sub
PKG_RELEASE:=1
PKG_VERSION:=2.3.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mqtt_sub
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Mqtt subscriber
	MAINTAINER:= karolis@test.com
	DEPENDS:=+libuci +libmosquitto +libsqlite3 +libjson-c +libcurl
endef

define Package/mqtt_sub/description
	Subscriber for mosquitto broker
endef

define Package/mqtt_sub/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/mqtt_sub.init $(1)/etc/init.d/mqtt_sub
	$(INSTALL_CONF) ./files/mqtt_sub.config $(1)/etc/config/mqtt_sub
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mqtt_sub $(1)/usr/sbin/mqtt_sub
endef

$(eval $(call BuildPackage,mqtt_sub))