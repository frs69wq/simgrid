/* Copyright (c) 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <unistd.h>
#include <string.h> // memcpy, memcp

#include <sys/mman.h>

#include <boost/foreach.hpp>

#include <xbt.h>

#include "mc_page_store.h"

#include "mc_mmu.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_page_snapshot, mc,
                                "Logging specific to mc_page_snapshot");

// ***** Utility:

/** @brief Compte a hash for the given memory page
 *
 *  The page is used before inserting the page in the page store
 *  in order to find duplicate of this pae in the page store.
 *
 *  @param data Memory page
 *  @return hash off the page
 */
static inline  __attribute__ ((always_inline))
uint64_t mc_hash_page(const void* data)
{
  const uint64_t* values = (const uint64_t*) data;
  size_t n = xbt_pagesize / sizeof(uint64_t);

  // This djb2:
  uint64_t hash = 5381;
  for (size_t i=0; i!=n; ++i) {
    hash = ((hash << 5) + hash) + values[i];
  }
  return hash;
}

// ***** snapshot_page_manager

s_mc_pages_store::s_mc_pages_store(size_t size) :
  memory_(NULL), capacity_(0), top_index_(0)
{
  // Using mmap in order to be able to expand the region
  // by relocating it somewhere else in the virtual memory
  // space:
  void * memory = ::mmap(NULL, size << xbt_pagebits, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
  if (memory==MAP_FAILED) {
    xbt_die("Could not mmap initial snapshot pages.");
  }

  this->top_index_ = 0;
  this->capacity_ = size;
  this->memory_ = memory;
  this->page_counts_.resize(size);
}

s_mc_pages_store::~s_mc_pages_store()
{
  ::munmap(this->memory_, this->capacity_ << xbt_pagebits);
}

void s_mc_pages_store::resize(size_t size)
{
  size_t old_bytesize = this->capacity_ << xbt_pagebits;
  size_t new_bytesize = size << xbt_pagebits;

  // Expand the memory region by moving it into another
  // virtual memory address if necessary:
  void* new_memory = mremap(this->memory_, old_bytesize, new_bytesize, MREMAP_MAYMOVE);
  if (new_memory == MAP_FAILED) {
    xbt_die("Could not mremap snapshot pages.");
  }

  this->capacity_ = size;
  this->memory_ = new_memory;
  this->page_counts_.resize(size, 0);
}

/** Allocate a free page
 *
 *  @return index of the free page
 */
size_t s_mc_pages_store::alloc_page()
{
  if (this->free_pages_.empty()) {

    // Expand the region:
    if (this->top_index_ == this->capacity_) {
      // All the pages are allocated, we need add more pages:
      this->resize(2 * this->capacity_);
    }

    // Use a page from the top:
    return this->top_index_++;

  } else {

    // Use a page from free_pages_ (inside of the region):
    size_t res = this->free_pages_[this->free_pages_.size() - 1];
    this->free_pages_.pop_back();
    return res;

  }
}

void s_mc_pages_store::remove_page(size_t pageno)
{
  this->free_pages_.push_back(pageno);
  const void* page = this->get_page(pageno);
  uint64_t hash = mc_hash_page(page);
  this->hash_index_[hash].erase(pageno);
}

/** Store a page in memory */
size_t s_mc_pages_store::store_page(void* page)
{
  xbt_assert(mc_page_offset(page)==0, "Not at the beginning of a page");
  xbt_assert(top_index_ <= this->capacity_, "top_index is not consistent");

  // First, we check if a page with the same content is already in the page
  // store:
  //  1. compute the hash of the page;
  //  2. find pages with the same hash using `hash_index_`;
  //  3. find a page with the same content.
  uint64_t hash = mc_hash_page(page);
  page_set_type& page_set = this->hash_index_[hash];
  BOOST_FOREACH (size_t pageno, page_set) {
    const void* snapshot_page = this->get_page(pageno);
    if (memcmp(page, snapshot_page, xbt_pagesize) == 0) {

      // If a page with the same content is already in the page store it is
      // reused and its reference count is incremented.
      page_counts_[pageno]++;
      return pageno;

    }
  }

  // Otherwise, a new page is allocated in the page store and the content
  // of the page is `memcpy()`-ed to this new page.
  size_t pageno = alloc_page();
  xbt_assert(this->page_counts_[pageno]==0, "Allocated page is already used");
  void* snapshot_page = (void*) this->get_page(pageno);
  memcpy(snapshot_page, page, xbt_pagesize);
  page_set.insert(pageno);
  page_counts_[pageno]++;
  return pageno;
}

// ***** Main C API

extern "C" {

mc_pages_store_t mc_pages_store_new()
{
  return new s_mc_pages_store_t(500);
}

void mc_pages_store_delete(mc_pages_store_t store)
{
  delete store;
}

}

#ifdef SIMGRID_TEST

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#include <memory>

#include "mc/mc_page_store.h"

static int value = 0;

static void new_content(void* data, size_t size)
{
  memset(data, ++value, size);
}

static void* getpage()
{
  return mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

extern "C" {

XBT_TEST_SUITE("mc_page_store", "Page store");

XBT_TEST_UNIT("base", test_mc_page_store, "Test adding/removing pages in the store")
{
  xbt_test_add("Init");
  size_t pagesize = (size_t) getpagesize();
  std::auto_ptr<s_mc_pages_store_t> store = std::auto_ptr<s_mc_pages_store_t>(new s_mc_pages_store(500));
  void* data = getpage();
  xbt_test_assert(store->size()==0, "Bad size");

  xbt_test_add("Store the page once");
  new_content(data, pagesize);
  size_t pageno1 = store->store_page(data);
  xbt_test_assert(store->get_ref(pageno1)==1, "Bad refcount");
  const void* copy = store->get_page(pageno1);
  xbt_test_assert(memcmp(data, copy, pagesize)==0, "Page data should be the same");
  xbt_test_assert(store->size()==1, "Bad size");

  xbt_test_add("Store the same page again");
  size_t pageno2 = store->store_page(data);
  xbt_test_assert(pageno1==pageno2, "Page should be the same");
  xbt_test_assert(store->get_ref(pageno1)==2, "Bad refcount");
  xbt_test_assert(store->size()==1, "Bad size");

  xbt_test_add("Store a new page");
  new_content(data, pagesize);
  size_t pageno3 = store->store_page(data);
  xbt_test_assert(pageno1 != pageno3, "New page should be different");
  xbt_test_assert(store->size()==2, "Bad size");

  xbt_test_add("Unref pages");
  store->unref_page(pageno1);
  xbt_assert(store->get_ref(pageno1)==1, "Bad refcount");
  xbt_assert(store->size()==2, "Bad size");
  store->unref_page(pageno2);
  xbt_test_assert(store->size()==1, "Bad size");

  xbt_test_add("Reallocate page");
  new_content(data, pagesize);
  size_t pageno4 = store->store_page(data);
  xbt_test_assert(pageno1 == pageno4, "Page was not reused");
  xbt_test_assert(store->get_ref(pageno4)==1, "Bad refcount");
  xbt_test_assert(store->size()==2, "Bad size");
}

}

#endif /* SIMGRID_TEST */