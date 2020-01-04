#include "sub_server.h"

namespace a17 {
namespace dispatch {

void SubServer::applyFilters() {
  if (logger_) {
    std::ostringstream subscribe_filters_ostream;
    for (const std::string &filter : filters_) {
      subscribe_filters_ostream << " \"" << filter << '\"';
    }
    logger_->debug("{} setting subscribe filters:{}", log_name_, subscribe_filters_ostream.str());
  }

  for (const message_type &filter : filters_) {
    azmqsocket_.set_option(azmq::socket::subscribe(filter));
  };
}

}  // namespace dispatch
}  // namespace a17
