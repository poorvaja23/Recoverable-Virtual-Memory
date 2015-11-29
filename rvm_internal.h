#include <map>
#include <string>
using std::string;

typedef struct seg_t{
  std::string   seg_nam  ;
  int           size;
  bool          is_mapped;
  bool          in_use;
  void*         address;
}seg_t;

typedef struct rvm_t{
  std::string                   store_dir;
  std::map<std::string, seg_t>  seg_map;
}rvm_t;
