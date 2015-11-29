#include "rvm.h"
#include <iostream>
#include <map>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <cstdio>
#include <time.h>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;

#ifdef DEBUG
#define PRINT_DEBUG(M, ...) printf("%s\n", M)
#else
#define PRINT_DEBUG(M, ...)
#endif

/*
Copy contents of the log file to the segment file, if applicable
Delete the log file after transfer
*/

void createLogFile(rvm_t rvm, string segname)
{
  string seg_file_path = rvm->store_dir + "/" + segname;
  string log_file_path = seg_file_path + ".log";
  int seg_file, log_file;
  int offset, size;
  void *value;
  int size_of_int = sizeof(int);
  seg_file = open(seg_file_path.c_str(), O_RDWR | O_CREAT, 0755);
  if(seg_file == -1)
  {
    PRINT_DEBUG("Error opening segment file");
    return;
  }
  log_file = open(log_file_path.c_str(), O_RDWR | O_CREAT, 0755);
  if(log_file == -1)
  {
    close(seg_file);
    PRINT_DEBUG("Error opening log file");
    return;
  }
  int file_size = lseek(log_file, 0, SEEK_END);
  if(file_size <= 0)
    return;
    lseek(log_file, 0, 0);
    while(lseek(log_file, 0, SEEK_CUR) < file_size)
    {
      read(log_file, &offset, size_of_int);
      read(log_file, &size, size_of_int);
      value = operator new(size);
      read(log_file, value, size);
      lseek(seg_file, offset, 0);
      write(seg_file, value, size);
      operator delete(value);
    }
    close(seg_file);
    close(log_file);
    string cmd = "rm " + log_file_path;
    system(cmd.c_str());
    cmd = "touch " + log_file_path;
    system(cmd.c_str());
  }

rvm_t rvm_init(const char *directory){
  struct stat st = {0};

  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }else{
    PRINT_DEBUG("DIRECTORY EXISTS.");
    //directory already exists
  }
  rvm_t rvm = new rvm_data(directory);
  return rvm;
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create)
{
  string seg_name = string(segname);
  int length = rvm->store_dir.length() + strlen(segname) + 1;
  char *file_path = new char(length);

  map<string, seg_t>::iterator i;
	i = rvm->seg_map.find(seg_name);
	if(i != rvm->seg_map.end())
	{
		if(i->second.is_mapped == 1)
			return NULL;    //NULL?
	}

  createLogFile(rvm, seg_name);

  /*get the file path*/
  strcpy(file_path, rvm->store_dir.c_str());
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
  seg_t seg;
	seg.seg_name = seg_name;
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
 delete(file_path);
	  rvm->seg_map[seg_name] = seg;

	close(file);
  cout << "this is final return" << "\n";
  return rvm->seg_map[seg_name].address;
}

void rvm_unmap(rvm_t rvm, void *segbase)
{
	map<string, seg_t>::iterator it;
	string seg_name;
	for(it = rvm->seg_map.begin(); it != rvm->seg_map.end(); it++)
	{
    seg_name = it->first;
    /*if the segment is mapped*/
		if(it->second.address == segbase)
			break;
	}
  	/* There is no such segment*/
	if(it->second.address != segbase)
	{
		PRINT_DEBUG("The segment to be unmapped does not exist");
		return;
	}
	/*This process has not being mapped to the segment*/
	if(it->second.is_mapped == 0)
	{
		PRINT_DEBUG("The segment to be unmapped has not been mapped before.");
		return;
	}
  /*The segment is being currently modified*/
	if(it->second.in_use)
	{
		PRINT_DEBUG("The segment to be unmapped is in use.");
		return;
	}

	operator delete(it->second.address);
	rvm->seg_map.erase(it);
}

void rvm_destroy(rvm_t rvm, const char *segname){

  map<string,seg_t>::iterator iterator;
  string name_to_compare = segname;
  iterator = rvm->seg_map.find(name_to_compare);
  if (iterator == rvm->seg_map.end()){
    //The segment doesn't exist
    return;
  }
  if (iterator->second.is_mapped == 1){
    //The segment is currently mapped
    return;
  }
  //Otherwise erase log and backing store
  operator delete(iterator->second.address);
  rvm->seg_map.erase(iterator);
  string del_log = "rm " + rvm->store_dir + "/" + name_to_compare;
  system(del_log.c_str());
  del_log += ".log";
  system(del_log.c_str());
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases)
{
  map<string, segment_t>::iterator it;
  trans_data transaction;
  transaction.rvm = rvm;
	int flag;
	for(int i = 0; i < numsegs; i++)
	{
    flag = 0;
    for(it = rvm->seg_map.begin(); it != rvm->seg_map.end(); it++)
		{
			if(it->second.address == segbases[i])
			{
        flag = 1;
        /*the segment is being used*/
        if(it->second.in_use == 1)
          return -1;
        /*the segment is not mapped*/
        if(it->second.is_mapped == 0)
          return -1;

        /*the segment is available so create a transaction map entry*/
        transaction.segments[segbases[i]] = &(it->second);
      }
    }
    /*The segment does not exist*/
    if(flag == 0)
      return -1;
   }

   /*if the segments were available set the in_use flag*/
  map<void*, seg_t*>::iterator trans;
	for(trans = transaction.segments.begin(); trans != transaction.segments.end(); trans++)
	{
		transaction.segments[trans->first]->in_use = 1;
	}
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){

}
