/* nrfdfu.c */
extern int main(int argc, char *argv[]);
/* util.c */
extern void *xmalloc(size_t s);
extern void *xrealloc(void *p, size_t s);
/* zip.c */
extern void fatal_zip(struct zip *zip);
extern struct zip *open_zip(const char *fn);
extern size_t read_file_from_zip(struct zip *zip, const char *fn, void *_buf);
/* ble.c */
extern void ble_close(BLE *ble);
extern void ble_init(void);
extern BLE *ble_open(const char *bdaddr);
extern int ble_register_notify(BLE *ble);
extern int ble_send_cp(BLE *ble, uint8_t *buf, size_t len);
extern int ble_send_cp_noresp(BLE *ble, uint8_t *buf, size_t len);
extern int ble_send_data(BLE *ble, uint8_t *buf, size_t len);
extern int ble_send_data_noresp(BLE *ble, uint8_t *buf, size_t len);
extern void ble_wait_setup(BLE *ble, uint8_t op);
extern int ble_wait_run(BLE *ble);
extern void ble_notify_pkts_start(BLE *ble);
extern void ble_notify_pkts_stop(BLE *ble);
extern size_t ble_notify_get_pkts(BLE *ble);
/* manifest.c */
extern json_object *_json_object_object_get(json_object *obj, const char *name);
extern struct manifest *parse_manifest(const char *str);
/* hexdump.c */
extern void hexdump(void *_d, int len);
