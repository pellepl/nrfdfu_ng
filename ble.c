#include "project.h"
#include "dfu.h"

#include "bluez/gatt-client.h"
#include "bluez/mainloop.h"

static int verbose = 0;

#define ATT_CID 4

static bt_uuid_t fota_uuid, cp_uuid, data_uuid, cccd_uuid;


static const char *
ecode_to_string (uint8_t ecode)
{
  switch (ecode)
    {
    case BT_ATT_ERROR_INVALID_HANDLE:
      return "Invalid Handle";
    case BT_ATT_ERROR_READ_NOT_PERMITTED:
      return "Read Not Permitted";
    case BT_ATT_ERROR_WRITE_NOT_PERMITTED:
      return "Write Not Permitted";
    case BT_ATT_ERROR_INVALID_PDU:
      return "Invalid PDU";
    case BT_ATT_ERROR_AUTHENTICATION:
      return "Authentication Required";
    case BT_ATT_ERROR_REQUEST_NOT_SUPPORTED:
      return "Request Not Supported";
    case BT_ATT_ERROR_INVALID_OFFSET:
      return "Invalid Offset";
    case BT_ATT_ERROR_AUTHORIZATION:
      return "Authorization Required";
    case BT_ATT_ERROR_PREPARE_QUEUE_FULL:
      return "Prepare Write Queue Full";
    case BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND:
      return "Attribute Not Found";
    case BT_ATT_ERROR_ATTRIBUTE_NOT_LONG:
      return "Attribute Not Long";
    case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
      return "Insuficient Encryption Key Size";
    case BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN:
      return "Invalid Attribute value len";
    case BT_ATT_ERROR_UNLIKELY:
      return "Unlikely Error";
    case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION:
      return "Insufficient Encryption";
    case BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE:
      return "Group type Not Supported";
    case BT_ATT_ERROR_INSUFFICIENT_RESOURCES:
      return "Insufficient Resources";
    default:
      return "Unknown error type";
    }
}






static void
att_debug_cb (const char *str, void *user_data)
{
  const char *prefix = user_data;

  fprintf (stderr, "%s:%s\n", prefix, str);
}

static void
gatt_debug_cb (const char *str, void *user_data)
{
  const char *prefix = user_data;

  fprintf (stderr, "%s:%s\n", prefix, str);
}

static void
log_service_event (struct gatt_db_attribute *attr, const char *str)
{
  char uuid_str[MAX_LEN_UUID_STR];
  bt_uuid_t uuid;
  uint16_t start, end;

  gatt_db_attribute_get_service_uuid (attr, &uuid);
  bt_uuid_to_string (&uuid, uuid_str, sizeof (uuid_str));

  gatt_db_attribute_get_service_handles (attr, &start, &end);

  fprintf (stderr, "%s - UUID: %s start: 0x%04x end: 0x%04x\n", str, uuid_str,
           start, end);
}

static void
service_added_cb (struct gatt_db_attribute *attr, void *user_data)
{
  log_service_event (attr, "Service Discovered");
}

static void
service_removed_cb (struct gatt_db_attribute *attr, void *user_data)
{
  log_service_event (attr, "Service Removed");
}


