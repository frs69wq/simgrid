#include "storage_interface.hpp"

#ifndef STORAGE_N11_HPP_
#define STORAGE_N11_HPP_

/***********
 * Classes *
 ***********/

class StorageN11Model;
typedef StorageN11Model *StorageN11ModelPtr;

class StorageN11;
typedef StorageN11 *StorageN11Ptr;

class StorageN11Lmm;
typedef StorageN11Lmm *StorageN11LmmPtr;

class StorageN11Action;
typedef StorageN11Action *StorageN11ActionPtr;

class StorageN11ActionLmm;
typedef StorageN11ActionLmm *StorageN11ActionLmmPtr;


/*********
 * Model *
 *********/

class StorageN11Model : public StorageModel {
public:
  StorageN11Model();
  ~StorageN11Model();
  StoragePtr createResource(const char* id, const char* type_id,
		   const char* content_name, const char* content_type, xbt_dict_t properties);
  double shareResources(double now);
  void updateActionsState(double now, double delta);

  xbt_dynar_t p_storageList;
};

/************
 * Resource *
 ************/

class StorageN11Lmm : public StorageLmm {
public:
  StorageN11Lmm(StorageModelPtr model, const char* name, xbt_dict_t properties,
		     lmm_system_t maxminSystem, double bread, double bwrite, double bconnection,
		     const char* type_id, char *content_name, char *content_type, sg_size_t size);

  StorageActionPtr open(const char* mount, const char* path);
  StorageActionPtr close(surf_file_t fd);
  StorageActionPtr ls(const char *path);
  xbt_dict_t getContent();
  sg_size_t getSize();
  StorageActionPtr read(surf_file_t fd, sg_size_t size);//FIXME:why we have a useless param ptr ??
  StorageActionPtr write(surf_file_t fd, sg_size_t size);//FIXME:why we have a useless param ptr ??
  void rename(const char *src, const char *dest);

  lmm_constraint_t p_constraintWrite;    /* Constraint for maximum write bandwidth*/
  lmm_constraint_t p_constraintRead;     /* Constraint for maximum write bandwidth*/
};

/**********
 * Action *
 **********/

class StorageN11ActionLmm : public StorageActionLmm {
public:
	StorageN11ActionLmm() {}; //FIXME:REMOVE
  StorageN11ActionLmm(ModelPtr model, double cost, bool failed, StorageLmmPtr storage, e_surf_action_storage_type_t type);
  void suspend();
  int unref();
  void cancel();
  //FIXME:??void recycle();
  void resume();
  bool isSuspended();
  void setMaxDuration(double duration);
  void setPriority(double priority);

};

#endif /* STORAGE_N11_HPP_ */