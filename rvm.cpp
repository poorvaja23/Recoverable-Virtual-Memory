#include "rvm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
using std::string;

rvm_t rvm_init(const char *directory){
  struct stat st = {0};
  rvm_t store;

  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }else{
    //directory already exists
  }
  store.segment_dir = string(directory);
  return store;
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create)
{
  string seg_name = string(segname);

  map<string, seg_t>::iterator i;
	i = rvm->seg_map.find(seg_name);
	if(i != rvm->segment_map.end())
	{
		if(i->second.is_mapped == 1)
			return;    //NULL?
	}

  //   createLogFile(rvm, seg_name);

  /*get the file path*/
	int length = rvm->store_dir.length() + strlen(segname) + 1;
	char *file_path = new char(length);
  strcpy(file_path, rvm->path.c_str());
	strcat(file_path, "/");
	strcat(file_path, segname);

  /*check size of the file and add padding if lesser than size_to_create*/
  int file, size, result;
	file = open(file_path, O_RDWR | O_CREAT, 0755);
  if(file == -1)
  {
	    PRINT_DEBUG("Error opening file");
	    return NULL;
	}
	size = lseek(file, 0, SEEK_END);
    if (size == -1)
    {
        close(file);
        PRINT_DEBUG("Error getting size");
        return NULL;
    }

    /* Size of the segment is less than the size to be created then pad it */
    if (size < size_to_create)
    {
        lseek(file, size_to_create - 1, SEEK_SET);
        result = write(file, "\0", 1);
        if (result == -1) {
            close(file);
            PRINT_DEBUG("Error extending file");
            return NULL;
        }
    }

/*update the segment structure*/
  segment_t seg;
	seg.segname = seg_name;
	seg.size = size_to_create;
	seg.is_mapped = 1;
	seg.address = operator new(size_to_create);
	lseek(file, 0, 0);
    result = read(file, seg.address, size_to_create);
    if(result == -1)
    {
    	PRINT_DEBUG("Error reading file");
    	close(file);
    	return NULL;
    }

	  rvm->segment_map[seg_name] = seg;

	close(file);
}

    
