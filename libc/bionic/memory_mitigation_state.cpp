/*
 * Copyright (C) 2020 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "memory_mitigation_state.h"

#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>

#include <bionic/malloc.h>
#include <bionic/mte.h>

#include <private/ScopedPthreadMutexLocker.h>
#include <private/ScopedRWLock.h>

#include "heap_tagging.h"
#include "pthread_internal.h"

extern "C" void scudo_malloc_set_zero_contents(int zero_contents);

bool DisableMemoryMitigations(void* arg, size_t arg_size) {
  if (arg || arg_size) {
    return false;
  }

#ifdef USE_SCUDO
  scudo_malloc_set_zero_contents(0);
#endif

  ScopedPthreadMutexLocker locker(&g_heap_tagging_lock);

  HeapTaggingLevel current_level = GetHeapTaggingLevel();
  if (current_level != M_HEAP_TAGGING_LEVEL_NONE && current_level != M_HEAP_TAGGING_LEVEL_TBI) {
    HeapTaggingLevel level = M_HEAP_TAGGING_LEVEL_NONE;
    SetHeapTaggingLevel(reinterpret_cast<void*>(&level), sizeof(level));
  }

  return true;
}
