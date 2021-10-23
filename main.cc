// No copyright
#include "mulan.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "icon.h"


class Compile final {
 public:
  Compile(int argc, char** argv) : args_(argc, argv) {
    std::ifstream ist(args_.in);
    if (!ist) {
      std::cerr << "error: input file open failure" << std::endl;
      std::exit(1);
    }
    std::ofstream ost(args_.out, std::ios::binary);
    if (!ost) {
      std::cerr << "error: output file open failure" << std::endl;
      std::exit(1);
    }

    enum {
      kInitial,
      kMsgId,
      kMsgStr,
    } state = kInitial;

    std::string msgid;
    std::string msgstr;

    bool exit = false;
    for (; !exit; ++line_n_) {
      line_ = "";
      if (!std::getline(ist, line_)) exit = true;
      line_.erase(line_.find_last_not_of("\r\n")+1);

      switch (state) {
      case kInitial:
        if (line_.find("msgid ") == 0) {
          state = kMsgId;
          msgid = GetLiteralContent(line_);
        }
        break;
      case kMsgId:
        if (line_.find("msgstr ") == 0) {
          state  = kMsgStr;
          msgstr = GetLiteralContent(line_);
        } else {
          msgid += GetLiteralContent(line_);
        }
        break;
      case kMsgStr:
        if (line_.empty()) {
          if (!msgstr.empty()) {
            EncodeIcon(&msgstr);
            if (msgstr.size() > UINT16_MAX) {
              Error("too long msgstr");
            }

            uint8_t hdr[MULAN_HDR];
            mulan::pack_header(hdr,
                               mulan::hash_n(msgid.data(), msgid.size()),
                               static_cast<uint16_t>(msgstr.size()));

            ost.write(reinterpret_cast<char*>(hdr), sizeof(hdr));
            ost.write(msgstr.data(), static_cast<std::streamsize>(msgstr.size()));
            if (!ost) {
              std::cerr << "error: file write failure" << std::endl;
              std::exit(1);
            }
          }
          state = kInitial;
        } else {
          msgstr += GetLiteralContent(line_);
        }
        break;
      }
    }
    if (state != kInitial) {
      Error("unexpected EOF");
    }
  }

 private:
  [[noreturn]] void Error(const std::string& msg) const {
    std::cerr << "error: " << msg << std::endl;
    std::cerr << "  " << line_n_ << ": " << line_ << std::endl;
    std::exit(1);
  }

  std::string GetLiteralContent(const std::string& msg) const {
    std::string ret;

    enum {
      kPrefix,
      kPostfix,
      kContent,
      kEscSeq,
    } state = kPrefix;

    for (const auto c : msg) {
      switch (state) {
      case kPrefix:
        switch (c) {
        case '"': state = kContent; break;
        }
        break;
      case kContent:
        switch (c) {
        case '\\': state = kEscSeq;  break;
        case '\"': state = kPostfix; break;
        default  : ret  += c;
        }
        break;
      case kEscSeq:
        state = kContent;
        switch (c) {
        case '\'': ret += '\''; break;
        case '\"': ret += '\"'; break;
        case '\\': ret += '\\'; break;
        case 'n' : ret += '\n'; break;
        default  : Error("unknown escseq");
        }
        break;
      case kPostfix:
        return ret;
      }
    }
    if (state != kPrefix && state != kPostfix) {
      Error("invalid string literal");
    }
    return ret;
  }

  void EncodeIcon(std::string* str) const {
    for (size_t i = 0; i < str->size();) {
      const size_t n = str->size();
      for (; i < n-1 && str->at(i) != ':'; ++i) continue;
      const size_t l = i++;
      for (; i < n-1 && str->at(i) != ':'; ++i) continue;
      const size_t r = i++;

      if (l == r || str->at(l) != ':' || str->at(r) != ':') {
        break;
      }

      const std::string name(
          str->begin()+static_cast<std::string::difference_type>(l+1),
          str->begin()+static_cast<std::string::difference_type>(r));

      std::string b;
      for (const auto& icon : args_.icons) {
        const std::unordered_map<std::string, std::string>* m = nullptr;
        if (icon == "fontawesome5") {
          m = &kIconMap_FontAwesome5;
        }
        assert(m);
        const auto itr = m->find(name);
        if (itr != m->end()) {
          b = itr->second;
          break;
        }
      }
      if (b.size()) {
        str->replace(l, r-l+1, b);
        i = l+b.size();
      }
    }
  }


  size_t      line_n_ = 0;
  std::string line_;


  struct Args {
   public:
    Args(int argc, char** argv) {
      if (argc < 1) {
        std::cerr << "error: invalid args" << std::endl;
        std::exit(1);
      }
      in = argv[argc-1];

      enum {
        kInitial,
        kOutput,
        kIcon,
      } state = kInitial;

      for (int i = 0; i < argc-1; ++i) {
        const std::string v = argv[i];
        switch (state) {
        case kInitial:
          if (v == "--output" || v == "-o") {
            state = kOutput;
          } else if (v == "--icon" || v == "-i") {
            state = kIcon;
          } else {
            std::cerr << "error: unknown argument, " << v << std::endl;
            std::exit(1);
          }
          break;
        case kOutput:
          out = v;
          state = kInitial;
          break;
        case kIcon:
          if (kAvailableIcons.find(v) == kAvailableIcons.end()) {
            std::cerr << "error: unknown icon, " << v << std::endl;
            std::exit(1);
          }
          icons.insert(v);
          state = kInitial;
          break;
        }
      }

      if (state != kInitial) {
        std::cerr << "error: args end unexpectedly" << std::endl;
        std::exit(1);
      }
      if (out.empty()) {
        std::cerr << "error: no output specified" << std::endl;
        std::exit(1);
      }
    }

    std::string in, out;

    std::set<std::string> icons;
  };

  const Args args_;
};


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "error: no command specified" << std::endl;
    return 1;
  }

  const std::string cmd = argv[1];
  if (cmd == "h" || cmd == "help") {
    std::cout << "usage: " << argv[0] << " <command>"         << std::endl;
    std::cout << "  command:"                                 << std::endl;
    std::cout << "    help"                                   << std::endl;
    std::cout << "    compile [options] <input file>"         << std::endl;
    std::cout << "      --output <output file>"               << std::endl;
    std::cout << "      --icon <fontawesome5|(coming soon?)>" << std::endl;
    std::cout << "    dump <input file>"                      << std::endl;
    return 0;
  }
  if (cmd == "c" || cmd == "compile") {
    Compile(argc-2, argv+2);
    return 0;
  }
  if (cmd == "d" || cmd == "dump") {
    return 0;
  }

  std::cerr << "error: unknown command, " << cmd << std::endl;
  return 1;
}
