#ifndef _NIKSS_HELPER_H_
#define _NIKSS_HELPER_H_

#include <linux/if_ether.h>
#include <nikss/nikss.h>

enum key_type {
    IP4_ADDR,
    IP4_LPM,
    U_32,
};

struct match_elem {
    enum key_type type;
    uint32_t val;
    uint8_t prefix_len;
};

enum data_type {
    IP4_DATA,
    U_32_DATA,
    MAC_DATA,
};

struct data_elem {
    enum data_type type;
    uint32_t val;
    uint8_t mac[ETH_ALEN];
};

struct entry_spec {
    nikss_pipeline_id_t pipe;
    const char *table_name;
    const char *action_name;
    struct match_elem *elems;
    uint8_t num_elems;
    struct data_elem *data;
    uint8_t num_data;
};

int table_entry_add(struct entry_spec *entry);
int table_entry_del(struct entry_spec *entry);

#endif /* _NIKSS_HELPER_H_ */