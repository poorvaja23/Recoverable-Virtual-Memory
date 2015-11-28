#include <map>
#include <string>
using std::string;

typedef struct segment{
  std::string   seg_name  ;
  int           size;
  bool          is_mapped;
  bool          in_use;
  void*         address;
}seg_t;

typedef struct rvm{
  std::string                   store_dir;
  std::map<std::string, seg_t>  seg_map;
}rvm_data;

typedef rvm_data* rvm_t;
