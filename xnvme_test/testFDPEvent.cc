#include <libxnvme.h>
#include <cstdio>
#include <cstdint>
#include <cerrno>

#define DEVICE "/dev/nvme1n1"
#define FDP_LOG_ID 0x23
#define FDP_LOG_SIZE 4096

#define SET_EVENT_TYPES ((uint8_t[]){0x0, 0x1, 0x2, 0x3, 0x80, 0x81})
#define SET_EVENT_BUF_SIZE sizeof(SET_EVENT_TYPES)

void print_fdp_event(const struct xnvme_spec_fdp_event *event) {
    printf("FDP Event:\n");
    printf("\tEvent Type: %u\n", event->type);
    printf("\tPID: %d\n", event->pid);
    printf("\tNSID: %d\n", event->nsid);
    printf("\tTimestamp: %ld\n", event->timestamp);
    printf("\tReclaim Group: %d\n", event->rgid);
    printf("\tReclaim Unit Handle: %d\n", event->ruhid);
    printf("\tType Specific: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", event->type_specific[i]);
    }
    printf("\n");
    printf("\tVendor Specific: ");
    for (int i = 0; i < 24; i++) {
        printf("%02x ", event->vs[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    uint32_t limit = 16; // Number of log entries to retrieve

    struct xnvme_opts opts = xnvme_opts_default();
    struct xnvme_dev *dev = NULL;
	struct xnvme_cmd_ctx ctx;
    void *log_buf = NULL;

    uint32_t nsid = 0;

    struct xnvme_spec_log_fdp_events *log = NULL;
    uint32_t log_nbytes = 0;
    int err;

    dev = xnvme_dev_open(DEVICE, &opts);
    if (!dev) {
        err = -errno;
        perror("Failed to open device");
        return err;
    }
	
    ctx = xnvme_cmd_ctx_from_dev(dev);
    nsid = xnvme_dev_get_nsid(dev);

    // log_nbytes = sizeof(*log) + limit * sizeof(struct xnvme_spec_fdp_event);
	log_nbytes = FDP_LOG_SIZE;
    xnvme_cli_pinf("Allocating and clearing buffer...");
    log = (struct xnvme_spec_log_fdp_events *) xnvme_buf_alloc(dev, log_nbytes);
    if (!log) {
        err = -errno;
        perror("xnvme_buf_alloc()");
        xnvme_buf_free(dev, log);
        return err;
    }
    memset(log, 0, log_nbytes);

    xnvme_cli_pinf("Retrieving fdp-events-log ...");
    xnvme_prep_adm_log(&ctx,
        XNVME_SPEC_LOG_FDPEVENTS,
        0x0,
        0,
        nsid,
        0,
        log_nbytes);
    ctx.cmd.log.lsi = 1;
	ctx.cmd.log.lsp = 0;

    err = xnvme_cmd_pass_admin(&ctx, log, log_nbytes, NULL, 0x0);
	if (err || xnvme_cmd_ctx_cpl_status(&ctx)) {
		xnvme_cli_perr("xnvme_adm_log(XNVME_SPEC_LOG_FDPEVENTS)", err);
		xnvme_cmd_ctx_pr(&ctx, XNVME_PR_DEF);
		err = err ? err : -EIO;
		xnvme_buf_free(dev, log);
	    return err;
	}

    printf("# %u fdp events log page entries:\n", limit);
	xnvme_spec_log_fdp_events_pr(log, limit, XNVME_PR_DEF);

    xnvme_buf_free(dev, log);
	return err;
}