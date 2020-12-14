#ifndef REMOTE_BACKUP_M1_SERVER_RESOURCE_H
#define REMOTE_BACKUP_M1_SERVER_RESOURCE_H

#include <string>
#include <utility>
#include <iostream>

/*
 * synced:
 *      true:             client is synced with server on this resource
 *      false:            client is not synced with server on this resource
 *                          = client has to sent remaining chunks
 *
 * digest: digest value of resource
 */

namespace directory {
    /*
     * This class represent details about a directory object with respect to the server vision
     * of the resource state on client
     */
    class resource {
        bool synced_;
        std::string digest_;
    public:
        resource(bool synced, std::string digest);

        // setters and getters section

        resource &synced(bool const &synced);

        [[nodiscard]] bool synced() const;

        resource &digest(std::string digest);

        [[nodiscard]] std::string digest() const;
    };

    std::ostream &operator<<(std::ostream &os, directory::resource const &rsrc);
}


#endif //REMOTE_BACKUP_M1_SERVER_RESOURCE_H
