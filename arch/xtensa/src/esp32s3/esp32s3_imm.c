/****************************************************************************
 * arch/xtensa/src/esp32s3/esp32s3_imm.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/mm/mm.h>

#include "xtensa.h"

#ifdef CONFIG_XTENSA_IMEM_USE_SEPARATE_HEAP

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct mm_heap_s *g_iheap;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: xtensa_imm_initialize
 *
 * Description:
 *   Initialize the internal heap.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

void xtensa_imm_initialize(void)
{
  void  *start;
  size_t size;

  start = (void *)_sheap;
  size  = CONFIG_XTENSA_IMEM_REGION_SIZE;
  g_iheap = mm_initialize("esp32s3-imem", start, size);
}

/****************************************************************************
 * Name: xtensa_imm_malloc
 *
 * Description:
 *   Allocate memory from the internal heap.
 *
 * Input Parameters:
 *   size - Size (in bytes) of the memory region to be allocated.
 *
 * Return Value:
 *   Adress of the allocated memory space. NULL, if allocation fails.
 *
 ****************************************************************************/

void *xtensa_imm_malloc(size_t size)
{
  return mm_malloc(g_iheap, size);
}

/****************************************************************************
 * Name: xtensa_imm_calloc
 *
 * Description:
 *   Calculates the size of the allocation and
 *   allocate memory the internal heap.
 *
 * Input Parameters:
 *   n         - Size (in types) of the memory region to be allocated.
 *   elem_size - Size (in bytes) of the type to be allocated.
 *
 * Return Value:
 *   Adress of the allocated memory space. NULL, if allocation fails.
 *
 ****************************************************************************/

void *xtensa_imm_calloc(size_t n, size_t elem_size)
{
  return mm_calloc(g_iheap, n, elem_size);
}

/****************************************************************************
 * Name: xtensa_imm_realloc
 *
 * Description:
 *   Reallocate memory from the internal heap.
 *
 * Input Parameters:
 *   ptr  - Adress to be reallocate.
 *   size - Size (in bytes) to be reallocate.
 *
 * Return Value:
 *   Adress of the possibly moved memory space. NULL, if allocation fails.
 *
 ****************************************************************************/

void *xtensa_imm_realloc(void *ptr, size_t size)
{
  return mm_realloc(g_iheap, ptr, size);
}

/****************************************************************************
 * Name: xtensa_imm_zalloc
 *
 * Description:
 *   Allocate and zero memory from the internal heap.
 *
 * Input Parameters:
 *   size - Size (in bytes) of the memory region to be allocated.
 *
 * Return Value:
 *   Adress of the allocated memory space. NULL, if allocation fails.
 *
 ****************************************************************************/

void *xtensa_imm_zalloc(size_t size)
{
  return mm_zalloc(g_iheap, size);
}

/****************************************************************************
 * Name: xtensa_imm_free
 *
 * Description:
 *   Free memory from the internal heap.
 *
 * Input Parameters:
 *   mem - Adress to be freed.
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

void xtensa_imm_free(void *mem)
{
  mm_free(g_iheap, mem);
}

/****************************************************************************
 * Name: xtensa_imm_memalign
 *
 * Description:
 *   memalign requests more than enough space from malloc, finds a region
 *   within that chunk that meets the alignment request and then frees any
 *   leading or trailing space.
 *
 *   The alignment argument must be a power of two (not checked).  8-byte
 *   alignment is guaranteed by normal malloc calls.
 *
 * Input Parameters:
 *   alignment - Requested alignment.
 *   size - Size (in bytes) of the memory region to be allocated.
 *
 * Return Value:
 *   Adress of the allocated adress. NULL, if allocation fails.
 *
 ****************************************************************************/

void *xtensa_imm_memalign(size_t alignment, size_t size)
{
  return mm_memalign(g_iheap, alignment, size);
}

/****************************************************************************
 * Name: xtensa_imm_heapmember
 *
 * Description:
 *   Check if an address lies in the internal heap.
 *
 * Input Parameters:
 *   mem - The address to check.
 *
 * Return Value:
 *   True if the address is a member of the internal heap. False if not.
 *
 ****************************************************************************/

bool xtensa_imm_heapmember(void *mem)
{
  return mm_heapmember(g_iheap, mem);
}

/****************************************************************************
 * Name: xtensa_imm_mallinfo
 *
 * Description:
 *   mallinfo returns a copy of updated current heap information for the
 *   user heap.
 *
 * Input Parameters:
 *   None.
 *
 * Return Value:
 *   info - Where memory information will be copied.
 *
 ****************************************************************************/

struct mallinfo xtensa_imm_mallinfo(void)
{
  return mm_mallinfo(g_iheap);
}

#endif /* CONFIG_XTENSA_IMEM_USE_SEPARATE_HEAP */
