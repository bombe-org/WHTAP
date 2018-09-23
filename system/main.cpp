#include <storage/index_hash.h>
#include "global.h"
#include "ycsb.h"
#include "tpcc.h"
#include "test.h"
#include "thread.h"
#include "manager.h"
#include "mem_alloc.h"
#include "query.h"
#include "plock.h"
#include "occ.h"
#include "vll.h"

void * f(void *);

thread_t ** m_thds;

// defined in parser.cpp
void parser(int argc, char * argv[]);
void *state_controller(void *pWorkload);
int main(int argc, char* argv[])
{
	parser(argc, argv);
	
	mem_allocator.init(g_part_cnt, MEM_SIZE / g_part_cnt); 
	stats.init();
	glob_manager = (Manager *) _mm_malloc(sizeof(Manager), 64);
	glob_manager->init();
	if (g_cc_alg == DL_DETECT) 
		dl_detector.init();
	printf("mem_allocator initialized!\n");
	workload * m_wl;
	switch (WORKLOAD) {
		case YCSB :
			m_wl = new ycsb_wl; break;
		case TPCC :
			m_wl = new tpcc_wl; break;
		case TEST :
			m_wl = new TestWorkload; 
			((TestWorkload *)m_wl)->tick();
			break;
		default:
			assert(false);
	}
	m_wl->init();
	printf("workload initialized!\n");
	
	uint64_t thd_cnt = g_thread_cnt;
	pthread_t p_thds[thd_cnt - 1];
	m_thds = new thread_t * [thd_cnt];
	for (uint32_t i = 0; i < thd_cnt; i++)
		m_thds[i] = (thread_t *) _mm_malloc(sizeof(thread_t), 64);
	// query_queue should be the last one to be initialized!!!
	// because it collects txn latency
	query_queue = (Query_queue *) _mm_malloc(sizeof(Query_queue), 64);
	if (WORKLOAD != TEST)
		query_queue->init(m_wl);
	pthread_barrier_init( &warmup_bar, NULL, g_thread_cnt );
	printf("query_queue initialized!\n");
#if CC_ALG == HSTORE
	part_lock_man.init();
#elif CC_ALG == OCC
	occ_man.init();
#elif CC_ALG == VLL
	vll_man.init();
#endif

	for (uint32_t i = 0; i < thd_cnt; i++) 
		m_thds[i]->init(i, m_wl);

	if (WARMUP > 0){
		printf("WARMUP start!\n");
		for (uint32_t i = 0; i < thd_cnt - 1; i++) {
			uint64_t vid = i;
			pthread_create(&p_thds[i], NULL, f, (void *)vid);
		}
		f((void *)(thd_cnt - 1));
		for (uint32_t i = 0; i < thd_cnt - 1; i++)
			pthread_join(p_thds[i], NULL);
		printf("WARMUP finished!\n");
	}
	warmup_finish = true;
	pthread_barrier_init( &warmup_bar, NULL, g_thread_cnt );
#ifndef NOGRAPHITE
	CarbonBarrierInit(&enable_barrier, g_thread_cnt);
#endif
	pthread_barrier_init( &warmup_bar, NULL, g_thread_cnt );

	// spawn and run txns again.
	int64_t starttime = get_server_clock();
    pthread_t state_thd;
    pthread_create(&state_thd, NULL, state_controller, (void *) m_wl);
	for (uint32_t i = 0; i < thd_cnt - 1; i++) {
		uint64_t vid = i;
		pthread_create(&p_thds[i], NULL, f, (void *)vid);
	}
	f((void *)(thd_cnt - 1));
	for (uint32_t i = 0; i < thd_cnt - 1; i++) 
		pthread_join(p_thds[i], NULL);
	int64_t endtime = get_server_clock();
	
	if (WORKLOAD != TEST) {
		printf("PASS! SimTime = %f\n", 1.0*(endtime - starttime)/1000000000.0);
		if (STATS_ENABLE)
			stats.print();
	} else {
		((TestWorkload *)m_wl)->summarize();
	}
	return 0;
}

void * f(void * id) {
	uint64_t tid = (uint64_t)id;
	m_thds[tid]->run();
	return NULL;
}
void *state_controller(void *pWorkload) {
    usleep(1);  //warmup
    while (((workload *) pWorkload)->sim_done) {
        global_state = NORMAL;
        usleep(1);
        global_state = TAKEN;
        pingpong = 1 - pingpong;
        usleep(10);
        global_state = WAITING;
        while (query_static_counter > 0);
        global_state = COMPACTION;
        if (pingpong == 0) {
            IndexHash *index = ((ycsb_wl *) pWorkload)->the_index;
            itemid_t *item;
            for (set<uint64_t>::iterator itr = set1.begin(); itr != set1.end(); itr++) {
                index->index_read(*itr, item, 0, 0);
                ((row_t *) item->location_ap)->copy((row_t *) item->location_v2);
                //free(item->location_v2);
            }
            set1.clear();
        } else if (pingpong == 1) {
            IndexHash *index = ((ycsb_wl *) pWorkload)->the_index;
            itemid_t *item;
            for (set<uint64_t>::iterator itr = set0.begin(); itr != set0.end(); itr++) {
                index->index_read(*itr, item, 0, 0);
                ((row_t *) item->location_ap)->copy((row_t *) item->location_v1);
                //free(item->location_v1);
            }
            set0.clear();
        }
        global_state = COMPLETE;
        while (query_delta_counter > 0);
    }
    return NULL;
}
