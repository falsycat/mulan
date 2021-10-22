// No copyright
#include <mulan.h>

#include <fstream>
#include <iostream>
#include <unordered_map>

std::unordered_map<uint64_t, std::string> lang_;

const char* gettext(uint64_t id, const char* str) {
  const auto itr = lang_.find(id);
  if (itr == lang_.end()) {
    return str;
  }
  return itr->second.c_str();
}
#define _(v) gettext(mulan::hash(v), v)


int main(int argc, char** argv) {
  // load language
  if (argc == 2) {
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
      std::cerr << "file open error" << std::endl;
      return 1;
    }

    for (;;) {
      uint8_t hdr[MULAN_HDR];
      input.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
      if (!input) break;

      uint64_t id;
      uint16_t strn;
      mulan::unpack_header(hdr, &id, &strn);

      std::string str(strn, 0);
      if (!input.read(str.data(), strn)) {
        std::cerr << "unexpected EOF" << std::endl;
        return 1;
      }
      lang_[id] = std::move(str);
    }
  }

  // app main
  std::cout << _("Hello World!!") << std::endl;
  std::cout << _("MULAN provides a easy way to support i18n") << std::endl;
  return 0;
}
