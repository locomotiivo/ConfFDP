#include <libxnvme.h>
#include <linux/nvme_ioctl.h>
#include <sys/ioctl.h>
#include <liburing.h>
#include <memory>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <queue>
#include <string>

class BackEnd {
public:
    virtual ~BackEnd() = 0;
    virtual void init() = 0;
};