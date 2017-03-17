#include <map>
#include <string>

using std::string;
using std::map;

namespace shadow {
namespace Crypto {

enum DIRECTION : int {
  DECRYPTO = 0,
  ENCRYPTO = 1,
};

void Init();

bool Encrypto(const string &src, string &des);

bool Decrypto(const string &src, string &des);

bool SignStr(const string &key, const string &src, string &des);

void SignDict(const string &key, map<string, string> dict, string &des);
}
}
