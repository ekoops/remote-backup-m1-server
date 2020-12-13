//
// Created by leonardo on 01/12/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_RESOURCE_H
#define REMOTE_BACKUP_M1_SERVER_RESOURCE_H


#include <string>
#include <boost/logic/tribool.hpp>
#include <utility>
#include <iostream>

//state: {processing, synced, failed} => synced tribool {indeterminate, true, false}


namespace directory {
    class resource {
        boost::logic::tribool synced_;
        std::string digest_;
    public:
        resource(boost::logic::tribool synced, std::string digest);

        resource& synced(boost::logic::tribool const &synced);

        [[nodiscard]] boost::logic::tribool synced() const;

        resource& digest(std::string digest);

        [[nodiscard]] std::string digest() const;
    };
    std::ostream& operator<<(std::ostream& os, directory::resource const& rsrc);
}


#endif //REMOTE_BACKUP_M1_SERVER_RESOURCE_H
