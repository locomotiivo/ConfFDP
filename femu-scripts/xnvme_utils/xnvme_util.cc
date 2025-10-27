#include "xnvme_util.h"
#include <cstdint>


void* Xnvme_Backend::AllocBuf(uint32_t size) {
    void* buf = xnvme_buf_alloc(dev_, size);
    return buf;
}

void Xnvme_Backend::FreeBuf(void* buf) { xnvme_buf_free(dev_, buf); }
void Xnvme_Backend::FreeBuf(void* buf, uint32_t size) { xnvme_buf_free(dev_, buf); }

int Xnvme_Backend::Deallocate(int dio) {
    int err;
    uint32_t nsid;
    struct xnvme_spec_dsm_range *dsm_range;
    struct xnvme_cmd_ctx ctx = xnvme_cmd_ctx_from_dev(dev_);
    struct xnvme_ident const *ident = xnvme_dev_get_ident(dev_);
    if (ident->dtype != 1 && ident->dtype != 2)
        return 0;  // we skip non nvme devices
    nsid = xnvme_dev_get_nsid(dev_);
    dsm_range = (struct xnvme_spec_dsm_range *)xnvme_buf_alloc(dev_, sizeof(*dsm_range));
    if (!dsm_range) {
        err = -errno;
        perror("Failed to allocate xNVMe buffer for deallocate command");
        return err;
    }

    dsm_range->cattr = 0;
    dsm_range->slba = geo_.offset_ >> geo_->ssw;
    dsm_range->llb = geo_.size_ >> geo_->ssw;

    err = xnvme_nvm_dsm(&ctx, nsid, dsm_range, 0, true, false, false);
    if (err) {
        perror(
            "xnvme_nvm_dsm() [%d] dsm->cattr: [%u] dsm->slba: [%lu] dsm->llb: [%u] "
             + "nsid: [%u]",
            err, dsm_range->cattr, dsm_range->slba, dsm_range->llb, nsid);
    }
    xnvme_buf_free(dev_, reinterpret_cast<void *>(dsm_range));
    return err;
}

struct xnvme_opts Xnvme_Backend::GetOpt() {
    struct xnvme_opts opts = xnvme_opts_default();
    opts.async = async_io.c_str();
    opts.direct = 1;
    return opts;
}
