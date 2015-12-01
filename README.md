# Recoverable-Virtual-Memory
## Team Members: Poorvaja Rajagopal and Shanu Salunke

This file describes our implementation of a recoverable memory management system inspired by elements of Satyanarayanan et al.'s "Lightweight Recoverable Virtual Memory " and Lowell and Chen's,"Free Transactions With Rio Vista".

##Compilation Instructions
Use the Makefile to generate executables or alternatively, copy rvm.cpp and rvm_internal.h to use in your project.

##Discussion of the Implementation
A short description of the implementations of each function are given below:
1. rvm_init: A directory with the given name is created that serves as a backing store for the segment and log files created in other functions.
2. rvm_map: Given a segment name (hereafter, segname) and a size, we create the appropriate files and data structures to store details about the segment. If such a segment was already in existence, its log files are used to ensure persistence. Segments are mapped in an internal structure. If someone attempts to re-map a segment, or other illegal operations, the function returns NULL.
3. rvm_unmap: The segment is removed from the internal mapping structure.
4. rvm_destroy: We delete the files created for this segment (segname and segname.log) in the backing store (created in rvm_init)
5. rvm_begin_trans: For each segment in the given list, check if it can be a part of a valid transaction. If so, map it in the internal data structure, assigning it a transaction id.
6. rvm_about_to_modify: Copy the current content of the segment into its undo_log. This is in case of an abort. The undo_record for the segment is then pushed to the front of the transactions map (an internal structure for storing in-progress trnasactions).
7. rvm_commit_trans: For a given transaction id, loop through all the undo_records in that transaction. Find the segment for each undo_record and transfer it to the log_file. Delete the undo record and remove the transaction from the internal structure since it is completed.
8. rvm_abort_trans: For a given transaction id, find all the undo_records associated with the transaction. Copy back the contents of the undo_record to its corresponding segment. Mark the segments as not being modified and delte the undo records from the internal data structure (transaction map).
9. rvm_truncate_log: For all log files in the backing store, transfer their contents to the associated segment file and delete the log files.


##Use of log files:
Two types of logging structures are used. The first is a temporary data structure called the "undo_log". And segment that is being modified is first backed up to the undo_log. The contents of the undo_log may be saved to the redo_log, deleted, or moved back into the segment, depending on the operations performed by the user. The undo_log is associated with a segment and a transaction and saved in a transaction map (our internal data strucutre).
The second type of log is the redo_log, implemented as a persistent file in the backup store with the same name as the segment base and an extension of ".log". This is used in the rvm_commit_trans method.

##How we clean up log files:
Log files are cleaned up in the rvm_truncate_log method. This transfers the contains of the redo_logs to the segment files. Undo_logs are deleted almost after use if the transaction is either committed or aborted. The log files of a single segment are also deleted along with the segment in rvm_destroy.
