#include <map>
#include <string>
using std::string;

typedef struct seg_t{
  std::string   seg_name;
  int           size;
  bool          is_mapped;
  bool          is_about_to_modify;
}seg_t;

typedef struct rvm_t{
  std::string                   store_dir;
  std::map<std::string, seg_t>  seg_map;
}rvm_t;