static int
l2cap_le_att_connect (bdaddr_t * src, bdaddr_t * dst, uint8_t dst_type,
                      int sec)
{
  int sock;
  struct sockaddr_l2 srcaddr, dstaddr;
  struct bt_security btsec;

  if (verbose)
    {
      char srcaddr_str[18], dstaddr_str[18];

      ba2str (src, srcaddr_str);
      ba2str (dst, dstaddr_str);

      printf ("btgatt-client: Opening L2CAP LE connection on ATT "
              "channel:\n\t src: %s\n\tdest: %s\n", srcaddr_str, dstaddr_str);
    }

  sock = socket (PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if (sock < 0)
    {
      perror ("Failed to create L2CAP socket");
      return -1;
    }

  /* Set up source address */
  memset (&srcaddr, 0, sizeof (srcaddr));
  srcaddr.l2_family = AF_BLUETOOTH;
  srcaddr.l2_cid = htobs (ATT_CID);
  srcaddr.l2_bdaddr_type = 0;
  bacpy (&srcaddr.l2_bdaddr, src);

  if (bind (sock, (struct sockaddr *) &srcaddr, sizeof (srcaddr)) < 0)
    {
      perror ("Failed to bind L2CAP socket");
      close (sock);
      return -1;
    }

  /* Set the security level */
  memset (&btsec, 0, sizeof (btsec));
  btsec.level = sec;
  if (setsockopt (sock, SOL_BLUETOOTH, BT_SECURITY, &btsec,
                  sizeof (btsec)) != 0)
    {
      fprintf (stderr, "Failed to set L2CAP security level\n");
      close (sock);
      return -1;
    }

  /* Set up destination address */
  memset (&dstaddr, 0, sizeof (dstaddr));
  dstaddr.l2_family = AF_BLUETOOTH;
  dstaddr.l2_cid = htobs (ATT_CID);
  dstaddr.l2_bdaddr_type = dst_type;
  bacpy (&dstaddr.l2_bdaddr, dst);

  printf ("Connecting to device...");
  fflush (stdout);

  if (connect (sock, (struct sockaddr *) &dstaddr, sizeof (dstaddr)) < 0)
    {
      perror (" Failed to connect");
      close (sock);
      return -1;
    }

  printf (" Done\n");

  return sock;
}



void ble_close (BLE * ble)
{
  if (!ble)
    return;

  mainloop_finish ();

  if (ble->notify_id)
    bt_gatt_client_unregister_notify (ble->gatt, ble->notify_id);

  if (ble->gatt)
    bt_gatt_client_unref (ble->gatt);

  if (ble->db)
    gatt_db_unref (ble->db);

  if (ble->att)
    bt_att_unref (ble->att);
  
  if (ble->fd > 0)
    close (ble->fd);

  free (ble);
}


static void
att_disconnect_cb (int err, void *user_data)
{
  printf ("Device disconnected: %s\n", strerror (err));

  mainloop_exit_failure ();
}

static void
print_uuid (const bt_uuid_t * uuid)
{
  char uuid_str[MAX_LEN_UUID_STR];
  bt_uuid_t uuid128;

  bt_uuid_to_uuid128 (uuid, &uuid128);
  bt_uuid_to_string (&uuid128, uuid_str, sizeof (uuid_str));

  printf ("%s\n", uuid_str);
}



static void
scan_desc (struct gatt_db_attribute *attr, void *user_data)
{
  BLE *ble = user_data;
  uint16_t handle = gatt_db_attribute_get_handle (attr);
  const bt_uuid_t *uuid = gatt_db_attribute_get_type (attr);

  printf ("\t\tdesc - handle: 0x%04x, uuid: ", handle);
  print_uuid (uuid);

  if (!bt_uuid_cmp (uuid, &cccd_uuid))
    ble->cccd_handle = handle;

}



static void
scan_chrc (struct gatt_db_attribute *attr, void *user_data)
{
  BLE *ble = user_data;
  uint16_t handle, value_handle;
  uint8_t properties;
  bt_uuid_t uuid;

  if (!gatt_db_attribute_get_char_data (attr, &handle,
                                        &value_handle, &properties, &uuid))
    return;

  printf ("\tchar - start: 0x%04x, value: 0x%04x, props: 0x%02x, uuid: ",
          handle, value_handle, properties);


  print_uuid (&uuid);

  if (!bt_uuid_cmp (&uuid, &data_uuid))
    ble->data_handle = value_handle;

  if (!bt_uuid_cmp (&uuid, &cp_uuid))
    {
      ble->cp_handle = value_handle;
      gatt_db_service_foreach_desc (attr, scan_desc, ble);
    }

}




static void
scan_service (struct gatt_db_attribute *attr, void *user_data)
{
  BLE *ble = user_data;
  uint16_t start, end;
  bool primary;
  bt_uuid_t uuid;

  printf("Service scan of the primary Nordic Semiconductor Service\n");
  
  if (!gatt_db_attribute_get_service_data (attr, &start, &end, &primary,
                                           &uuid))
    return;

  gatt_db_service_foreach_char (attr, scan_chrc, ble);

  printf ("\n");
}


static void
ready_cb (bool success, uint8_t att_ecode, void *user_data)
{
  BLE *ble = user_data;

  if (!success)
    {
      fprintf (stderr,
               "GATT discovery procedures failed - error code: 0x%02x\n",
               att_ecode);
      mainloop_exit_failure ();
      return;
    }

  printf ("GATT discovery procedures complete\n");


  gatt_db_foreach_service (ble->db, &fota_uuid, scan_service, ble);


  printf ("Handles:\n\tdata: 0x%04x\n\tcp  : 0x%04x\n\tcccd: 0x%04x\n",
          ble->data_handle, ble->cp_handle, ble->cccd_handle);

  if (ble->cccd_handle && ble->cp_handle && ble->data_handle)
    {
      mainloop_exit_success ();
      return;
    }

  mainloop_exit_failure ();
}


void
ble_init (void)
{
  bt_string_to_uuid (&fota_uuid, "0000fe59-0000-1000-8000-00805f9b34fb");
  bt_string_to_uuid (&cp_uuid,   "8EC90001-F315-4F60-9FB8-838830DAEA50");
  bt_string_to_uuid (&data_uuid, "8EC90002-F315-4F60-9FB8-838830DAEA50");
  bt_string_to_uuid (&cccd_uuid, "00002902-0000-1000-8000-00805f9b34fb");

  mainloop_init ();
}

BLE *
ble_open (const char *bdaddr)
{
  BLE *ble;

  int mainloopReturnCode;

  ble = xmalloc (sizeof (*ble));
  memset (ble, 0, sizeof (*ble));

  ble->notify_waiting_for_op = -1;
  ble->notify_code = -1;

  ble->sec = BT_SECURITY_LOW;
  ble->dst_type = BDADDR_LE_RANDOM;
  bacpy (&ble->src_addr, BDADDR_ANY);

  if (str2ba (bdaddr, &ble->dst_addr) < 0)
    {
      fprintf (stderr, "Invalid remote address: %s\n", bdaddr);
      ble_close (ble);
      return NULL;
    }


  ble->fd =
    l2cap_le_att_connect (&ble->src_addr, &ble->dst_addr, ble->dst_type,
                          ble->sec);
  if (ble->fd < 0)
    {
      perror ("l2cap_le_att_connect");
      ble_close (ble);
      return NULL;
    }


  ble->att = bt_att_new (ble->fd);

  if (!ble->att)
    {
      fprintf (stderr, "Failed to initialze ATT transport layer\n");
      ble_close (ble);
      return NULL;
    }

  if (!bt_att_set_close_on_unref (ble->att, true))
    {
      fprintf (stderr, "Failed to set up ATT transport layer\n");
      ble_close (ble);
      return NULL;
    }

  if (!bt_att_register_disconnect (ble->att, att_disconnect_cb, NULL, NULL))
    {
      fprintf (stderr, "Failed to set ATT disconnect handler\n");
      ble_close (ble);
      return NULL;
    }


  ble->db = gatt_db_new ();
  ble->gatt = bt_gatt_client_new (ble->db, ble->att, ble->mtu);


  gatt_db_register (ble->db, service_added_cb, service_removed_cb,
                    NULL, NULL);

  if (verbose)
    {
      bt_att_set_debug (ble->att, att_debug_cb, "att: ", NULL);
      bt_gatt_client_set_debug (ble->gatt, gatt_debug_cb, "gatt: ", NULL);
    }

  bt_gatt_client_set_ready_handler (ble->gatt, ready_cb, ble, NULL);


  mainloopReturnCode = mainloop_run();
  if (mainloopReturnCode == EXIT_SUCCESS)
    return ble;
  printf("mainloop_run call failed with code %d !\n", mainloopReturnCode);

  ble_close (ble);
  return NULL;
}

static void
notify_cb (uint16_t value_handle, const uint8_t * value,
           uint16_t length, void *user_data)
{
  int i;
  BLE *ble = user_data;

  if (ble->debug){
    printf ("Handle Value Not/Ind: 0x%04x - ", value_handle);
    
    if (length == 0) {
      printf ("(0 bytes)\n");
      return;
    }
    
    printf ("(%u bytes): ", length);
    
    for (i = 0; i < length; i++){
      printf ("%02x ", value[i]);
    }
    printf ("\n");
  }



  if ((value_handle == ble->cp_handle) && (length >= 3)
      && (value[0] == OP_CODE_RESPONSE_CODE)
      && (value[1] == ble->notify_waiting_for_op)) {
    memcpy(ble->last_notification_package, value, length);
    ble->last_notification_package_size = length;
    ble->notify_code = value[2];
    mainloop_quit();
  }
}

static void
register_notify_cb (uint16_t att_ecode, void *user_data)
{
  if (att_ecode)
    {
      printf ("Failed to register notify handler "
              "- error code: 0x%02x\n", att_ecode);
      mainloop_exit_failure ();
      return;
    }
  
  //printf ("Registered notify handler!\n");
  mainloop_exit_success ();
}


int
ble_register_notify (BLE * ble)
{



  if (!bt_gatt_client_is_ready (ble->gatt))
    {
      printf ("GATT client not initialized\n");
      return EXIT_FAILURE;
    }

  ble->notify_id = bt_gatt_client_register_notify (ble->gatt, ble->cp_handle,
                                                   register_notify_cb,
                                                   notify_cb, ble, NULL);
  if (!ble->notify_id)
    {
      printf ("Failed to register notify handler\n");
      return EXIT_FAILURE;
    }

  if (ble->debug){
    printf ("requesting notify\n");
  }

  return mainloop_run ();
}

static void
write_cb (bool success, uint8_t att_ecode, void *user_data)
{
//BLE *ble=user_data;

  if (success)
    {
      mainloop_exit_success ();
    }
  else
    {
      printf ("Write failed: %s (0x%02x)\n",
              ecode_to_string (att_ecode), att_ecode);
      mainloop_exit_failure ();
    }
}


int
ble_send_cp (BLE * ble, uint8_t * buf, size_t len)
{
  if (ble->debug){
    printf ("Sending control:\n");
    hexdump (buf, len);
  }

  if (!bt_gatt_client_write_value (ble->gatt, ble->cp_handle, buf, len,
                                   write_cb, ble, NULL))
    {
      printf ("Failed to initiate write procedure\n");
      return EXIT_FAILURE;
    }

  return mainloop_run ();
}


int
ble_send_cp_noresp (BLE * ble, uint8_t * buf, size_t len)
{
  if (ble->debug){
    printf ("Sending control (but ignoring error):\n");
    hexdump (buf, len);
  }

#if 0
  if (!bt_gatt_client_write_without_response
      (ble->gatt, ble->cp_handle, false, buf, len))
    {
      printf ("Failed to initiate write procedure\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
#else
  if (!bt_gatt_client_write_value (ble->gatt, ble->cp_handle, buf, len,
                                   write_cb, ble, NULL))
    {
      printf ("Failed to initiate write procedure\n");
      return EXIT_FAILURE;
    }

  mainloop_run ();
  return EXIT_SUCCESS;
#endif
}


#if 0
int
ble_send_data (BLE * ble, uint8_t * buf, size_t len)
{
  size_t mtu = 16, tx;

  printf ("Sending data:\n");


  hexdump (buf, (len < 64) ? len : 64);


  while (len)
    {

      printf ("%u bytes left\n", (unsigned) len);

      tx = (len > mtu) ? mtu : len;

      if (!bt_gatt_client_write_value (ble->gatt, ble->data_handle, buf, tx,
                                       write_cb, ble, NULL))
        {
          printf ("Failed to initiate write procedure\n");
          return EXIT_FAILURE;
        }

      if (mainloop_run () == EXIT_FAILURE)
        return EXIT_FAILURE;

      len -= tx;
      buf += tx;
    }

  return EXIT_SUCCESS;
}
#else
int
ble_send_data (BLE * ble, uint8_t * buf, size_t len)
{
  if (!bt_gatt_client_write_value (ble->gatt, ble->data_handle, buf, len,
                                   write_cb, ble, NULL))
    {
      printf ("Failed to initiate write procedure\n");
      return EXIT_FAILURE;
    }

  return mainloop_run ();
}
#endif


int
ble_send_data_noresp (BLE * ble, uint8_t * buf, size_t len)
{
  if (!bt_gatt_client_write_without_response
      (ble->gatt, ble->data_handle, false, buf, len))
    {
      printf ("Failed to initiate write procedure\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}


void
ble_wait_setup (BLE * ble, uint8_t op)
{
  ble->notify_waiting_for_op = op;
  ble->notify_code = -1;
}


int
ble_wait_run (BLE * ble)
{
  mainloop_run ();

  ble->notify_waiting_for_op = -1;

  if (ble->debug){
    printf ("Returning response 0x%02x\n", ble->notify_code);
  }

  return ble->notify_code;
}

void
ble_notify_pkts_start (BLE * ble)
{
  ble->notify_waiting_for_pkts = 1;
}


void
ble_notify_pkts_stop (BLE * ble)
{
  ble->notify_waiting_for_pkts = 0;
}

size_t
ble_notify_get_pkts (BLE * ble)
{
  size_t ret = 0;

  mainloop_run ();

  if (!ble->notify_waiting_for_pkts)
    ret = ble->notify_pkts;
  ble->notify_waiting_for_pkts = 1;

  return ret;
}
