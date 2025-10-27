#include "xnvme_util.h"
#include <cstdint>
#include <cstdarg>

void xnvme_perr(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("ERROR ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
    fflush(stdout);
}

void* Xnvme_Backend::AllocBuf(uint32_t size) {
    return xnvme_buf_alloc(dev_, size);
}

void Xnvme_Backend::FreeBuf(void* buf) { xnvme_buf_free(dev_, buf); }
// void Xnvme_Backend::FreeBuf(void* buf, uint32_t size) { xnvme_buf_free(dev_, buf); }

int Xnvme_Backend::Init() {
    int err = 0;
    struct xnvme_opts opts = GetOpt();
    dev_ = xnvme_dev_open(opts.dev, &opts);
    if (!dev_) {
        xnvme_perr("Cannot open the device: %s", opts.dev);
        return -1;
    }
    geo_ = xnvme_dev_get_geo(dev_);
    for (int i = 0; i < MAX_NR_QUEUE; i++) {
        queues_[i] = nullptr;
        err = xnvme_queue_init(dev_, qdepth_, 0, &queues_[i]);
        if (!err) {
            xnvme_queues_.push(queues_[i]);
        } else {
            xnvme_perr("Failed to initialize xNVMe");
            return -1;
        }
    }
    return err;
}

void Xnvme_Backend::Exit() {
    int err;
    for (int i = 0; i < MAX_NR_QUEUE; i++) {
        if (queues_[i] == nullptr) {
            break;
        }
        err = xnvme_queue_term(queues_[i]);
        if (err) {
            xnvme_perr("The [%d]th queue is failed", i);
        }
    }
    xnvme_dev_close(dev_);
}


int Xnvme_Backend::Deallocate(const DeviceGeometry& geo) {
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
        xnvme_perr("Failed to allocate xNVMe buffer for deallocate command");
        return err;
    }

    dsm_range->cattr = 0;
    dsm_range->slba = geo.offset_ >> geo_->ssw;
    dsm_range->llb = geo.size_ >> geo_->ssw;

    err = xnvme_nvm_dsm(&ctx, nsid, dsm_range, 0, true, false, false);
    if (err) {
        xnvme_perr(
            "xnvme_nvm_dsm() [%d] dsm->cattr: [%u] dsm->slba: [%lu] dsm->llb: [%u] "
            , "nsid: [%u]",
            err, dsm_range->cattr, dsm_range->slba, dsm_range->llb, nsid);
    }
    xnvme_buf_free(dev_, reinterpret_cast<void *>(dsm_range));
    return err;
}

struct xnvme_opts Xnvme_Backend::GetOpt(std::string dev_type) {
    struct xnvme_opts opts = xnvme_opts_default();

    if (dev_type == "block") {
        opts.async = "sync";
        opts.direct = 1;
    } else if (dev_type == "char") {
        opts.async = "sync";
        opts.direct = 1;
    } else if (dev_type == "spdk") {
        opts.be = "spdk";
        opts.async = "sync";
        opts.direct = 1;    
    }

    return opts;
}
