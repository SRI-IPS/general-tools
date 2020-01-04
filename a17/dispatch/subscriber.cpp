#include "subscriber.h"

#include <sstream>
#include <string>

namespace a17 {
namespace dispatch {

void Subscriber::applyFilters() {
  if (logger_) {
    std::ostringstream subscribe_filters_ostream;
    for (const std::string &filter : filters_) {
      subscribe_filters_ostream << " \"" << filter << '\"';
    }
    logger_->debug("{} setting subscribe filters:{}", log_name_, subscribe_filters_ostream.str());
  }

  for (const message_type &filter : filters_) {
    if (filter != "") {
      unsigned long long id = std::stoull(filter, 0, 0);
      azmqsocket_.set_option(azmq::socket::subscribe(reinterpret_cast<void *>(&id), sizeof(id)));
    } else {
      azmqsocket_.set_option(azmq::socket::subscribe());
    }
  };
}

}  // namespace dispatch
}  // namespace a17
