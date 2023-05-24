#include <stdio.h>
#include <errno.h>
#include <nikss/nikss.h>
#include <nikss/nikss_pipeline.h>

#include "nikss_helper.h"

int table_entry_add(struct entry_spec *spec)
{
    nikss_table_entry_t entry;
    nikss_table_entry_ctx_t ctx;
    nikss_action_t action;
    nikss_context_t nikss_ctx;
    int error_code = EPERM;
    uint32_t action_id;
    struct match_elem *elem;
    nikss_match_key_t mk;
    struct data_elem *data;
    nikss_action_param_t param;

    nikss_context_init(&nikss_ctx);
    nikss_table_entry_ctx_init(&ctx);
    nikss_table_entry_init(&entry);
    nikss_action_init(&action);

    nikss_context_set_pipeline(&nikss_ctx, spec->pipe);
    if (!nikss_pipeline_exists(&nikss_ctx)) {
        fprintf(stderr, "ERROR: p4 pipeline number %d does not exist\n", spec->pipe);
        error_code = ENOENT;
        goto clean_up;
    }

    if (nikss_table_entry_ctx_tblname(&nikss_ctx, &ctx, spec->table_name) != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot find table %s\n", spec->table_name);
        error_code = EINVAL;
        goto clean_up;
    }

    action_id = nikss_table_get_action_id_by_name(&ctx, spec->action_name);
    if (action_id == NIKSS_INVALID_ACTION_ID) {
        fprintf(stderr, "ERROR: cannot find action %s\n", spec->action_name);
        error_code = EINVAL;
        goto clean_up;
    }
    nikss_action_set_id(&action, action_id);

    for (uint8_t i = 0; i < spec->num_elems; ++i) {
        elem = spec->elems + i;
        nikss_matchkey_init(&mk);

        switch (elem->type) {
            case IP4_ADDR:
                nikss_matchkey_type(&mk, NIKSS_EXACT);
                if (nikss_matchkey_data(&mk, (const char *)&(elem->val), sizeof(elem->val)) != NO_ERROR) {
                    fprintf(stderr, "ERROR: cannot add key exact addr %d\n", elem->val);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
            case U_32:
                nikss_matchkey_type(&mk, NIKSS_EXACT);
                if (nikss_matchkey_data(&mk, (const char *)&(elem->val), sizeof(elem->val)) != NO_ERROR) {
                    fprintf(stderr, "ERROR: cannot add key number 32 %d\n", elem->val);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
            case IP4_LPM:
                nikss_matchkey_type(&mk, NIKSS_LPM);
                if ((nikss_matchkey_data(&mk, (const char *)&(elem->val), sizeof(elem->val)) != NO_ERROR) ||
                    (nikss_matchkey_prefix_len(&mk, elem->prefix_len) != NO_ERROR)) {
                    fprintf(stderr, "ERROR: cannot add key lpm addr %d/%d\n", elem->val, elem->prefix_len);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
        }

        error_code = nikss_table_entry_matchkey(&entry, &mk);
        nikss_matchkey_free(&mk);
        if (error_code != NO_ERROR) {
            fprintf(stderr, "ERROR: cannot add key to table entry\n");
            goto clean_up;
        }

    }
    for (uint8_t i = 0; i < spec->num_data; ++i) {
        data = spec->data + i;

        switch (data->type) {
            case IP4_DATA:
                if (nikss_action_param_create(&param, (const char *)&(data->val), sizeof(data->val)) != NO_ERROR) {
                    fprintf(stderr, "ERROR: cannot add action data ip4 addr %d\n", data->val);
                    nikss_action_param_free(&param);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
            case U_32_DATA:
                if (nikss_action_param_create(&param, (const char *)&(data->val), sizeof(data->val)) != NO_ERROR) {
                    fprintf(stderr, "ERROR: cannot add action data u32 %d\n", data->val);
                    nikss_action_param_free(&param);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
            case MAC_DATA:
                if (nikss_action_param_create(&param, (const char *)(data->mac), ETH_ALEN) != NO_ERROR) {
                    fprintf(stderr, "ERROR: cannot add action data mac\n");
                    nikss_action_param_free(&param);
                    error_code = EINVAL;
                    goto clean_up;
                }
                break;
        }

        error_code = nikss_action_param(&action, &param);
        if (error_code != NO_ERROR) {
            fprintf(stderr, "ERROR: cannot add action data to table entry\n");
            error_code = EINVAL;
            goto clean_up;
        }

    }
    nikss_table_entry_action(&entry, &action);
    error_code = nikss_table_entry_add(&ctx, &entry);
    if (error_code == EEXIST)
        error_code = nikss_table_entry_update(&ctx, &entry);

clean_up:
    nikss_action_free(&action);
    nikss_table_entry_free(&entry);
    nikss_table_entry_ctx_free(&ctx);
    nikss_context_free(&nikss_ctx);

    return error_code;
}