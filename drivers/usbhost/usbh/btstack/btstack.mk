BTSTACKPATH = $(DRIVERPATH)/usbhost/usbh/btstack

#DRIVERSRC += $(BTSTACKPATH)/ad_parser.c
DRIVERSRC += $(BTSTACKPATH)/btstack_linked_list.c
DRIVERSRC += $(BTSTACKPATH)/btstack_memory.c
DRIVERSRC += $(BTSTACKPATH)/btstack_memory_pool.c
#DRIVERSRC += $(BTSTACKPATH)/btstack_ring_buffer.c
DRIVERSRC += $(BTSTACKPATH)/btstack_run_loop.c
#DRIVERSRC += $(BTSTACKPATH)/btstack_slip.c
DRIVERSRC += $(BTSTACKPATH)/btstack_util.c
DRIVERSRC += $(BTSTACKPATH)/hci.c
DRIVERSRC += $(BTSTACKPATH)/hci_cmd.c
#DRIVERSRC += $(BTSTACKPATH)/hci_dump.c
DRIVERSRC += $(BTSTACKPATH)/l2cap.c
DRIVERSRC += $(BTSTACKPATH)/l2cap_signaling.c
DRIVERSRC += $(BTSTACKPATH)/port/btstack_run_loop_embedded.c
DRIVERSRC += $(BTSTACKPATH)/port/hci_transport_h2.c

#DRIVERSRC += $(BTSTACKPATH)/classic/bnep.c
#DRIVERSRC += $(BTSTACKPATH)/classic/btstack_cvsd_plc.c
#DRIVERSRC += $(BTSTACKPATH)/classic/btstack_link_key_db_memory.c
#DRIVERSRC += $(BTSTACKPATH)/classic/btstack_sbc_bludroid.c
#DRIVERSRC += $(BTSTACKPATH)/classic/btstack_sbc_plc.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hfp.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hfp_ag.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hfp_gsm_model.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hfp_hf.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hfp_msbc.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hsp_ag.c
#DRIVERSRC += $(BTSTACKPATH)/classic/hsp_hs.c
#DRIVERSRC += $(BTSTACKPATH)/classic/pan.c
#DRIVERSRC += $(BTSTACKPATH)/classic/rfcomm.c
#DRIVERSRC += $(BTSTACKPATH)/classic/sdp_client.c
#DRIVERSRC += $(BTSTACKPATH)/classic/sdp_client_rfcomm.c
#DRIVERSRC += $(BTSTACKPATH)/classic/sdp_server.c
#DRIVERSRC += $(BTSTACKPATH)/classic/sdp_util.c
#DRIVERSRC += $(BTSTACKPATH)/classic/spp_server.c

#DRIVERSRC += $(BTSTACKPATH)/ble/ancs_client.c
#DRIVERSRC += $(BTSTACKPATH)/ble/att_cb.c
#DRIVERSRC += $(BTSTACKPATH)/ble/att_db_util.c
#DRIVERSRC += $(BTSTACKPATH)/ble/att_dispatch.c
#DRIVERSRC += $(BTSTACKPATH)/ble/att_server.c
#DRIVERSRC += $(BTSTACKPATH)/ble/gatt_client.c
#DRIVERSRC += $(BTSTACKPATH)/ble/le_device_db_memory.c
#DRIVERSRC += $(BTSTACKPATH)/ble/sm.c
#DRIVERSRC += $(BTSTACKPATH)/ble/sm_mbedtls_allocator.c


DRIVERINC += $(BTSTACKPATH)
DRIVERINC += $(BTSTACKPATH)/port
