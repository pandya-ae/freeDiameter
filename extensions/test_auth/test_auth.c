#include <freeDiameter/extension.h>

static int test_cb(struct msg **pmsg, struct avp *avp, struct session **sess, void *opaque) {
    struct msg *answer = NULL;
    struct avp *avp_out = NULL;

    // Create answer message from request
    CHECK_FCT(fd_msg_new_answer_from_req(pmsg, 0));
    answer = *pmsg;

    // Add Result-Code AVP (2001 = DIAMETER_SUCCESS)
    CHECK_FCT(fd_msg_avp_new(fd_dict_avp_RESULT_CODE, 0, &avp_out));
    uint32_t result_code = 2001;
    CHECK_FCT(fd_msg_avp_setvalue(avp_out, &result_code));
    CHECK_FCT(fd_msg_avp_add(answer, MSG_BRW_LAST_CHILD, avp_out));

    return 0;
}

static int test_entry(char * conffile) {
    struct dict_object *cmd = NULL;

    // Find AA-Request command in dictionary
    CHECK_FCT(fd_dict_search(fd_g_config->cnf_dict, DICT_COMMAND, CMD_BY_NAME, "AA", &cmd, ENOENT));

    // Register callback for AA-Request
    CHECK_FCT(fd_disp_register(cmd, test_cb, NULL, 0, NULL));

    return 0;
}

EXTENSION_ENTRY("test_auth", test_entry);
