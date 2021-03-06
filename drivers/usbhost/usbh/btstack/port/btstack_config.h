//
// btstack_config.h for Arduino port
//

#ifndef __BTSTACK_CONFIG
#define __BTSTACK_CONFIG

// Port related features
#define HAVE_EMBEDDED_TIME_MS

// BTstack features that can be enabled
//#define ENABLE_BLE
#define ENABLE_CLASSIC
//#define ENABLE_LOG_INTO_HCI_DUMP
//#define ENABLE_LOG_INFO
//#define ENABLE_LOG_DEBUG
//#define ENABLE_LOG_ERROR

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE 300
#define MAX_NR_BNEP_SERVICES 0
#define MAX_NR_BNEP_CHANNELS 0
#define MAX_NR_GATT_SUBCLIENTS 0
#define MAX_NR_HCI_CONNECTIONS 1
#define MAX_NR_L2CAP_SERVICES  0
#define MAX_NR_L2CAP_CHANNELS  2
#define MAX_NR_RFCOMM_MULTIPLEXERS 0
#define MAX_NR_RFCOMM_SERVICES 0
#define MAX_NR_RFCOMM_CHANNELS 0
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES  0
#define MAX_NR_GATT_CLIENTS 0
#define MAX_ATT_DB_SIZE 100
#define MAX_NR_HFP_CONNECTIONS 0
#define MAX_NR_WHITELIST_ENTRIES 0
#define MAX_NR_SM_LOOKUP_ENTRIES 0
#define MAX_NR_SERVICE_RECORD_ITEMS 0
#endif
