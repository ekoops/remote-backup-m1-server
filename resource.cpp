//
// Created by leonardo on 01/12/20.
//

#include "resource.h"

#include "resource.h"
#include <boost/logic/tribool_io.hpp>

using namespace directory;

resource::resource(boost::logic::tribool synced, std::string digest) : synced_{synced},
                                                                       digest_{std::move(
                                                                               digest)} {}

resource& resource::synced(boost::logic::tribool const &synced) {
    this->synced_ = synced;
    return *this;
}

[[nodiscard]] boost::logic::tribool resource::synced() const {
    return this->synced_;
}

resource& resource::digest(std::string digest) {
    this->digest_ = std::move(digest);
    return *this;
}

[[nodiscard]] std::string resource::digest() const {
    return this->digest_;
}

std::ostream& directory::operator<<(std::ostream& os, directory::resource const& rsrc) {
    return os << "{ synced: " << std::boolalpha << rsrc.synced()
              << "; digest: " << rsrc.digest() << "}" << std::endl;
}