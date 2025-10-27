#ifndef BLOCK_FDP_EVENT_H
#define BLOCK_FDP_EVENT_H

#include "hw/registerfields.h"

typedef struct QEMU_PACKED fdp_event {
    uint8_t type; ///< Event type

	/** FDP event flags */
	union {
		struct {
			uint8_t piv   : 1; ///< Placement identifier valid
			uint8_t nsidv : 1; ///< NSID valid
			uint8_t lv    : 1; ///< Location valid
			uint8_t rsvd1 : 5;
		};
		uint8_t val;
	} fdpef;

	uint16_t pid;              ///< Placement identifier
	uint64_t timestamp;        ///< Event timestamp
	uint32_t nsid;             ///< Namespace identifier
	uint8_t type_specific[16]; ///< Event type specific
	uint16_t rgid;             ///< Reclaim group identifier
	uint16_t ruhid;            ///< Reclaim unit handle identifier
	uint8_t rsvd1[4];
	uint8_t vs[24];
} fdp_event;

typedef struct QEMU_PACKED fdp_log_events {
    uint32_t nevents; ///< Number of FDP events
    uint8_t rsvd[60];
    struct fdp_event event[];
} fdp_log_events;

typedef struct QEMU_PACKED fdp_event_desc {
    uint8_t type; ///< Event type

    /** FDP event type attributes */
    union {
        struct {
            uint8_t ee   : 1; ///< FDP event enabled
            uint8_t rsvd : 7;
        };
        uint8_t val;
    } fdpeta;
} fdp_event_desc;

enum fdp_event_type {
    FDP_EVT_RU_NOT_FULLY_WRITTEN = 0x0,
    FDP_EVT_RU_ATL_EXCEEDED = 0x1,
    FDP_EVT_CTRL_RESET_RUH = 0x2,
    FDP_EVT_INVALID_PID = 0x3,
    FDP_EVT_MEDIA_REALLOC = 0x80,
    FDP_EVT_RUH_IMPLICIT_RU_CHANGE = 0x81,
};

#endif /* BLOCK_FDP_EVENT_H */