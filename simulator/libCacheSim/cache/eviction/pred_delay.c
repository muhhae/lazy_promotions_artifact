//
//  a LRU module that supports different obj size
//
//
//  LRU.c
//  libCacheSim
//
//  Created by Juncheng on 12/4/18.
//  Copyright © 2018 Juncheng. All rights reserved.
//

#include "../../dataStructure/hashtable/hashtable.h"
#include "../../include/libCacheSim/evictionAlgo.h"

typedef struct {
  cache_obj_t *q_head;
  cache_obj_t *q_tail;
  // fields added in addition to clock-
  uint64_t delay_time; // determines how often promotion is performed
  float delay_ratio;
  uint64_t n_insertion;
} PredDelay_params_t;

static const char *DEFAULT_PARAMS = "delay-time=0.1";
#ifdef __cplusplus
extern "C" {
#endif

// #define USE_BELADY

// ***********************************************************************
// ****                                                               ****
// ****                   function declarations                       ****
// ****                                                               ****
// ***********************************************************************

static void PredDelay_parse_params(cache_t *cache, const char *cache_specific_params);
static void PredDelay_free(cache_t *cache);
static bool PredDelay_get(cache_t *cache, const request_t *req);
static cache_obj_t *PredDelay_find(cache_t *cache, const request_t *req,
                             const bool update_cache);
static cache_obj_t *PredDelay_insert(cache_t *cache, const request_t *req);
static cache_obj_t *PredDelay_to_evict(cache_t *cache, const request_t *req);
static void PredDelay_evict(cache_t *cache, const request_t *req);
static bool PredDelay_remove(cache_t *cache, const obj_id_t obj_id);

// ***********************************************************************
// ****                                                               ****
// ****                   end user facing functions                   ****
// ****                                                               ****
// ****                       init, free, get                         ****
// ***********************************************************************
/**
 * @brief initialize a LRU cache
 *
 * @param ccache_params some common cache parameters
 * @param cache_specific_params LRU specific parameters, should be NULL
 */
cache_t *PredDelay_init(const common_cache_params_t ccache_params,
                  const char *cache_specific_params) {
  cache_t *cache = cache_struct_init("PredDelay", ccache_params, cache_specific_params);
  cache->cache_init = PredDelay_init;
  cache->cache_free = PredDelay_free;
  cache->get = PredDelay_get;
  cache->find = PredDelay_find;
  cache->insert = PredDelay_insert;
  cache->evict = PredDelay_evict;
  cache->remove = PredDelay_remove;
  cache->to_evict = PredDelay_to_evict;
  cache->get_occupied_byte = cache_get_occupied_byte_default;

  if (ccache_params.consider_obj_metadata) {
    cache->obj_md_size = 8 * 2;
  } else {
    cache->obj_md_size = 0;
  }

#ifdef USE_BELADY
  snprintf(cache->cache_name, CACHE_NAME_ARRAY_LEN, "LRU_Belady");
#endif

  PredDelay_params_t *params = malloc(sizeof(PredDelay_params_t));
  params->q_head = NULL;
  params->q_tail = NULL;
  params->delay_ratio = 0.1;
  params->delay_time = params->delay_ratio * cache->cache_size;
  cache->eviction_params = params;
  params->n_insertion = 0;

  if (cache_specific_params != NULL) {
    PredDelay_parse_params(cache, cache_specific_params);
  }
  
  snprintf(cache->cache_name, CACHE_NAME_ARRAY_LEN, "PredDelay_%f",
           params->delay_ratio);

  return cache;
}

/**
 * free resources used by this cache
 *
 * @param cache
 */
static void PredDelay_free(cache_t *cache) { 
    PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;
    printf("num promotion: %lld\n", cache -> n_promotion);
    free(cache->eviction_params);
    cache_struct_free(cache);
}

/**
 * @brief this function is the user facing API
 * it performs the following logic
 *
 * ```
 * if obj in cache:
 *    update_metadata
 *    return true
 * else:
 *    if cache does not have enough space:
 *        evict until it has space to insert
 *    insert the object
 *    return false
 * ```
 *
 * @param cache
 * @param req
 * @return true if cache hit, false if cache miss
 */
static bool PredDelay_get(cache_t *cache, const request_t *req) {
  return cache_get_base(cache, req);
}

// ***********************************************************************
// ****                                                               ****
// ****       developer facing APIs (used by cache developer)         ****
// ****                                                               ****
// ***********************************************************************

/**
 * @brief check whether an object is in the cache
 *
 * @param cache
 * @param req
 * @param update_cache whether to update the cache,
 *  if true, the object is promoted
 *  and if the object is expired, it is removed from the cache
 * @return true on hit, false on miss
 */
static cache_obj_t *PredDelay_find(cache_t *cache, const request_t *req,
                             const bool update_cache) {
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;
  cache_obj_t *cache_obj = cache_find_base(cache, req, update_cache);

  // no matter what, we need to check the buffer and see i

  if (cache_obj == NULL) {
    return NULL;
  }
  float remain_life = (float)(cache -> cache_size - params->n_insertion + cache_obj->delay_count.last_promo_vtime);
  remain_life = (float)(remain_life - cache -> n_promotion + cache_obj->delay_count.last_promotion);
  remain_life = (float)(params->n_insertion - cache_obj->delay_count.last_promo_vtime);
  float current_waiting_time = (float) params->n_insertion - cache_obj -> delay_count.last_hit_vtime;
  float miss_ratio = 0.35f;
  current_waiting_time = (float)req->next_access_vtime - cache->n_req;

  current_waiting_time = current_waiting_time * miss_ratio;
  // printf("current waiting time: %f next access %ld %ld\n", current_waiting_time, req->next_access_vtime, cache->n_req);
  // printf("remain life: %f\n", remain_life);

  

  // remain life is a lower bound on the value of lives
  cache_obj->delay_count.last_hit_vtime = params->n_insertion;
  if (cache_obj && likely(update_cache) && remain_life < current_waiting_time) {
    /* lru_head is the newest, move cur obj to lru_head */
#ifdef USE_BELADY
    if (req->next_access_vtime != INT64_MAX)
#endif
    //  check whether the last access time is greater than the delay time
      move_obj_to_head(&params->q_head, &params->q_tail, cache_obj);
      cache -> n_promotion++;
      // update the last access time
      cache_obj->delay_count.last_promo_vtime = params->n_insertion;
      cache_obj->delay_count.last_promotion = cache -> n_promotion;
      // cache_obj->delay_count.scaler *= 1.5;
  }

  return cache_obj;
}


/**
 * @brief insert an object into the cache,
 * update the hash table and cache metadata
 * this function assumes the cache has enough space
 * and eviction is not part of this function
 *
 * @param cache
 * @param req
 * @return the inserted object
 */
static cache_obj_t *PredDelay_insert(cache_t *cache, const request_t *req) {
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;
  params->n_insertion++;
  cache_obj_t *obj = cache_insert_base(cache, req);
  obj->delay_count.last_promo_vtime = params->n_insertion;
  obj->delay_count.last_promotion = cache -> n_promotion;
  // obj->delay_count.scaler = 1.0;
  obj->delay_count.last_hit_vtime = params->n_insertion;
  prepend_obj_to_head(&params->q_head, &params->q_tail, obj);

  return obj;
}

/**
 * @brief find the object to be evicted
 * this function does not actually evict the object or update metadata
 * not all eviction algorithms support this function
 * because the eviction logic cannot be decoupled from finding eviction
 * candidate, so use assert(false) if you cannot support this function
 *
 * @param cache the cache
 * @return the object to be evicted
 */
static cache_obj_t *PredDelay_to_evict(cache_t *cache, const request_t *req) {
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;

  DEBUG_ASSERT(params->q_tail != NULL || cache->occupied_byte == 0);

  cache->to_evict_candidate_gen_vtime = cache->n_req;
  return params->q_tail;
}

/**
 * @brief evict an object from the cache
 * it needs to call cache_evict_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param req not used
 */
static void PredDelay_evict(cache_t *cache, const request_t *req) {
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;
  cache_obj_t *obj_to_evict = params->q_tail;
  obj_to_evict->delay_count.last_promo_vtime = 0;
  obj_to_evict->delay_count.last_promotion = 0;
  obj_to_evict->delay_count.last_hit_vtime = 0;
  // obj_to_evict->delay_count.freq = 0;
  // obj_to_evict->delay_count.sum = 0;
  remove_obj_from_list(&params->q_head, &params->q_tail, obj_to_evict);
  cache_remove_obj_base(cache, obj_to_evict, true);
}

/**
 * @brief remove the given object from the cache
 * note that eviction should not call this function, but rather call
 * `cache_evict_base` because we track extra metadata during eviction
 *
 * and this function is different from eviction
 * because it is used to for user trigger
 * remove, and eviction is used by the cache to make space for new objects
 *
 * it needs to call cache_remove_obj_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param obj
 */
static void PredDelay_remove_obj(cache_t *cache, cache_obj_t *obj_to_remove) {
  DEBUG_ASSERT(obj_to_remove != NULL);
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;

  remove_obj_from_list(&params->q_head, &params->q_tail, obj_to_remove);
  cache_remove_obj_base(cache, obj_to_remove, true);
}

/**
 * @brief remove an object from the cache
 * this is different from cache_evict because it is used to for user trigger
 * remove, and eviction is used by the cache to make space for new objects
 *
 * it needs to call cache_remove_obj_base before returning
 * which updates some metadata such as n_obj, occupied size, and hash table
 *
 * @param cache
 * @param obj_id
 * @return true if the object is removed, false if the object is not in the
 * cache
 */
static bool PredDelay_remove(cache_t *cache, const obj_id_t obj_id) {
  cache_obj_t *obj = hashtable_find_obj_id(cache->hashtable, obj_id);
  if (obj == NULL) {
    return false;
  }

  PredDelay_remove_obj(cache, obj);

  return true;
}

// ***********************************************************************
// ****                                                               ****
// ****                  parameter set up functions                   ****
// ****                                                               ****
// ***********************************************************************
static const char *PredDelay_current_params(
                                        PredDelay_params_t *params) {
  static __thread char params_str[128];
  int n =
      snprintf(params_str, 128, "delay-time=%llu\n", params->delay_time);
  return params_str;
}

static void PredDelay_parse_params(cache_t *cache,
                                  const char *cache_specific_params) {
  PredDelay_params_t *params = (PredDelay_params_t *)cache->eviction_params;
  char *params_str = strdup(cache_specific_params);
  char *old_params_str = params_str;
  char *end;

  while (params_str != NULL && params_str[0] != '\0') {
    /* different parameters are separated by comma,
     * key and value are separated by = */
    char *key = strsep((char **)&params_str, "=");
    char *value = strsep((char **)&params_str, ",");

    // skip the white space
    while (params_str != NULL && *params_str == ' ') {
      params_str++;
    }

    if (strcasecmp(key, "delay-time") == 0) {
      if (strchr(value, '.') != NULL) {
        params->delay_time = (uint64_t)((strtof(value, &end)) * cache->cache_size);
        params->delay_ratio = strtof(value, &end);
        if (params->delay_time == 0) {
          params->delay_time = 1;
        }
      }else{
        params->delay_time = (int)strtol(value, &end, 0);
        if (params->delay_time == 0) {
          params->delay_time = 1;
        }
        if (strlen(end) > 2) {
          ERROR("param parsing error, find string \"%s\" after number\n", end);
        }
      }
    } else {
      ERROR("%s does not have parameter %s\n", cache->cache_name, key);
      exit(1);
    }
  }
  free(old_params_str);
}

#ifdef __cplusplus
}
#endif
