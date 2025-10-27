#pragma once
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <memory>
#include <libxnvme.h>

#define KB (2<<10)
#define MB (2<<20)
#define GB (2<<30)

#define MAX_NR_QUEUE 128
#define THREAD_NUM 4
#define TORFS_MAX_BUF (8 * MB)
#define TORFS_ALIGNMENT (4 * KB)
#define MAX_IO_SIZE (64 * KB)

class Xnvme_Backend {
public:
    Xnvme_Backend(std::string dev, std::string async_io, std::string be): dev(dev), async_io(async_io), be(be), nsid(0) {};
    ~Xnvme_Backend() { Exit(); }

    uint32_t nsid;
    std::string dev;
    std::string dev_type;
    std::string async_io;
    std::string be;

    std::mutex xnvme_mutex;
    std::queue<struct xnvme_queue*> xnvme_queues_;

    struct xnvme_dev *dev_ = nullptr;
    const struct xnvme_geo *geo_ = nullptr;
    struct xnvme_queue *queues_[MAX_NR_QUEUE];

    std::mutex buf_mutex;
    std::queue<void*> buf_queue_;

    void* AllocBuf(uint32_t size);
    void FreeBuf(void* buf);
    void FreeBuf(void* buf, uint32_t size);

    void SetDev(const std::string &name) { dev = name; }
    void SetAsyncType(const std::string &type) { async_io = type; }

private:
    int Async(int dio);
    virtual struct xnvme_opts GetOpt() = 0;
    int Init();
    void Exit();
    uint64_t GetNlb();
    uint32_t GetLbaShift();
    static void async_cb(struct xnvme_cmd_ctx *ctx, void *cb_arg);
    // int SubmitXNvmeAsyncCmd(struct xnvme_queue *xqueue, const DeviceGeometry &geo, TorfsDIO dio);
    // int SubmitDeallocate(const DeviceGeometry &geo);

};
