#include "resource.h"

using namespace directory;

/**
 * Construct a resource instance setting up all its field
 *
 * @param synced value specifying if client is synced with server on this resource
 * @param digest the resource digest
 * @return a new constructed resource instance
 */
resource::resource(bool synced, std::string digest)
        : synced_{synced}, digest_{std::move(digest)} {}

/**
* Setter for synced field.
*
* @param synced the new synced field value
* @return the resource on which it has been applied
*/
resource &resource::synced(bool const &synced) {
    this->synced_ = synced;
    return *this;
}

/**
* Getter for synced field.
*
* @return the synced field value
*/
[[nodiscard]] bool resource::synced() const {
    return this->synced_;
}

/**
* Setter for digest field.
*
* @param digest the new digest field value
* @return the resource on which it has been applied
*/
resource &resource::digest(std::string digest) {
    this->digest_ = std::move(digest);
    return *this;
}

/**
* Getter for digest field.
*
* @return the digest field value
*/
[[nodiscard]] std::string resource::digest() const {
    return this->digest_;
}

std::ostream &directory::operator<<(std::ostream &os, directory::resource const &rsrc) {
    return os << "{ synced: " << std::boolalpha << rsrc.synced()
              << "; digest: " << rsrc.digest() << "}" << std::endl;
}