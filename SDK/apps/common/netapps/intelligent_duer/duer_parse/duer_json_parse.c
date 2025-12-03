#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_parse.data.bss")
#pragma data_seg(".intelligent_duer_parse.data")
#pragma const_seg(".intelligent_duer_parse.text.const")
#pragma code_seg(".intelligent_duer_parse.text")
#endif
#include "duer_common.h"

#if INTELLIGENT_DUER
#define LOG_TAG_CONST       NET_DUER
#define LOG_TAG             "[DUER_JSON_PARSE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


TokenData *duer_parse_token_json(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        log_error("JSON parse error:\n");
        return NULL;
    }
    TokenData *data = (TokenData *)net_interface_malloc(sizeof(TokenData));
    if (!data) {
        log_error("Memory allocation failed");
        cJSON_Delete(root);
        return NULL;
    }
    memset(data, 0, sizeof(TokenData));
    cJSON *item;
    if ((item = cJSON_GetObjectItem(root, "refresh_token")) && cJSON_IsString(item)) {
        data->refresh_token = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "expires_in")) && cJSON_IsNumber(item)) {
        data->expires_in = item->valueint;
    }
    if ((item = cJSON_GetObjectItem(root, "session_key")) && cJSON_IsString(item)) {
        data->session_key = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "access_token")) && cJSON_IsString(item)) {
        data->access_token = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "scope")) && cJSON_IsString(item)) {
        data->scope = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "session_secret")) && cJSON_IsString(item)) {
        data->session_secret = strdup(item->valuestring);
    }
    cJSON_Delete(root);
    return data;
}


void duer_free_token_data(TokenData *data)
{
    if (data) {
        net_interface_free(data->refresh_token);
        net_interface_free(data->session_key);
        net_interface_free(data->access_token);
        net_interface_free(data->scope);
        net_interface_free(data->session_secret);
        net_interface_free(data);
    }
}


