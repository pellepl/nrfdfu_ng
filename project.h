#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include <zip.h>
#include <json.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
//#include <bluetooth/uuid.h>


#include "bluez/uuid.h"
#include "bluez/att.h"
#include "bluez/queue.h"
#include "bluez/gatt-db.h"

#include "manifest.h"
#include "ble.h"

#include "prototypes.h"
