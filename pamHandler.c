#include <security/_pam_types.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

#define false 0
#define true 1

struct pam_response *reply;

//function used to get user input
int function_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    *resp = reply;
    return PAM_SUCCESS;
}

int checkPassword(const char* username, char* password, const char* appName)
{
    pam_handle_t *pamh = NULL;
    const struct pam_conv local_conversation = { function_conversation, NULL };
    if (pam_start(appName, username, &local_conversation, &pamh) == PAM_SUCCESS)
    {
        reply = (struct pam_response *)malloc(sizeof(struct pam_response));
        reply[0].resp = strdup(password);
        reply[0].resp_retcode = 0;
        return pam_authenticate(pamh, 0) == PAM_SUCCESS ? true : false;
    }
    return false;
}