InsideRCResponse *duer_parse_inside_rc_json(const char *json_string)
{
    InsideRCResponse *response = net_interface_calloc(1, sizeof(InsideRCResponse));
    if (!response) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        log_error("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        net_interface_free(response);
        return NULL;
    }

    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (cJSON_IsString(type)) {
        response->type = strdup(type->valuestring);
    }

    if (!response->type || strcmp(response->type, "inside_rc") != 0) {
        cJSON_Delete(root);
        return response;
    }

    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (cJSON_IsString(status)) {
        response->status = strdup(status->valuestring);
    }

    cJSON *sn = cJSON_GetObjectItem(root, "sn");
    if (cJSON_IsString(sn)) {
        response->sn = strdup(sn->valuestring);
    }

    cJSON *end = cJSON_GetObjectItem(root, "end");
    if (cJSON_IsNumber(end)) {
        response->end = end->valueint;
    }

    cJSON *data_obj = cJSON_GetObjectItem(root, "data");
    if (data_obj && cJSON_IsObject(data_obj)) {
        response->data = net_interface_calloc(1, sizeof(Data));
        if (!response->data) {
            cJSON_Delete(root);
            duer_free_inside_rc_response(response);
            return NULL;
        }

        cJSON *code = cJSON_GetObjectItem(data_obj, "code");
        if (cJSON_IsNumber(code)) {
            response->data->code = code->valueint;
        }

        cJSON *msg = cJSON_GetObjectItem(data_obj, "msg");
        if (cJSON_IsString(msg)) {
            response->data->msg = strdup(msg->valuestring);
        }

        cJSON *logid = cJSON_GetObjectItem(data_obj, "logid");
        if (cJSON_IsString(logid)) {
            response->data->logid = strdup(logid->valuestring);
        }

        cJSON *qid = cJSON_GetObjectItem(data_obj, "qid");
        if (cJSON_IsString(qid)) {
            response->data->qid = strdup(qid->valuestring);
        }

        cJSON *is_end = cJSON_GetObjectItem(data_obj, "is_end");
        if (cJSON_IsNumber(is_end)) {
            response->data->is_end = is_end->valueint;
        }

        cJSON *need_clear_history = cJSON_GetObjectItem(data_obj, "need_clear_history");
        if (cJSON_IsNumber(need_clear_history)) {
            response->data->need_clear_history = need_clear_history->valueint;
        }

        cJSON *assistant_answer = cJSON_GetObjectItem(data_obj, "assistant_answer");
        if (assistant_answer) {
            response->data->assistant_answer = net_interface_calloc(1, sizeof(AssistantAnswer));
            if (response->data->assistant_answer) {
                if (cJSON_IsString(assistant_answer)) {
                    cJSON *aa_root = cJSON_Parse(assistant_answer->valuestring);
                    if (aa_root) {
                        cJSON *content = cJSON_GetObjectItem(aa_root, "content");
                        if (cJSON_IsString(content)) {
                            response->data->assistant_answer->content = strdup(content->valuestring);
                        }

                        cJSON *nlu = cJSON_GetObjectItem(aa_root, "nlu");
                        if (cJSON_IsString(nlu)) {
                            response->data->assistant_answer->nlu = strdup(nlu->valuestring);
                        }

                        cJSON *aa_is_end = cJSON_GetObjectItem(aa_root, "is_end");
                        if (cJSON_IsNumber(aa_is_end)) {
                            response->data->assistant_answer->is_end = aa_is_end->valueint;
                        }

                        cJSON *metadata = cJSON_GetObjectItem(aa_root, "metadata");
                        if (metadata && cJSON_IsObject(metadata)) {
                            response->data->assistant_answer->metadata = net_interface_calloc(1, sizeof(Metadata));
                            if (response->data->assistant_answer->metadata) {
                                cJSON *multi_round_info = cJSON_GetObjectItem(metadata, "multi_round_info");
                                if (multi_round_info && cJSON_IsObject(multi_round_info)) {
                                    response->data->assistant_answer->metadata->multi_round_info = net_interface_calloc(1, sizeof(MultiRoundInfo));
                                    if (response->data->assistant_answer->metadata->multi_round_info) {
                                        cJSON *is_in_multi = cJSON_GetObjectItem(multi_round_info, "is_in_multi");
                                        if (cJSON_IsBool(is_in_multi)) {
                                            response->data->assistant_answer->metadata->multi_round_info->is_in_multi = is_in_multi->valueint;
                                        }

                                        cJSON *target_bot_id = cJSON_GetObjectItem(multi_round_info, "target_bot_id");
                                        if (cJSON_IsString(target_bot_id)) {
                                            response->data->assistant_answer->metadata->multi_round_info->target_bot_id = strdup(target_bot_id->valuestring);
                                        }

                                        cJSON *intent = cJSON_GetObjectItem(multi_round_info, "intent");
                                        if (cJSON_IsString(intent)) {
                                            response->data->assistant_answer->metadata->multi_round_info->intent = strdup(intent->valuestring);
                                        }
                                    }
                                }
                            }
                        }

                        cJSON_Delete(aa_root);
                    } else {
                        response->data->assistant_answer->content = strdup(assistant_answer->valuestring);
                    }
                } else if (cJSON_IsObject(assistant_answer)) {
                    cJSON *content = cJSON_GetObjectItem(assistant_answer, "content");
                    if (cJSON_IsString(content)) {
                        response->data->assistant_answer->content = strdup(content->valuestring);
                    }

                    cJSON *nlu = cJSON_GetObjectItem(assistant_answer, "nlu");
                    if (cJSON_IsString(nlu)) {
                        response->data->assistant_answer->nlu = strdup(nlu->valuestring);
                    }

                    cJSON *aa_is_end = cJSON_GetObjectItem(assistant_answer, "is_end");
                    if (cJSON_IsNumber(aa_is_end)) {
                        response->data->assistant_answer->is_end = aa_is_end->valueint;
                    }

                    cJSON *metadata = cJSON_GetObjectItem(assistant_answer, "metadata");
                    if (metadata && cJSON_IsObject(metadata)) {
                        response->data->assistant_answer->metadata = net_interface_calloc(1, sizeof(Metadata));
                        if (response->data->assistant_answer->metadata) {
                            cJSON *multi_round_info = cJSON_GetObjectItem(metadata, "multi_round_info");
                            if (multi_round_info && cJSON_IsObject(multi_round_info)) {
                                response->data->assistant_answer->metadata->multi_round_info = net_interface_calloc(1, sizeof(MultiRoundInfo));
                                if (response->data->assistant_answer->metadata->multi_round_info) {
                                    cJSON *is_in_multi = cJSON_GetObjectItem(multi_round_info, "is_in_multi");
                                    if (cJSON_IsBool(is_in_multi)) {
                                        response->data->assistant_answer->metadata->multi_round_info->is_in_multi = is_in_multi->valueint;
                                    }

                                    cJSON *target_bot_id = cJSON_GetObjectItem(multi_round_info, "target_bot_id");
                                    if (cJSON_IsString(target_bot_id)) {
                                        response->data->assistant_answer->metadata->multi_round_info->target_bot_id = strdup(target_bot_id->valuestring);
                                    }

                                    cJSON *intent = cJSON_GetObjectItem(multi_round_info, "intent");
                                    if (cJSON_IsString(intent)) {
                                        response->data->assistant_answer->metadata->multi_round_info->intent = strdup(intent->valuestring);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        cJSON *data_array = cJSON_GetObjectItem(data_obj, "data");
        if (data_array && cJSON_IsArray(data_array)) {
            int array_size = cJSON_GetArraySize(data_array);
            response->data->data_items_count = array_size;

            if (array_size > 0) {
                response->data->data_items = net_interface_calloc(array_size, sizeof(DataItem *));
                if (!response->data->data_items) {
                    cJSON_Delete(root);
                    duer_free_inside_rc_response(response);
                    return NULL;
                }

                for (int i = 0; i < array_size; i++) {
                    DataItem *item = net_interface_calloc(1, sizeof(DataItem));
                    if (!item) {
                        continue;
                    }
                    response->data->data_items[i] = item;

                    cJSON *array_item = cJSON_GetArrayItem(data_array, i);

                    cJSON *header = cJSON_GetObjectItem(array_item, "header");
                    if (header && cJSON_IsObject(header)) {
                        item->header = net_interface_calloc(1, sizeof(Header));
                        if (item->header) {
                            cJSON *namespace = cJSON_GetObjectItem(header, "namespace");
                            if (cJSON_IsString(namespace)) {
                                item->header->namespace = strdup(namespace->valuestring);
                            }

                            cJSON *name = cJSON_GetObjectItem(header, "name");
                            if (cJSON_IsString(name)) {
                                item->header->name = strdup(name->valuestring);
                            }

                            cJSON *messageId = cJSON_GetObjectItem(header, "messageId");
                            if (cJSON_IsString(messageId)) {
                                item->header->messageId = strdup(messageId->valuestring);
                            }

                            cJSON *dialogRequestId = cJSON_GetObjectItem(header, "dialogRequestId");
                            if (cJSON_IsString(dialogRequestId)) {
                                item->header->dialogRequestId = strdup(dialogRequestId->valuestring);
                            }
                        }
                    }

                    cJSON *payload = cJSON_GetObjectItem(array_item, "payload");
                    if (payload && cJSON_IsObject(payload)) {
                        item->payload = net_interface_calloc(1, sizeof(Payload));
                        if (item->payload) {
                            cJSON *rolling = cJSON_GetObjectItem(payload, "rolling");
                            if (cJSON_IsBool(rolling)) {
                                item->payload->rolling = rolling->valueint;
                            }

                            cJSON *text = cJSON_GetObjectItem(payload, "text");
                            if (cJSON_IsString(text)) {
                                item->payload->text = strdup(text->valuestring);
                            }

                            cJSON *content = cJSON_GetObjectItem(payload, "content");
                            if (cJSON_IsString(content)) {
                                item->payload->content = strdup(content->valuestring);
                            }

                            cJSON *format = cJSON_GetObjectItem(payload, "format");
                            if (cJSON_IsString(format)) {
                                item->payload->format = strdup(format->valuestring);
                            }

                            cJSON *token = cJSON_GetObjectItem(payload, "token");
                            if (cJSON_IsString(token)) {
                                item->payload->token = strdup(token->valuestring);
                            }

                            cJSON *url = cJSON_GetObjectItem(payload, "url");
                            if (cJSON_IsString(url)) {
                                item->payload->url = strdup(url->valuestring);
                                net_url_set(item->payload->url);
                                net_print_urls();
                            }

                            cJSON *type = cJSON_GetObjectItem(payload, "type");
                            if (cJSON_IsString(type)) {
                                item->payload->type = strdup(type->valuestring);
                            }

                            cJSON *answer = cJSON_GetObjectItem(payload, "answer");
                            if (cJSON_IsString(answer)) {
                                item->payload->answer = strdup(answer->valuestring);
                            }

                            cJSON *id = cJSON_GetObjectItem(payload, "id");
                            if (cJSON_IsString(id)) {
                                item->payload->id = strdup(id->valuestring);
                            }

                            cJSON *index = cJSON_GetObjectItem(payload, "index");
                            if (cJSON_IsNumber(index)) {
                                item->payload->index = index->valueint;
                            }

                            cJSON *payload_is_end = cJSON_GetObjectItem(payload, "is_end");
                            if (cJSON_IsNumber(payload_is_end)) {
                                item->payload->payload_is_end = payload_is_end->valueint;
                            }

                            cJSON *part = cJSON_GetObjectItem(payload, "part");
                            if (cJSON_IsString(part)) {
                                item->payload->part = strdup(part->valuestring);
                            }

                            cJSON *reasoning_part = cJSON_GetObjectItem(payload, "reasoning_part");
                            if (cJSON_IsString(reasoning_part)) {
                                item->payload->reasoning_part = strdup(reasoning_part->valuestring);
                            }

                            cJSON *tts = cJSON_GetObjectItem(payload, "tts");
                            if (cJSON_IsString(tts)) {
                                item->payload->tts = strdup(tts->valuestring);
                            }

                            cJSON *timeoutInMilliseconds = cJSON_GetObjectItem(payload, "timeoutInMilliseconds");
                            if (cJSON_IsNumber(timeoutInMilliseconds)) {
                                item->payload->timeoutInMilliseconds = timeoutInMilliseconds->valueint;
                            }
                        }
                    }

                    cJSON *property = cJSON_GetObjectItem(array_item, "property");
                    if (property && cJSON_IsObject(property)) {
                        cJSON *serviceCategory = cJSON_GetObjectItem(property, "serviceCategory");
                        if (cJSON_IsString(serviceCategory)) {
                            item->serviceCategory = strdup(serviceCategory->valuestring);
                        }
                    }
                }
            }
        }

        cJSON *lj_thread_id = cJSON_GetObjectItem(data_obj, "lj_thread_id");
        if (cJSON_IsString(lj_thread_id)) {
            response->data->lj_thread_id = strdup(lj_thread_id->valuestring);
        }

        cJSON *ab_conversation_id = cJSON_GetObjectItem(data_obj, "ab_conversation_id");
        if (cJSON_IsString(ab_conversation_id)) {
            response->data->ab_conversation_id = strdup(ab_conversation_id->valuestring);
        }

        cJSON *xiaoice_session_id = cJSON_GetObjectItem(data_obj, "xiaoice_session_id");
        if (cJSON_IsString(xiaoice_session_id)) {
            response->data->xiaoice_session_id = strdup(xiaoice_session_id->valuestring);
        }

        cJSON *dialog_request_id = cJSON_GetObjectItem(data_obj, "dialog_request_id");
        if (cJSON_IsString(dialog_request_id)) {
            response->data->dialog_request_id = strdup(dialog_request_id->valuestring);
        }
    }
    cJSON_Delete(root);
    return response;
}

void duer_free_inside_rc_response(InsideRCResponse *response)
{
    if (!response) {
        return;
    }

    net_interface_free(response->status);
    net_interface_free(response->type);
    net_interface_free(response->sn);

    if (response->data) {
        net_interface_free(response->data->msg);
        net_interface_free(response->data->logid);
        net_interface_free(response->data->qid);
        net_interface_free(response->data->lj_thread_id);
        net_interface_free(response->data->ab_conversation_id);
        net_interface_free(response->data->xiaoice_session_id);
        net_interface_free(response->data->dialog_request_id);

        if (response->data->assistant_answer) {
            net_interface_free(response->data->assistant_answer->content);
            net_interface_free(response->data->assistant_answer->nlu);

            if (response->data->assistant_answer->metadata) {
                if (response->data->assistant_answer->metadata->multi_round_info) {
                    net_interface_free(response->data->assistant_answer->metadata->multi_round_info->target_bot_id);
                    net_interface_free(response->data->assistant_answer->metadata->multi_round_info->intent);
                    net_interface_free(response->data->assistant_answer->metadata->multi_round_info);
                }
                net_interface_free(response->data->assistant_answer->metadata);
            }
            net_interface_free(response->data->assistant_answer);
        }

        if (response->data->data_items) {
            for (int i = 0; i < response->data->data_items_count; i++) {
                DataItem *item = response->data->data_items[i];
                if (!item) {
                    continue;
                }

                if (item->header) {
                    net_interface_free(item->header->namespace);
                    net_interface_free(item->header->name);
                    net_interface_free(item->header->messageId);
                    net_interface_free(item->header->dialogRequestId);
                    net_interface_free(item->header);
                }

                if (item->payload) {
                    net_interface_free(item->payload->text);
                    net_interface_free(item->payload->content);
                    net_interface_free(item->payload->format);
                    net_interface_free(item->payload->token);
                    net_interface_free(item->payload->url);
                    net_interface_free(item->payload->type);
                    net_interface_free(item->payload->answer);
                    net_interface_free(item->payload->id);
                    net_interface_free(item->payload->part);
                    net_interface_free(item->payload->reasoning_part);
                    net_interface_free(item->payload->tts);
                    net_interface_free(item->payload);
                }

                net_interface_free(item->serviceCategory);
                net_interface_free(item);
            }
            net_interface_free(response->data->data_items);
        }

        net_interface_free(response->data);
    }

    net_interface_free(response);
}
#endif
