#ifndef REMOTE_BACKUP_M1_SERVER_TYPES_H
#define REMOTE_BACKUP_M1_SERVER_TYPES_H

#include <unordered_map>

namespace communication {
    /*
     * These enums define the allowed message type, the allowed
     * TLV type, the possible server error response and the
     * communication result for logging.
     */
    enum MSG_TYPE {
        NONE = 0,
        CREATE = 1,
        UPDATE = 2,
        ERASE = 3,
        SYNC = 4,
        AUTH = 5,
        KEEP_ALIVE = 6
    };

    enum TLV_TYPE {
        USRN = 0,
        PSWD = 1,
        ITEM = 2,
        END = 3,
        OK = 4,
        ERROR = 5,
        CONTENT = 6
    };

    enum ERR_TYPE {
        ERR_NONE = 0,
        ERR_NO_CONTENT = 1,
        ERR_MSG_TYPE_REJECTED = 2,
        ERR_CREATE_NO_ITEM = 101,
        ERR_CREATE_NO_CONTENT = 102,
        ERR_CREATE_ALREADY_EXIST = 103,
        ERR_CREATE_FAILED = 104,
        ERR_CREATE_NO_MATCH = 105,
        ERR_UPDATE_NO_ITEM = 201,
        ERR_UPDATE_NO_CONTENT = 202,
        ERR_UPDATE_NOT_EXIST = 203,
        ERR_UPDATE_ALREADY_UPDATED = 204,
        ERR_UPDATE_FAILED = 205,
        ERR_UPDATE_NO_MATCH = 206,
        ERR_ERASE_NO_ITEM = 301,
        ERR_ERASE_NO_MATCH = 302,
        ERR_SYNC_FAILED = 401,
        ERR_AUTH_NO_USRN = 501,
        ERR_AUTH_NO_PSWD = 502,
        ERR_AUTH_FAILED = 503
    };

    // communication result for logging
    enum RESULT_TYPE {
        RES_OK,
        RES_ERR,
        RES_NONE
    };
}

#endif //REMOTE_BACKUP_M1_SERVER_TYPES_H
