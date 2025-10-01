
#include <strings.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/libCacheSim/cache.h"
#include "../../include/libCacheSim/evictionAlgo.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline cache_t *create_cache(const char *trace_path, const char *eviction_algo, const uint64_t cache_size,
                                    const char *eviction_params, const bool consider_obj_metadata) {
  common_cache_params_t cc_params = {
      .cache_size = cache_size,
      .default_ttl = 86400 * 300,
      .hashpower = 24,
      .consider_obj_metadata = consider_obj_metadata,
  };
  cache_t *cache;

  /* the trace provided is small */
  if (trace_path != NULL && strstr(trace_path, "data/trace.") != NULL) cc_params.hashpower -= 8;

  if (strcasecmp(eviction_algo, "lru") == 0) {
    cache = LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo") == 0) {
    cache = FIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-delay") == 0) {
    cache = TwoQ_Delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-batch") == 0) {
    cache = TwoQ_Batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-prob") == 0) {
    cache = TwoQ_Prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-fr") == 0) {
    cache = TwoQ_FR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-lru") == 0) {
    cache = TwoQ_LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-lru") == 0) {
    cache = ARC_LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-delay") == 0) {
    cache = ARC_Delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-prob") == 0) {
    cache = ARC_Prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-batch") == 0) {
    cache = ARC_Batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-fr") == 0) {
    cache = ARC_FR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc") == 0) {
    cache = ARC_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arcv0") == 0) {
    cache = ARCv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lhd") == 0) {
    cache = LHD_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "random") == 0) {
    cache = Random_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomTwo") == 0) {
    cache = RandomTwo_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomK") == 0) {
    cache = RandomK_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfu") == 0) {
    cache = LFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "gdsf") == 0) {
    cache = GDSF_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfuda") == 0) {
    cache = LFUDA_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq") == 0 || strcasecmp(eviction_algo, "2q") == 0) {
    cache = TwoQ_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "slru") == 0) {
    cache = SLRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "slruv0") == 0) {
    cache = SLRUv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "hyperbolic") == 0) {
    cc_params.hashpower = MAX(cc_params.hashpower - 8, 16);
    cache = Hyperbolic_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lecar") == 0) {
    cache = LeCaR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lecarv0") == 0) {
    cache = LeCaRv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "cacheus") == 0) {
    cache = Cacheus_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "size") == 0) {
    cache = Size_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfucpp") == 0) {
    cache = LFUCpp_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "tinyLFU") == 0) {
    if (eviction_params == NULL) {
      cache = WTinyLFU_init(cc_params, eviction_params);
    } else {
      const char *window_size = strstr(eviction_params, "window-size=");
      if (window_size == NULL) {
        char *new_params = malloc(strlen(eviction_params) + 20);
        sprintf(new_params, "%s,window-size=0.01", eviction_params);
        cache = WTinyLFU_init(cc_params, new_params);
      } else {
        cache = WTinyLFU_init(cc_params, eviction_params);
      }
    }
  } else if (strcasecmp(eviction_algo, "wtinyLFU") == 0) {
    cache = WTinyLFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "belady") == 0) {
    if (strcasestr(trace_path, "oracleGeneral") == NULL) {
      WARN("belady is only supported for oracleGeneral trace\n");
      WARN("to convert a trace to oracleGeneral format\n");
      WARN("./bin/traceConv input_trace trace_format output_trace\n");
      WARN("./bin/traceConv ../data/cloudPhysicsIO.txt txt cloudPhysicsIO.oracleGeneral.bin\n");
      exit(1);
    }
    cache = Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "nop") == 0) {
    cache = nop_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "beladySize") == 0) {
    if (strcasestr(trace_path, "oracleGeneral") == NULL) {
      WARN("belady is only supported for oracleGeneral trace\n");
    }
    cc_params.hashpower = MAX(cc_params.hashpower - 8, 16);
    cache = BeladySize_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo-reinsertion") == 0 || strcasecmp(eviction_algo, "clock") == 0 ||
             strcasecmp(eviction_algo, "second-chance") == 0) {
    cache = Clock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "beladyclock") == 0) {
    cache = BeladyClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "offlineFR") == 0) {
    cache = offlineFR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "offlinedelay") == 0) {
    cache = Delay_offline_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "onlinedelay") == 0) {
    cache = Delay_online_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "delayFR") == 0) {
    cache = DelayFR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "predclock") == 0) {
    cache = PredClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "age") == 0) {
    cache = AGE_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "cr-lfu") == 0) {
    cache = CR_LFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lirs") == 0) {
    cache = LIRS_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifomerge") == 0 || strcasecmp(eviction_algo, "fifo-merge") == 0) {
    cache = FIFO_Merge_init(cc_params, eviction_params);
    // } else if (strcasecmp(eviction_algo, "fifo-reinsertion") == 0) {
    //   cache = FIFO_Reinsertion_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "flashProb") == 0) {
    // used to measure application level write amp
    cache = flashProb_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sfifo") == 0) {
    cache = SFIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sfifov0") == 0) {
    cache = SFIFOv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-prob") == 0) {
    cache = lpLRU_prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-delay") == 0) {
    cache = LRU_delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "delayclock") == 0) {
    cache = DelayClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "freqprobclock") == 0) {
    cache = FreqprobClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "ageprobclock") == 0) {
    cache = AgeprobClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "preddelay") == 0) {
    cache = PredDelay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-probv0") == 0) {
    cache = lpLRU_prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "predprob") == 0) {
    cache = PredProb_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fh") == 0) {
    cache = FH_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo-belady") == 0) {
    cache = FIFO_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-belady") == 0) {
    cache = LRU_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sieve-belady") == 0) {
    cache = Sieve_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3lru") == 0) {
    cache = S3LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "bc") == 0) {
    cache = bc_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-delayv1") == 0) {
    cache = LRU_delayv1_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifo") == 0 || strcasecmp(eviction_algo, "s3-fifo") == 0) {
    cache = S3FIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifod") == 0) {
    cache = S3FIFOd_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "hotcache") == 0) {
    cache = HOTCache_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "qdlp") == 0) {
    cache = QDLP_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sharding") == 0) {
    cache = lpFIFO_shards_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "batch") == 0) {
    cache = lpFIFO_batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sieve") == 0) {
    cache = Sieve_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomBelady") == 0) {
    cache = RandomBelady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomLRU") == 0) {
    cache = RandomLRU_init(cc_params, eviction_params);
#ifdef ENABLE_GLCACHE
  } else if (strcasecmp(eviction_algo, "GLCache") == 0 || strcasecmp(eviction_algo, "gl-cache") == 0) {
    cache = GLCache_init(cc_params, eviction_params);
#endif
#ifdef ENABLE_LRB
  } else if (strcasecmp(eviction_algo, "lrb") == 0) {
    cache = LRB_init(cc_params, eviction_params);
#endif
#ifdef INCLUDE_PRIV
  } else if (strcasecmp(eviction_algo, "mclock") == 0) {
    cache = MClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-sfifo") == 0) {
    cache = LP_SFIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-arc") == 0) {
    cache = LP_ARC_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-twoq") == 0) {
    cache = LP_TwoQ_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "qdlpv0") == 0) {
    cache = QDLPv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifodv2") == 0) {
    cache = S3FIFOdv2_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "myMQv1") == 0) {
    cache = myMQv1_init(cc_params, eviction_params);
#endif
  } else {
    ERROR("do not support algorithm %s\n", eviction_algo);
    abort();
  }

  return cache;
}

