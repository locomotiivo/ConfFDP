#pragma once
#include <cstdint>
#include <mutex>
#include <queue>
#include <stdint.h>
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

class DeviceGeometry {
public:
    DeviceGeometry() = default;
    DeviceGeometry& operator=(const DeviceGeometry&) = default;

    explicit DeviceGeometry(uint64_t offset, uint32_t size, uint8_t *buf)
        : offset_{offset}, size_{size}, buf_{buf} {}

    uint64_t offset_;
    uint32_t size_;
    uint8_t *buf_;
    uint8_t dtype_ = 0;
    uint16_t dspec_ = 0;
    void EnableDirective(uint16_t dspec);
};

class Xnvme_Backend {
public:
    Xnvme_Backend(): nsid(0) {};
    ~Xnvme_Backend() { Exit(); }

    uint64_t slba = 0;
    uint64_t nlb;
    uint64_t nsid;

    struct xnvme_opts opts;

    std::mutex xnvme_mutex;
    std::queue<struct xnvme_queue*> xnvme_queues_;

    struct xnvme_dev *dev_ = nullptr;
    const struct xnvme_geo *geo_ = nullptr;
    struct xnvme_queue *queues_[MAX_NR_QUEUE];

    std::mutex buf_mutex;
    std::queue<void*> buf_queue_;

    void* AllocBuf(uint32_t size);
    void FreeBuf(void* buf);
    int Deallocate(const DeviceGeometry& geo);

    int Init();
    void Exit();

    uint64_t GetNlb() { return geo_->nsect; }
    uint32_t GetLbaShift() { return geo_->ssw; }

private:
    const unsigned int qdepth_ = MAX_NR_QUEUE;
    int Async(int dio);
    virtual struct xnvme_opts GetOpt(std::string dev_type) = 0;
    static void async_cb(struct xnvme_cmd_ctx *ctx, void *cb_arg);
    int SubmitXNvmeAsyncCmd(struct xnvme_queue *xqueue, const DeviceGeometry &geo, int io);
    int SubmitDeallocate(const DeviceGeometry &geo);

};

class IOInterface {
public:
    explicit IOInterface(const std::string &dev, const std::string &async_io, const std::string &be);
    ~IOInterface();
  
    int Read(uint64_t offset, uint32_t size, void *buf);
    int Write(uint64_t offset, uint32_t size, const void *buf);
    int Deallocate(uint64_t offset, uint32_t size);
    uint64_t GetNlba();
    uint64_t GetBlockSize();
    void* AllocBuf(uint32_t size);
    void FreeBuf(void* buf);
    void FreeBuf(void* buf, uint32_t size);
    std::shared_ptr<Xnvme_Backend> be_interface_;

private:
    DeviceGeometry PrepIO(uint64_t offset, uint32_t size, const void *buf);
    int DoIO(uint64_t offset, uint32_t size, const void *buf, uint16_t placementID, int io);
};