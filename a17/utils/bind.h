#define bind0(fn, ...) std::bind(fn, this, ##__VA_ARGS__)

#define bind1(fn, ...) std::bind(fn, this, std::placeholders::_1, ##__VA_ARGS__)

#define bind2(fn, ...) \
  std::bind(fn, this, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)

#define bind3(fn, ...)                                                                     \
  std::bind(fn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, \
            ##__VA_ARGS__)

#define bind4(fn, ...)                                                                     \
  std::bind(fn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, \
            std::placeholders::_4, ##__VA_ARGS__)