static inline cache_t *create_cache_with_version_num(const char *trace_path, const char *eviction_algo,
                                                     const uint64_t cache_size, const char *eviction_params,
                                                     const bool consider_obj_metadata, const int version_num) {
  common_cache_params_t cc_params = {
      .cache_size = cache_size,
      .default_ttl = 86400 * 300,
      .hashpower = 24,
      .consider_obj_metadata = consider_obj_metadata,
      .version_num = version_num,
  };
  cache_t *cache;

  /* the trace provided is small */
  if (trace_path != NULL && strstr(trace_path, "data/trace.") != NULL) cc_params.hashpower -= 8;

  if (strcasecmp(eviction_algo, "lru") == 0) {
    cache = LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo") == 0) {
    cache = FIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-delay") == 0) {
    cache = TwoQ_Delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-batch") == 0) {
    cache = TwoQ_Batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-prob") == 0) {
    cache = TwoQ_Prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-fr") == 0) {
    cache = TwoQ_FR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq-lru") == 0) {
    cache = TwoQ_LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc") == 0) {
    cache = ARC_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-lru") == 0) {
    cache = ARC_LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-delay") == 0) {
    cache = ARC_Delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-prob") == 0) {
    cache = ARC_Prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-batch") == 0) {
    cache = ARC_Batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arc-fr") == 0) {
    cache = ARC_FR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "arcv0") == 0) {
    cache = ARCv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lhd") == 0) {
    cache = LHD_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "random") == 0) {
    cache = Random_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomTwo") == 0) {
    cache = RandomTwo_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomK") == 0) {
    cache = RandomK_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfu") == 0) {
    cache = LFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "gdsf") == 0) {
    cache = GDSF_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfuda") == 0) {
    cache = LFUDA_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "twoq") == 0 || strcasecmp(eviction_algo, "2q") == 0) {
    cache = TwoQ_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "slru") == 0) {
    cache = SLRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "slruv0") == 0) {
    cache = SLRUv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "hyperbolic") == 0) {
    cc_params.hashpower = MAX(cc_params.hashpower - 8, 16);
    cache = Hyperbolic_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lecar") == 0) {
    cache = LeCaR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lecarv0") == 0) {
    cache = LeCaRv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "cacheus") == 0) {
    cache = Cacheus_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "size") == 0) {
    cache = Size_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lfucpp") == 0) {
    cache = LFUCpp_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "tinyLFU") == 0) {
    if (eviction_params == NULL) {
      cache = WTinyLFU_init(cc_params, eviction_params);
    } else {
      const char *window_size = strstr(eviction_params, "window-size=");
      if (window_size == NULL) {
        char *new_params = malloc(strlen(eviction_params) + 20);
        sprintf(new_params, "%s,window-size=0.01", eviction_params);
        cache = WTinyLFU_init(cc_params, new_params);
      } else {
        cache = WTinyLFU_init(cc_params, eviction_params);
      }
    }
  } else if (strcasecmp(eviction_algo, "wtinyLFU") == 0) {
    cache = WTinyLFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "belady") == 0) {
    if (strcasestr(trace_path, "oracleGeneral") == NULL) {
      WARN("belady is only supported for oracleGeneral trace\n");
      WARN("to convert a trace to oracleGeneral format\n");
      WARN("./bin/traceConv input_trace trace_format output_trace\n");
      WARN("./bin/traceConv ../data/cloudPhysicsIO.txt txt cloudPhysicsIO.oracleGeneral.bin\n");
      exit(1);
    }
    cache = Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "nop") == 0) {
    cache = nop_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "beladySize") == 0) {
    if (strcasestr(trace_path, "oracleGeneral") == NULL) {
      WARN("belady is only supported for oracleGeneral trace\n");
    }
    cc_params.hashpower = MAX(cc_params.hashpower - 8, 16);
    cache = BeladySize_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo-reinsertion") == 0 || strcasecmp(eviction_algo, "clock") == 0 ||
             strcasecmp(eviction_algo, "second-chance") == 0) {
    cache = Clock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "beladyclock") == 0) {
    cache = BeladyClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "offlineFR") == 0) {
    cache = offlineFR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "offlinedelay") == 0) {
    cache = Delay_offline_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "onlinedelay") == 0) {
    cache = Delay_online_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "delayFR") == 0) {
    cache = DelayFR_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "delayclock") == 0) {
    cache = DelayClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "freqprobclock") == 0) {
    cache = FreqprobClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "ageprobclock") == 0) {
    cache = AgeprobClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "predclock") == 0) {
    cache = PredClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "age") == 0) {
    cache = AGE_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "cr-lfu") == 0) {
    cache = CR_LFU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lirs") == 0) {
    cache = LIRS_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifomerge") == 0 || strcasecmp(eviction_algo, "fifo-merge") == 0) {
    cache = FIFO_Merge_init(cc_params, eviction_params);
    // } else if (strcasecmp(eviction_algo, "fifo-reinsertion") == 0) {
    //   cache = FIFO_Reinsertion_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "flashProb") == 0) {
    // used to measure application level write amp
    cache = flashProb_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sfifo") == 0) {
    cache = SFIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sfifov0") == 0) {
    cache = SFIFOv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-prob") == 0) {
    cache = lpLRU_prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-delay") == 0) {
    cache = LRU_delay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "preddelay") == 0) {
    cache = PredDelay_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-probv0") == 0) {
    cache = lpLRU_prob_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "predprob") == 0) {
    cache = PredProb_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fh") == 0) {
    cache = FH_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "fifo-belady") == 0) {
    cache = FIFO_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-belady") == 0) {
    cache = LRU_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sieve-belady") == 0) {
    cache = Sieve_Belady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3lru") == 0) {
    cache = S3LRU_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "bc") == 0) {
    cache = bc_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lru-delayv1") == 0) {
    cache = LRU_delayv1_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifo") == 0 || strcasecmp(eviction_algo, "s3-fifo") == 0) {
    cache = S3FIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifod") == 0) {
    cache = S3FIFOd_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "hotcache") == 0) {
    cache = HOTCache_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "qdlp") == 0) {
    cache = QDLP_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sharding") == 0) {
    cache = lpFIFO_shards_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "batch") == 0) {
    cache = lpFIFO_batch_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "sieve") == 0) {
    cache = Sieve_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomBelady") == 0) {
    cache = RandomBelady_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "randomLRU") == 0) {
    cache = RandomLRU_init(cc_params, eviction_params);
#ifdef ENABLE_GLCACHE
  } else if (strcasecmp(eviction_algo, "GLCache") == 0 || strcasecmp(eviction_algo, "gl-cache") == 0) {
    cache = GLCache_init(cc_params, eviction_params);
#endif
#ifdef ENABLE_LRB
  } else if (strcasecmp(eviction_algo, "lrb") == 0) {
    cache = LRB_init(cc_params, eviction_params);
#endif
#ifdef INCLUDE_PRIV
  } else if (strcasecmp(eviction_algo, "mclock") == 0) {
    cache = MClock_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-sfifo") == 0) {
    cache = LP_SFIFO_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-arc") == 0) {
    cache = LP_ARC_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "lp-twoq") == 0) {
    cache = LP_TwoQ_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "qdlpv0") == 0) {
    cache = QDLPv0_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "s3fifodv2") == 0) {
    cache = S3FIFOdv2_init(cc_params, eviction_params);
  } else if (strcasecmp(eviction_algo, "myMQv1") == 0) {
    cache = myMQv1_init(cc_params, eviction_params);
#endif
  } else {
    ERROR("do not support algorithm %s\n", eviction_algo);
    abort();
  }

  return cache;
}

#ifdef __cplusplus
}
#endif
