
typedef enum {
  BLE_DFU_RESP_VAL_INVALID_CODE                  = 0x00,
  BLE_DFU_RESP_VAL_SUCCESS                       = 0x01,
  BLE_DFU_RESP_VAL_OPCODE_NOT_SUPPORTED          = 0x02,
  BLE_DFU_RESP_VAL_INVALID_PARAMETER             = 0x03,
  BLE_DFU_RESP_VAL_INSUFFICIENT_RESOURCES        = 0x04,
  BLE_DFU_RESP_VAL_INVALID_OBJECT                = 0x05,
  BLE_DFU_RESP_VAL_UNSUPPORTED_TYPE              = 0x07,
  BLE_DFU_RESP_VAL_OPPERATION_NOT_PERMITTED      = 0x08,
  BLE_DFU_RESP_VAL_OPPERATION_FAILED             = 0x0A
} ble_dfu_resp_val_t;

enum {
  OP_CODE_CREATE                                 = 0x01,
  OP_CODE_SET_PACKET_RECEPTION_NOTIFICATIN_VALUE = 0x02,
  OP_CODE_CALCULATE_CHECKSUM                     = 0x03,
  OP_CODE_EXECUTE                                = 0x04,
  OP_CODE_SELECT                                 = 0x06,
  OP_CODE_RESPONSE_CODE                          = 0x60
};

typedef enum {
  BLE_OBJ_TYPE_COMMAND = 0x01,
  BLE_OBJ_TYPE_DATA    = 0x02
} BleObjType;

int dfu(const char *bdaddr, uint8_t *dat, size_t dat_sz, uint8_t *bin, size_t bin_sz);
